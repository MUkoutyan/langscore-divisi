#ifndef LANGSCORE_DIVISI_H
#define LANGSCORE_DIVISI_H

#include <string>
#include <memory>
#include <filesystem>
#include "errorstatus.hpp"
#include "config.h"

namespace langscore
{
enum class MergeTextMode : char;

class divisi
{
public:
    divisi(std::filesystem::path appPath, std::filesystem::path configPath);
    ~divisi();

    ErrorStatus analyze();
    ErrorStatus reanalysis();
    ErrorStatus exportCSV();
    ErrorStatus updatePlugin();
    ErrorStatus validate();
    ErrorStatus packing();

    ErrorStatus createConfig(std::filesystem::path gameProjectPath);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif // LANGSCORE_DIVISI_H
