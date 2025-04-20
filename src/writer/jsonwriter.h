// src/writer/jsonwriter.h
#ifndef JSONWRITER_H
#define JSONWRITER_H

#include "writerbase.h"
#include "utility.hpp"
#include <nlohmann/json.hpp>

namespace langscore
{
    class jsonwriter : public writerbase
    {
    public:
        using writerbase::writerbase;
        constexpr static const char* extension = "lsjson";

        bool merge(std::filesystem::path sourceFilePath) override;
        ErrorStatus write(std::filesystem::path path, std::u8string defaultLanguage, MergeTextMode overwriteMode = MergeTextMode::AcceptSource) override;
        ErrorStatus writeForAnalyze(std::filesystem::path path, std::u8string defaultLanguage, MergeTextMode overwriteMode = MergeTextMode::AcceptSource);
        static ErrorStatus writePlain(std::filesystem::path path, std::vector<utility::u8stringlist> text, MergeTextMode overwriteMode = MergeTextMode::AcceptSource);
    };
}

#endif // JSONWRITER_H
