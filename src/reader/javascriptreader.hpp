//javascriptreaderクラスを作成
#include "readerbase.h"
#include "nlohmann/json.hpp"
#include "serialize_base.h"
#include "csvreader.h"
#include "config.h"
#include "utility.hpp"
#include "treesitter_wrapper.hpp"
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
    bool isIdentifierNode(const TSNode& node) const {
        return std::string(ts_node_type(node)) == "identifier";
    }

    // 指定したノードから最初の identifier ノードを探す関数
    TSNode findFirstIdentifierNode(TSNode node) const {
        uint32_t childCount = ts_node_child_count(node);

        for(uint32_t i = 0; i < childCount; i++) {
            TSNode child = ts_node_child(node, i);

            // 子ノードが identifier の場合、そのノードを返す
            if(isIdentifierNode(child)) {
                return child;
            }

            // 子ノードが identifier でない場合、その子ノード内を再帰的に探索
            TSNode identifierNode = findFirstIdentifierNode(child);
            if(ts_node_is_null(identifierNode) == false) {
                return identifierNode;
            }
        }

        // identifier ノードが見つからなかった場合、nullノードを返す
        return TSNode();
    }

    // string_fragmentのノードを探して出力する関数
    TSLanguage* tree_sittrer_lang = tree_sitter_javascript();
    // string_fragmentのノードを探して出力する関数

    struct ParseInfo {
        TSNode node;
        std::string_view source_code;

        std::string_view substr(TSNode n) const {
            auto start_byte = ts_node_start_byte(n);
            auto end_byte = ts_node_end_byte(n);
            auto text = this->source_code.substr(start_byte, end_byte - start_byte);
            return text;
        }
        std::tuple<std::string_view, uint32_t, uint32_t> substrWithLineInfo(TSNode n) const {
            auto start_byte = ts_node_start_byte(n);
            auto end_byte = ts_node_end_byte(n);
            auto text = this->source_code.substr(start_byte, end_byte - start_byte);
            auto [row, column] = GetNodePoint(n, false, false);
            return {text, row, column};
        }
    };
    void extractStringFragments(ParseInfo parseInfo, std::vector<TSNode>& result, bool through_string = false) const
    {

        static const std::vector<std::string> ignore_type = {
            "binary_expression", "subscript_expression",
            "regex", "throw_statement",
            "identifier",
            "(", ")", "[", "]", ";", "if" "\"", "||", "&&", ".", "{", "}", "="
        };
        static const std::vector<std::string> ignore_method = {
            "console.log", "xhr.open"
        };
        static const std::vector<std::string> ignore_function = {
            "require"
        };
        static const std::vector<std::string> ignore_identifier = {
            "RegExp", "Error"
        };

        std::string node_type = ts_node_type(parseInfo.node);
        //それ以上探索する必要のないノード
        if(std::ranges::find(ignore_type, node_type) != ignore_type.cend()) {
            return;
        }

        if(through_string) {
            //左辺値に文字列が含まれている場合に無視するための処理。
            if(node_type == "string" || node_type == "string_fragment") {
                return;
            }
        }

        if(through_string == false && node_type == "string_fragment") {
            auto text = parseInfo.substr(parseInfo.node);
            if(text == "use strict") {
                return;
            }

            result.emplace_back(parseInfo.node);
        }
        //関数呼び出し
        else if(node_type == "function_declaration")
        {
            //関数名の抽出
            {
                auto child = findFirstChildNode(parseInfo.node, "identifier");
                if(ts_node_is_null(child) == false)
                {
                    auto [text, row, column] = parseInfo.substrWithLineInfo(child);

                    if(std::ranges::find(ignore_function, text) != ignore_function.cend()) {
                        return;
                    }
                }
            }
            //引数の抽出
            auto formal = findFirstChildNode(parseInfo.node, "formal_parameters");
            if(ts_node_is_null(formal) == false)
            {

                //identifierなら変数指定
                auto args = findChildNodeList(formal, "identifier");
                if(args.empty() == false)
                {
                    for(auto& arg : args) {
                        auto [text, row, column] = parseInfo.substrWithLineInfo(arg);
                    }
                }

                auto str_args = findChildNodeList(formal, "string_fragment", true);
                if(str_args.empty() == false)
                {
                    for(auto& arg : str_args) {
                        result.emplace_back(arg);
                        auto [text, row, column] = parseInfo.substrWithLineInfo(arg);
                    }
                }
                return;
            }
        }
        //メソッドの呼び出し
        else if(node_type == "call_expression")
        {
            auto member_expressions = findChildNodeList(parseInfo.node, "member_expression");
            if(member_expressions.empty() == false)
            {
                //メソッド名の抽出
                //member_expressionは二重
                auto method_name = findMethodName(member_expressions[0], parseInfo.source_code, "member_expression");

                if(std::ranges::find(ignore_method, method_name) != ignore_method.cend()) {
                    return;
                }

                //メソッドの特殊対応。組み込み済みのメソッドのみに使用。
                if(method_name == "Object.defineProperty")
                {
                    //引数の抽出
                    auto formal = findFirstChildNode(parseInfo.node, "arguments");
                    //Object.definePropertyは第2引数までを無視する。
                    //第3引数以降はgetterに文字列が含まれている可能性があるため一応残す。
                    auto numChilds = ts_node_child_count(formal);
                    for(auto i = 0; i < numChilds; ++i)
                    {
                        //,が含まれるので*2
                        if(i < (2 * 2)) { continue; }
                        auto child_node = ts_node_child(formal, i);
                        extractStringFragments({child_node, parseInfo.source_code}, result);
                    }
                    return;
                }
            }
        }
        //代入式
        else if(node_type == "assignment_expression")
        {
            auto numChilds = ts_node_child_count(parseInfo.node);
            bool findEqual = false;
            for(auto i = 0; i < numChilds; ++i) {

                auto child_node = ts_node_child(parseInfo.node, i);
                if(ts_node_is_null(child_node)) { continue; }

                std::string child_type = ts_node_type(child_node);
                if(findEqual == false && child_type == "=") {
                    findEqual = true;
                    continue;
                }

                extractStringFragments({child_node, parseInfo.source_code}, result, findEqual == false);
            }
            return;
        }
        //[]の判定
        else if(node_type == "subscript_expression")
        {
            auto parent_node = ts_node_parent(parseInfo.node);
            if(std::string(ts_node_type(parent_node)) == "assignment_expression")
            {
                if(through_string) {
                    //左辺値の配列内の文字列は無視
                    return;
                }
            }
        }
        else if(node_type == "pair")
        {
            auto numChilds = ts_node_child_count(parseInfo.node);
            bool findCoron = false;
            for(auto i = 0; i < numChilds; ++i) {

                auto child_node = ts_node_child(parseInfo.node, i);
                if(ts_node_is_null(child_node)) { continue; }

                std::string child_type = ts_node_type(child_node);
                if(findCoron == false && child_type == ":") {
                    findCoron = true;
                    continue;
                }

                extractStringFragments({child_node, parseInfo.source_code}, result, findCoron == false);
            }
            return;
        }
        //var hoge = new Hogeのようなnewの場合の処理。
        else if(node_type == "new_expression")
        {
            auto numChilds = ts_node_child_count(parseInfo.node);
            for(auto i = 0; i < numChilds; ++i)
            {
                auto child_node = ts_node_child(parseInfo.node, i);
                if(std::string(ts_node_type(child_node)) == "identifier") {

                    auto text = parseInfo.substr(child_node);

                    if(std::ranges::find(ignore_identifier, text) != ignore_identifier.cend()) {
                        return;
                    }
                }
            }
        }
        else if(node_type == "switch_statement")
        {
            //switchの条件式を無視。switch_bodyが処理本体なのでそこまで読み飛ばす。
            auto num_child = ts_node_child_count(parseInfo.node);
            bool find_switch_body = false;
            for(unsigned i = 0; i < num_child; ++i) 
            {
                auto child_node = ts_node_child(parseInfo.node, i);
                if(ts_node_type(child_node) == std::string("switch_body")) {
                    find_switch_body = true;
                }
                if(find_switch_body == false) { continue; }
                extractStringFragments({child_node, parseInfo.source_code}, result);
            }
            return;
        }
        else if(node_type == "switch_case")
        {
            //case ~:の無視。念のためcaseの開始を判定する。
            auto num_child = ts_node_child_count(parseInfo.node);
            bool through_type = false;
            for(unsigned i = 0; i < num_child; ++i)
            {
                auto child_node = ts_node_child(parseInfo.node, i);
                std::string type = ts_node_type(child_node);
                if(through_type == false && type == "case") {
                    through_type = true;
                    continue;
                }
                else if(through_type && type == ":") {
                    through_type = false;
                    continue;
                }

                if(through_type) { continue; }
                extractStringFragments({child_node, parseInfo.source_code}, result);
            }
            return;
        }

        // 再帰的に子ノードを処理
        auto num_child = ts_node_child_count(parseInfo.node);
        for(unsigned i = 0; i < num_child; ++i) {
            auto child_node = ts_node_child(parseInfo.node, i);
            extractStringFragments({child_node, parseInfo.source_code}, result);
        }
    }

    ScriptTextParser::DataType findStrings(std::u8string line) const override {
        ScriptTextParser scriptParser;
        return scriptParser.findStrings(line);
    }

    
    std::vector<TranslateText> convertScriptToCSV(std::filesystem::path path) const override
    {
        std::ifstream input_file(path);
        if(!input_file.is_open()) {
            std::cerr << "Could not open the file - '" << path << "'" << std::endl;
            return {};
        }

        std::string file_contents;
        {
            auto tmp = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
            file_contents.reserve(tmp.size());
            std::istringstream stream(tmp);
            std::string line;
            size_t row = 0;

            auto filename = path.filename().stem();
            while(std::getline(stream, line)) {
                // 列数が多すぎる場合、外部ソースコードが埋め込まれている可能性がある。
                // 解析からは弾く。
                if(line.length() <= 4096) {
                    file_contents += line + "\n";  // 行を追加し、改行を保持する
                }
#ifdef _DEBUG
                else {
                    std::cout << filename << " Ignore Row : " << row << std::endl;
                }
#endif
                row++;
            }
        }

        //記号のみのテキストは無視するかのフラグ。要configオプション化。
        const bool ignore_not_include_alpha = true;

        // tree-sitterの初期化
        TSParser* parser = ts_parser_new();
        ts_parser_set_language(parser, tree_sittrer_lang);

        // JavaScriptのソースコードを解析。
        TSTree* tree = ts_parser_parse_string(parser, nullptr, file_contents.c_str(), file_contents.length());

        // ルートから文字列の抽出
        TSNode root_node = ts_tree_root_node(tree);
        std::vector<TSNode> parse_result;
        extractStringFragments({root_node, file_contents}, parse_result);

        ScriptTextParser wordConverter;
        std::vector<TranslateText> result;
        auto filename = path.filename().stem().u8string();
        for(auto& node : parse_result)
        {
            auto [row, column] = GetNodePoint(node);
            auto [start_byte, end_byte] = GetNodeStartEndByte(node);
            std::string text = file_contents.substr(start_byte, end_byte - start_byte);

            langscore::TranslateText t = {
                utility::cnvStr<std::u8string>(text),
                this->useLangList
            };

            auto lineCountStr = std::to_string(row);
            std::u8string u8lineCount(lineCountStr.begin(), lineCountStr.end());
            auto colCountStr = std::to_string(column);
            std::u8string u8ColCountStr(colCountStr.begin(), colCountStr.end());
            auto scriptPos = filename + u8":" + u8lineCount + u8":" + u8ColCountStr;
            t.scriptLineInfo = scriptPos;
            result.emplace_back(std::move(t));
        }


        // tree-sitterの開放
        ts_tree_delete(tree);
        ts_parser_delete(parser);

        return result;
    }
    
};

}