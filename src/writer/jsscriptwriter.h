#ifndef JSSCRIPTWRITER_H
#define JSSCRIPTWRITER_H

#include "writerbase.h"
#include "config.h"

#ifdef ENABLE_TEST
class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, CheckRubyCommentLine);
class IUTEST_TEST_CLASS_NAME_(Langscore_Divisi, VXAce_WriteScriptCSV);
#endif

namespace langscore
{
    class jsscriptwriter: public writerbase
    {
#ifdef ENABLE_TEST
        IUTEST_FRIEND_TEST(Langscore_Writer, CheckRubyCommentLine);
        IUTEST_FRIEND_TEST(Langscore_Divisi, VXAce_WriteScriptCSV);
#endif
    public:
        using ScriptPathList = std::vector<std::tuple<std::u8string, std::vector<TranslateText>>>;
        jsscriptwriter(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList);
        constexpr static const char* extension = "js";

        bool merge(std::filesystem::path filePath) override;
        ErrorStatus write(std::filesystem::path path, MergeTextMode overwriteMode = MergeTextMode::AcceptSource) override;
        const ScriptPathList& getScriptTexts() const { return scriptTranslates; }

        std::vector<TranslateText> acceptIgnoreScripts(const std::vector<config::ScriptData>&, std::vector<TranslateText> texts);

    private:
        ScriptPathList scriptTranslates;
        std::vector<std::filesystem::path> scriptFileList;

        ProgressNextStep checkCommentLine(TextCodec&) override;
        void WriteVocab(std::ofstream& file, std::vector<TranslateText> texts);

    };
}

#endif // RBSCRIPTWRITER_H

