#ifndef LANGSCORE_INVOKER_H
#define LANGSCORE_INVOKER_H

#include <string>
#include <functional>
#include <filesystem>
#include "errorstatus.hpp"

namespace langscore
{

class invoker
{
public:

    enum ProjectType {
        None,
        VXAce,
        MV,
        MZ,
    };

    invoker();
    ~invoker();

    void setApplicationFolder(std::filesystem::path path);
    void setProjectPath(ProjectType type, std::filesystem::path path);
    
    ProjectType projectType() const noexcept;

    ErrorStatus analyze();
    ErrorStatus update();
    ErrorStatus recompressVXAce();
    ErrorStatus packingVXAce();

private:
    std::filesystem::path appPath;
    std::filesystem::path _projectPath;
    std::vector<std::u8string> langs;
    ProjectType currentProjectType;

    ErrorStatus exec(std::vector<std::string> args);
};


}

#endif // LANGSCORE_DESERIALIZER_H
