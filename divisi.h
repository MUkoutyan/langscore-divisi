#ifndef LANGSCORE_DIVISI_H
#define LANGSCORE_DIVISI_H

#include <string>
#include <memory>
#include <filesystem>

namespace langscore
{

class divisi
{
public:
    divisi(std::string appPath);
    ~divisi();

    void setProjectPath(std::string projectPath);
    void exec();

    void setIgnoreScriptPath(std::vector<std::filesystem::path> ignoreScriptPath);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
    std::vector<std::filesystem::path> ignoreScriptPath;
};

}

#endif // LANGSCORE_DIVISI_H
