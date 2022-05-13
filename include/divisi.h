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
    divisi(std::filesystem::path appPath);
    ~divisi();

    void prepareAnalyzeProject(std::filesystem::path projectPath);

    const std::vector<std::filesystem::path>& dataFileList() const;
    const std::vector<std::filesystem::path>& scriptFileList() const;
    const std::vector<std::filesystem::path>& graphicFileList() const;

    void setSupportLanguages(std::vector<std::u8string> langs);
    void setIgnoreScriptPath(std::vector<std::filesystem::path> ignoreScriptPath);
    void setProjectPath(std::filesystem::path projectPath);
    void exec(std::filesystem::copy_options outputFileOption = std::filesystem::copy_options::none);

    void setOverwriteMode(OverwriteTextMode mode);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

}

#endif // LANGSCORE_DIVISI_H
