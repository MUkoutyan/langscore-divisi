#include "tree_sitter/api.h"
#include <string>
#include <vector>
#include <tuple>

static std::tuple<uint32_t, uint32_t> GetNodeStartEndByte(TSNode node)
{
    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    return {start, end};
}

static std::tuple<uint32_t, uint32_t> GetNodePoint(TSNode node, bool start_row_zero = false, bool start_col_zero = true)
{
    auto pos = ts_node_start_point(node);
    unsigned row = pos.row;
    unsigned column = pos.column;

    if(start_row_zero == false) {
        row += 1;
    }
    if(start_col_zero == false) {
        column += 1;
    }

    return {row, column};
}


static std::string joinChildNodeTexts(const TSNode root, std::string_view source_code, bool recursive = false)
{
    auto numChilds = ts_node_child_count(root);
    std::string result;
    for(auto i = 0; i < numChilds; ++i) {

        auto child_node = ts_node_child(root, i);
        if(ts_node_is_null(child_node)) { continue; }

        if(recursive && ts_node_child_count(child_node) > 0) {
            auto child_result = joinChildNodeTexts(child_node, source_code, recursive);
            result += child_result;
        }
        else
        {
            //ノード全体の範囲を抜き出すため、再帰的に検索する場合と合わせて文字列の結合をすると
            //重複した文字列となってしまう。
            auto start = ts_node_start_byte(child_node);
            auto end = ts_node_end_byte(child_node);
            auto text = source_code.substr(start, end - start);
            result += text;
        }
    }
    return result;
}

static TSNode findFirstChildNode(const TSNode& node, std::string_view findNodeType, bool recursive = false)
{
    auto numChilds = ts_node_child_count(node);
    for(auto i = 0; i < numChilds; ++i) {

        auto child_node = ts_node_child(node, i);
        if(ts_node_is_null(child_node)) { continue; }

        if(ts_node_type(child_node) == findNodeType) {
            return child_node;
        }

        if(recursive) {
            auto child_result = findFirstChildNode(child_node, findNodeType, recursive);
            if(ts_node_is_null(child_result) == false) { child_result; }
        }
    }

    return TSNode();
}

static std::vector<TSNode> findChildNodeList(const TSNode& node, std::string_view findNodeType, bool recursive = false)
{
    std::vector<TSNode> result;

    auto numChilds = ts_node_child_count(node);
    for(auto i = 0; i < numChilds; ++i)
    {
        auto child_node = ts_node_child(node, i);
        if(ts_node_is_null(child_node)) { continue; }

        if(ts_node_type(child_node) == findNodeType) {
            result.emplace_back(child_node);
        }

        if(recursive) {
            auto child_result = findChildNodeList(child_node, findNodeType, recursive);
            std::copy(child_result.begin(), child_result.end(), std::back_inserter(result));
        }
    }

    return result;
}

static std::string findMethodName(const TSNode& node, std::string_view source_code, std::string_view function_type)
{
    if(ts_node_is_null(node)) {
        return "";
    }
    std::string method_chain = "";
    auto numChilds = ts_node_child_count(node);
    for(auto i = 0; i < numChilds; ++i)
    {
        auto child_node = ts_node_child(node, i);
        if(ts_node_type(child_node) == function_type) {
            method_chain += findMethodName(child_node, source_code, function_type);
        }
        auto [start_byte, end_byte] = GetNodeStartEndByte(child_node);
        method_chain += source_code.substr(start_byte, end_byte - start_byte);
    }
    return method_chain;
}