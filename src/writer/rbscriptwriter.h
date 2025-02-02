#ifndef RBSCRIPTWRITER_H
#define RBSCRIPTWRITER_H

#include "writerbase.h"
#include "config.h"

namespace langscore
{
    //Langscore_CustomとなるRubyスクリプトを書き出すクラス
    class rbscriptwriter: public writerbase
    {
    public:
        template<typename Reader, typename = std::enable_if_t<!std::is_same_v<std::unique_ptr<readerbase>, std::decay_t<Reader>>>>
        rbscriptwriter(Reader&& reader): writerbase(reader){
            pluginInfoList = reader.curerntPluginInfoList();
        }
        rbscriptwriter(const std::unique_ptr<readerbase>& reader): rbscriptwriter(*reader) {}
        constexpr static const char* extension = "rb";

        bool merge(std::filesystem::path filePath) override;
        ErrorStatus write(std::filesystem::path path, std::u8string defaultLanguage, MergeTextMode overwriteMode = MergeTextMode::AcceptSource) override;

    private:
        std::vector<PluginInfo> pluginInfoList;

        void WriteVocab(std::ofstream& file, std::vector<TranslateText> texts);
        std::u8string GetScriptName(std::u8string filePath);

    };
}

#endif // RBSCRIPTWRITER_H
