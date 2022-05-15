#include "divisi.h"
#include "utility.hpp"
#include "project_deserializer.h"
#include "nlohmann/json.hpp"

#include "writer/rbscriptwriter.h"
#include "writer/csvwriter.h"
#include "reader/csvreader.h"
#include "platform/divisi_vxace.h"
#include <iostream>
#include <fstream>
#include <semaphore>
#include <chrono>
#include <thread>
#include <future>
#include <mutex>

using namespace langscore;
namespace fs = std::filesystem;

class divisi::Impl
{
public:
    utility::filelist ignoreScriptPath;
    utility::u8stringlist supportLangs;
    fs::path appPath;

    std::unique_ptr<platform_base> converter;
};

divisi::divisi(fs::path appPath, std::vector<std::u8string> langs)
    : pImpl(std::make_unique<Impl>())
{
    pImpl->converter = nullptr;
    pImpl->appPath = std::move(appPath);
    if(langs.empty()){
        pImpl->supportLangs = {u8"ja",u8"en",u8"zh-sc"};
    }
    else{
        pImpl->supportLangs = std::move(langs);
    }
}

divisi::~divisi(){}

void langscore::divisi::prepareAnalyzeProject(std::filesystem::path projectPath)
{
    if(this->setProjectPath(projectPath)){
        pImpl->converter->prepareAnalyzeProject();
    }

}

void divisi::setIgnoreScriptPath(std::vector<fs::path> ignoreScriptPath){
    this->pImpl->ignoreScriptPath = std::move(ignoreScriptPath);
}


void divisi::setSupportLanguages(std::vector<std::u8string> langs){
    this->pImpl->supportLangs = std::move(langs);
}

bool divisi::setProjectPath(std::filesystem::path projectPath)
{
    const auto hasHeader = [](fs::path path, std::string_view headerText)
    {
        std::ifstream proj(path);
        if(proj.is_open()){
            std::string header(5, ' ');
            proj.read(&header[0], 5);
            return header == headerText;
        }
        return false;
    };
    fs::directory_iterator it(projectPath);
    auto type = deserializer::ProjectType::None;
    for(auto& file : it){
        auto ext = file.path().extension();
        if(ext == ".rvproj2"){
            type = deserializer::ProjectType::VXAce;
            pImpl->converter = std::make_unique<divisi_vxace>(this->pImpl->supportLangs);
            break;
        }
        else if(ext == ".rmmzproject")
        {
            if(hasHeader(file.path(), "RPGMZ")){
                type = deserializer::ProjectType::MZ;
            }
        }
        else if(ext == ".pgmproject")
{
            if(hasHeader(file.path(), "RPGMV")){
                type = deserializer::ProjectType::MV;
            }
        }
    }

    if(pImpl->converter){
        pImpl->converter->setAppPath(pImpl->appPath);
        pImpl->converter->setIgnoreScriptPath(pImpl->ignoreScriptPath);
        pImpl->converter->setProjectPath(std::move(projectPath));
        return true;
    }
    return false;
}

void divisi::exec(std::filesystem::copy_options option)
{
    if(pImpl->converter){
        pImpl->converter->convert(option);
    }
}


