#pragma once
#include <string>
#include <vector>
#include <map>

namespace langscore
{
    enum class MergeTextMode
    {
        AcceptSource = 0,   //���̃t�@�C�����c��
        AcceptTarget = 1,   //��̃t�@�C�����c��
        MergeKeepSource,    //���������ꍇ�A���t�@�C���̓��e��ێ�����B
        MergeKeepTarget,    //���������ꍇ�A��t�@�C���̓��e��ێ�����B
        Both,               //�����c��
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