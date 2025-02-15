#pragma once
#include <filesystem>

#include "utility.hpp"
#include "config_keys.h"

namespace langscore
{
	class config
	{
	public:

		enum ProjectType {
			None,
			VXAce,
			MV,
			MZ,
		};

        enum ValidateTextMode : char {
            Ignore,
            TextCount,
            TextWidth,
        };

		struct FontData
		{
			std::u8string name = u8"";
			std::filesystem::path file;
			int size = 22;
		};
		struct Language
		{
            bool isEnable;
			std::string name;
			FontData font;
		};



        struct ValidateTextInfo {
            ValidateTextMode mode = ValidateTextMode::Ignore;
            int width = 0;
            int count = 0;

            bool operator==(const ValidateTextInfo& infos) const noexcept {
                return !(this->operator!=(infos));
            }
            bool operator!=(const ValidateTextInfo& infos) const noexcept
            {
                if(infos.mode != this->mode) { return true; }
                switch(this->mode) {
                case ValidateTextMode::Ignore:
                    return false;
                case ValidateTextMode::TextCount:
                    return infos.count != this->count;
                case ValidateTextMode::TextWidth:
                    return infos.width != this->width;
                }
                return false;
            }

            int value() const noexcept {
                if(this->mode == ValidateTextMode::TextCount) {
                    return this->count;
                }
                else if(this->mode == ValidateTextMode::TextWidth) {
                    return this->width;
                }
                return 0;
            }
            void setValue(int v) noexcept {
                if(this->mode == ValidateTextMode::TextCount) {
                    this->count = v;
                }
                else if(this->mode == ValidateTextMode::TextWidth) {
                    this->width = v;
                }
            }
        };

        using TextValidationLangMap = std::map<std::u8string, ValidateTextInfo>;
        using TextValidateTypeMap = std::map<std::u8string, TextValidationLangMap>;


		struct BasicData
		{
			std::u8string filename = u8"";
			bool ignore = false;
			char writeMode = 0;
            TextValidateTypeMap textValidateInfos = {};
		};

		struct ScriptData : public BasicData
		{
			struct TextPoint
			{
				int row = 0;
				int col = 0;
				bool disable = false;	//元スクリプト変更によって位置が噛み合わなくなった場合true
				bool ignore = false;
                char writeMode = 0;
				int ignoreContext = 0;
				std::u8string text;
			};
			std::vector<TextPoint> texts;
		};


		static const char* key(JsonKey key);

		static void attachConfigFile(std::filesystem::path path);
		static void detachConfigFile();

		config(std::filesystem::path path);
		config();
		~config();

		std::u8string langscoreProjectPath();

		ProjectType projectType();

		std::vector<Language> languages();
		std::string defaultLanguage();
		std::u8string gameProjectPath();
		std::u8string langscoreAnalyzeDirectorty();
		std::u8string langscoreUpdateDirectorty();
		std::string usScriptFuncComment();
		utility::u8stringlist exportDirectory(std::u8string& root);
		bool exportByLanguage();
		bool overwriteLangscore();
		bool overwriteLangscoreCustom();
		std::u8string outputTranslateFilePathForRPGMaker();
		std::u8string packingInputDirectory();
        bool packingEnablePerLang();
        std::u8string packingPerLangOutputDir();
		std::vector<BasicData> rpgMakerBasicData();
		std::vector<ScriptData> rpgMakerScripts();
		utility::u8stringlist ignorePictures();
		utility::u8stringlist globalFontList();
		utility::u8stringlist localFontList();
        utility::u8stringlist extendControlCharList();
        char globalWriteMode();
        bool enableLanguagePatch();

        //int validateTextMode();
        //std::vector<int> validateSizeList();

        //言語, パスの組み合わせ
        std::vector<std::pair<std::u8string, std::u8string>> exportDirectoryWithLang(std::u8string& root);

	private:
		class Impl;
		std::unique_ptr<Impl> pImpl;
	};
}