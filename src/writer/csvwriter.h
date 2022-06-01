#ifndef CSVWRITER_H
#define CSVWRITER_H

#include "writerbase.h"

namespace langscore
{
    class csvwriter: public writerbase
    {
    public:
        using writerbase::writerbase;
        constexpr static const char* extension = "csv";

        bool merge(std::filesystem::path filePath) override;
        bool write(std::filesystem::path path, OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOld) override;

#ifdef ENABLE_TEST
        friend class Langscore_Test_WriterBase;
#endif
    };
}

#endif // CSVWRITER_H
