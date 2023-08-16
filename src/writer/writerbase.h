#ifndef WRITERBASE_H
#define WRITERBASE_H

#include "reader/jsonreader.hpp"
#include "errorstatus.hpp"
#include "reader/readerbase.h"
#include <tuple>
#include <filesystem>

#ifdef ENABLE_TEST
#define NOMINMAX
#include "iutest.hpp"

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
        //Determine if Reader type is unique_ptr type
        template<typename Reader, typename = std::enable_if_t<!std::is_same_v<std::unique_ptr<readerbase>, std::decay_t<Reader>>>>
        writerbase(Reader&& reader)
            : useLangs(reader.curerntUseLangList()), texts(reader.curerntTexts())
            , scriptTranslatesMap(reader.curerntScriptTransMap())
            , overwriteMode(MergeTextMode::AcceptSource), fillDefLangCol(false){
        }
        writerbase(const std::unique_ptr<readerbase>& reader): writerbase(*reader){}
        virtual ~writerbase();

        virtual ErrorStatus write(std::filesystem::path writePath, MergeTextMode overwriteMode = MergeTextMode::AcceptSource) = 0;
        virtual bool merge(std::filesystem::path srcPath) { return true; }

        void setOverwriteMode(MergeTextMode overwriteMode){
            this->overwriteMode = overwriteMode;
        }
        void setFillDefLangCol(bool is){
            this->fillDefLangCol = is;
        }
        std::vector<TranslateText>& curerntTexts() { return texts; }
        const ScriptPackage& getScriptTexts() const { return scriptTranslatesMap; }

        bool isDebug = false;

#ifdef _DEBUG
        static void ReplaceDebugTextByOrigin(std::vector<TranslateText>& transTexts);
        static void ReplaceDebugTextByLang(std::vector<TranslateText>& transTexts, std::u8string def_lang);
#endif

    protected:
        std::vector<std::u8string> useLangs;
        std::vector<TranslateText> texts;
        ScriptPackage scriptTranslatesMap;
        MergeTextMode overwriteMode;
        bool fillDefLangCol;

        static void writeU8String(std::ofstream& out, std::u8string text);
    };
}

#endif // WRITERBASE_H
