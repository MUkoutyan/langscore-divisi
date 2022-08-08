#include "config.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <array>

using namespace langscore;
using namespace std::literals::string_literals;

#define MAKE_KEYVALUE(k) {config::JsonKey::k, #k}

static std::map<config::JsonKey, const char*> jsonKeys = {
	MAKE_KEYVALUE(Languages),
	MAKE_KEYVALUE(LanguageName),
	MAKE_KEYVALUE(Enable),
	MAKE_KEYVALUE(Disable),
	MAKE_KEYVALUE(FontName),
	MAKE_KEYVALUE(FontSize),
	MAKE_KEYVALUE(Project),
	MAKE_KEYVALUE(Analyze),
	MAKE_KEYVALUE(Write),
	MAKE_KEYVALUE(Name),
	MAKE_KEYVALUE(Ignore),
	MAKE_KEYVALUE(IgnorePoints),
	MAKE_KEYVALUE(Row),
	MAKE_KEYVALUE(Col),
	MAKE_KEYVALUE(WriteType),
	MAKE_KEYVALUE(Text),
	MAKE_KEYVALUE(IgnorePictures),
	MAKE_KEYVALUE(DefaultLanguage),
	MAKE_KEYVALUE(TmpDir),
	MAKE_KEYVALUE(UsCustomFuncComment),
	MAKE_KEYVALUE(ExportDirectory),
	MAKE_KEYVALUE(ExportByLang),
	MAKE_KEYVALUE(RPGMakerOutputPath),
	MAKE_KEYVALUE(RPGMakerBasicData),
	MAKE_KEYVALUE(RPGMakerScripts),
};

const char* config::key(JsonKey key)
{
	if(jsonKeys.find(key) == jsonKeys.end()){ return nullptr; }
	return jsonKeys[key];
}

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
		ss << "\t\"" << key(JsonKey::Languages) << R"(": ["ja", "en", "zh-cn"])" << ", \n";
		ss << "\t\"" << key(JsonKey::DefaultLanguage) << "\": \"ja\", \n";
		ss << "\t\"" << key(JsonKey::TmpDir) << "\": \"" << (std::filesystem::temp_directory_path() / "ls-tmp/Translate").generic_string() << "\", \n";
		ss << R"(
	key(JsonKey::Write): {
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
	auto& langs = pImpl->json[key(JsonKey::Languages)];

	std::vector<Language> result;

	for(auto s = langs.begin(); s != langs.end(); ++s)
	{
		auto& lang = s.value();
		if(pImpl->get(lang[key(JsonKey::Enable)], false) == false){ continue; }

		result.emplace_back(
			pImpl->get(lang[key(JsonKey::LanguageName)], "ja"s),
			FontData{
				utility::cnvStr<std::u8string>(pImpl->get(lang[key(JsonKey::FontName)], "VL Gothic"s)),
				pImpl->get(lang[key(JsonKey::FontSize)], 22)
			}
		);
	}

	return result;
}

std::string langscore::config::defaultLanguage() {
	return pImpl->get(pImpl->json[key(JsonKey::DefaultLanguage)], "ja"s);
}

std::u8string langscore::config::projectPath()
{
	return utility::cnvStr<std::u8string>(pImpl->get(pImpl->json[key(JsonKey::Project)], ""s));
}

std::u8string config::tempDirectorty() {
	return utility::cnvStr<std::u8string>(pImpl->get(pImpl->json[key(JsonKey::Analyze)][key(JsonKey::TmpDir)], (std::filesystem::temp_directory_path() / "Data/").string()));
}

std::string config::usScriptFuncComment(){
	return pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::UsCustomFuncComment)], "project://Scripts/{0}#{1},{2}"s);
}

std::vector<std::u8string> langscore::config::exportDirectory()
{
	auto baseFolder = pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::ExportDirectory)], ""s);
	if(this->exportByLanguage() == false){
		return {utility::cnvStr<std::u8string>(baseFolder)};
	}

	auto langs = this->languages();
	std::vector<std::u8string> result;
	for(auto& lang : langs){
		result.emplace_back(utility::cnvStr<std::u8string>(baseFolder + "/" + lang.name));
	}

	return result;
}

bool langscore::config::exportByLanguage()
{
	return pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::ExportByLang)], false);
}

std::u8string config::outputTranslateFilePathForRPGMaker(){
	return utility::cnvStr<std::u8string>(pImpl->get(pImpl->json[key(JsonKey::Analyze)][key(JsonKey::RPGMakerOutputPath)], "Translate/"s));
}

std::vector<config::BasicData> langscore::config::vxaceBasicData()
{
	auto& write = pImpl->json[key(JsonKey::Write)];
	auto& dataList = write[key(JsonKey::RPGMakerBasicData)];
	std::vector<BasicData> result;
	for(auto s = dataList.begin(); s != dataList.end(); ++s)
	{
		auto& script = s.value();
		BasicData data = {
			utility::cnvStr<std::u8string>(pImpl->get(script[key(JsonKey::Name)], ""s)),
			pImpl->get(script[key(JsonKey::Ignore)], false),
			0
		};
		if(data.filename.empty() == false){
			result.emplace_back(std::move(data));
		}
	}
	return result;
}

std::vector<config::ScriptData> langscore::config::vxaceScripts()
{
	auto& write = pImpl->json[key(JsonKey::Write)];
	auto& scripts = write[key(JsonKey::RPGMakerScripts)];
	std::vector<ScriptData> result;
	for(auto s = scripts.begin(); s != scripts.end(); ++s)
	{
		auto& script = s.value();
		ScriptData data = {
			utility::cnvStr<std::u8string>(pImpl->get(script[key(JsonKey::Name)], ""s)),
			pImpl->get(script[key(JsonKey::Ignore)], false),
			0,
			{}
		};
		auto& ignorePoints = script[key(JsonKey::IgnorePoints)];
		for(auto i = ignorePoints.begin(); i != ignorePoints.end(); ++i){
			auto& rc = i.value();
			data.texts.emplace_back(rc[key(JsonKey::Row)], rc[key(JsonKey::Col)],
									pImpl->get(rc[key(JsonKey::Disable)], false),
									pImpl->get(rc[key(JsonKey::Ignore)], false),
									pImpl->get(rc[key(JsonKey::WriteType)], 0),
									utility::cnvStr<std::u8string>(pImpl->get(rc[key(JsonKey::Text)], ""s)));
		}

		if(data.filename.empty() == false){
			result.emplace_back(std::move(data));
		}
	}
	return result;
}

utility::u8stringlist langscore::config::ignorePictures()
{
	auto& write = pImpl->json[key(JsonKey::Write)];
	auto& pictures = write[key(JsonKey::IgnorePictures)];
	utility::u8stringlist result;
	for(auto s = pictures.begin(); s != pictures.end(); ++s){
		result.emplace_back(utility::cnvStr<std::u8string>(pImpl->get(s.value(), ""s)));
	}
	return result;
}
