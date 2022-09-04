#include "config.h"
#include "config.h"
#include "config.h"
#include "config.h"
#include "config.h"
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
	MAKE_KEYVALUE(FontPath),
	MAKE_KEYVALUE(Global),
	MAKE_KEYVALUE(Local),
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
	MAKE_KEYVALUE(OverwriteLangscore),
	MAKE_KEYVALUE(OverwriteLangscoreCustom),
	MAKE_KEYVALUE(ApplicationVersion),
	MAKE_KEYVALUE(ConfigVersion),
};

const char* config::key(JsonKey key)
{
	if(jsonKeys.find(key) == jsonKeys.end()){ return nullptr; }
	return jsonKeys[key];
}

class config::Impl
{
public:
	static nlohmann::json json;
	static std::filesystem::path configPath;
	std::filesystem::path path;

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
		this->path = std::move(path);
		if(json.empty() == false){ return; }

		std::ifstream loadFile(this->path);
		if(loadFile.good()){
			loadFile >> json;
		}
		else{
			throw "Can't Open File";
		}
	}

	std::filesystem::path toAbsolutePathWeak(std::filesystem::path p)
	{
		if(p.is_relative()){
			auto joinPath = this->path.parent_path() / p;
			p = std::filesystem::weakly_canonical(joinPath);
		}
		return p;
	}
	std::filesystem::path toAbsolutePath(std::filesystem::path p)
	{
		if(p.is_relative()){
			auto joinPath = this->path.parent_path() / p;
			p = std::filesystem::canonical(joinPath);
		}
		return p;
	}

};
std::filesystem::path config::Impl::configPath = "";
nlohmann::json config::Impl::json;


void langscore::config::attachConfigFile(std::filesystem::path path){
	Impl::configPath = std::move(path);
}

void langscore::config::detachConfigFile(){
	Impl::configPath = "";
	Impl::json.clear();
}

config::config(std::filesystem::path path)
	: pImpl(std::make_unique<Impl>())
{
	assert(jsonKeys.size() == size_t(JsonKey::NumKeys));
	if(Impl::configPath != ""){
		path = Impl::configPath;
	}

	pImpl->load(std::move(path));
}

langscore::config::config()
	: pImpl(std::make_unique<Impl>())
{
	assert(jsonKeys.size() == size_t(JsonKey::NumKeys));
	if(Impl::configPath != ""){
		pImpl->load(Impl::configPath);
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

std::u8string langscore::config::gameProjectPath()
{
	auto u8Path = utility::cnvStr<std::u8string>(pImpl->get(pImpl->json[key(JsonKey::Project)], ""s));
	auto path = pImpl->toAbsolutePath(u8Path);
	path.make_preferred();
	return path.u8string();
}

std::u8string config::langscoreAnalyzeDirectorty() {
	auto u8Path = utility::cnvStr<std::u8string>(pImpl->get(pImpl->json[key(JsonKey::Analyze)][key(JsonKey::TmpDir)], ""s));
	auto path = pImpl->toAbsolutePathWeak(u8Path);
	path.make_preferred();
	return path.u8string();
}

std::string config::usScriptFuncComment(){
	return pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::UsCustomFuncComment)], "project://Scripts/{0}#{1},{2}"s);
}

std::vector<std::u8string> langscore::config::exportDirectory()
{
	auto u8Path = utility::cnvStr<std::u8string>(pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::ExportDirectory)], ""s));
	u8Path = pImpl->toAbsolutePathWeak(u8Path).u8string();

	if(this->exportByLanguage() == false){
		return {u8Path};
	}

	auto langs = this->languages();
	std::vector<std::u8string> result;
	for(auto& lang : langs){
		result.emplace_back(u8Path + utility::cnvStr<std::u8string>("/" + lang.name));
	}

	return result;
}

bool langscore::config::exportByLanguage()
{
	return pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::ExportByLang)], false);
}

bool langscore::config::overwriteLangscore()
{
	return pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::OverwriteLangscore)], false);
}

bool langscore::config::overwriteLangscoreCustom()
{
	return pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::OverwriteLangscoreCustom)], false);
}

std::u8string config::outputTranslateFilePathForRPGMaker()
{
	return u8"Data/Translate"s;
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

utility::u8stringlist langscore::config::globalFontList()
{
	auto& write = pImpl->json[key(JsonKey::Write)];
	auto& globals = write[key(JsonKey::Global)];
	utility::u8stringlist result;
	for(auto s = globals.begin(); s != globals.end(); ++s){
		result.emplace_back(utility::cnvStr<std::u8string>(pImpl->get(s.value(), ""s)));
	}
	return result;
}

utility::u8stringlist langscore::config::localFontList()
{
	auto& write = pImpl->json[key(JsonKey::Write)];
	auto& locals = write[key(JsonKey::Local)];
	utility::u8stringlist result;
	for(auto s = locals.begin(); s != locals.end(); ++s){
		result.emplace_back(utility::cnvStr<std::u8string>(pImpl->get(s.value(), ""s)));
	}
	return result;
}

