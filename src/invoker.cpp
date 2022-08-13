#include "invoker.h"
#include "config.h"
#include "utility.hpp"

#include <process.h>
#include <fstream>
#include "../rvcnv/rvcnv_hash.cpp"
#include "md5.h"

using namespace langscore;

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
    auto tempPath = std::filesystem::path(config.langscoreAnalyzeDirectorty());
    tempPath.make_preferred();
    if(std::filesystem::exists(tempPath) == false){
        if(std::filesystem::create_directories(tempPath) == false){
            return Result(6);
        }
    }
    return exec({"-i", _projectPath.string(), "-o", tempPath.string()});
}

invoker::Result langscore::invoker::recompressVXAce(){
    return exec({"-i", _projectPath.string(), "-c"});
}

invoker::Result langscore::invoker::exec(std::vector<std::string> args)
{
    auto basePath = appPath.empty() ? "./" : appPath;
    if(currentProjectType == VXAce)
    {
        auto rvcnvPath = (basePath / "rvcnv.exe");

        rvcnvPath.make_preferred();
        if(std::filesystem::exists(rvcnvPath) == false){
            return Result(3);
        }

#if !defined(_DEBUG)
        {
            MD5 md5;
            std::ifstream rvcnv_bin(rvcnvPath, std::ios::binary | std::ios::in);
            rvcnv_bin.seekg(0, std::ios_base::end);
            auto size = rvcnv_bin.tellg();
            rvcnv_bin.seekg(0, std::ios_base::beg);
            std::vector<std::uint8_t> bin(size, 0);
            rvcnv_bin.read((char*)bin.data(), size);

            auto hash = md5((void*)bin.data(), bin.size());
            if(hash != rvcnv_hash){
                return Result(5);
            }
        }
#endif

        auto process = "\"" + rvcnvPath.string() + "\"" + " " + utility::join(args, std::string(" "));
        auto ret = system(process.c_str());
        if(ret != 0){
            return Result(4);
        }
    }
    else //if(currentProjectType == None)
    {
        return Result(1);
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
    case 5: return "error code 5 : Invalid ExecuteFile!!!!";
    case 6: return "error code 6 : Failed Create Output File.";
    case 255: return this->specMsg;
    }

    return "";
}
