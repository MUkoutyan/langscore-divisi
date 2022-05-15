#include "divisi_vxace.h"

#include "../writer/rbscriptwriter.h"
#include "../writer/csvwriter.h"
#include "../reader/csvreader.h"

#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

using namespace langscore;
using namespace std::literals::string_literals;
namespace fs = std::filesystem;

constexpr char Script_File_Name[] = "unison.rb";

divisi_vxace::~divisi_vxace(){}

void divisi_vxace::setProjectPath(std::filesystem::path path)
{
    this->deserializer.setProjectPath(deserializer::ProjectType::VXAce, std::move(path));

    fs::path outPath = this->deserializer.outputTmpPath();
    if(fs::exists(outPath)){
        fs::remove_all(outPath);
    }
}

std::filesystem::path langscore::divisi_vxace::outputProjectDataPath(std::filesystem::path fileName, std::filesystem::path dir)
{
    const auto& projectPath = this->deserializer.projectPath();

    fs::path to = projectPath / dir;
    return to /= fileName;
}

void divisi_vxace::prepareAnalyzeProject()
{
    this->deserializer.exec();

    const auto deserializeOutPath = this->deserializer.outputTmpPath();
    fs::recursive_directory_iterator dataItr(deserializeOutPath);
    for(auto& f : dataItr)
    {
        auto extension = f.path().extension();
        if(extension == ".json"){
            this->dataFileList.emplace_back(f.path());
        }
        else if(extension == ".rb"){
            this->scriptFileList.emplace_back(f.path());
        }
    }

    auto projectPath = this->deserializer.projectPath();
    auto graphicsPath = projectPath / "Graphics";
    fs::recursive_directory_iterator graphItr(graphicsPath);
    for(auto& f : graphItr)
    {
        if(f.is_directory()){ continue; }

        auto relative_path = f.path().lexically_relative(projectPath);
        //パス区切り文字は\\ではなく/に統一(\\はRubyで読み取れない)
        this->graphicFileList.emplace_back(relative_path.replace_extension(""));
    }

}

void langscore::divisi_vxace::convert(std::filesystem::copy_options option)
{    
    this->convertRvData();
    this->convertRvScript();
    this->convertGraphFileNameData();
    this->copyData(option);
}

void langscore::divisi_vxace::setIgnoreScriptPath(utility::filelist ignoreScriptPath)
{
    this->ignoreScriptPath = std::move(ignoreScriptPath);
    const auto AddIgnore = [&](fs::path file){
        auto result = std::find(this->ignoreScriptPath.begin(), this->ignoreScriptPath.end(), file);
        if(result == this->ignoreScriptPath.end()){
            this->ignoreScriptPath.emplace_back(file);
        }
    };
    AddIgnore(Script_File_Name);
    AddIgnore("unison_custom.rb");
}

void divisi_vxace::convertRvData()
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


void divisi_vxace::convertRvScript()
{
    std::cout << "convertRvScript" << std::endl;

    auto scriptList = scriptFileList;
    auto rm_result = std::remove_if(scriptList.begin(), scriptList.end(), [this](auto path){
        auto osPath = path.filename();
        auto result = std::find(this->ignoreScriptPath.cbegin(), this->ignoreScriptPath.cend(), osPath);
        return result != this->ignoreScriptPath.cend();
    });
    scriptList.erase(rm_result, scriptList.end());

    //Rubyスクリプトを予め解析してテキストを生成しておく。
    rbscriptwriter scriptWriter(this->supportLangs, scriptList);
    auto& scriptTrans = scriptWriter.getScriptTexts();
    auto& transTexts = scriptWriter.curerntTexts();

    //デフォルトスクリプトのVocab.rb内の文字列は予め対応
    auto resourceFolder = this->appPath.parent_path() / "resource";
    csvreader reader;
    auto vocabs = reader.parse(resourceFolder/"vocab.csv");
    for(auto& pathPair : scriptTrans)
    {
        if(pathPair.first.filename() != "Vocab.rb"){ continue; }

        const auto searchFunc = [&](const auto& x){
            return x.original.find(u8"Vocab.rb") != std::u8string::npos;
        };
        auto begin = std::find_if(transTexts.begin(), transTexts.end(), searchFunc);
        auto end = std::find_if(transTexts.rbegin(), transTexts.rend(), searchFunc).base();

        std::for_each(begin, end, [&](auto& texts)
        {
            auto result = std::find_if(vocabs.begin(), vocabs.end(), [&texts](const auto& x){
                return x.original == texts.memo;
            });
            if(result == vocabs.end()){ return; }
            for(const auto& key : this->supportLangs){
                if(result->translates.find(key) != result->translates.end()){
                    texts.translates[key] = result->translates[key];
                }
                else if(key == u8"ja"){
                    texts.translates[key] = result->original;
                }
            }
        });
        break;
    }
    //=================================

    const auto deserializeOutPath = this->deserializer.outputTmpPath();
    auto outputPath = deserializeOutPath / "Scripts";
    writeTranslateText<csvwriter>(outputPath, transTexts);
    
    writeTranslateText<rbscriptwriter>(outputPath / "unison_custom.rb", scriptList);

    auto outputScriptFilePath = outputPath / Script_File_Name;
    fs::copy(resourceFolder / Script_File_Name, outputScriptFilePath, std::filesystem::copy_options::overwrite_existing);
    std::ifstream inScriptFile(outputScriptFilePath);

    std::string _linTmp;
    utility::u8stringlist result;
    while(std::getline(inScriptFile, _linTmp))
    {
        std::u8string _line(_linTmp.begin(), _linTmp.end());
        if(_line.find(u8"%{SUPPORT_LANGUAGE}%") != std::string::npos){
            auto list = this->supportLangs;
            for(auto& t : list){ t = u8"\"" + t + u8"\""; }
            auto langs = utility::join(list, u8","s);
            _line = u8"\tSUPPORT_LANGUAGE = [" + langs + u8"]";
        }
        else  if(_line.find(u8"%{DEFAULT_LANGUAGE}%") != std::string::npos){
            _line = u8"\tDEFAULT_LANGUAGE = \"" + this->defaultLanguage + u8"\"";
        }
        result.emplace_back(std::move(_line));
    }
    inScriptFile.close();

    std::ofstream outScriptFile(outputScriptFilePath, std::ios_base::trunc);
    for(const auto& l : result){
        outScriptFile << utility::toString(l) << "\n";
    }

    std::cout << "Finish." << std::endl;
}

void divisi_vxace::convertGraphFileNameData()
{
    std::cout << "convertGraphFileNameData" << std::endl;

    std::vector<TranslateText> transTextList;
    for(auto& f : graphicFileList){
        transTextList.emplace_back(f.generic_u8string(), supportLangs);
    }

    const auto& projectPath = deserializer.projectPath();
    writeTranslateText<csvwriter>(projectPath / "Data/Graphics", transTextList);
    std::cout << "Finish." << std::endl;
}

void divisi_vxace::copyData(std::filesystem::copy_options option)
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

    fs::directory_iterator it2{deserializeOutPath / "Scripts"};
    for(auto& f : it2)
    {
        auto srcPath = f.path().filename();
        if(srcPath != "unison_custom.rb" && srcPath != Script_File_Name){
            continue;
        }
        if(srcPath.extension().string().find(rbscriptwriter::extension) == std::string::npos){ continue; }

        fs::path to = outputProjectDataPath(srcPath, "Scripts");
        fs::copy(f, to, option);
    }
}