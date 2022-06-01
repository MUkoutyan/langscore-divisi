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
    divisi(std::filesystem::path appPath);
    ~divisi();

    void prepareAnalyzeProject(std::filesystem::path projectPath);

    bool setProjectPath(std::filesystem::path projectPath);
    void exec();

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif // LANGSCORE_DIVISI_H
