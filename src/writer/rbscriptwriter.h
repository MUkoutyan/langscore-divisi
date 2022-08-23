#ifndef RBSCRIPTWRITER_H
#define RBSCRIPTWRITER_H

#include "writerbase.h"


#ifdef ENABLE_TEST
class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, CheckRubyCommentLine);
#endif

namespace langscore
{
    class rbscriptwriter: public writerbase
    {
#ifdef ENABLE_TEST
        IUTEST_FRIEND_TEST(Langscore_Writer, CheckRubyCommentLine);
#endif
    public:
        using ScriptPathList = std::vector<std::tuple<std::filesystem::path, std::vector<TranslateText>>>;
        rbscriptwriter(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList);
        constexpr static const char* extension = "rb";

        bool merge(std::filesystem::path filePath) override;
        ErrorStatus write(std::filesystem::path path, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld) override;
        const ScriptPathList& getScriptTexts() const { return scriptTranslates; }

    private:
        ScriptPathList scriptTranslates;
        std::map<std::filesystem::path, std::u8string> scriptNameMap;

        ProgressNextStep checkCommentLine(TextCodec&) override;
        void WriteVocab(std::ofstream& file, std::vector<TranslateText> texts);

    };
}

#endif // RBSCRIPTWRITER_H
