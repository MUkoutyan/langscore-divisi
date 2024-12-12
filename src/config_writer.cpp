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
            if(extension.empty()) { return config::None; }
            if(extension[0] == u8'.') {
                extension.erase(0, 1);
            }
            if(projectExtensionAndType.find(extension) != projectExtensionAndType.end()) {
                return projectExtensionAndType.at(extension);
            }
        }
        else if(fs::is_directory(filePath)) {
            for(const auto& entry : fs::directory_iterator(filePath)) {
                auto extension = entry.path().extension().u8string();
                if(extension.empty()) { continue; }
                if(extension[0] == u8'.') {
                    extension.erase(0, 1);
                }
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
        if(fs::exists(analyzeDir) == false) { return {}; }
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
        if(fs::exists(analyzeDir) == false) { return {}; }

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


config_writer::config_writer(std::filesystem::path gameProjpath)
    : projectType(config::None)
    , gameProjectPath(gameProjpath)
    , exportByLanguage(false)
    , overwriteLangscore(false)
    , overwriteLangscoreCustom(false)
    , globalWriteMode(0)
{

    auto projType = ::getProjectType(gameProjectPath);
    if(projType == config::None) { return; }

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

    fs::path langscoreProj = gameProjpath.parent_path() / (folderName + u8"_langscore"s);
    auto u8Path = fs::absolute(langscoreProj).u8string();
    std::replace(u8Path.begin(), u8Path.end(), '\\', '/');
    this->langscoreProjectPath = u8Path;

}

config_writer::~config_writer()
{
}

bool langscore::config_writer::write()
{
    if(this->projectType == config::ProjectType::None) {
        return false;
    }

    fs::path gameFolderPath;
    if(fs::is_regular_file(this->gameProjectPath)) {
        gameFolderPath = this->gameProjectPath.parent_path();
    }
    else {
        gameFolderPath = this->gameProjectPath;
    }

    this->langscoreProjectPath = gameFolderPath.u8string() + u8"_langscore"s;
    if(fs::exists(this->langscoreProjectPath) == false)
    {
        fs::create_directory(this->langscoreProjectPath);
    }

    this->langscoreAnalyzeDirectorty = "./analyze";
    this->langscoreUpdateDirectorty = this->langscoreProjectPath / "update";
    this->vxaceBasicData = ::fetchBasicDataInfo(this->langscoreProjectPath / this->langscoreAnalyzeDirectorty);
    this->rpgMakerScripts = ::fetchScriptDataInfo(this->langscoreProjectPath / this->langscoreAnalyzeDirectorty);

    std::cout << "output folder " << this->langscoreProjectPath << std::endl;
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

    std::ofstream configJsonFile(this->langscoreProjectPath / "config.json", std::ios_base::trunc);

    configJsonFile << createJson();

    return true;
}

std::string config_writer::createJson() const
{
    nlohmann::json root;

    root[config::key(config::JsonKey::ApplicationVersion)] = "";
    root[config::key(config::JsonKey::ConfigVersion)] = "";

    nlohmann::json langs = nlohmann::json::array();
    for(const auto& l : this->languages) {
        nlohmann::json langObj;
        langObj[config::key(config::JsonKey::LanguageName)] = l.name;
        langObj[config::key(config::JsonKey::FontName)] = utility::cnvStr<std::string>(l.font.name);
        langObj[config::key(config::JsonKey::FontPath)] = utility::cnvStr<std::string>(l.font.file.u8string());
        langObj[config::key(config::JsonKey::FontSize)] = l.font.size;
        langObj[config::key(config::JsonKey::Enable)] = l.isEnable;

        langs.push_back(langObj);
    }
    root[config::key(config::JsonKey::Languages)] = langs;
    root[config::key(config::JsonKey::DefaultLanguage)] = this->defaultLanguage;
    root[config::key(config::JsonKey::Project)] = utility::cnvStr<std::string>(this->gameProjectPath.generic_u8string());

    nlohmann::json analyze;
    analyze[config::key(config::JsonKey::TmpDir)] = utility::cnvStr<std::string>(this->langscoreAnalyzeDirectorty.generic_u8string());

    root[config::key(config::JsonKey::Analyze)] = analyze;

    nlohmann::json write;
    write[config::key(config::JsonKey::UsCustomFuncComment)] = "Scripts/{0}#{1},{2}";

    //nlohmann::json exportDirList = nlohmann::json::array();
    //for(const auto& dir : this->exportDirectory) {
    //    exportDirList.push_back(utility::cnvStr<std::string>(dir));
    //}
    //write[config::key(config::JsonKey::ExportDirectory)] = exportDirList;
    if(this->exportDirectory.empty() == false) {
        write[config::key(config::JsonKey::ExportDirectory)] = utility::cnvStr<std::string>(this->exportDirectory[0]);
    }
    else {
        write[config::key(config::JsonKey::ExportDirectory)] = "";
    }
    write[config::key(config::JsonKey::ExportByLang)] = this->exportByLanguage;
    write[config::key(config::JsonKey::OverwriteLangscore)] = this->overwriteLangscore;
    write[config::key(config::JsonKey::OverwriteLangscoreCustom)] = this->overwriteLangscoreCustom;
    write[config::key(config::JsonKey::WriteType)] = 0;

    nlohmann::json basicDataList = nlohmann::json::array();
    for(const auto& info : vxaceBasicData) {
        nlohmann::json script;
        script[config::key(config::JsonKey::Name)] = utility::cnvStr<std::string>(info.filename);
        script[config::key(config::JsonKey::Ignore)] = info.ignore;
        basicDataList.push_back(script);
    }
    write[config::key(config::JsonKey::RPGMakerBasicData)] = basicDataList;

    nlohmann::json scripts = nlohmann::json::array();
    for(const auto& info : rpgMakerScripts) {
        nlohmann::json script;
        script[config::key(config::JsonKey::Name)] = utility::cnvStr<std::string>(info.filename);
        script[config::key(config::JsonKey::Ignore)] = info.ignore;
        nlohmann::json ignorePointArray = nlohmann::json::array();
        for(const auto& point : info.texts) {
            nlohmann::json obj;
            obj[config::key(config::JsonKey::Row)] = point.row;
            obj[config::key(config::JsonKey::Col)] = point.col;
            obj[config::key(config::JsonKey::Disable)] = false;
            obj[config::key(config::JsonKey::Ignore)] = point.ignore;
            ignorePointArray.push_back(obj);
        }
        script[config::key(config::JsonKey::IgnorePoints)] = ignorePointArray;
        scripts.push_back(script);
    }
    write[config::key(config::JsonKey::RPGMakerScripts)] = scripts;

    nlohmann::json ignorePictures = nlohmann::json::array();
    write[config::key(config::JsonKey::IgnorePictures)] = ignorePictures;

    root[config::key(config::JsonKey::Write)] = write;

    root[config::key(config::JsonKey::PackingInputDir)] = "";
    root[config::key(config::JsonKey::PackingEnablePerLang)] = false;
    root[config::key(config::JsonKey::PackingPerLangOutputDir)] = "";

    return root.dump(4);
}
