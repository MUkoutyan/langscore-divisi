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
    divisi(std::filesystem::path appPath);
    ~divisi();

    void setProjectPath(std::filesystem::path projectPath);
    void exec(std::filesystem::copy_options outputFileOption = std::filesystem::copy_options::none);

    void setIgnoreScriptPath(std::vector<std::filesystem::path> ignoreScriptPath);


private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif // LANGSCORE_DIVISI_H
