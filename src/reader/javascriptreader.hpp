//javascriptreaderクラスを作成
#include "readerbase.h"
#include "nlohmann/json.hpp"
#include "serialize_base.h"
#include "csvreader.h"
#include "config.h"
#include "utility.hpp"
#include <filesystem>

#ifdef ENABLE_TEST
#define NOMINMAX
#include "iutest.hpp"

class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, CheckJavaScriptCommentLine);
#endif


namespace langscore {

class javascriptreader: public readerbase
{
#ifdef ENABLE_TEST
    IUTEST_FRIEND_TEST(Langscore_Writer, CheckJavaScriptCommentLine);
#endif
public:
	javascriptreader(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList)
        : readerbase(std::move(langs), std::move(scriptFileList))
	{
        this->setComment(u8"//", u8"/*", u8"*/");
        //使用しているプラグインの抽出
        this->pluginInfoList = readPluginInfo();

        for(auto& path : this->scriptFileList)
        {
            //parseの結果をthis->textsに格納
			auto scriptTexts = this->parse(path);
			this->texts.insert(this->texts.end(), scriptTexts.begin(), scriptTexts.end());
        }
	}

	~javascriptreader() override = default;

	void applyIgnoreScripts(const std::vector<config::ScriptData>& scriptInfoList)
	{
        if(this->texts.empty()){ return; }
        using namespace std::string_literals;
        namespace fs = std::filesystem;

        config config;
        auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());

        for(auto& t : this->texts){
            if(t.translates.find(def_lang) == t.translates.end()){ continue; }
            t.translates[def_lang] = t.original;
            t.scriptLineInfo.swap(t.original);
        }

        //無視する行の判定
        utility::u8stringlist ignoreRowName;
        for(auto& scriptInfo : scriptInfoList)
        {
            auto fileName = fs::path(scriptInfo.filename).filename().stem().u8string();
            if(fileName == u8"Langscore"s || fileName == u8"Langscore_Custom"s){
                continue;
            }

            if(scriptInfo.ignore == false){
                for(const auto& textInfo : scriptInfo.texts)
                {
                    if(textInfo.disable){ continue; }
                    if(textInfo.ignore){ continue; }
                    auto name = fileName + utility::cnvStr<std::u8string>(":" + std::to_string(textInfo.row) + ":" + std::to_string(textInfo.col));
                    ignoreRowName.emplace_back(std::move(name));
                }
            }
            else{
                std::erase_if(this->texts, [&fileName](const auto& x){
                    return x.original.find(fileName) != std::remove_const_t<std::remove_reference_t<decltype(x.original)>>::npos;
                });
            }
        }
        {
            std::erase_if(this->texts, [&ignoreRowName](const auto& t){
                return std::ranges::find(ignoreRowName, t.original) != ignoreRowName.cend();
            });
        }
	}

    //parse関数のオーバーロード
    std::vector<TranslateText> parse(std::filesystem::path path)
    {
        auto fileName = path.filename().stem().u8string();
        auto result = std::find_if(pluginInfoList.begin(), pluginInfoList.end(), [&fileName](const auto& info){
            return info.name == fileName;
        });
        if(result == pluginInfoList.end()){
            return {};
        }
        const auto& pluginInfo = *result;

        //更新時の場合はここでlangscoreスクリプトが含まれている可能性がある
        if(fileName == u8"Langscore"){ return {}; }
        else if(fileName == u8"Langscore_Custom"){ return {}; }

        for(auto& info : pluginInfo.parameters){
            TranslateText t(u8"", this->useLangList);
            t.scriptLineInfo = fileName + u8":" + info.first;
            t.original = info.second;
            this->texts.emplace_back(std::move(t));
        }

        auto transTextList = convertScriptToCSV(path);

        std::copy(transTextList.begin(), transTextList.end(), std::back_inserter(this->texts));
        scriptTranslatesMap.emplace_back(fileName, transTextList);

        return transTextList;
    }

private:

    std::vector<PluginInfo> readPluginInfo()
    {
        using namespace std::string_literals;
        config config;
        auto pluginsPath = std::filesystem::path(config.gameProjectPath()) / u8"js/plugins.js"s;
        std::ifstream input_file(pluginsPath.generic_string());
        if(!input_file.is_open()) {
            return {};
        }

        std::string content((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
        input_file.close();

        std::size_t startPos = content.find('[');
        std::size_t endPos = content.rfind(']');

        if(startPos == std::string::npos || endPos == std::string::npos) {
            return {};
        }

        auto jsonStr = utility::cnvStr<std::u8string>(content.substr(startPos, endPos - startPos + 1));

        nlohmann::json j = nlohmann::json::parse(jsonStr);

        std::vector<PluginInfo> result;
        for(auto item = j.begin(); item != j.end(); ++item)
        {
            PluginInfo data;
            data.name = utility::cnvStr<std::u8string>((*item)["name"].get<std::string>());
            data.filename = data.name;
            data.status = (*item)["status"].get<bool>();
            data.description = utility::cnvStr<std::u8string>((*item)["description"].get<std::string>());

            for(const auto& [key, value] : (*item)["parameters"].items()) {
                data.parameters[utility::cnvStr<std::u8string>(key)] = utility::cnvStr<std::u8string>(value.get<std::string>());
            }

            result.emplace_back(std::move(data));
        }

        return result;
    }


};

}