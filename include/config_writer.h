#pragma once
#include "config.h"
#include <filesystem>

namespace langscore
{
    class config_writer
    {
    public:
        config_writer(std::filesystem::path path);
        ~config_writer();

        bool write();

        std::filesystem::path langscoreProjectPath;
        config::ProjectType projectType;
        std::vector<config::Language> languages;
        std::string defaultLanguage;
        std::filesystem::path gameProjectPath;
        std::filesystem::path langscoreAnalyzeDirectorty;
        std::filesystem::path langscoreUpdateDirectorty;
        std::string usScriptFuncComment;
        utility::u8stringlist exportDirectory;
        bool exportByLanguage;
        bool overwriteLangscore;
        bool overwriteLangscoreCustom;
        std::u8string outputTranslateFilePathForRPGMaker;
        std::u8string packingInputDirectory;
        std::vector<config::BasicData> vxaceBasicData;
        std::vector<config::ScriptData> rpgMakerScripts;
        utility::u8stringlist ignorePictures;
        utility::u8stringlist globalFontList;
        utility::u8stringlist localFontList;
        int globalWriteMode;

    private:
        std::string createJson() const;
    };
}