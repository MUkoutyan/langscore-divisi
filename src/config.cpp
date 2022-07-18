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
	static std::filesystem::path configPath;

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

	void load(std::filesystem::path path)
	{
		std::ifstream loadFile(path);
		if(loadFile.good()){
			loadFile >> json;
		}
		else{
			throw "Can't Open File";
		}
	}

};
std::filesystem::path config::Impl::configPath = "";

void langscore::config::attachConfigFile(std::filesystem::path path){
	Impl::configPath = std::move(path);
}

void langscore::config::detachConfigFile(){
	Impl::configPath = "";
}

config::config(std::filesystem::path path)
	: pImpl(std::make_unique<Impl>())
{
	if(Impl::configPath != ""){
		path = Impl::configPath;
	}

	pImpl->load(std::move(path));
}

langscore::config::config()
	: pImpl(std::make_unique<Impl>())
{
	if(Impl::configPath != ""){
		pImpl->load(Impl::configPath);
	}
	else
	{
		std::stringstream ss;
		ss << "{\n";
		ss << "\t\"" << KEY_LANGUAGES << R"(": ["ja", "en", "zh-cn"])" << ", \n";
		ss << "\t\"" << KEY_DEFAULT_LANGUAGE << "\": \"ja\", \n";
		ss << "\t\"" << KEY_TEMP_DIR << "\": \"" << (std::filesystem::temp_directory_path() / "ls-tmp/Translate").generic_string() << "\", \n";
		ss << R"(
	"write": {
		"RPGMakerOutputPath": "Translate",
		"UsCustomFuncComment": "project://Scripts/{0}#{1},{2}",
		"fonts": {
			"default": {
				"name": "VL Gothic",
				"size": 24
			},
			"zh-cn": {
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

std::vector<config::Language> config::languages()
{
	auto& langs = pImpl->json[KEY_LANGUAGES];

	std::vector<Language> result;

	for(auto s = langs.begin(); s != langs.end(); ++s)
	{
		auto lang = s.value();
		result.emplace_back(
			pImpl->get(lang["languageName"], "ja"s),
			FontData{
				utility::cnvStr<std::u8string>(pImpl->get(lang["fontName"], "VL Gothic"s)),
				pImpl->get(lang["fontSize"], 22)
			}
		);
	}

	return result;
}

std::string langscore::config::defaultLanguage() {
	return pImpl->get(pImpl->json[KEY_DEFAULT_LANGUAGE], "ja"s);
}

std::string langscore::config::projectPath()
{
	return pImpl->get(pImpl->json["Project"], ""s);
}

std::string config::tempDirectorty() {
	return pImpl->get(pImpl->json["analyze"][KEY_TEMP_DIR], (std::filesystem::temp_directory_path() / "Data/").string());
}

std::string config::usScriptFuncComment(){
	return pImpl->get(pImpl->json["write"][KEY_US_CUSTOM_FUNC_COMMENT], "project://Scripts/{0}#{1},{2}"s);
}

std::string config::outputTranslateFilePathForRPGMaker(){
	return pImpl->get(pImpl->json["analyze"][KEY_RPGMAKER_OUTPUT_PATH], "Translate/"s);
}

std::vector<config::ScriptData> langscore::config::vxaceIgnoreScripts()
{
	auto& write = pImpl->json["write"];
	auto& scripts = write[KEY_RPGMAKER_IGNORE_SCRIPTS];
	std::vector<ScriptData> result;
	for(auto s = scripts.begin(); s != scripts.end(); ++s)
	{
		auto script = s.value();
		ScriptData data = {
			utility::cnvStr<std::u8string>(pImpl->get(script["name"], ""s)),
			pImpl->get(script["ignore"], false),
			0,
			{}
		};
		auto& ignorePoints = script["ignorePoints"];
		for(auto i = ignorePoints.begin(); i != ignorePoints.end(); ++i){
			auto rc = i.value();
			data.ignoreLines.emplace_back(std::pair<int, int>{rc["row"], rc["col"]});
		}

		if(data.filename.empty() == false){
			result.emplace_back(std::move(data));
		}
	}
	return result;
}

utility::u8stringlist langscore::config::ignorePictures()
{
	auto& write = pImpl->json["write"];
	auto& pictures = write["ignorePictures"];
	utility::u8stringlist result;
	for(auto s = pictures.begin(); s != pictures.end(); ++s){
		result.emplace_back(utility::cnvStr<std::u8string>(pImpl->get(s.value(), ""s)));
	}
	return result;
}
