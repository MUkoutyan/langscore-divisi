#include "invoker.h"
#include "config.h"
#include "utility.hpp"

#include <process.h>

using namespace langscore;

namespace
{

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
    return exec({"-i", _projectPath.string(), "-o", outputTmpPath().string()});
}

invoker::Result langscore::invoker::recompressVXAce(){
    return exec({"-i", _projectPath.string(), "-c"});
}

invoker::Result langscore::invoker::exec(std::vector<std::string> args)
{
    std::filesystem::path rvcnvPath = "";
    if(currentProjectType == VXAce){
        rvcnvPath = (appPath / "rvcnv.exe");
    }
    else //if(currentProjectType == None)
    {
        return Result(1);
    }

    rvcnvPath.make_preferred();

    if(std::filesystem::exists(rvcnvPath) == false){
        return Result(3);
    }
    auto outPath = outputTmpPath();
    if(std::filesystem::exists(outPath) == false){
        std::filesystem::create_directories(outPath);
    }

    auto process = rvcnvPath.string() + " " + utility::join(args, std::string(" "));
    FILE* fp = NULL;
    if(process_stdout)
    {
        if(fp = _popen(process.c_str(), "r"))
        {
            char buffer[256] = {};
            while(fgets(buffer, 256, fp) != NULL){
                process_stdout(buffer);
            }
            _pclose(fp);
        }
        else{
            return Result(4);
        }
    }
    else
    {
        auto ret = system(process.c_str());
        if(ret != 0){
            return Result(4);
        }
    }


    return Result(0);
}

std::filesystem::path invoker::outputTmpPath() const 
{
    try {
        config config;
        return config.tempDirectorty();
    }
    catch(...){
    }
    return appPath / "tmp/Translate/";
}

const std::filesystem::path& invoker::projectPath() const {
    return this->_projectPath;
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
