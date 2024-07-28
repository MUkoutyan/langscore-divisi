#include "config_writer.h"
#include <nlohmann/json.hpp>
#include "reader/csvreader.h"


using namespace std::string_literals;
using namespace langscore;
namespace fs = std::filesystem;


namespace 
{
    const static std::map<std::u8string, config::ProjectType> projectExtensionAndType = {
        {u8"rvproj2"s,     config::VXAce},
        {u8"rpgproject"s,  config::MV},
        {u8"rmmzproject"s, config::MZ},
    };

    config::ProjectType getProjectType(std::filesystem::path filePath)
    {
        if(fs::is_regular_file(filePath)) {
            auto extension = filePath.extension().u8string();
            if(projectExtensionAndType.find(extension) != projectExtensionAndType.end()) {
                return projectExtensionAndType.at(extension);
            }
        }
        else if(fs::is_directory(filePath)) {
            for(const auto& entry : fs::directory_iterator(filePath)) {
                auto extension = entry.path().extension().u8string();
                if(projectExtensionAndType.find(extension) != projectExtensionAndType.end()) {
                    return projectExtensionAndType.at(extension);
                }
            }
        }
        return config::None;
    }

    template <typename StringType>
    StringType withoutAllExtension(const StringType& fileName) {
        typename StringType::size_type index = fileName.find('.');
        if(index == StringType::npos) {
            return fileName;
        }

        return fileName.substr(0, index);
    }

    std::vector<config::BasicData> fetchBasicDataInfo(fs::path analyzeDir)
    {
        std::vector<config::BasicData> result;
        for(auto& file : fs::directory_iterator{analyzeDir}) 
        {
            auto filePath = file.path();
            if(filePath.extension() != ".json") { continue; }

            auto fileName = withoutAllExtension(filePath.stem().u8string());
            if(fileName.empty()) {
                throw "Load Invalid Basic Script Info";
            }

            fileName += u8".json";
            result.emplace_back(
                config::BasicData{fileName, false, 0}
            );
        }
        return result;
    }
    std::vector<config::ScriptData> fetchScriptDataInfo(fs::path analyzeDir)
    {
        auto scriptDir = analyzeDir / "Scripts";
        auto csv = scriptDir / "_list.csv";
        plaincsvreader reader(csv);
        auto texts = reader.getPlainCsvTexts();
        std::vector<config::ScriptData> result;
        for(const auto& row : texts)
        {
            if(row[0].empty()) { continue; }

            auto fileName = (row[0] + u8".rb");
            auto filePath = scriptDir / fileName;
            if(fs::exists(filePath) == false) { continue; }

            config::ScriptData data{};
            data.filename = fileName;
            result.emplace_back(std::move(data));
        }
        return result;
    }
}


config_writer::config_writer(std::filesystem::path path)
    : projectType(config::None)
    , gameProjectPath(path)
    , exportByLanguage(false)
    , overwriteLangscore(false)
    , overwriteLangscoreCustom(false)
    , globalWriteMode(0)
{

    auto projType = ::getProjectType(gameProjectPath);

    if(this->projectType == config::None) { return; }

    this->projectType = projType;

    if(projType == config::VXAce) {
        this->exportDirectory = { u8"./Data/Translate"s };
    }
    else if(projType == config::MV || projType == config::MZ) {
        this->exportDirectory = { u8"./data/translate"s };
    }

    // ƒtƒHƒ‹ƒ_–¼‚ÌŽæ“¾
    auto lastSlashPos = gameProjectPath.u8string().find_last_of(u8"/\\"s);
    auto folderName = gameProjectPath.u8string().substr(lastSlashPos + 1);

    fs::path langscoreProj = path.parent_path() / (folderName + u8"_langscore"s);
    auto u8Path = fs::absolute(langscoreProj).u8string();
    std::replace(u8Path.begin(), u8Path.end(), '\\', '/');
    this->langscoreProjectPath = u8Path;

}

config_writer::~config_writer()
{
}

bool langscore::config_writer::write()
{
    std::array<std::u8string, 3> supportedExtention = {
        u8".rpgproject",u8".rvproj2",u8".rmmzproject"
    };
    const static std::map<std::u8string, config::ProjectType> projectExtensionAndType = {
        {u8"rvproj2",     config::ProjectType::VXAce},
        {u8"rpgproject",  config::ProjectType::MV},
        {u8"rmmzproject", config::ProjectType::MZ},
    };


    auto folderPath = this->gameProjectPath;
    if(fs::is_regular_file(this->gameProjectPath)){
        folderPath = this->gameProjectPath.parent_path();
        auto ext = this->gameProjectPath.extension().u8string();
        if(projectExtensionAndType.contains(ext.substr(1))) {
            this->projectType = projectExtensionAndType.at(ext.substr(1));
        }
    }
    else if(fs::is_directory(this->gameProjectPath)) 
    {
        for(const auto& entry : fs::directory_iterator{this->gameProjectPath})
        {
            auto ext = entry.path().extension().u8string();
            if(ext.empty()) { continue; }
            if(projectExtensionAndType.contains(ext.substr(1))) {
                this->projectType = projectExtensionAndType.at(ext.substr(1));
                break;
            }
        }
    }

    this->langscoreProjectPath = u8"./../" + folderPath.filename().u8string();
    fs::path langscoreWorkFolder = folderPath.u8string() + u8"_langscore"s;
    this->langscoreAnalyzeDirectorty = "./analyze";
    this->langscoreUpdateDirectorty = langscoreWorkFolder / "update";
    this->vxaceBasicData = ::fetchBasicDataInfo(langscoreWorkFolder / this->langscoreAnalyzeDirectorty);
    this->rpgMakerScripts = ::fetchScriptDataInfo(langscoreWorkFolder / this->langscoreAnalyzeDirectorty);

    std::vector<std::string> languages = {
        "ja","en","zh-cn","zh-tw","ko","es","de","fr","it","ru"
    };


    if(this->projectType == config::ProjectType::VXAce) 
    {
        this->exportDirectory = { u8"./Data/Translate" };

        auto fontName = u8"VL Gothic";
        auto fontPath = u8"resources/fonts/VL-Gothic-Regular.ttf";
        for(auto& lang : languages) {
            this->languages.emplace_back(config::Language{lang == "ja", lang, {fontName, fontPath}});
        }
    }
    else if(this->projectType == config::ProjectType::MV || this->projectType == config::ProjectType::MZ) 
    {
        this->exportDirectory = { u8"./data/translate" };

        auto fontName = u8"M+ 1m regular";
        auto fontPath = u8"resources/fonts/mplus-1m-regular.ttf";
        for(auto& lang : languages) {
            this->languages.emplace_back(config::Language{lang == "ja", lang, {fontName, fontPath}});
        }
    }


    if(fs::exists(langscoreWorkFolder) == false) {
        fs::create_directory(langscoreWorkFolder);
    }
    std::ofstream configJsonFile(langscoreWorkFolder / "config.json", std::ios_base::trunc);

    configJsonFile << createJson();

    return true;
}

std::string config_writer::createJson() const
{
    nlohmann::json root;

    root["ApplicationVersion"] = "";
    root["ConfigVersion"] = "";

    nlohmann::json langs = nlohmann::json::array();
    for(const auto& l : this->languages) {
        nlohmann::json langObj;
        langObj["LanguageName"] = l.name;
        langObj["FontName"] = utility::cnvStr<std::string>(l.font.name);
        langObj["FontPath"] = utility::cnvStr<std::string>(l.font.file.u8string());
        langObj["FontSize"] = l.font.size;
        langObj["Enable"] = l.isEnable;

        langs.push_back(langObj);
    }
    root["Languages"] = langs;
    root["DefaultLanguage"] = this->defaultLanguage;
    root["Project"] = utility::cnvStr<std::string>(this->langscoreProjectPath.generic_u8string());

    nlohmann::json analyze;
    analyze["TmpDir"] = utility::cnvStr<std::string>(this->langscoreAnalyzeDirectorty.generic_u8string());

    root["Analyze"] = analyze;

    nlohmann::json write;
    write["UsCustomFuncComment"] = "Scripts/{0}#{1},{2}";

    //nlohmann::json exportDirList = nlohmann::json::array();
    //for(const auto& dir : this->exportDirectory) {
    //    exportDirList.push_back(utility::cnvStr<std::string>(dir));
    //}
    //write["ExportDirectory"] = exportDirList;
    write["ExportDirectory"] = utility::cnvStr<std::string>(this->exportDirectory[0]);
    write["ExportByLang"] = this->exportByLanguage;
    write["OverwriteLangscore"] = this->overwriteLangscore;
    write["OverwriteLangscoreCustom"] = this->overwriteLangscoreCustom;
    write["WriteType"] = 0;

    nlohmann::json basicDataList = nlohmann::json::array();
    for(const auto& info : vxaceBasicData) {
        nlohmann::json script;
        script["Name"] = utility::cnvStr<std::string>(info.filename);
        script["Ignore"] = info.ignore;
        basicDataList.push_back(script);
    }
    write["RPGMakerBasicData"] = basicDataList;

    nlohmann::json scripts = nlohmann::json::array();
    for(const auto& info : rpgMakerScripts) {
        nlohmann::json script;
        script["Name"] = utility::cnvStr<std::string>(info.filename);
        script["Ignore"] = info.ignore;
        nlohmann::json ignorePointArray = nlohmann::json::array();
        for(const auto& point : info.texts) {
            nlohmann::json obj;
            obj["Row"] = point.row;
            obj["Col"] = point.col;
            obj["Disable"] = false;
            obj["Ignore"] = point.ignore;
            ignorePointArray.push_back(obj);
        }
        script["IgnorePoints"] = ignorePointArray;
        scripts.push_back(script);
    }
    write["RPGMakerScripts"] = scripts;

    nlohmann::json ignorePictures = nlohmann::json::array();
    write["IgnorePictures"] = ignorePictures;

    root["Write"] = write;
    root["PackingInputDir"] = "";

    return root.dump(4);
}
