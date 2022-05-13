#include "project_deserializer.h"
#include "utility.hpp"

#include <process.h>

using namespace langscore;

namespace
{

}

deserializer::deserializer()
    : appPath("")
    , _projectPath("")
    , currentProjectType(None)
{
}

deserializer::~deserializer(){
}

void deserializer::setApplicationFolder(std::filesystem::path path){
    appPath = std::move(path);
    appPath = appPath.parent_path();
}

void deserializer::setProjectPath(ProjectType type, std::filesystem::path path)
{
    this->currentProjectType = type;
    this->_projectPath = std::move(path);
}


deserializer::Result deserializer::exec()
{
    auto rubyPath = (appPath / "ruby/bin/ruby.exe");
    rubyPath.make_preferred();

    std::filesystem::path scriptPath = "";
    if(currentProjectType == VXAce){
        scriptPath = appPath / "rvdata2json/to_json2.rb";
    }
    else //if(currentProjectType == None)
    {
        return Result(1);
    }

    scriptPath.make_preferred();


    if(std::filesystem::exists(rubyPath) == false){
        return Result(2);
    }
    if(std::filesystem::exists(scriptPath) == false){
        return Result(3);
    }
    auto outPath = outputTmpPath();
    if(std::filesystem::exists(outPath) == false){
        std::filesystem::create_directories(outPath);
    }

    char buffer[256] = {};
    auto process = utility::join({ rubyPath.generic_string(), scriptPath.string(), _projectPath.string(), outputTmpPath().string()}, std::string(" "));
    FILE* fp = NULL;
    if(process_stdout)
    {
        if(fp = _popen(process.c_str(), "r"))
        {
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
        if(ret != -1){
            return Result(4);
        }
        
    }


    return Result(0);
}

std::filesystem::path deserializer::outputTmpPath() const {
    return appPath/"tmp/Data/";
}

const std::filesystem::path& deserializer::projectPath() const {
    return this->_projectPath;
}

deserializer::ProjectType deserializer::projectType() const noexcept {
    return this->currentProjectType;
}

std::string deserializer::Result::toStr() const
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
