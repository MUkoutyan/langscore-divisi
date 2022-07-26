#include "divisi.h"
#include "utility.hpp"
#include "invoker.h"
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

    void createConverter(std::string projectPath)
    {
        this->converter = nullptr;
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
        auto type = invoker::ProjectType::None;
        for(auto& file : it){
            auto ext = file.path().extension();
            if(ext == ".rvproj2"){
                type = invoker::ProjectType::VXAce;
                this->converter = std::make_unique<divisi_vxace>();
                break;
            }
            else if(ext == ".rmmzproject")
            {
                if(hasHeader(file.path(), "RPGMZ")){
                    type = invoker::ProjectType::MZ;
                }
            }
            else if(ext == ".pgmproject")
            {
                if(hasHeader(file.path(), "RPGMV")){
                    type = invoker::ProjectType::MV;
                }
            }
        }
    }
};

divisi::divisi(fs::path appPath, std::filesystem::path configPath)
    : pImpl(std::make_unique<Impl>())
{
    config::attachConfigFile(std::move(configPath));
    pImpl->converter = nullptr;
    pImpl->appPath = std::move(appPath);
}

divisi::~divisi(){}

bool divisi::analyze()
{
    config config;
    auto projectPath = config.projectPath();
    if(projectPath.empty()){ return false; }
    pImpl->createConverter(projectPath);

    if(pImpl->converter){
        pImpl->converter->setAppPath(pImpl->appPath);
        pImpl->converter->setProjectPath(std::move(projectPath));
        pImpl->converter->analyze();
    }

    return true;
}

bool langscore::divisi::write()
{
    config config;
    auto projectPath = config.projectPath();
    if(projectPath.empty()){ return false; }
    pImpl->createConverter(projectPath);

    if(pImpl->converter){
        pImpl->converter->setAppPath(pImpl->appPath);
        pImpl->converter->setProjectPath(std::move(projectPath));
        pImpl->converter->write();
    }

    return true;

}

