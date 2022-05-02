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

void deserializer::setApplicationFolder(std::string path){
    appPath = std::move(path);
}

void deserializer::setProjectPath(ProjectType type, std::string path)
{
    this->currentProjectType = type;
    this->projectPath = std::move(path);
}

deserializer::Result deserializer::exec()
{
    //auto ruby = new QProcess(this);
    //connect(ruby, &QProcess::readyReadStandardOutput, this, [this, ruby](){
    //    auto mes = std::string(ruby->readAllStandardOutput());
    //    auto strList = mes.split("\r\n");
    //    for(auto& str : strList){
    //        qDebug() << str;
    //    }
    //});
    auto rubyPath = appPath+"/ruby/bin/ruby.exe";
    //ruby->setProgram(rubyPath);

    std::string scriptPath = "";
    if(currentProjectType == VXAce){
        scriptPath = appPath+"/rvdata2json/to_json2.rb";
        //ruby->setArguments({scriptPath,projectPath,outputPath()});
    }
    else //if(currentProjectType == None)
    {
        return Result(1);
    }

    //qDebug() << rubyPath << appPath+"/rvdata2json/to_json2.rb" << projectPath << appPath+"/tmp/";
    //ruby->start();

    char buffer[256] = {};
    auto process = utility::join(utility::stringlist{ rubyPath, scriptPath, projectPath, outputPath() }, std::string(" "));
    if(process_stdout)
    {
        FILE* fp = _popen(process.c_str(), "r");
        while(fgets(buffer, 256, fp) != NULL){
            process_stdout(buffer);
        }
    }
    //if(!ruby->waitForStarted(-1)){
    //    Result r(255);
    //    r.setSpecMsg(ruby->errorString());
    //    return r;
    //}
    //qDebug() << "Processing...";
    
    //if(!ruby->waitForFinished(-1)){
    //    Result r(255);
    //    r.setSpecMsg(ruby->errorString());
    //    return r;
    //}
    //qDebug() << "Success";

    return Result(0);
}

std::string deserializer::outputPath() const {
    return appPath+"/tmp";
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
