#ifndef RBSCRIPTWRITER_H
#define RBSCRIPTWRITER_H

#include "writerbase.h"

namespace langscore
{
    class rbscriptwriter: public writerbase
    {
    public:
        rbscriptwriter(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList);
        constexpr static const char* extension = "rb";

        bool merge(std::filesystem::path filePath) override;
        bool write(std::filesystem::path path, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld) override;
        const std::map<std::filesystem::path, std::vector<TranslateText>>& getScriptTexts() const { return scriptTranslates; }

    private:
        std::map<std::filesystem::path, std::vector<TranslateText>> scriptTranslates;
        std::map<std::filesystem::path, std::u8string> scriptNameMap;

        bool ConvertScriptToCSV(std::filesystem::path path, std::u8string scriptName);
        void WriteVocab(std::ofstream& file, std::vector<TranslateText> texts);
    };
}

#endif // RBSCRIPTWRITER_H
