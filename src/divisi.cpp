#include "divisi.h"
#include "divisi.h"
#include "divisi.h"
#include "divisi.h"
#include "utility.hpp"
#include "invoker.h"
#include "nlohmann/json.hpp"

#include "writer/rbscriptwriter.h"
#include "writer/csvwriter.h"
#include "reader/csvreader.h"
#include "platform/divisi_vxace.h"
#include "platform/divisi_mvmz.h"
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

    void createConverter(fs::path gameProjectPath)
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

        fs::directory_iterator it(gameProjectPath);
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
                    this->converter = std::make_unique<divisi_mvmz>();
                    break;
                }
            }
            else if(ext == ".rpgproject")
            {
                if(hasHeader(file.path(), "RPGMV")){
                    type = invoker::ProjectType::MV;
                    this->converter = std::make_unique<divisi_mvmz>();
                    break;
                }
            }
        }
    }

    ErrorStatus setupConverter()
    {
        config config;
        auto gameProjectPath = config.gameProjectPath();
        if(gameProjectPath.empty()){
            return ErrorStatus(ErrorStatus::Module::DIVISI, 1);
        }
        this->createConverter(gameProjectPath);

        if(this->converter == nullptr){
            return ErrorStatus(ErrorStatus::Module::DIVISI, 2);
        }

        this->converter->setAppPath(this->appPath);
        this->converter->setProjectPath(std::move(gameProjectPath));

        return Status_Success;
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

ErrorStatus divisi::analyze()
{
    auto result = pImpl->setupConverter();
    if(result.invalid()){ return result; }
    return pImpl->converter->analyze();
}

ErrorStatus langscore::divisi::update()
{
    auto result = pImpl->setupConverter();
    if(result.invalid()){ return result; }
    return pImpl->converter->update();
}

ErrorStatus divisi::write()
{
    auto result = pImpl->setupConverter();
    if(result.invalid()){ return result; }
    return pImpl->converter->write();
}

ErrorStatus langscore::divisi::validate()
{
    auto result = pImpl->setupConverter();
    if(result.invalid()){ return result; }
    return pImpl->converter->validate();
}

ErrorStatus divisi::packing()
{
    auto result = pImpl->setupConverter();
    if(result.invalid()){ return result; }
    return pImpl->converter->packing();
}

