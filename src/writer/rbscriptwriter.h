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

        bool write(std::filesystem::path path, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld) override;

    private:
        std::map<std::filesystem::path, std::vector<TranslateText>> scriptTranslates;

        bool ConvertScriptToCSV(std::filesystem::path path);
    };
}

#endif // RBSCRIPTWRITER_H
