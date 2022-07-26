#ifndef LANGSCORE_DIVISI_H
#define LANGSCORE_DIVISI_H

#include <string>
#include <memory>
#include <filesystem>
#include "config.h"

namespace langscore
{
enum class OverwriteTextMode;

class divisi
{
public:
    divisi(std::filesystem::path appPath, std::filesystem::path configPath);
    ~divisi();

    bool analyze();
    bool write();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif // LANGSCORE_DIVISI_H
