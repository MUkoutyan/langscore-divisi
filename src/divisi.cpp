#include "divisi.h"
#include "divisi.h"
#include "utility.hpp"
#include "invoker.h"
#include "config_writer.h"
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
#include <ranges>
#include <mutex>

using namespace langscore;
namespace fs = std::filesystem;

class divisi::Impl
{
public:
    fs::path appPath;
    fs::path configPath;

    std::unique_ptr<platform_base> converter;

    void createConverter()
    {
        this->converter = nullptr;

        config config;
        switch(config.projectType())
        {
        case config::ProjectType::VXAce:
            this->converter = std::make_unique<divisi_vxace>();
            break;
        case config::ProjectType::MZ:
            this->converter = std::make_unique<divisi_mvmz>();
            break;
        case config::ProjectType::MV:
            this->converter = std::make_unique<divisi_mvmz>();
            break;
        }
    }

    ErrorStatus setupConverter()
    {
        config config;
        auto gameProjectPath = config.gameProjectPath();
        if(gameProjectPath.empty()){
            return ErrorStatus(ErrorStatus::Module::DIVISI, 1);
        }
        this->createConverter();

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
    pImpl->converter = nullptr;
    pImpl->appPath = std::move(appPath);
    pImpl->configPath = configPath;
    config::attachConfigFile(std::move(configPath));
}

divisi::~divisi(){}

ErrorStatus divisi::analyze()
{
    auto result = pImpl->setupConverter();
    if(result.invalid()){ return result; }
    return pImpl->converter->analyze();
}

ErrorStatus langscore::divisi::reanalysis()
{
    auto result = pImpl->setupConverter();
    if(result.invalid()){ return result; }
    return pImpl->converter->reanalysis();
}

ErrorStatus divisi::exportCSV()
{
    auto result = pImpl->setupConverter();
    if(result.invalid()){ return result; }
    return pImpl->converter->exportCSV();
}

ErrorStatus langscore::divisi::updatePlugin()
{
    auto result = pImpl->setupConverter();
    if(result.invalid()) { return result; }
    return pImpl->converter->updatePlugin();
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

ErrorStatus langscore::divisi::createConfig(std::filesystem::path gameProjectPath)
{
    config_writer writer(gameProjectPath);

    std::cout << "create config" << std::endl;
    return writer.write() ? ErrorStatus::Module::None : ErrorStatus::Module::None;
}
