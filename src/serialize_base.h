#pragma once
#include <string>
#include <vector>
#include <map>

namespace langscore
{
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
        std::u8string memo;

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