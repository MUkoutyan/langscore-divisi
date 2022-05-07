#include "divisi.h"
#include "utility.hpp"
#include "deserializer.h"
#include "nlohmann/json.hpp"

#include "writer/csvwriter.h"
#include <iostream>
#include <fstream>
#include <regex>

using namespace langscore;
namespace fs = std::filesystem;

class divisi::Impl
{
public:
    deserializer deserializer;
    std::vector<std::filesystem::path> ignoreScriptPath;

    void convertRvData();
    void convertRvScript();
};

divisi::divisi(std::filesystem::path appPath)
    : pImpl(std::make_unique<Impl>())
{
    pImpl->deserializer.setApplicationFolder(std::move(appPath));
    fs::path outPath = pImpl->deserializer.outputTmpPath();

    if(fs::exists(outPath)){
        fs::remove_all(outPath);
    }
}

divisi::~divisi(){}

void divisi::setIgnoreScriptPath(std::vector<fs::path> ignoreScriptPath){
    this->pImpl->ignoreScriptPath = std::move(ignoreScriptPath);
}

void divisi::setProjectPath(std::filesystem::path projectPath)
{
    fs::directory_iterator it(projectPath);
    auto type = deserializer::ProjectType::None;
    for(auto& file : it){
        if(file.path().extension() == ".rvproj2"){
            type = deserializer::ProjectType::VXAce;
            break;
        }
    }
    pImpl->deserializer.setProjectPath(type, std::move(projectPath));
}

void divisi::exec(std::filesystem::copy_options option)
{
    pImpl->deserializer.exec();

    pImpl->convertRvData();
    pImpl->convertRvScript();

    const auto deserializeOutPath = this->pImpl->deserializer.outputTmpPath();
    const auto& projectPath = this->pImpl->deserializer.projectPath();
    fs::directory_iterator it(deserializeOutPath);

    const auto projectType = this->pImpl->deserializer.projectType();
    for(auto& f : it)
    {
        auto srcPath = f.path();
        if(f.is_directory() && srcPath.filename() == "Script"){
            continue;
        }
        fs::path to = projectPath / "Data";

        if(projectType == deserializer::ProjectType::VXAce){
            to /= srcPath.filename();
        }
        else if(projectType == deserializer::ProjectType::MV || 
                projectType == deserializer::ProjectType::MZ)
        {
            to /= srcPath.filename();
        }

        fs::copy(f, to, option);
    }
    
}

void divisi::Impl::convertRvData()
{
    const auto deserializeOutPath = this->deserializer.outputTmpPath();
    std::vector<fs::path> jsonList;
    fs::recursive_directory_iterator it(deserializeOutPath);
    for(auto& f : it){
        if(f.path().extension() == ".json"){
            jsonList.emplace_back(f.path());
        }
    }

    const utility::stringlist ignoreFile = {"Animations.json", "Tilesets.json"};
    for(auto& path : jsonList)
    {
        auto result = std::find_if(ignoreFile.cbegin(), ignoreFile.cend(), [f = path.filename()](const auto& x){
            return f == x;
        });
        if(result != ignoreFile.cend()){ continue; }

        std::ifstream loadFile(path);
        nlohmann::json json;
        loadFile >> json;

        csvwriter writer({"ja", "en", "ch"}, json);

        auto csvFilePath = path.make_preferred().replace_extension(".csv");
        writer.write(csvFilePath.u8string());

#ifndef _DEBUG
        fs::remove(path);
#endif
    }
}

void divisi::Impl::convertRvScript()
{
    const auto deserializeOutPath = this->deserializer.outputTmpPath();

    auto scriptLocalize = deserializeOutPath / "Scripts.csv";
    std::ofstream csvWrite(scriptLocalize);

    std::vector<fs::path> scriptList;
    fs::recursive_directory_iterator scriptIt(deserializeOutPath);
    for(auto& f : scriptIt){
        if(f.path().extension() == ".rb"){
            scriptList.emplace_back(f.path());
        }
    }

    std::regex parseStr(R"(".+?")");
    for(auto& path : scriptList)
    {
        auto osPath = path.filename();
        auto result = std::find(this->ignoreScriptPath.cbegin(), this->ignoreScriptPath.cend(), osPath);
        if(result != this->ignoreScriptPath.cend()){ continue; }

        std::ifstream loadFile(path);

        size_t lineCount = 0;
        bool rangeComment = false;
        while(loadFile.eof() == false)
        {
            lineCount++;
            std::string line;
            std::getline(loadFile, line);

            if(line == "=begin"){
                rangeComment = true;
                continue;
            }
            else if(rangeComment){
                if(line == "=end"){
                    rangeComment = false;
                }
                else{ continue; }
            }

            if(line.empty()){ continue; }
            if(line[0] == '#'){ continue; }

            std::smatch matchList;
            if(std::regex_search(line, matchList, parseStr) == false){
                continue;
            }

            for(auto& m : matchList)
            {
                line = m.str();
                while(true){
                    auto p = line.find('\"');
                    if(p == std::string::npos){ break; }
                    line.erase(p, 1);
                }

                auto fname = path.filename().u8string();
                std::string fileName(fname.begin(), fname.end());

                csvWrite << line << ",,,," << fileName << " Line : " << lineCount << '\n';
            }
        }
    }
}
