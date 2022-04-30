#include "divisi.h"
#include "utility.hpp"
#include "deserializer.h"
#include "nlohmann/json.hpp"

#include "writer/csvwriter.h"
#include <fstream>

using namespace langscore;
namespace fs = std::filesystem;

class divisi::Impl
{
public:
    deserializer deserializer;
};

divisi::divisi(std::string appPath)
    : pImpl(std::make_unique<Impl>())
    , ignoreScriptPath()
{
    pImpl->deserializer.setApplicationFolder(std::move(appPath));
    fs::path outPath = pImpl->deserializer.outputPath();

    fs::directory_iterator it(outPath);
    for(auto& f : it){
        fs::remove(f.path());
    }
}

divisi::~divisi(){}

void divisi::setIgnoreScriptPath(std::vector<fs::path> ignoreScriptPath){
    this->ignoreScriptPath = std::move(ignoreScriptPath);
}

void divisi::setProjectPath(std::string projectPath)
{
    auto p = fs::path(projectPath);
    fs::directory_iterator it(p);
    auto type = deserializer::ProjectType::None;
    for(auto& file : it){
        if(file.path().extension() == ".rvproj2"){
            type = deserializer::ProjectType::VXAce;
            break;
        }
    }
    pImpl->deserializer.setProjectPath(type, std::move(projectPath));
}

void divisi::exec()
{
    pImpl->deserializer.exec();

    const auto deserializeOutPath = pImpl->deserializer.outputPath();

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
      
        auto csvFilePath = path.filename().replace_extension(".csv");
        writer.write(csvFilePath.u8string());
    }
    
    
    auto scriptLocalize = deserializeOutPath + "/Scripts.csv";
    std::ofstream csvWrite(scriptLocalize);
    
    const utility::stringlist ignoreScriptFile = {u8"TES基本.rb"};

    std::vector<fs::path> scriptList;
    fs::recursive_directory_iterator scriptIt(deserializeOutPath);
    for(auto& f : scriptIt){
        if(f.path().extension() == ".rb"){
            scriptList.emplace_back(f.path());
        }
    }

    for(auto& path : scriptList)
    {
        auto osPath = path.filename().u8string();
        auto result = std::find(ignoreScriptFile.cbegin(), ignoreScriptFile.cend(), osPath);
        if(result != ignoreScriptFile.cend()){ continue; }
        
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
            
            if(line.find("\"") != std::string::npos)
            {
                line.erase(0, line.find("\""));
                line.erase(line.find_last_of("\"")+1, line.size());
                
                csvWrite << line << ",,,," << path.filename() << " Line : " << lineCount << "\n";
            }
        }
    }
}
