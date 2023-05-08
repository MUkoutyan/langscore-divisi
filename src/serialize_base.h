#pragma once
#include <string>
#include <vector>
#include <map>

namespace langscore
{
    enum class MergeTextMode
    {
        AcceptSource = 0,   //元のファイルを残す
        AcceptTarget = 1,   //先のファイルを残す
        MergeKeepSource,    //競合した場合、元ファイルの内容を保持する。
        MergeKeepTarget,    //競合した場合、先ファイルの内容を保持する。
        Both,               //両方残す
    };

    struct PluginInfo
    {
        std::u8string name;
        std::u8string filename; //拡張子はつけること
        bool status = false;
        std::u8string description;
        std::map<std::u8string, std::u8string> parameters;
    };

    //翻訳テキストのクラス
    class TranslateText
    {
    public:
        TranslateText():TranslateText(u8"", {}) {}
        TranslateText(std::u8string origin, std::vector<std::u8string> langs)
            : original(std::move(origin)), translates(), scriptLineInfo(), code(0)
        {
            for(auto& lang : langs){
                this->translates[lang] = u8"";
            }
        }
        std::u8string original; //オリジナルの文章
        std::map<std::u8string, std::u8string> translates;  //各言語毎の文章
        std::u8string scriptLineInfo;   //スクリプトでのオリジナルの文章の行情報
        int code;   //RPGツクールでコマンドを検出した際に使用

        //CSV用のヘッダーを作成
        auto createHeader() const
        {
            std::vector<std::u8string> result;
            result.emplace_back(u8"original");
            for(const auto& l : translates){
                result.emplace_back(l.first);
            }
            return result;
        }
    };

    using ScriptPackage = std::vector<std::tuple<std::u8string, std::vector<TranslateText>>>;
}