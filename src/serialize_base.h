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

    class TranslateText
    {
    public:
        TranslateText():TranslateText(u8"", {}) {}
        TranslateText(std::u8string origin, std::vector<std::u8string> langs)
            : original(std::move(origin))
        {
            for(auto& lang : langs){
                this->translates[lang] = u8"";
            }
        }
        std::u8string original;
        std::map<std::u8string, std::u8string> translates;
        std::u8string scriptLineInfo;

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
}