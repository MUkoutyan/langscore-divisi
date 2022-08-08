#include "invoker.h"
#include "config.h"
#include "utility.hpp"

#include <process.h>
#include <iostream>
#include <Windows.h>

using namespace langscore;

namespace
{
    std::wstring Utf8ToWStr(std::u8string str)
    {
        auto s = utility::cnvStr<std::string>(str);
        auto size = MultiByteToWideChar(CP_UTF8, 0, s.data(), -1, nullptr, 0);
        std::wstring wstr(size, 0x0);
        MultiByteToWideChar(CP_UTF8, 0, s.c_str(), s.length(), wstr.data(), wstr.size());
        return wstr;
    }
}

invoker::invoker()
    : appPath("")
    , _projectPath("")
    , currentProjectType(None)
{
}

invoker::~invoker(){
}

void invoker::setApplicationFolder(std::filesystem::path path){
    appPath = std::move(path);
    appPath = appPath.parent_path();
}

void invoker::setProjectPath(ProjectType type, std::filesystem::path path)
{
    this->currentProjectType = type;
    this->_projectPath = std::move(path);
}


invoker::Result invoker::analyze(){
    config config;
    auto tempPath = std::filesystem::path(config.tempDirectorty());
    return exec({"-i", _projectPath.string(), "-o", tempPath.string()});
}

invoker::Result langscore::invoker::recompressVXAce(){
    return exec({"-i", _projectPath.string(), "-c"});
}

invoker::Result langscore::invoker::exec(std::vector<std::string> args)
{
    std::filesystem::path rvcnvPath = "";
    auto basePath = appPath.empty() ? "./" : appPath;
    if(currentProjectType == VXAce){
        rvcnvPath = (basePath / "rvcnv.exe");
    }
    else //if(currentProjectType == None)
    {
        return Result(1);
    }

    rvcnvPath.make_preferred();

    if(std::filesystem::exists(rvcnvPath) == false){
        return Result(3);
    }
    config config;
    auto outPath = config.tempDirectorty();
    if(std::filesystem::exists(outPath) == false){
        std::filesystem::create_directories(outPath);
    }

    auto process = rvcnvPath.string() + " " + utility::join(args, std::string(" "));
    auto ret = system(process.c_str());
    if(ret != 0){
        return Result(4);
    }


    return Result(0);
}

invoker::ProjectType invoker::projectType() const noexcept {
    return this->currentProjectType;
}

std::string invoker::Result::toStr() const
{
    switch(code)
    {
    case 0: return "";
    case 1: return "error code 1 : Unsupport Project Type";
    case 2: return "error code 2 : Not Found Execute file.";
    case 3: return "error code 3 : Not Found Convert file.";
    case 4: return "error code 4 : Failed to convert.";
    case 255: return this->specMsg;
    }

    return "";
}
