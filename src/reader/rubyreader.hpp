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
            : readerbase(std::move(scriptFileList))
            , print_debug(false)
        {
            this->useLangList = std::move(langs);

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

        // --- ヒアドキュメントを再パースする関数例 ---
        void extractStringNodesWithOffset(
            TSNode node,
            const std::string& text,                   // ヒアドキュメントの文字列
            std::vector<ContentsView>& result,
            uint32_t base_byte,                        // 元ソースにおけるヒアドキュメント開始バイト
            int base_row,                              // 元ソースにおけるヒアドキュメント開始行
            int base_col,                              // 元ソースにおけるヒアドキュメント開始列
            bool through_string = false
        ) const {
            if(ts_node_is_null(node)) return;

            std::string type = ts_node_type(node);

            // --- 文字列リテラル類が見つかったら、オフセットを足して result へ ---
            if(type == "string" || type == "string_literal" || type == "string_content") {
                // ヒアドキュメント内部での位置
                auto [row, col] = GetNodePoint(node);
                auto [s_byte, e_byte] = GetNodeStartEndByte(node);

                // 元のソースにおける行・列・バイト位置に補正
                int abs_row = base_row + row;
                int abs_col = (row == 0 ? base_col + col : col);
                uint32_t abs_s_byte = base_byte + s_byte;
                uint32_t abs_e_byte = base_byte + e_byte;

                // 空でなければ追加
                if(abs_s_byte < abs_e_byte) {
                    result.emplace_back(
                        std::make_pair(abs_row, abs_col),
                        std::make_pair(abs_s_byte, abs_e_byte)
                    );
                }
                // string_content は deeper も不要
                if(type != "string_content") return;
            }

            // --- それ以外は通常の再帰探索 ---
            uint32_t count = ts_node_child_count(node);
            for(uint32_t i = 0; i < count; ++i) {
                extractStringNodesWithOffset(
                    ts_node_child(node, i),
                    text, result, base_byte, base_row, base_col, through_string
                );
            }
        }

        // Rubyの構文木から文字列ノードを抽出する関数
        void extractStringNodes(
            TSNode node,
            const std::string_view& source_code,
            std::vector<ContentsView>& result,
            bool through_string = false
        ) const
        {
            // ノードがない場合は何もしない
            if(ts_node_is_null(node)) {
                return;
            }

            std::string node_type = ts_node_type(node);

            // 無視するノードタイプ
            //element_referenceは args["hoge"] のような記述
            static const std::vector<std::string_view> ignore_type = {
                "identifier", "operator", "integer", "simple_symbol", "element_reference"
                "symbol", "ERROR", ".", ":", "::", "=>", "=", "==", ",", "{", "}", "(", ")",
                "comment", "regex", "when", "if", "else", "elsif", "case", "do", "while"
            };

            // 無視するメソッド/関数名
            static const std::vector<std::string_view> ignore_method = {
                "require", "puts", "print", "p", "include?", "gsub!", "gsub"
                "attr_accessor", "attr_reader", "attr_writer", "split",
                "ls_output_log", "raise", "load_data"
            };

            // 無視すべきノードタイプかどうかを確認
            if(std::ranges::find(ignore_type, node_type) != ignore_type.cend()) {
                return;
            }

            if(print_debug) {
                std::cout << "Node type: " << node_type << std::endl;
            }

            // 代入式の左辺など、文字列を通過させない場合
            if(through_string) {
                if(node_type == "string" || node_type == "string_content" || node_type == "escape_sequence")
                {
                    return;
                }
            }

            // ダブル／シングルクォート文字列 ("..." または '...') をまとめて取得
            if(node_type == "string_content" || node_type == "escape_sequence") {
                // ノード全体の範囲から、先頭と末尾のクォートを除いた範囲を直接取得
                auto [row, column] = GetNodePoint(node);
                std::uint32_t start_byte = ts_node_start_byte(node);
                std::uint32_t end_byte = ts_node_end_byte(node);

                if(start_byte < end_byte) {
                    // (row, column+1) は実際の文字列開始位置
                    result.emplace_back(
                        std::make_pair(row, column),
                        std::make_pair(start_byte, end_byte)
                    );
                }
                return;
            }
            else if(node_type == "string")
            {
                bool check_string = false;
                //interpolation("#{var}"の記述), escape_sequenceがあればcheck_stringをtrueにする。
                auto interpolation = findFirstChildNode(node, "interpolation");
                auto esc_seq = findFirstChildNode(node, "escape_sequence");
                if(false == ts_node_is_null(interpolation) || false == ts_node_is_null(esc_seq)) {
                    check_string = true;
                }

                auto str_array = findFirstChildNode(node, "string_array");
                if(ts_node_is_null(str_array) == false) {
                    check_string = false;
                }

                auto var = std::string(
                    source_code.substr(
                        ts_node_start_byte(node),
                        ts_node_end_byte(node)
                        - ts_node_start_byte(node)
                    )
                );

                if(check_string) 
                {
                    // 例: "Hello #{name}!"
                    // string内ではstring_contentとinterpolationが別々で格納されているため、
                    // interpolationが含まれる場合はstringで文字列を直接扱う。
                    // ""が含まれる点に注意。

                    auto dq_list = findChildNodeList(node, "\"");
                    if(dq_list.size() != 2) {
                        //構文ミスしてそうな文字列
                        return;
                    }

                    //%q{}や%Q{}が使用されている場合、%Q{" と " は、 node_type == "として扱われる。
                    int begin_dq_length = 1;
                    {
                        auto begin = ts_node_start_byte(dq_list[0]);
                        auto end = ts_node_end_byte(dq_list[0]);
                        begin_dq_length = end - begin;
                    }
                    int end_dq_length = 1;
                    {
                        auto begin = ts_node_start_byte(dq_list[1]);
                        auto end = ts_node_end_byte(dq_list[1]);
                        end_dq_length = end - begin;
                    }

                    // ノード全体の範囲から、先頭と末尾のクォートを除いた範囲を直接取得
                    auto [row, column] = GetNodePoint(node);
                    std::uint32_t start_byte = ts_node_start_byte(node) + begin_dq_length;
                    std::uint32_t end_byte = ts_node_end_byte(node) - end_dq_length;

                    if(start_byte < end_byte) {
                        // (row, column+1) は実際の文字列開始位置
                        result.emplace_back(
                            std::make_pair(row, column+1),
                            std::make_pair(start_byte, end_byte)
                        );
                    }
                    return;
                }
            }
            // 配列リテラル内の文字列を処理する新しい処理
            else if(node_type == "array") {
                // 配列の各要素を処理
                uint32_t child_count = ts_node_child_count(node);
                for(uint32_t i = 0; i < child_count; i++) {
                    TSNode child = ts_node_child(node, i);
                    // 各子ノードを再帰的に処理 (string, string_content等を含む)
                    extractStringNodes(child, source_code, result);
                }
                return;
            }
            // %q{} や %Q{} などの代替クォート文字列も処理
            else if(node_type == "string_array") {
                // 例: %q{メッセージ} や %Q[テキスト] など
                auto [row, column] = GetNodePoint(node);
                auto [start_byte, end_byte] = GetNodeStartEndByte(node);

                // 最低でも "%q{" (3 バイト) + "}" (1 バイト) が必要
                if(start_byte + 3 < end_byte) {
                    // 開始プレフィックス "%q" の次が区切り文字
                    char open_delim = source_code[start_byte + 2];
                    char close_delim = (open_delim == '{' ? '}' :
                        open_delim == '[' ? ']' :
                        open_delim == '(' ? ')' :
                        open_delim == '<' ? '>' : open_delim);

                    // 中身の範囲を計算
                    std::uint32_t content_start = start_byte + 3;
                    std::uint32_t content_end = end_byte;
                    if(content_end > content_start && source_code[content_end - 1] == close_delim) {
                        content_end--;
                    }

                    if(content_start < content_end) {
                        ContentsView view;
                        // 列も中身の先頭位置に合わせてオフセット
                        view.row_col = {row, column + (int)(content_start - start_byte)};
                        view.start_end_byte = {content_start, content_end};
                        result.emplace_back(std::move(view));
                    }
                }
                return;
            }
            // メソッド呼び出し: .lstrans は無視、他のメソッドは引数中の文字列を抽出
            else if(node_type == "method_call" || node_type == "call") 
            {
                // メソッド名取得
                auto constant_node = findFirstChildNode(node, "constant");
                std::string constant_name;
                if(!ts_node_is_null(constant_node)) {
                    constant_name = std::string(
                        source_code.substr(
                            ts_node_start_byte(constant_node),
                            ts_node_end_byte(constant_node)
                            - ts_node_start_byte(constant_node)
                        )
                    );
                }
                auto method_node_list = findChildNodeList(node, "identifier");
                if(method_node_list.empty()) {
                    return;
                }
                auto method_node = method_node_list.back();
                std::string method_name;
                if(!ts_node_is_null(method_node)) {
                    method_name = std::string(
                        source_code.substr(
                            ts_node_start_byte(method_node),
                            ts_node_end_byte(method_node)
                            - ts_node_start_byte(method_node)
                        )
                    );
                }

                //特定の組み合わせを検出
                if(constant_name == "File" && method_name == "open") {
                    return;
                }
                if(constant_name == "Win32API" && method_name == "new") {
                    return;
                }

                // .lstrans の場合は何もしない
                if(method_name == "lstrans") {
                    return;
                }

                // 無視メソッドでなければ、引数リスト中を探索
                if(std::ranges::find(ignore_method, method_name) == ignore_method.cend()) {
                    auto args_node = findFirstChildNode(node, "argument_list", true);
                    if(!ts_node_is_null(args_node)) {
                        uint32_t n = ts_node_child_count(args_node);
                        for(uint32_t i = 0; i < n; ++i) {
                            extractStringNodes(
                                ts_node_child(args_node, i),
                                source_code,
                                result
                            );
                        }
                    }
                }
                return;
            }
            else if(node_type == "pair") {
                // 値部分だけを処理
                TSNode value_node = ts_node_child(node, 2);
                extractStringNodes(value_node, source_code, result);
                return;
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
                    extractStringNodes(child, source_code, result, !found_equal);
                }
                return;
            }
            else if(node_type == "heredoc_body" || node_type == "heredoc_content") 
            {
                // 元ソース上のヒアドキュメント開始位置
                auto [start_row, start_col] = GetNodePoint(node);
                auto [start_byte, end_byte] = GetNodeStartEndByte(node);

                // 実際のヒアドキュメント文字列を抜き出し
                std::string heredoc_text = std::string(
                    source_code.substr(start_byte, end_byte - start_byte)
                );

                // 別パーサで再パース
                TSParser* inner_parser = ts_parser_new();
                ts_parser_set_language(inner_parser, tree_sitter_ruby());
                TSTree* inner_tree = ts_parser_parse_string(
                    inner_parser,
                    nullptr,
                    heredoc_text.c_str(),
                    static_cast<uint32_t>(heredoc_text.size())
                );
                TSNode inner_root = ts_tree_root_node(inner_tree);

                // 補正付きで再帰的に抽出
                extractStringNodesWithOffset(
                    inner_root,
                    heredoc_text,
                    result,
                    start_byte,
                    start_row,
                    start_col
                );

                // 後片付け
                ts_tree_delete(inner_tree);
                ts_parser_delete(inner_parser);
                return;
            }

            // 再帰的に子ノードを処理
            uint32_t child_count = ts_node_child_count(node);
            for(uint32_t i = 0; i < child_count; i++) 
            {
                extractStringNodes(
                    ts_node_child(node, i),
                    source_code,
                    result
                );
            }
        }

        std::vector<TranslateText> convertScriptToCSV(std::filesystem::path path) const
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

            if(file_contents.empty()) {
                return {};
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

            // ファイル全体を行ごとに分割
            std::vector<std::u8string> lines;
            {
                std::istringstream stream(file_contents);
                std::string line;
                while(std::getline(stream, line)) {
                    lines.emplace_back(utility::cnvStr<std::u8string>(line));
                }
            }

            if(lines.empty()) { return {}; }

            for(auto& node : parse_result) {
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

                // UTF-8文字単位で列数を計算
                ScriptTextParser parser;
                if(row == 0 || row-1 < lines.size()) {
                    const auto& target_line = lines[row-1];
                    column = parser.wordCountUTF8(target_line.substr(0, column));
                }
                else {
                    column = 0; // 行が範囲外の場合は列数を0に設定
                }

                // TranslateTextオブジェクトを作成
                langscore::TranslateText t = {
                    utility::cnvStr<std::u8string>(text),
                    this->useLangList
                };

                // 行と列の情報を文字列に変換
                auto lineCountStr = std::to_string(row);
                std::u8string u8lineCount(lineCountStr.begin(), lineCountStr.end());
                auto colCountStr = std::to_string(column+1);
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
    };
}
