#include "deserializer.h"

#include <QDebug>
#include <QProcess>

using namespace langscore;

deserializer::deserializer()
    : QObject()
    , appPath("")
    , projectPath("")
    , currentProjectType(None)
{
}

deserializer::~deserializer(){
}

void deserializer::setApplicationFolder(QString path){
    appPath = std::move(path);
}

void deserializer::setProjectPath(ProjectType type, QString path)
{
    this->currentProjectType = type;
    this->projectPath = std::move(path);
}

deserializer::Result deserializer::exec()
{
    auto ruby = new QProcess(this);
    connect(ruby, &QProcess::readyReadStandardOutput, this, [this, ruby](){
        auto mes = QString(ruby->readAllStandardOutput());
        auto strList = mes.split("\r\n");
        for(auto& str : strList){
            qDebug() << str;
        }
        emit this->recvStdOut(mes);
    });
    auto rubyPath = appPath+"/ruby/bin/ruby.exe";
    ruby->setProgram(rubyPath);

    if(currentProjectType == VXAce){
        auto scriptPath = appPath+"/rvdata2json/to_json2.rb";
        ruby->setArguments({scriptPath,projectPath,outputPath()});
    }
    else //if(currentProjectType == None)
    {
        return Result(1);
    }

    qDebug() << rubyPath << appPath+"/rvdata2json/to_json2.rb" << projectPath << appPath+"/tmp/";
    ruby->start();
    if(!ruby->waitForStarted(-1)){
        Result r(255);
        r.setSpecMsg(ruby->errorString());
        return r;
    }
    qDebug() << "Processing...";
    
    if(!ruby->waitForFinished(-1)){
        Result r(255);
        r.setSpecMsg(ruby->errorString());
        return r;
    }
    qDebug() << "Success";

    return Result(0);
}

QString deserializer::outputPath() const {
    return appPath+"/tmp";
}


QString deserializer::Result::toStr() const
{
    switch(code)
    {
    case 0: return "";
    case 1: return "Unsupport Project Type";
    case 255: return this->specMsg;
    }

    return "";
}
