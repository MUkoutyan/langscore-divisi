#include "deserializer.h"
#include "utility.hpp"

#include <process.h>

using namespace langscore;

namespace
{

}

deserializer::deserializer()
    : appPath("")
    , projectPath("")
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
    this->projectPath = std::move(path);
}

deserializer::Result deserializer::exec()
{
    auto rubyPath = appPath / "/ruby/bin/ruby.exe";

    std::filesystem::path scriptPath = "";
    if(currentProjectType == VXAce){
        scriptPath = appPath / "/rvdata2json/to_json2.rb";
    }
    else //if(currentProjectType == None)
    {
        return Result(1);
    }


    char buffer[256] = {};
    auto process = utility::join({ rubyPath.string(), scriptPath.string(), projectPath.string(), outputPath().string()}, std::string(" "));
    FILE* fp = NULL;
    if(process_stdout)
    {
        fp = _popen(process.c_str(), "r");
        while(fgets(buffer, 256, fp) != NULL){
            process_stdout(buffer);
        }
    }

    _pclose(fp);

    return Result(0);
}

std::filesystem::path deserializer::outputPath() const {
    return appPath/"tmp";
}


std::string deserializer::Result::toStr() const
{
    switch(code)
    {
    case 0: return "";
    case 1: return "Unsupport Project Type";
    case 255: return this->specMsg;
    }

    return "";
}
