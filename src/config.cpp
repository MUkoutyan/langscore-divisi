#include "config.h"
#include "config.h"
#include "config.h"
#include "config.h"
#include "config.h"
#include "config.h"
#include "config.h"
#include "config.h"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>

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
	MAKE_KEYVALUE(PackingInputDir),
    MAKE_KEYVALUE(PackingEnablePerLang),
    MAKE_KEYVALUE(PackingPerLangOutputDir),
	MAKE_KEYVALUE(ApplicationVersion),
	MAKE_KEYVALUE(ConfigVersion),
	MAKE_KEYVALUE(AttachLsTransType),
	MAKE_KEYVALUE(ExportAllScriptStrings),
	MAKE_KEYVALUE(EnableLanguagePatch),
	MAKE_KEYVALUE(IsFirstExported),
	MAKE_KEYVALUE(Validate),
	MAKE_KEYVALUE(ValidateTextMode),
	MAKE_KEYVALUE(ValidateSizeList),
	MAKE_KEYVALUE(ValidateCSVList)
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
	config::ProjectType projectType = config::ProjectType::None;

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
			std::cerr << "Can't Open  config File : " << this->path;
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

std::u8string langscore::config::langscoreProjectPath()
{
	return pImpl->path.parent_path().u8string();
}

std::vector<config::Language> config::languages()
{
	auto& langs = pImpl->json[key(JsonKey::Languages)];

	std::vector<Language> result;


	std::string fontFamily = "VL Gothic"s;
	std::string fontFileName = "VL-Gothic-Regular.ttf"s;

	for(auto s = langs.begin(); s != langs.end(); ++s)
	{
		auto& lang = s.value();
		if(pImpl->get(lang[key(JsonKey::Enable)], false) == false){ continue; }


		result.emplace_back(
            lang[key(JsonKey::Enable)],
			pImpl->get(lang[key(JsonKey::LanguageName)], "ja"s),
			FontData{
				utility::cnvStr<std::u8string>(pImpl->get(lang[key(JsonKey::FontName)], fontFamily)),
				pImpl->get(lang[key(JsonKey::FontPath)], fontFileName),
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

std::u8string langscore::config::langscoreUpdateDirectorty()
{
	auto path = pImpl->toAbsolutePathWeak(u8"update"s);
	path.make_preferred();
	return path.u8string();
}

std::string config::usScriptFuncComment(){
	return pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::UsCustomFuncComment)], "project://Scripts/{0}#{1},{2}"s);
}

utility::u8stringlist langscore::config::exportDirectory(std::u8string& root)
{
	auto u8Path = utility::cnvStr<std::u8string>(pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::ExportDirectory)], ""s));
	u8Path = pImpl->toAbsolutePathWeak(u8Path).u8string();
	root = u8Path;

	if(this->exportByLanguage() == false){
		return {u8Path};
	}

	auto langs = this->languages();
	utility::u8stringlist result;
	for(auto& lang : langs){
		result.emplace_back(u8Path + utility::cnvStr<std::u8string>("/" + lang.name));
	}

	return result;
}

std::vector<std::pair<std::u8string, std::u8string>> langscore::config::exportDirectoryWithLang(std::u8string& root)
{
    if(this->exportByLanguage() == false) {
        return {};
    }

    auto u8Path = utility::cnvStr<std::u8string>(pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::ExportDirectory)], ""s));
    u8Path = pImpl->toAbsolutePathWeak(u8Path).u8string();
    root = u8Path;

    auto langs = this->languages();
    std::vector<std::pair<std::u8string, std::u8string>> result;
    for(auto& lang : langs) {
        auto u8Lang = utility::cnvStr<std::u8string>(lang.name);
        result.emplace_back(std::make_pair(u8Lang, u8Path + u8"/"s + u8Lang));
    }

    return result;
}


bool langscore::config::exportByLanguage()
{
    auto exportByLang = pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::ExportByLang)], false);
    auto enableLangPatch = pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::EnableLanguagePatch)], false);
    return exportByLang || enableLangPatch;
}

bool langscore::config::overwriteLangscore()
{
	return pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::OverwriteLangscore)], false);
}

config::ProjectType langscore::config::projectType()
{
	if(pImpl->projectType != ProjectType::None) {
		return pImpl->projectType;
	}

	const auto hasHeader = [](std::filesystem::path path, std::string_view headerText)
	{
		std::ifstream proj(path);
		if(proj.is_open()) {
			std::string header(5, ' ');
			proj.read(&header[0], 5);
			return header == headerText;
		}
		return false;
	};

	std::filesystem::directory_iterator it(gameProjectPath());
	for(auto& file : it) {
		auto ext = file.path().extension();
		if(ext == ".rvproj2") {
			pImpl->projectType = config::ProjectType::VXAce;
			break;
		}
		else if(ext == ".rmmzproject")
		{
			if(hasHeader(file.path(), "RPGMZ")) {
				pImpl->projectType = config::ProjectType::MZ;
				break;
			}
		}
		else if(ext == ".rpgproject")
		{
			if(hasHeader(file.path(), "RPGMV")) {
				pImpl->projectType = config::ProjectType::MV;
				break;
			}
		}
	}

	return pImpl->projectType;
}

bool langscore::config::overwriteLangscoreCustom()
{
	return pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::OverwriteLangscoreCustom)], false);
}

std::u8string config::outputTranslateFilePathForRPGMaker()
{
	return u8"Data/Translate"s;
}

std::u8string langscore::config::packingInputDirectory()
{
	return utility::cnvStr<std::u8string>(pImpl->get(pImpl->json[key(JsonKey::PackingInputDir)], ""s));
}

bool langscore::config::packingEnablePerLang()
{
    return pImpl->get(pImpl->json[key(JsonKey::PackingEnablePerLang)], false);
}

std::u8string langscore::config::packingPerLangOutputDir()
{
    return utility::cnvStr<std::u8string>(pImpl->get(pImpl->json[key(JsonKey::PackingPerLangOutputDir)], ""s));
}

std::vector<config::BasicData> langscore::config::rpgMakerBasicData()
{
	auto& write = pImpl->json[key(JsonKey::Write)];
	auto& dataList = write[key(JsonKey::RPGMakerBasicData)];
	std::vector<BasicData> result;
	for(auto s = dataList.begin(); s != dataList.end(); ++s)
	{
        auto& script = s.value();

        auto& sizeListlistData = script[key(JsonKey::ValidateSizeList)];
        std::vector<std::uint16_t> sizeList;
        for(auto s = sizeListlistData.begin(); s != sizeListlistData.end(); ++s) {
            sizeList.emplace_back(pImpl->get(s.value(), 0));
        }

        std::sort(sizeList.begin(), sizeList.end());
        //0ÇçÌèú
        sizeList.erase(std::remove(sizeList.begin(), sizeList.end(), 0), sizeList.end());

		BasicData data = {
			utility::cnvStr<std::u8string>(pImpl->get(script[key(JsonKey::Name)], ""s)),
			pImpl->get(script[key(JsonKey::Ignore)], false),
            0,	//writeMode
            pImpl->get(script[key(JsonKey::ValidateTextMode)], 0),  //textValidateMode
            std::move(sizeList)  //textValidateSize
		};
		if(data.filename.empty() == false){
			result.emplace_back(std::move(data));
		}
	}
	return result;
}

std::vector<config::ScriptData> langscore::config::rpgMakerScripts()
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
            0,	//writeMode,
            0,  //textValidateMode
            {}, //textValidateSize
			{}
		};
		auto& ignorePoints = script[key(JsonKey::IgnorePoints)];
		for(auto i = ignorePoints.begin(); i != ignorePoints.end(); ++i){
			auto& rc = i.value();
			data.texts.emplace_back(rc[key(JsonKey::Row)], rc[key(JsonKey::Col)],
									pImpl->get(rc[key(JsonKey::Disable)], false),
									pImpl->get(rc[key(JsonKey::Ignore)], false),
									pImpl->get(rc[key(JsonKey::WriteType)], -1),
									pImpl->get(script[key(JsonKey::AttachLsTransType)], 0),
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
	auto& fontPaths = write[key(JsonKey::FontPath)];
	auto& globals = fontPaths[key(JsonKey::Global)];
	utility::u8stringlist result;
	for(auto s = globals.begin(); s != globals.end(); ++s){
		result.emplace_back(utility::cnvStr<std::u8string>(pImpl->get(s.value(), ""s)));
	}
	return result;
}

utility::u8stringlist langscore::config::localFontList()
{
	auto& write = pImpl->json[key(JsonKey::Write)];
	auto& fontPaths = write[key(JsonKey::FontPath)];
	auto& locals = fontPaths[key(JsonKey::Local)];
	utility::u8stringlist result;
	for(auto s = locals.begin(); s != locals.end(); ++s){
		result.emplace_back(utility::cnvStr<std::u8string>(pImpl->get(s.value(), ""s)));
	}
	return result;
}

char langscore::config::globalWriteMode()
{
	auto& write = pImpl->json[key(JsonKey::Write)];
	return pImpl->get(write[key(JsonKey::WriteType)], (char)0);
}

bool langscore::config::enableLanguagePatch()
{
    return pImpl->get(pImpl->json[key(JsonKey::Write)][key(JsonKey::EnableLanguagePatch)], false);
}

//int langscore::config::validateTextMode()
//{
//    auto val = pImpl->get(pImpl->json[key(JsonKey::ValidateTextMode)], 0);
//    return val;
//}
//
//std::vector<int> langscore::config::validateSizeList()
//{
//    auto& list = pImpl->json[key(JsonKey::ValidateSizeList)];
//    std::vector<int> result;
//    for(auto s = list.begin(); s != list.end(); ++s) {
//        result.emplace_back(pImpl->get(s.value(), 0));
//    }
//
//    std::sort(result.begin(), result.end());
//    //0ÇçÌèú
//    result.erase(std::remove(result.begin(), result.end(), 0), result.end());
//
//    return result;
//}