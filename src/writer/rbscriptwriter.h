#ifndef RBSCRIPTWRITER_H
#define RBSCRIPTWRITER_H

#include "writerbase.h"

namespace langscore
{
    class rbscriptwriter: public writerbase
    {
    public:
        using ScriptPathList = std::vector<std::tuple<std::filesystem::path, std::vector<TranslateText>>>;
        rbscriptwriter(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList);
        constexpr static const char* extension = "rb";

        bool merge(std::filesystem::path filePath) override;
        bool write(std::filesystem::path path, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld) override;
        const ScriptPathList& getScriptTexts() const { return scriptTranslates; }

    private:
        ScriptPathList scriptTranslates;
        std::map<std::filesystem::path, std::u8string> scriptNameMap;

        ProgressNextStep checkCommentLine(TextCodec&) override;
        void WriteVocab(std::ofstream& file, std::vector<TranslateText> texts);

    };
}

#endif // RBSCRIPTWRITER_H
