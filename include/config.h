#pragma once
#include <filesystem>
#include <unordered_map>

#include "utility.hpp"

namespace langscore
{
	class config
	{
	public:

		struct FontData
		{
			std::u8string name = u8"";
			int size = 22;
		};
		struct Language
		{
			std::string name;
			FontData font;
		};

		struct ScriptData
		{
			std::u8string filename = u8"";
			bool ignore = false;
			int writeMode = 0;
			std::vector<std::pair<int, int>> ignoreLines;
		};

		constexpr static char KEY_LANGUAGES[] = "Languages";
		constexpr static char KEY_DEFAULT_LANGUAGE[] = "DefaultLanguage";
		constexpr static char KEY_TEMP_DIR[] = "TmpDir";
		constexpr static char KEY_US_CUSTOM_FUNC_COMMENT[] = "UsCustomFuncComment";
		constexpr static char KEY_EXPORT_DIRECTORY[] = "ExportDirectory";
		constexpr static char KEY_EXPORT_BY_LANG[] = "ExportByLang";
		constexpr static char KEY_RPGMAKER_OUTPUT_PATH[] = "RPGMakerOutputPath";
		constexpr static char KEY_RPGMAKER_IGNORE_SCRIPTS[] = "RPGMakerIgnoreScripts";

		static void attachConfigFile(std::filesystem::path path);
		static void detachConfigFile();

		config(std::filesystem::path path);
		config();
		~config();

		std::vector<Language> languages();
		std::string defaultLanguage();
		std::string projectPath();
		std::string tempDirectorty();
		std::string usScriptFuncComment();
		std::vector<std::string> exportDirectory();
		bool exportByLanguage();
		std::string outputTranslateFilePathForRPGMaker();
		std::vector<ScriptData> vxaceIgnoreScripts();
		utility::u8stringlist ignorePictures();

	private:
		class Impl;
		std::unique_ptr<Impl> pImpl;
	};
}