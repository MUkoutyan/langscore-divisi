//javascriptreaderクラスを作成
#include "readerbase.h"
#include "nlohmann/json.hpp"
#include "serialize_base.h"
#include "csvreader.h"
#include "config.h"
#include "utility.hpp"
#include "tree_sitter/api.h"
#include <filesystem>

#include <iostream>

#ifdef ENABLE_TEST
#define NOMINMAX
#include "iutest.hpp"

class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, CheckJavaScriptCommentLine);
#endif

// tree-sitter-javascriptの言語関数を提供する。
extern "C" TSLanguage * tree_sitter_javascript();


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

        auto transTextList = convertScriptToCSV(path);

        for(auto& info : pluginInfo.parameters){

            const auto& params = info.second;

            for(auto& param : params)
            {
                TranslateText t(param.value, this->useLangList);
                //typeの末尾には/が付与されている。
                if(param.key.empty()) {
                    t.scriptLineInfo = fileName + u8":" + info.first;
                }
                else {
                    t.scriptLineInfo = fileName + u8":" + info.first + u8"/" + param.key;
                }
                transTextList.emplace_back(std::move(t));
            }
        }

        scriptTranslatesMap.emplace_back(fileName, transTextList);

        return transTextList;
    }

private:

    void processJson(const nlohmann::json& j, std::vector<PluginParameter>& result, std::vector<std::u8string> path)
    {
        using namespace std::string_literals;
        if(j.is_object()) {
            for(auto it = j.begin(); it != j.end(); ++it) {
                path.push_back(utility::cnvStr<std::u8string>(it.key()));
                processJson(it.value(), result, path);
                path.pop_back();
            }
        }
        else if(j.is_array()) {
            for(size_t i = 0; i < j.size(); ++i) {
                path.push_back(utility::cnvStr<std::u8string>(std::to_string(i)));
                processJson(j[i], result, path);
                path.pop_back();
            }
        }
        else if(j.is_string()) {
            std::string stringValue = j.get<std::string>();
            try {
                auto nestedJson = nlohmann::json::parse(stringValue);
                // ネストされたJSONを処理
                processJson(nestedJson, result, path);
            }
            catch(nlohmann::json::parse_error&) {
                // ネストされたJSONではないので、通常どおり処理
                PluginParameter param;
                param.key   = utility::join(path, u8"/"s);
                param.value = utility::cnvStr<std::u8string>(stringValue);
                result.emplace_back(std::move(param));
            }
        }
    }

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

            if(data.status == false) { continue; }

            for(const auto& [key, value] : (*item)["parameters"].items()) 
            {
                ////パラメータが構造体(JSON)かどうかを判別
                auto str = utility::cnvStr<std::u8string>(value.get<std::string>());
                nlohmann::json jsonStruct = nlohmann::json::parse(str, nullptr, false);
                if(jsonStruct.is_discarded()) {
                    data.parameters[utility::cnvStr<std::u8string>(key)].emplace_back(u8"", str);
                    continue;
                }

                if(jsonStruct.is_string() || jsonStruct.is_object() || jsonStruct.is_array()) {
                    processJson(jsonStruct, data.parameters[utility::cnvStr<std::u8string>(key)], {});
                }
            }

            result.emplace_back(std::move(data));
        }

        return result;
    }


    // 引数として与えられたnodeがargumentsまたはassignment_expressionかどうかをチェックするヘルパー関数
    bool isTargetParentNode(TSNode node) const {
        std::string type = ts_node_type(node);
        return type == "arguments" || type == "assignment_expression";
    }

    // string_fragmentのノードを探して出力する関数
    std::vector<TranslateText> convertStringFragmentsIfParentMatches(TSNode node, const std::string& source_code, const std::u8string& filename) const
    {
        std::vector<TranslateText> result;

        if(strcmp(ts_node_type(node), "string_fragment") == 0) {
            TSNode parent_node = ts_node_parent(node);
            // 親がargumentsまたはassignment_expressionノードであることを確認
            // ※つまり、引数か代入となっている文字列のみを抽出
            if(isTargetParentNode(ts_node_parent(parent_node))) 
            {
                //該当文字列の抽出
                auto start_byte = ts_node_start_byte(node);
                auto end_byte = ts_node_end_byte(node);
                std::string text = source_code.substr(start_byte, end_byte - start_byte);

                //行・列の抽出
                auto start_point = ts_node_start_point(node);
                auto start_row = start_point.row + 1;
                auto start_column = start_point.column;

                langscore::TranslateText t = {
                    utility::cnvStr<std::u8string>(text),
                    this->useLangList
                };

                auto lineCountStr = std::to_string(start_row);
                std::u8string u8lineCount(lineCountStr.begin(), lineCountStr.end());
                auto colCountStr = std::to_string(start_column);
                std::u8string u8ColCountStr(colCountStr.begin(), colCountStr.end());
                auto scriptPos = filename + u8":" + u8lineCount + u8":" + u8ColCountStr;
                t.scriptLineInfo = scriptPos;
                result.emplace_back(std::move(t));
            }
        }
        else 
        {
            // 再帰的に子ノードを処理
            auto nodes = ts_node_child_count(node);
            for(unsigned i = 0; i < nodes; ++i) {
                auto r = convertStringFragmentsIfParentMatches(ts_node_child(node, i), source_code, filename);
                std::copy(r.begin(), r.end(), std::back_inserter(result));
            }
        }

        return result;
    }

    std::vector<TranslateText> convertScriptToCSV(std::filesystem::path path) const override
    {
        std::ifstream input_file(path);
        if(!input_file.is_open()) {
            std::cerr << "Could not open the file - '" << path << "'" << std::endl;
            return {};
        }
        auto file_contents = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

        // tree-sitterの初期化
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sitter_javascript());

        // JavaScriptのソースコードを解析。
        TSTree* tree = ts_parser_parse_string(parser, nullptr, file_contents.c_str(), file_contents.length());

        // ルートから文字列の抽出
        TSNode root_node = ts_tree_root_node(tree);
        auto result = convertStringFragmentsIfParentMatches(root_node, file_contents, path.filename().stem().u8string());

        // tree-sitterの開放
        ts_tree_delete(tree);
        ts_parser_delete(parser);

        return result;
    }

};

}