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
			std::u8string name;
			int size;
		};

		constexpr static char KEY_LANGUAGES[] = "Languages";
		constexpr static char KEY_DEFAULT_LANGUAGE[] = "DefaultLanguage";
		constexpr static char KEY_TEMP_DIR[] = "TmpDir";
		constexpr static char KEY_US_CUSTOM_FUNC_COMMENT[] = "UsCustomFuncComment";
		constexpr static char KEY_RPGMAKER_OUTPUT_PATH[] = "RPGMakerOutputPath";
		constexpr static char KEY_RPGMAKER_IGNORE_SCRIPTS[] = "RPGMakerIgnoreScripts";

		config(std::filesystem::path path = "./config.json");
		~config();

		utility::stringlist languages();
		std::string defaultLanguage();
		std::string tempDirectorty();
		std::string usScriptFuncComment();
		std::string rpgmakerOutputPath();
		std::unordered_map<std::string, FontData> vxaceFonts();
		std::vector<std::string> vxaceIgnoreScripts();

	private:
		class Impl;
		std::unique_ptr<Impl> pImpl;
	};
}