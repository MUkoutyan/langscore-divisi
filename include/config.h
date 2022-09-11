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

		struct BasicData
		{
			std::u8string filename = u8"";
			bool ignore = false;
			int writeMode = 0;
		};

		struct ScriptData : public BasicData
		{
			struct TextPoint
			{
				int row = 0;
				int col = 0;
				bool disable = false;	//元スクリプト変更によって位置が噛み合わなくなった場合true
				bool ignore = false;
				int writeMode = 0;
				std::u8string text;
			};
			std::vector<TextPoint> texts;
		};

		enum class JsonKey : size_t
		{
			Languages,
			LanguageName,
			Enable,
			Disable,
			FontName,
			FontSize,
			FontPath,
			Global,
			Local,
			Project,
			Analyze,
			Write,
			Name,
			Ignore,
			IgnorePoints,
			Row,
			Col,
			WriteType,
			Text,
			IgnorePictures,
			DefaultLanguage,
			TmpDir,
			UsCustomFuncComment,
			ExportDirectory,
			ExportByLang,
			RPGMakerOutputPath,
			RPGMakerBasicData,
			RPGMakerScripts,
			OverwriteLangscore,
			OverwriteLangscoreCustom,
			PackingInputDir,
			ApplicationVersion,
			ConfigVersion,

			NumKeys,
		};

		static const char* key(JsonKey key);

		static void attachConfigFile(std::filesystem::path path);
		static void detachConfigFile();

		config(std::filesystem::path path);
		config();
		~config();

		std::vector<Language> languages();
		std::string defaultLanguage();
		std::u8string gameProjectPath();
		std::u8string langscoreAnalyzeDirectorty();
		std::string usScriptFuncComment();
		std::vector<std::u8string> exportDirectory(std::u8string& root);
		bool exportByLanguage();
		bool overwriteLangscore();
		bool overwriteLangscoreCustom();
		std::u8string outputTranslateFilePathForRPGMaker();
		std::u8string packingInputDirectory();
		std::vector<BasicData> vxaceBasicData();
		std::vector<ScriptData> vxaceScripts();
		utility::u8stringlist ignorePictures();
		utility::u8stringlist globalFontList();
		utility::u8stringlist localFontList();

	private:
		class Impl;
		std::unique_ptr<Impl> pImpl;
	};
}