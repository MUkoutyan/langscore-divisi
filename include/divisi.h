#ifndef LANGSCORE_DIVISI_H
#define LANGSCORE_DIVISI_H

#include <string>
#include <memory>
#include <filesystem>

namespace langscore
{
enum class OverwriteTextMode;

class divisi
{
public:
    divisi(std::filesystem::path appPath, std::vector<std::u8string> langs);
    ~divisi();

    void prepareAnalyzeProject(std::filesystem::path projectPath);


    void setSupportLanguages(std::vector<std::u8string> langs);
    void setIgnoreScriptPath(std::vector<std::filesystem::path> ignoreScriptPath);
    bool setProjectPath(std::filesystem::path projectPath);
    void exec(std::filesystem::copy_options outputFileOption = std::filesystem::copy_options::none);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif // LANGSCORE_DIVISI_H
