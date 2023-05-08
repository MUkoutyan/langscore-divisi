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
    public:
        struct PluginInfo
        {
            std::u8string name;
            bool status;
            std::u8string description;
            std::map<std::u8string, std::u8string> parameters;
        };

        using writerbase::writerbase;
        //jsscriptwriter(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList);
        constexpr static const char* extension = "js";

        bool merge(std::filesystem::path filePath) override;
        ErrorStatus write(std::filesystem::path path, MergeTextMode overwriteMode = MergeTextMode::AcceptSource) override;

        
    private:
        std::vector<std::filesystem::path> scriptFileList;

        void WriteVocab(std::ofstream& file, std::vector<TranslateText> texts);
    };

}

#endif // RBSCRIPTWRITER_H

