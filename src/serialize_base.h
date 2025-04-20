#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace langscore
{
    enum class MergeTextMode : char
    {
        AcceptSource = 0,   //元のファイルを残す
        AcceptTarget = 1,   //先のファイルを残す
        MergeKeepSource,    //競合した場合、元ファイルの内容を保持する。
        MergeKeepTarget,    //競合した場合、先ファイルの内容を保持する。
        Both,               //両方残す
    };

    struct PluginParameter {
        std::u8string key;
        std::u8string value;
    };

    struct PluginInfo
    {
        std::u8string name;
        std::u8string filename; //拡張子はつけること
        bool status = false;
        std::u8string description;
        std::unordered_map<std::u8string, std::vector<PluginParameter>> parameters;
    };

    //翻訳テキストのクラス
    class TranslateText
    {
    public:

        constexpr static const char8_t* nameType = u8"name";
        constexpr static const char8_t* descriptionType = u8"description";
        constexpr static const char8_t* messageWithGraphic = u8"messageWithIcon";
        constexpr static const char8_t* battleName = u8"battleName";
        constexpr static const char8_t* battleMessage = u8"battleMessage";
        constexpr static const char8_t* message = u8"message";
        constexpr static const char8_t* other = u8"other";

        TranslateText():TranslateText(u8"", {}) {}
        TranslateText(std::u8string origin, std::vector<std::u8string> langs)
            : original(std::move(origin)), translates(), scriptLineInfo(), scriptParamType()
            , code(0), textType({})
        {
            for(auto& lang : langs){
                this->translates[lang] = u8"";
            }
        }
        std::u8string original; //オリジナルの文章
        std::unordered_map<std::u8string, std::u8string> translates;  //各言語毎の文章。[言語コード, 文章]のマップ
        std::u8string scriptLineInfo;   //スクリプトでのオリジナルの文章の行情報
        std::u8string scriptParamType;  //(MV,MZ)プラグインのパラメータの型
        int code;   //RPGツクールでコマンドを検出した際に使用
        std::vector<std::u8string> textType; //ツクール内での種類(名前、説明、アイコン付きテキスト、テキスト等)

        //CSV用のヘッダーを作成
        auto createHeader(const std::vector<std::u8string>& useLanguageList = {}) const
        {
            std::vector<std::u8string> result;
            result.emplace_back(u8"original");
            if(useLanguageList.empty()) {
                for(const auto& l : translates) {
                    result.emplace_back(l.first);
                }
            }
            else {
                for(const auto& l : translates) 
                {
                    if(std::ranges::find(useLanguageList, l.first) == useLanguageList.end()) {
                        continue;
                    }
                    result.emplace_back(l.first);
                }
            }
            return result;
        }


        auto createHeaderForAnalyze(const std::vector<std::u8string>& useLanguageList = {}) const
        {
            auto result = createHeader(useLanguageList);
            result.emplace_back(u8"type");
            return result;
        }

        bool operator==(const TranslateText& obj) const {
            return this->original == obj.original
                && this->translates == obj.translates
                && this->scriptLineInfo == obj.scriptLineInfo
                && this->scriptParamType == obj.scriptParamType;
        }

    };

    using ScriptPackage = std::vector<std::tuple<std::u8string, std::vector<TranslateText>>>;
}