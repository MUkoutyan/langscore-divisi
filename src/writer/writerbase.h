#ifndef WRITERBASE_H
#define WRITERBASE_H

#include "reader/jsonreader.hpp"
#include "errorstatus.hpp"
#include <tuple>
#include <filesystem>

#ifdef ENABLE_TEST
#include "iutest.hpp"

#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif

class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, CheckRubyCommentLine);
class IUTEST_TEST_CLASS_NAME_(Langscore_Writer, DetectStringPositionFromFile);

#endif

namespace langscore
{
    class writerbase
    {
#ifdef ENABLE_TEST
        IUTEST_FRIEND_TEST(Langscore_Writer, CheckRubyCommentLine);
        IUTEST_FRIEND_TEST(Langscore_Writer, DetectStringPositionFromFile);
#endif
    public:

        using TextCodec = std::u8string;

        writerbase(std::vector<std::u8string> langs, const std::unique_ptr<jsonreaderbase>& json);
        writerbase(std::vector<std::u8string> langs, std::vector<TranslateText> texts);
        virtual ~writerbase();

        virtual ErrorStatus write(std::filesystem::path writePath, MergeTextMode overwriteMode = MergeTextMode::AcceptSource) = 0;
        virtual bool merge(std::filesystem::path srcPath) { return true; }

        void setOverwriteMode(MergeTextMode overwriteMode){
            this->overwriteMode = overwriteMode;
        }
        std::vector<TranslateText>& curerntTexts() { return texts; }
        bool isDebug = false;

#ifdef _DEBUG
        static void ReplaceDebugTextByOrigin(std::vector<TranslateText>& transTexts);
        static void ReplaceDebugTextByLang(std::vector<TranslateText>& transTexts, std::u8string def_lang);
#endif

    protected:
        std::vector<std::u8string> useLangs;
        std::vector<TranslateText> texts;
        MergeTextMode overwriteMode;
        bool rangeComment;

        static void writeU8String(std::ofstream& out, std::u8string text);

        std::vector<TranslateText> convertScriptToCSV(std::filesystem::path path);
        enum class ProgressNextStep
        {
            Next,
            Break,
            Throught
        };
        virtual ProgressNextStep checkCommentLine(TextCodec&) { return ProgressNextStep::Throught;  }

    };
}

#endif // WRITERBASE_H
