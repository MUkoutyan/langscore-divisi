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
    fs::path appPath;

    std::unique_ptr<platform_base> converter;
};

divisi::divisi(fs::path appPath)
    : pImpl(std::make_unique<Impl>())
{
    pImpl->converter = nullptr;
    pImpl->appPath = std::move(appPath);
}

divisi::~divisi(){}

void divisi::prepareAnalyzeProject(std::filesystem::path projectPath)
{
    if(this->setProjectPath(projectPath)){
        pImpl->converter->prepareAnalyzeProject();
    }

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
            pImpl->converter = std::make_unique<divisi_vxace>();
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
        pImpl->converter->setProjectPath(std::move(projectPath));
        return true;
    }
    return false;
}

void divisi::exec()
{
    if(pImpl->converter){
        pImpl->converter->convert();
    }
}


