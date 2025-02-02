#ifndef LANGSCORE_INVOKER_H
#define LANGSCORE_INVOKER_H

#include <string>
#include <functional>
#include <filesystem>
#include "errorstatus.hpp"
#include "config.h"

namespace langscore
{

class invoker
{
public:


    invoker(config::ProjectType projectType);
    ~invoker();

    void setApplicationFolder(std::filesystem::path path);
    void setProjectPath(std::filesystem::path path);
    
    ErrorStatus analyze(std::filesystem::path analyzeDir);
    ErrorStatus reanalysis(std::filesystem::path updateDir);
    ErrorStatus recompressVXAce();
    ErrorStatus packingVXAce(std::filesystem::path destGameTranslateFolder, std::filesystem::path packingDir);

private:
    config::ProjectType projectType;
    std::filesystem::path appPath;
    std::filesystem::path _projectPath;
    std::vector<std::u8string> langs;

    ErrorStatus exec(std::vector<std::string> args);
};


}

#endif // LANGSCORE_DESERIALIZER_H
