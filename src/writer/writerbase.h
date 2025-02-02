#ifndef WRITERBASE_H
#define WRITERBASE_H

#include "reader/jsonreader.hpp"
#include "errorstatus.hpp"
#include "reader/readerbase.h"
#include <tuple>
#include <filesystem>

namespace langscore
{
    class writerbase
    {
    public:
        //Determine if Reader type is unique_ptr type
        template<typename Reader, typename = std::enable_if_t<!std::is_same_v<std::unique_ptr<readerbase>, std::decay_t<Reader>>>>
        writerbase(Reader&& reader)
            : useLangs(reader.curerntUseLangList()), texts(reader.currentTexts())
            , scriptTranslatesMap(reader.curerntScriptTransMap())
            , overwriteMode(MergeTextMode::AcceptSource), fillDefaultLanguageColumn(false){
        }
        writerbase(const std::unique_ptr<readerbase>& reader): writerbase(*reader){}
        virtual ~writerbase();

        virtual ErrorStatus write(std::filesystem::path writePath, std::u8string defaultLanguage, MergeTextMode overwriteMode = MergeTextMode::AcceptSource) = 0;
        virtual bool merge(std::filesystem::path srcPath) { return true; }

        void setOverwriteMode(MergeTextMode overwriteMode) noexcept {
            this->overwriteMode = overwriteMode;
        }
        void setFillDefLangCol(bool is) noexcept {
            this->fillDefaultLanguageColumn = is;
        }
        void setSupportLanguage(std::vector<std::u8string> list) noexcept {
            this->useLangs = std::move(list);
        }
        std::vector<TranslateText>& currentTexts() noexcept { return texts; }
        const ScriptPackage& getScriptTexts() const noexcept { return scriptTranslatesMap; }

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
        bool fillDefaultLanguageColumn;

        static void writeU8String(std::ofstream& out, std::u8string text);
    };
}

#endif // WRITERBASE_H
