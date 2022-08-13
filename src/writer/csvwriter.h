#ifndef CSVWRITER_H
#define CSVWRITER_H

#include "writerbase.h"
#include "utility.hpp"

namespace langscore
{
    class csvwriter: public writerbase
    {
    public:
        using writerbase::writerbase;
        constexpr static const char* extension = "csv";

        bool merge(std::filesystem::path filePath) override;
        bool write(std::filesystem::path path, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld) override;
        static bool writePlain(std::filesystem::path path, std::vector<utility::u8stringlist> text, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld);

#ifdef ENABLE_TEST
        friend class Langscore_Test_WriterBase;
#endif
    };
}

#endif // CSVWRITER_H
