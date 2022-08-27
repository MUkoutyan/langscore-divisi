#ifndef RBSCRIPTWRITER_H
#define RBSCRIPTWRITER_H

#include "writerbase.h"
#include "config.h"

#ifdef ENABLE_TEST
class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, CheckRubyCommentLine);
class IUTEST_TEST_CLASS_NAME_(Langscore_Divisi, VXAce_WriteScriptCSV);
#endif

namespace langscore
{
    class rbscriptwriter: public writerbase
    {
#ifdef ENABLE_TEST
        IUTEST_FRIEND_TEST(Langscore_Writer, CheckRubyCommentLine);
        IUTEST_FRIEND_TEST(Langscore_Divisi, VXAce_WriteScriptCSV);
#endif
    public:
        using ScriptPathList = std::vector<std::tuple<std::u8string, std::vector<TranslateText>>>;
        rbscriptwriter(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList);
        constexpr static const char* extension = "rb";

        bool merge(std::filesystem::path filePath) override;
        ErrorStatus write(std::filesystem::path path, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld) override;
        const ScriptPathList& getScriptTexts() const { return scriptTranslates; }

        std::vector<TranslateText> acceptIgnoreScripts(const std::vector<config::ScriptData>&, std::vector<TranslateText> texts);

    private:
        ScriptPathList scriptTranslates;
        std::vector<std::filesystem::path> scriptFileList;
        std::map<std::u8string, std::u8string> scriptNameMap;

        ProgressNextStep checkCommentLine(TextCodec&) override;
        void WriteVocab(std::ofstream& file, std::vector<TranslateText> texts);
        std::u8string GetScriptName(std::u8string filePath);

    };
}

#endif // RBSCRIPTWRITER_H
