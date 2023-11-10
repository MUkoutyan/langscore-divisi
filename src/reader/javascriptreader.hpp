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

        for(auto& info : pluginInfo.parameters){
            TranslateText t(u8"", this->useLangList);

            const auto& params = info.second;

            for(auto& param : params)
            {

                if(param.type.empty()) {
                    t.scriptLineInfo = fileName + u8":" + info.first;
                }
                else {
                    //typeの末尾には/が付与されている。
                    t.scriptLineInfo = fileName + u8":" + info.first + u8"/" + param.type + u8"/" + param.key;
                }
                t.original = param.value;
                this->texts.emplace_back(std::move(t));
            }
        }

        auto transTextList = convertScriptToCSV(path);

        scriptTranslatesMap.emplace_back(fileName, transTextList);

        return transTextList;
    }

private:

    //std::vector<PluginParameter> enumerate_strings(const nlohmann::json& j, const std::string& key, const std::string& typePath)
    //{
    //    std::vector<PluginParameter> strings;

    //    //typeの末尾には/を付与すること。

    //    if(j.is_string()) 
    //    {
    //        auto str = j.get<std::string>();

    //        auto sub_json = nlohmann::json::parse(str, nullptr, false);
    //        if(sub_json.is_discarded() == false) {
    //            auto newType = typePath + "o/";
    //            if(key.empty()) {
    //                newType.clear();
    //            }
    //            auto sub_strings = enumerate_strings(sub_json, key, newType);
    //            std::copy(sub_strings.begin(), sub_strings.end(), std::back_inserter(strings));
    //        }
    //        else 
    //        {
    //            strings.emplace_back(PluginParameter{
    //                utility::cnvStr<std::u8string>(key),
    //                utility::cnvStr<std::u8string>(typePath),
    //                utility::cnvStr<std::u8string>(str)
    //            });
    //        }

    //    }
    //    else if(j.is_array()) {
    //        int index = 0;
    //        for(const auto& element : j) 
    //        {
    //            auto newType = typePath + "a" + std::to_string(index) + "/";
    //            if(key.empty()) {
    //                newType.clear();
    //            }
    //            auto sub_strings = enumerate_strings(element, key, newType);
    //            std::copy(sub_strings.begin(), sub_strings.end(), std::back_inserter(strings));
    //            ++index;
    //        }
    //    }
    //    else if(j.is_object()) {
    //        for(const auto& [_key, value] : j.items()) 
    //        {
    //            auto sub_strings = enumerate_strings(value, _key, typePath + "o/");
    //            std::copy(sub_strings.begin(), sub_strings.end(), std::back_inserter(strings));
    //        }
    //    }

    //    return strings;
    //}

    std::vector<PluginParameter> enumerate_strings(const nlohmann::json& j, const std::string& key, const std::string& typePath)
    {
        std::vector<PluginParameter> results;
        std::vector<nlohmann::json> parsedData;

        for(const auto& valueData : j) {
            if(valueData.is_string() == false) { continue; }
            auto str = valueData.get<std::string>();

            auto data = nlohmann::json::parse(str, nullptr, false);
            if(data.is_discarded() == false) {
                parsedData.push_back(data);
            }
            else {
                results.emplace_back(utility::cnvStr<std::u8string>(key), u8"", utility::cnvStr<std::u8string>(str));
            }
        }

        int index_val = 0;
        for(const auto& obj : parsedData) 
        {
            int index_in_obj = 0;
            std::u8string type;
            for(const auto& [key, value] : obj.items()) 
            {
                if(key.empty()) { continue; }
                std::cout << "Key : " << key << " : " << index_val << std::endl;

                type = utility::cnvStr<std::u8string>(std::format("[{}]", index_val));

                auto valueJson = nlohmann::json::parse(value.get<std::string>(), nullptr, false);

                if(valueJson.is_discarded() == false) {
                    if(valueJson.is_array()) {
                        for(const auto& v : valueJson) {
                            std::cout << "Array:" << index_in_obj++ << std::endl;
                            if(v.is_string()) {
                                results.emplace_back(PluginParameter{
                                    utility::cnvStr<std::u8string>(key) + utility::cnvStr<std::u8string>(std::format("[{}]", index_in_obj)),
                                    type,
                                    utility::cnvStr<std::u8string>(v.get<std::string>())
                                });
                                std::cout << "Hit! " << v.get<std::string>() << std::endl;
                            }
                        }
                    }
                    else if(valueJson.is_string()) {
                        results.emplace_back(PluginParameter{
                            utility::cnvStr<std::u8string>(key),
                            type,
                            utility::cnvStr<std::u8string>(valueJson.get<std::string>())
                        });
                        std::cout << "Hit! " << valueJson.get<std::string>() << " is String" << std::endl;
                    }
                }
                else {
                    if(value.is_string()) {
                        results.emplace_back(PluginParameter{
                            utility::cnvStr<std::u8string>(key),
                            type,
                            utility::cnvStr<std::u8string>(value.get<std::string>())
                        });
                        std::cout << "Not Json, Hit! " << value.get<std::string>() << " is String" << std::endl;
                    }
                }
            }
            index_val++;
        }

        return results;
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
                //パラメータが構造体(JSON)かどうかを判別
                auto str = utility::cnvStr<std::u8string>(value.get<std::string>());
                data.parameters[utility::cnvStr<std::u8string>(key)].emplace_back(u8"", u8"", str);
                ////パラメータが構造体(JSON)かどうかを判別
                //auto str = utility::cnvStr<std::u8string>(value.get<std::string>());
                //nlohmann::json jsonStruct = nlohmann::json::parse(str, nullptr, false);
                //if(jsonStruct.is_discarded()) {
                //    data.parameters[utility::cnvStr<std::u8string>(key)].emplace_back(u8"", u8"", str);
                //    continue;
                //}

                //if(jsonStruct.is_string() || jsonStruct.is_object() || jsonStruct.is_array()) {
                //    data.parameters[utility::cnvStr<std::u8string>(key)] = enumerate_strings(jsonStruct, key, "");
                //}
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
        // string_fragmentのノードのみを対象とする
        if(strcmp(ts_node_type(node), "string_fragment") == 0) {
            TSNode parent_node = ts_node_parent(node);
            // 親がargumentsまたはassignment_expressionノードであることを確認
            if(isTargetParentNode(ts_node_parent(parent_node))) {
                auto start_byte = ts_node_start_byte(node);
                auto end_byte = ts_node_end_byte(node);
                std::string text = source_code.substr(start_byte, end_byte - start_byte);

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
        else {
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
            exit(EXIT_FAILURE);
        }
        auto file_contents = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());

        // tree-sitterパーサーのインスタンスを作成。
        TSParser* parser = ts_parser_new();
        // JavaScriptの言語をパーサーにセット。
        ts_parser_set_language(parser, tree_sitter_javascript());

        // JavaScriptのソースコードを解析。
        TSTree* tree = ts_parser_parse_string(parser, nullptr, file_contents.c_str(), file_contents.length());


        TSNode root_node = ts_tree_root_node(tree);
        auto result = convertStringFragmentsIfParentMatches(root_node, file_contents, path.filename().stem().u8string());

        //==============================================

        // ツリーとパーサーのリソースを解放。
        ts_tree_delete(tree);
        ts_parser_delete(parser);

        return result;
    }

    ScriptTextParser::DataType findStrings(std::u8string line) const 
    {
        if(line.empty()) { return {}; }

        using namespace std::string_view_literals;
        ScriptTextParser parser;
        //行内の文字を抽出。マルチバイト文字を1文字とカウントするための形式。
        auto wordList = parser.ConvertWordList(line);

        //文字列が無ければ無視
        if(std::find_if(wordList.cbegin(), wordList.cend(), [](const auto& x) { return std::get<0>(x) == u8"\""sv; }) == wordList.cend() &&
            std::find_if(wordList.cbegin(), wordList.cend(), [](const auto& x) { return std::get<0>(x) == u8"'"sv; }) == wordList.cend())
        {
            return {};
        }

        const auto FindArrayOperator = [&line](size_t strStart, size_t endPos){
            return line[strStart - 1] == u8'[' && endPos < line.size() && line[endPos + 1] == u8']';
        };
        const utility::u8stringlist IgnoreText{u8"use strict"};

        ScriptTextParser::DataType transTextList;
        size_t strStart = 0;
        bool findDq = false;
        bool findSq = false;
        size_t col = 0;
        size_t index = 1;
        bool beforeEsc = false;
        for(auto& strView : wordList)
        {
            auto& str = std::get<0>(strView);
            if(beforeEsc == false)
            {
                if(str == u8"\"" && findSq == false) {
                    if(findDq == false) {
                        col = index;
                        strStart = std::get<1>(strView) + 1;
                    }
                    else 
                    {
                        auto endPos = std::get<1>(strView);
                        if(FindArrayOperator(strStart, endPos)) {
                            continue;
                        }
                        auto text = line.substr(strStart, endPos - strStart);
                        if(std::ranges::find(IgnoreText, text) == IgnoreText.end()) {
                            parser.convertTranslateTextFromMatch(text, col, transTextList);
                        }
                    }
                    findDq = !findDq;
                }
                else if(str == u8"'" && findDq == false) {
                    if(findSq == false) {
                        col = index;
                        strStart = std::get<1>(strView) + 1;
                    }
                    else {
                        auto endPos = std::get<1>(strView);
                        if(FindArrayOperator(strStart, endPos)) {
                            continue;
                        }
                        auto text = line.substr(strStart, endPos - strStart);
                        if(std::ranges::find(IgnoreText, text) == IgnoreText.end()) {
                            parser.convertTranslateTextFromMatch(text, col, transTextList);
                        }
                    }
                    findSq = !findSq;
                }
            }

            beforeEsc = str == u8"\\";

            index++;
        }
        return transTextList;
    }

};

}