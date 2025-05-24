#pragma once
#include <filesystem>

namespace langscore
{
    class convert_patch_csv
    {
    public:
        convert_patch_csv(std::filesystem::path directory_path);
    };

    class convert_bound_csv
    {
    public:
        convert_bound_csv(std::filesystem::path directory_path);
    };
}