// rubyreaderクラスを実装
#include "readerbase.h"
#include "utility.hpp"
#include "csvreader.h"
#include "config.h"
#include "scripttextparser.hpp"
#include "treesitter_wrapper.hpp"
#include <filesystem>
#include <ranges>
#include <iostream>

// tree-sitter-rubyの言語関数を提供する
extern "C" TSLanguage* tree_sitter_ruby();

namespace langscore
{
    class rubyreader : public readerbase
    {
        bool print_debug = false;

    public:
        rubyreader(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList, bool print_debug = false)
            : readerbase(std::move(langs), std::move(scriptFileList))
            , print_debug(false)
        {
            this->setComment(u8"#", u8"=begin", u8"=end");

            config config;
            const auto lsAnalyzePath = std::filesystem::path(config.langscoreAnalyzeDirectorty());
            auto filePathPair = plaincsvreader{lsAnalyzePath / "Scripts/_list.csv"}.getPlainCsvTexts();
            for(const auto& row : filePathPair)
            {
                auto info = PluginInfo{
                    utility::cnvStr<std::u8string>(row[1]),
                    utility::cnvStr<std::u8string>(row[0]) + u8".rb",
                    true, u8"", {}
                };
                pluginInfoList.emplace_back(std::move(info));
            }

            for(auto& path : this->scriptFileList)
            {
                //parseの結果をthis->textsに格納
                auto scriptTexts = this->parse(path);
                auto result = std::remove_if(scriptTexts.begin(), scriptTexts.end(), [](const auto& x) {
                    return x.original.empty();
                    });
                scriptTexts.erase(result, scriptTexts.end());
                if(scriptTexts.empty()) { continue; }

                this->texts.insert(this->texts.end(), scriptTexts.begin(), scriptTexts.end());
                scriptTranslatesMap.emplace_back(path.filename().u8string(), std::move(scriptTexts));
            }
        }

        ~rubyreader() override = default;

        void applyIgnoreScripts(const std::vector<config::ScriptData>& scriptInfoList)
        {
            if(this->texts.empty()) { return; }
            using namespace std::string_literals;
            namespace fs = std::filesystem;

            config config;
            auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());

            for(auto& t : this->texts) {
                if(t.translates.find(def_lang) == t.translates.end()) { continue; }
                t.translates[def_lang] = t.original;
                t.scriptLineInfo.swap(t.original);
            }

            //無視する行の判定
            utility::u8stringlist ignoreRowName;
            for(auto& scriptInfo : scriptInfoList)
            {
                auto fileName = fs::path(scriptInfo.filename).filename().stem().u8string();
                if(fileName == u8"langscore"s || fileName == u8"langscore_custom"s) {
                    std::erase_if(this->texts, [fileName = utility::removeExtension(fileName)](const auto& x) {
                        return x.original.find(fileName) != std::remove_const_t<std::remove_reference_t<decltype(x.original)>>::npos;
                        });
                    continue;
                }

                if(scriptInfo.ignore == false) {
                    for(const auto& textInfo : scriptInfo.texts)
                    {
                        if(textInfo.disable) { continue; }
                        if(textInfo.ignore) { continue; }
                        auto name = fileName + utility::cnvStr<std::u8string>(":" + std::to_string(textInfo.row) + ":" + std::to_string(textInfo.col));
                        ignoreRowName.emplace_back(std::move(name));
                    }
                }
                else {
                    std::erase_if(this->texts, [&fileName](const auto& x) {
                        return x.original.find(fileName) != std::remove_const_t<std::remove_reference_t<decltype(x.original)>>::npos;
                        });
                }
            }
            {
                std::erase_if(this->texts, [&ignoreRowName](const auto& t) {
                    return std::ranges::find(ignoreRowName, t.original) != ignoreRowName.cend();
                    });
            }
        }

    private:
        std::vector<TranslateText> parse(std::filesystem::path path)
        {
            return convertScriptToCSV(path);
        }

        std::u8string GetScriptName(std::u8string fileName) const
        {
            using namespace std::string_literals;

            auto result = std::ranges::find_if(pluginInfoList, [&fileName](const auto& x) {
                return x.filename == fileName;
                });
            if(result == pluginInfoList.end()) { return u8""s; }
            auto scriptName = (*result).name;
            if(scriptName == u8"langscore") { return u8""s; }
            if(scriptName == u8"langscore_custom") { return u8""s; }
            return scriptName;
        }

        struct ContentsView {
            std::pair<uint32_t, uint32_t> row_col;
            std::pair<uint32_t, uint32_t> start_end_byte;
        };

        // Rubyの構文木から文字列ノードを抽出する関数
        void extractStringNodes(TSNode node, const std::string_view& source_code, std::vector<ContentsView>& result, bool through_string = false) const
        {
            // ノードがない場合は何もしない
            if(ts_node_is_null(node)) {
                return;
            }

            std::string node_type = ts_node_type(node);

            // 無視するノードタイプ
            static const std::vector<std::string_view> ignore_type = {
                "binary", "call", "identifier", "operator", "constant", "integer", "array", "hash",
                "symbol", "ERROR", ".", ":", "::", "=>", "=", "if", "else", "while", "for", "class", "module"
            };

            // 無視するメソッド/関数名
            static const std::vector<std::string_view> ignore_method = {
                "require", "puts", "print", "p", "attr_accessor", "attr_reader", "attr_writer"
            };

            // 無視すべきノードタイプかどうかを確認
            if(std::ranges::find(ignore_type, node_type) != ignore_type.cend()) {
                return;
            }

            if(print_debug) {
                std::cout << "Node type: " << node_type << std::endl;
            }

            if(through_string) {
                // 左辺値に文字列が含まれている場合に無視するための処理
                if(node_type == "string" || node_type == "string_content" || node_type == "string_literal") {
                    return;
                }
            }

            // Rubyの文字列リテラルを処理
            if(node_type == "string" || node_type == "string_literal")
            {
                // ダブルクォート文字列を検出
                auto string_content = findFirstChildNode(node, "string_content", true);
                if(!ts_node_is_null(string_content)) {
                    auto [row, column] = GetNodePoint(node);
                    auto [start_byte, end_byte] = GetNodeStartEndByte(string_content);

                    result.emplace_back(
                        std::make_pair(row, column),
                        std::make_pair(start_byte, end_byte)
                    );
                    return;
                }

                // 文字列内容がない場合（空文字列など）は直接文字列ノード自体を使用
                if(ts_node_child_count(node) > 0) {
                    // クォートマークを探す（"や'）
                    // 最初のクォートの後ろと最後のクォートの前を取得
                    auto numChildren = ts_node_child_count(node);
                    bool foundFirstQuote = false;
                    ContentsView view;

                    for(uint32_t i = 0; i < numChildren; i++) {
                        auto child = ts_node_child(node, i);
                        std::string child_type = ts_node_type(child);

                        if(child_type == "\"" || child_type == "'") {
                            if(!foundFirstQuote) {
                                // 最初のクォート
                                auto [row, column] = GetNodePoint(child);
                                auto [start, end] = GetNodeStartEndByte(child);
                                view.row_col = {row, column + 1}; // クォートの次の位置
                                view.start_end_byte.first = end;  // クォートの後ろ
                                foundFirstQuote = true;
                            }
                            else {
                                // 最後のクォート
                                auto [start, end] = GetNodeStartEndByte(child);
                                view.start_end_byte.second = start; // クォートの前
                                result.emplace_back(std::move(view));
                                return;
                            }
                        }
                    }
                }
            }
            // %q{} や %Q{} などの代替クォート文字列も処理
            else if(node_type == "string_array" || node_type == "heredoc_body" || node_type == "simple_symbol") {
                auto [row, column] = GetNodePoint(node);
                auto [start_byte, end_byte] = GetNodeStartEndByte(node);

                // シンボルの場合は:を除外
                if(node_type == "simple_symbol") {
                    start_byte++; // :の後ろから
                }

                result.emplace_back(
                    std::make_pair(row, column),
                    std::make_pair(start_byte, end_byte)
                );
                return;
            }
            // 文字列の内容
            else if(node_type == "string_content") {
                auto [row, column] = GetNodePoint(node);
                auto [start_byte, end_byte] = GetNodeStartEndByte(node);

                if(start_byte < end_byte) { // 空でないことを確認
                    result.emplace_back(
                        std::make_pair(row, column),
                        std::make_pair(start_byte, end_byte)
                    );
                }
                return;
            }
            // メソッド呼び出し
            else if(node_type == "method_call" || node_type == "call") {
                // メソッド名を取得
                auto method_node = findFirstChildNode(node, "identifier");
                if(!ts_node_is_null(method_node)) {
                    std::string method_name = std::string(source_code.substr(
                        ts_node_start_byte(method_node),
                        ts_node_end_byte(method_node) - ts_node_start_byte(method_node)
                    ));

                    // 無視するメソッドリストにあれば処理しない
                    if(std::ranges::find(ignore_method, method_name) != ignore_method.cend()) {
                        return;
                    }
                }

                // 引数内の文字列を抽出（深さ優先探索）
                auto args_node = findFirstChildNode(node, "argument_list", true);
                if(!ts_node_is_null(args_node)) {
                    uint32_t child_count = ts_node_child_count(args_node);
                    for(uint32_t i = 0; i < child_count; i++) {
                        TSNode child = ts_node_child(args_node, i);
                        extractStringNodes(child, source_code, result);
                    }
                    return;
                }
            }
            // 代入式
            else if(node_type == "assignment") {
                uint32_t child_count = ts_node_child_count(node);
                bool found_equal = false;

                for(uint32_t i = 0; i < child_count; i++) {
                    TSNode child = ts_node_child(node, i);
                    std::string child_type = ts_node_type(child);

                    if(!found_equal && child_type == "=") {
                        found_equal = true;
                        continue;
                    }

                    // 左辺値には文字列が含まれていても処理しない
                    extractStringNodes(child, source_code, result, !found_equal);
                }
                return;
            }

            // 再帰的に子ノードを処理
            uint32_t child_count = ts_node_child_count(node);
            for(uint32_t i = 0; i < child_count; i++) {
                TSNode child = ts_node_child(node, i);
                extractStringNodes(child, source_code, result);
            }
        }

        ScriptTextParser::DataType findStrings(std::u8string line) const override {
            ScriptTextParser scriptParser;
            return scriptParser.findStrings(line);
        }

        std::vector<TranslateText> convertScriptToCSV(std::filesystem::path path) const override
        {
            // Rubyファイルを読み込む
            std::ifstream input_file(path);
            if(!input_file.is_open()) {
                std::cerr << "Could not open the file - '" << path << "'" << std::endl;
                return {};
            }

            // ファイル内容を読み込む
            std::string file_contents;
            {
                auto tmp = std::string((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
                file_contents.reserve(tmp.size());
                std::istringstream stream(tmp);
                std::string line;
                size_t row = 0;

                auto filename = path.filename().stem();
                while(std::getline(stream, line)) {
                    // 極端に長い行は無視（外部コードの可能性）
                    if(line.length() <= 4096) {
                        file_contents += line + "\n";  // 行を追加し、改行を保持
                    }
#ifdef _DEBUG
                    else {
                        std::cout << filename << " Ignore Row : " << row << std::endl;
                    }
#endif
                    row++;
                }
            }

            // Tree-sitterの初期化
            TSLogger logger = {(void*)&this->print_debug, &ts_logger_func};
            TSParser* parser = ts_parser_new();
            ts_parser_set_language(parser, tree_sitter_ruby());
            ts_parser_set_logger(parser, logger);

            // Rubyのソースコードを解析
            TSTree* tree = ts_parser_parse_string(parser, nullptr, file_contents.c_str(), file_contents.length());

            // ルートから文字列の抽出
            TSNode root_node = ts_tree_root_node(tree);
            std::vector<ContentsView> parse_result;
            extractStringNodes(root_node, file_contents, parse_result);

            // 結果をTranslateTextオブジェクトに変換
            std::vector<TranslateText> result;
            auto filename = path.filename().stem().u8string();

            for(auto& node : parse_result)
            {
                auto [row, column] = node.row_col;
                auto [start_byte, end_byte] = node.start_end_byte;

                // 範囲チェック
                if(start_byte >= end_byte || start_byte >= file_contents.length() || end_byte > file_contents.length()) {
                    continue;
                }

                std::string text = file_contents.substr(start_byte, end_byte - start_byte);

                // 空文字列や意味のない文字列は無視
                if(text.empty() || text == "\\n" || text == "\\r" || text == "\\t") {
                    continue;
                }

                // TranslateTextオブジェクトを作成
                langscore::TranslateText t = {
                    utility::cnvStr<std::u8string>(text),
                    this->useLangList
                };

                // 行と列の情報を文字列に変換
                auto lineCountStr = std::to_string(row);
                std::u8string u8lineCount(lineCountStr.begin(), lineCountStr.end());
                auto colCountStr = std::to_string(column);
                std::u8string u8ColCountStr(colCountStr.begin(), colCountStr.end());

                // スクリプト位置情報を設定
                auto scriptPos = filename + u8":" + u8lineCount + u8":" + u8ColCountStr;
                t.scriptLineInfo = scriptPos;

                result.emplace_back(std::move(t));
            }

            // リソースの解放
            ts_tree_delete(tree);
            ts_parser_delete(parser);

            return result;
        }

        ProgressNextStep checkLineComment(TextCodec& lineText) const override
        {
            //先頭が行コメントなら無視する
            auto lineCommentBegin = lineText.find(this->lineComment);
            if(lineCommentBegin == TextCodec::npos) {
                return lineText.empty() ? ProgressNextStep::NextLine : ProgressNextStep::Throught;
            }

            //検出位置が0なら文字列が入る余地が無いのでコメントとして扱う
            if(lineCommentBegin == 0) {
                return ProgressNextStep::NextLine;
            }

            //文字列中の場合はコメントとして扱わない ("#{number}"など)
            if(findBeginEnclose(lineText, lineCommentBegin) != TextCodecChar{}) {
                return ProgressNextStep::Throught;
            }

            std::fill_n(lineText.begin() + lineCommentBegin, lineText.size() - lineCommentBegin, TextCodecChar(' '));

            if(isValidProgramLine(lineText) == false) {
                return ProgressNextStep::NextLine;
            }
            return lineText.empty() ? ProgressNextStep::NextLine : ProgressNextStep::Throught;
        }
    };
}


/*
//javascriptreaderのruby版のクラスを作成
#include "readerbase.h"
#include "utility.hpp"
#include "csvreader.h"
#include "config.h"
#include "scripttextparser.hpp"
#include <filesystem>
#include <ranges>

namespace langscore
{

	class rubyreader: public readerbase
	{
	public:
		rubyreader(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList)
			: readerbase(std::move(langs), std::move(scriptFileList))
		{
			this->setComment(u8"#", u8"=begin", u8"=end");

			config config;
			const auto lsAnalyzePath = std::filesystem::path(config.langscoreAnalyzeDirectorty());
			auto filePathPair = plaincsvreader{lsAnalyzePath / "Scripts/_list.csv"}.getPlainCsvTexts();
			for(const auto& row : filePathPair)
			{
				auto info = PluginInfo{
					utility::cnvStr<std::u8string>(row[1]),
					utility::cnvStr<std::u8string>(row[0]) + u8".rb",
					true, u8"", {}
				};
				pluginInfoList.emplace_back(std::move(info));
			}

			for(auto& path : this->scriptFileList)
			{
				auto scriptTexts = this->parse(path);
				this->texts.insert(this->texts.end(), scriptTexts.begin(), scriptTexts.end());

				scriptTranslatesMap.emplace_back(path.filename().u8string(), std::move(scriptTexts));
			}
		}
		~rubyreader() override {}


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
			rubyreader reader({def_lang}, {});
			utility::u8stringlist ignoreRowName;
			for(auto& scriptInfo : pluginInfoList)
			{
				auto fileName = fs::path(scriptInfo.filename).filename().u8string();
                auto scriptName = scriptInfo.name;

                if(scriptName == u8"langscore" || scriptName == u8"langscore_custom") {
                    std::erase_if(this->texts, [fileName = utility::removeExtension(fileName)](const auto& x) {
                        return x.original.find(fileName) != std::remove_const_t<std::remove_reference_t<decltype(x.original)>>::npos;
                    });
                    continue;
                }

                auto result = std::find_if(scriptInfoList.cbegin(), scriptInfoList.cend(), [&](const auto& x) {
                    return x.filename == fileName;
                });

                if(result == scriptInfoList.end()) {
                    continue;
                }

				if(result->ignore == false){
					for(const auto& textInfo : result->texts)
					{
						if(textInfo.disable){ continue; }
						if(textInfo.ignore){ continue; }
                        //スクリプト内の行を無視する。
						auto name = fileName + utility::cnvStr<std::u8string>(":" + std::to_string(textInfo.row) + ":" + std::to_string(textInfo.col));
						ignoreRowName.emplace_back(std::move(name));
					}
				}
				else{
                    //スクリプト自体を無視する。
					std::erase_if(this->texts, [&fileName](const auto& x){
						return x.original.find(fileName) != std::remove_const_t<std::remove_reference_t<decltype(x.original)>>::npos;
					});
				}
			}

            if(ignoreRowName.empty() == false)
			{
                //無視する行の削除。
				std::erase_if(this->texts, [&ignoreRowName](const auto& t){
					return std::find(ignoreRowName.cbegin(), ignoreRowName.cend(), t.original) != ignoreRowName.cend();
				});
			}
		}

	private:

		std::vector<TranslateText> parse(std::filesystem::path path)
		{
			//パース自体はあくまでコンストラクタに渡されたファイルパスをそのまま読み込ませるべきなので、
			//ここでスクリプト名によるフィルター等は行わない。
			return convertScriptToCSV(path);
		}

		std::u8string GetScriptName(std::u8string fileName) const
		{
			using namespace std::string_literals;

			auto result = std::ranges::find_if(pluginInfoList, [&fileName](const auto& x){
				return x.filename == fileName;
			});
			if(result == pluginInfoList.end()){ return u8""s; }
			auto scriptName = (*result).name;
			if(scriptName == u8"langscore"){ return u8""s; }
			if(scriptName == u8"langscore_custom"){ return u8""s; }
			return scriptName;
		}

		ProgressNextStep checkLineComment(TextCodec& lineText) const override
		{
			//先頭が行コメントなら無視する
			auto lineCommentBegin = lineText.find(this->lineComment);
			if(lineCommentBegin == TextCodec::npos){
				return lineText.empty() ? ProgressNextStep::NextLine : ProgressNextStep::Throught;
			}

			//検出位置が0なら文字列が入る余地が無いのでコメントとして扱う
			if(lineCommentBegin == 0){
				return ProgressNextStep::NextLine;
			}

			//文字列中の場合はコメントとして扱わない ("#{number}"など)
			if(findBeginEnclose(lineText, lineCommentBegin) != TextCodecChar{}){
				return ProgressNextStep::Throught;
			}

			std::fill_n(lineText.begin() + lineCommentBegin, lineText.size() - lineCommentBegin, TextCodecChar(' '));

			if(isValidProgramLine(lineText) == false){
				return ProgressNextStep::NextLine;
			}
			return lineText.empty() ? ProgressNextStep::NextLine : ProgressNextStep::Throught;
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
						else {
							auto endPos = std::get<1>(strView);
							parser.convertTranslateTextFromMatch(line.substr(strStart, endPos - strStart), col, transTextList);
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
							parser.convertTranslateTextFromMatch(line.substr(strStart, endPos - strStart), col, transTextList);
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
*/