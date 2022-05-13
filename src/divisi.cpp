#include "divisi.h"
#include "divisi.h"
#include "divisi.h"
#include "divisi.h"
#include "divisi.h"
#include "utility.hpp"
#include "project_deserializer.h"
#include "nlohmann/json.hpp"

#include "writer/rbscriptwriter.h"
#include "writer/csvwriter.h"
#include "reader/csvreader.h"
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
    deserializer deserializer;
    utility::filelist ignoreScriptPath;
    utility::u8stringlist supportLangs;

    OverwriteTextMode overwriteMode = OverwriteTextMode::LeaveOldNonBlank;

    std::vector<std::filesystem::path> dataFileList;
    std::vector<std::filesystem::path> scriptFileList;
    std::vector<std::filesystem::path> graphicFileList;

    void convertRvData();
    void convertRvScript();
    void convertGraphFileNameData();
    void copyData(std::filesystem::copy_options option);

    template<typename Writer, typename TsData>
    void writeTranslateText(fs::path path, TsData texts)
    {
        //最終的な出力先にCSVが存在するか
        Writer csvWrite(supportLangs, std::move(texts));
        const auto csvFileInProject = outputProjectDataPath(path.filename());
        if(fs::exists(csvFileInProject)){
            csvWrite.merge(csvFileInProject);
        }

        csvWrite.write(path);
    }

    fs::path outputProjectDataPath(fs::path fileName, fs::path dir = "Data");
};

divisi::divisi(std::filesystem::path appPath)
    : pImpl(std::make_unique<Impl>())
{
    pImpl->deserializer.setApplicationFolder(std::move(appPath));
    pImpl->supportLangs = {u8"ja",u8"en",u8"ch"};
    fs::path outPath = pImpl->deserializer.outputTmpPath();

    if(fs::exists(outPath)){
        fs::remove_all(outPath);
    }
}

divisi::~divisi(){}

void langscore::divisi::prepareAnalyzeProject(std::filesystem::path projectPath)
{
    this->setProjectPath(projectPath);
    pImpl->deserializer.exec();

    const auto deserializeOutPath = pImpl->deserializer.outputTmpPath();
    fs::recursive_directory_iterator dataItr(deserializeOutPath);
    for(auto& f : dataItr)
    {
        auto extension = f.path().extension();
        if(extension == ".json"){
            pImpl->dataFileList.emplace_back(f.path());
        }
        else if(extension == ".rb"){
            pImpl->scriptFileList.emplace_back(f.path());
        }
    }

    auto graphicsPath = projectPath / "Graphics";
    fs::recursive_directory_iterator graphItr(graphicsPath);
    for(auto& f : graphItr)
    {
        if(f.is_directory()){ continue; }

        auto relative_path = f.path().lexically_relative(projectPath);
        //パス区切り文字は\\ではなく/に統一(\\はRubyで読み取れない)
        pImpl->graphicFileList.emplace_back(relative_path.replace_extension(""));
    }

}

const utility::filelist& langscore::divisi::dataFileList() const {
    return pImpl->dataFileList;
}

const utility::filelist& langscore::divisi::scriptFileList() const {
    return pImpl->scriptFileList;
}

const utility::filelist& langscore::divisi::graphicFileList() const {
    return pImpl->graphicFileList;
}

void divisi::setIgnoreScriptPath(std::vector<fs::path> ignoreScriptPath){
    this->pImpl->ignoreScriptPath = std::move(ignoreScriptPath);
    auto result = std::find(this->pImpl->ignoreScriptPath.begin(), this->pImpl->ignoreScriptPath.end(), "union.rb");
    if(result == this->pImpl->ignoreScriptPath.end()){
        this->pImpl->ignoreScriptPath.emplace_back("unison.rb");
    }
}

void divisi::setOverwriteMode(OverwriteTextMode mode){
    pImpl->overwriteMode = mode;
}

void divisi::setSupportLanguages(std::vector<std::u8string> langs){
    this->pImpl->supportLangs = std::move(langs);
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

fs::path divisi::Impl::outputProjectDataPath(fs::path fileName, fs::path dir)
{
    const auto& projectPath = this->deserializer.projectPath();
    const auto projectType = this->deserializer.projectType();

    fs::path to = projectPath / dir;

    if(projectType == deserializer::ProjectType::VXAce){
        to /= fileName;
    }
    else if(projectType == deserializer::ProjectType::MV ||
            projectType == deserializer::ProjectType::MZ)
    {
        to /= fileName;
    }

    return to;
}

void divisi::exec(std::filesystem::copy_options option)
{
    pImpl->convertRvData();
    pImpl->convertRvScript();
    pImpl->convertGraphFileNameData();
    pImpl->copyData(option);
}

void divisi::Impl::convertRvData()
{
    std::cout << "convertRvData" << std::endl;

    auto writeRvCsv = [this](fs::path path)
    {
        std::ifstream loadFile(path);
        nlohmann::json json;
        loadFile >> json;

        auto csvFilePath = path.make_preferred().replace_extension(".csv");
        writeTranslateText<csvwriter>(csvFilePath, json);

#ifndef _DEBUG
        fs::remove(path);
#endif
    };

    const utility::stringlist ignoreFile = {"Animations.json", "Tilesets.json"};
    for(auto& path : dataFileList)
    {
        auto result = std::find_if(ignoreFile.cbegin(), ignoreFile.cend(), [f = path.filename()](const auto& x){
            return f == x;
        });
        if(result != ignoreFile.cend()){ continue; }

        writeRvCsv(path);
    }
    std::cout << "Finish." << std::endl;
}

void divisi::Impl::convertRvScript()
{
    std::cout << "convertRvScript" << std::endl;

    auto scriptList = scriptFileList;
    auto rm_result = std::remove_if(scriptList.begin(), scriptList.end(), [this](auto path){
        auto osPath = path.filename();
        auto result = std::find(this->ignoreScriptPath.cbegin(), this->ignoreScriptPath.cend(), osPath);
        return result != this->ignoreScriptPath.cend();
    });
    scriptList.erase(rm_result, scriptList.end());

    const auto deserializeOutPath = this->deserializer.outputTmpPath();
    auto outputPath = deserializeOutPath / "Scripts";

    //最終的な出力先にCSVが存在するか
    rbscriptwriter scriptWriter(this->supportLangs, scriptList);
    writeTranslateText<csvwriter>(outputPath, scriptWriter.curerntTexts());

    outputPath /= "../Scripts/unison_custom.rb";
    writeTranslateText<rbscriptwriter>(outputPath, scriptList);
    std::cout << "Finish." << std::endl;
}

void divisi::Impl::convertGraphFileNameData()
{
    std::cout << "convertGraphFileNameData" << std::endl;

    std::vector<TranslateText> transTextList;
    for(auto& f : graphicFileList){
        transTextList.emplace_back(f.generic_u8string(), supportLangs);
    }

    const auto& projectPath = deserializer.projectPath();
    writeTranslateText<csvwriter>(projectPath/"Data/Graphics", transTextList);
    std::cout << "Finish." << std::endl;
}

void divisi::Impl::copyData(std::filesystem::copy_options option)
{
    const auto deserializeOutPath = this->deserializer.outputTmpPath();
    fs::directory_iterator it(deserializeOutPath);
    for(auto& f : it)
    {
        auto srcPath = f.path().filename();
        if(f.is_directory() && srcPath == "Script"){
            continue;
        }
        if(srcPath.extension().string().find(csvwriter::extension) == std::string::npos){ continue; }

        fs::path to = outputProjectDataPath(srcPath);
        fs::copy(f, to, option);
    }

    fs::directory_iterator it2{deserializeOutPath/"Scripts"};
    for(auto& f : it2)
    {
        auto srcPath = f.path().filename();
        if(srcPath != "unison_custom.rb"){
            continue;
        }
        if(srcPath.extension().string().find(rbscriptwriter::extension) == std::string::npos){ continue; }

        fs::path to = outputProjectDataPath(srcPath, "Scripts");
        fs::copy(f, to, option);
        break;
    }
}



