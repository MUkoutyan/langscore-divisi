#include "config.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>

using namespace langscore;
using namespace std::literals::string_literals;

class config::Impl
{
public:
	nlohmann::json json;

	template<typename Value>
	Value get(nlohmann::json::reference j, Value defValue = 0)
	{
		try{
			Value v;
			return j.get_to<Value>(v);
		}
		catch(...){
			return defValue;
		}
		return defValue;
	}

};

config::config(std::filesystem::path path)
	: pImpl(std::make_unique<Impl>())
{
	std::ifstream loadFile(path);
	if(loadFile.good()){
		loadFile >> pImpl->json;
	}
	else
	{
		std::stringstream ss;
		ss << "{\n";
		ss << "\t\"" << KEY_LANGUAGES << R"(": ["ja", "en", "zh-sc"])" << ", \n";
		ss << "\t\"" << KEY_DEFAULT_LANGUAGE << "\": \"ja\", \n";
		ss << "\t\"" << KEY_TEMP_DIR << "\": \"" << (std::filesystem::temp_directory_path() / "ls-tmp/Translate").generic_string() << "\", \n";
		ss << "\t\"" << KEY_US_CUSTOM_FUNC_COMMENT << "\": \"project://Scripts/{0}#{1},{2}\",\n";
		ss << "\t\"" << KEY_RPGMAKER_OUTPUT_PATH << "\": \"Translate\", \n";
		ss << R"(
	"vxace": {
		"fonts": {
			"default": {
				"name": "VL Gothic",
				"size": 24
			},
			"zh-sc": {
				"name": "SourceHanSansSC",
				"size": 21
			}
		},
		"RPGMakerIgnoreScripts" : ["DataManager.rb", "BattleManager.rb", "Game_Interpreter.rb",
		   "Game_Map.rb", "Game_Message.rb", "Spriteset_Battle.rb",
		   "Window_Base.rb", "Window_ShopStatus.rb", "Window_Status.rb",
		   "Window_DebugRight.rb", "Window_DebugLeft.rb", "Window_Message.rb",
		   "Window_NameInput.rb", "Window_NumberInput.rb", "Window_SaveFile.rb"]
	})";
		ss << "\n}";

		auto str = ss.str();

		ss >> pImpl->json;
	}
}

config::~config()
{}

utility::stringlist config::languages()
{
	utility::stringlist r = {"ja", "en", "zh-sc"};
	return  pImpl->get(pImpl->json[KEY_LANGUAGES], r);
}

std::string langscore::config::defaultLanguage() {
	return pImpl->get(pImpl->json[KEY_DEFAULT_LANGUAGE], "ja"s);
}

std::string config::tempDirectorty() {
	return pImpl->get(pImpl->json[KEY_TEMP_DIR], (std::filesystem::temp_directory_path() / "Data/").string());
}

std::string config::usScriptFuncComment(){
	return pImpl->get(pImpl->json[KEY_US_CUSTOM_FUNC_COMMENT], "project://Scripts/{0}#{1},{2}"s);
}

std::string config::rpgmakerOutputPath(){
	return pImpl->get(pImpl->json[KEY_RPGMAKER_OUTPUT_PATH], "Translate/"s);
}

std::unordered_map<std::string, config::FontData> config::vxaceFonts()
{
	auto& vxace = pImpl->json["vxace"];
	auto& fonts = vxace["fonts"];
	std::unordered_map<std::string, config::FontData> m = {
		{"default", {u8"VL Gothic", 22}},
		{"zh-sc",   {u8"SourceHanSansSC", 20}}
	};

	std::unordered_map<std::string, config::FontData> result;

	for(auto s = fonts.begin(); s != fonts.end(); ++s)
	{
		auto f = s.value();
		FontData fontData = {utility::cnvStr<std::u8string>(pImpl->get(f["name"], "VL Gothic"s)), f["size"]};
		result[s.key()] = std::move(fontData);
	}

	return result;
}

std::vector<std::string> langscore::config::vxaceIgnoreScripts()
{
	return std::vector<std::string>();
}
