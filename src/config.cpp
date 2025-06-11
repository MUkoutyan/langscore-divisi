#include "config.h"
#include "config.h"
#include "config.h"
#include "config.h"
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
        try {
            Value v;
            return j.get_to<Value>(v);
        }
        catch(...) {
            return defValue;
        }
        return defValue;
    }

    template<typename Value>
    Value get(const nlohmann::json& j, const char* key, Value defValue = Value())
    {
        if(j.contains(key)) {
            try {
                return j.value(key, defValue);
            }
            catch(...) {
                return defValue;
            }
        }
        return defValue;
    }

    template<typename Value>
    Value getNested(const nlohmann::json& j, std::initializer_list<const char*> keys, Value defValue = Value())
    {
        const nlohmann::json* current = &j;
        for(auto it = keys.begin(); it != keys.end(); ++it) {
            if(current->contains(*it)) {
                if(it == keys.end() - 1) {
                    try {
                        return current->value(*it, defValue);
                    }
                    catch(...) {
                        return defValue;
                    }
                }
                current = &(*current)[*it];
            }
            else {
                return defValue;
            }
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


const char* config::key(JsonKey key)
{
    return configKey(key);
}

void config::attachConfigFile(std::filesystem::path path){
	Impl::configPath = std::move(path);
}

void config::detachConfigFile(){
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

config::config()
	: pImpl(std::make_unique<Impl>())
{
	assert(jsonKeys.size() == size_t(JsonKey::NumKeys));
	if(Impl::configPath != ""){
		pImpl->load(Impl::configPath);
	}
}

config::~config()
{}

std::u8string config::langscoreProjectPath()
{
	return pImpl->path.parent_path().u8string();
}

std::vector<config::Language> config::allLanguages()
{
    auto langs = pImpl->get(pImpl->json, key(JsonKey::Languages), nlohmann::json::array());

    std::vector<Language> result;

    std::string fontFamily = "VL Gothic"s;
    std::string fontFileName = "VL-Gothic-Regular.ttf"s;

    for(auto s = langs.begin(); s != langs.end(); ++s)
    {
        auto& lang = s.value();
        result.emplace_back(
            lang[key(JsonKey::Enable)],
            pImpl->get(lang, key(JsonKey::LanguageName), "ja"s),
            FontData{
                utility::cnvStr<std::u8string>(pImpl->get(lang, key(JsonKey::FontName), fontFamily)),
                pImpl->get(lang, key(JsonKey::FontPath), fontFileName),
                pImpl->get(lang, key(JsonKey::FontSize), 22)
            }
        );
    }

    return result;
}

std::vector<config::Language> config::enableLanguages()
{
    std::vector<Language> result = this->allLanguages();
    auto itr = std::ranges::remove_if(result, [](const Language& lang) { return lang.isEnable == false; });
    result.erase(itr.begin(), itr.end());

    return result;
}

std::string config::defaultLanguage() {
    return pImpl->get(pImpl->json, key(JsonKey::DefaultLanguage), "ja"s);
}

std::u8string config::gameProjectPath()
{
    auto u8Path = utility::cnvStr<std::u8string>(pImpl->get(pImpl->json, key(JsonKey::Project), ""s));
    auto path = pImpl->toAbsolutePath(u8Path);
    path.make_preferred();

    if(std::filesystem::is_directory(path) == false) {
        path = path.parent_path();
    }

    return path.u8string();
}

std::u8string config::langscoreAnalyzeDirectorty() {
    auto u8Path = utility::cnvStr<std::u8string>(
        pImpl->getNested(pImpl->json, {key(JsonKey::Analyze), key(JsonKey::TmpDir)}, ""s)
    );
    auto path = pImpl->toAbsolutePathWeak(u8Path);
    path.make_preferred();
    return path.u8string();
}

std::u8string config::langscoreUpdateDirectorty()
{
    auto path = pImpl->toAbsolutePathWeak(u8"update"s);
    path.make_preferred();
    return path.u8string();
}


std::string config::usScriptFuncComment() {
    return pImpl->getNested(pImpl->json, {key(JsonKey::Write), key(JsonKey::UsCustomFuncComment)},
        "project://Scripts/{0}#{1},{2}"s);
}

utility::u8stringlist config::exportDirectory(std::u8string& root)
{
    auto u8Path = utility::cnvStr<std::u8string>(
        pImpl->getNested(pImpl->json, {key(JsonKey::Write), key(JsonKey::ExportDirectory)}, ""s)
    );
    u8Path = pImpl->toAbsolutePathWeak(u8Path).u8string();
    root = u8Path;

    if(this->exportByLanguage() == false) {
        return {u8Path};
    }

    auto langs = this->enableLanguages();
    utility::u8stringlist result;
    for(auto& lang : langs) {
        result.emplace_back(u8Path + utility::cnvStr<std::u8string>("/" + lang.name));
    }

    return result;
}

std::vector<std::pair<std::u8string, std::u8string>> config::exportDirectoryWithLang(std::u8string& root)
{
    if(this->exportByLanguage() == false) {
        return {};
    }

    auto u8Path = utility::cnvStr<std::u8string>(
        pImpl->getNested(pImpl->json, {key(JsonKey::Write), key(JsonKey::ExportDirectory)}, ""s)
    );
    u8Path = pImpl->toAbsolutePathWeak(u8Path).u8string();
    root = u8Path;

    auto langs = this->enableLanguages();
    std::vector<std::pair<std::u8string, std::u8string>> result;
    for(auto& lang : langs) {
        auto u8Lang = utility::cnvStr<std::u8string>(lang.name);
        result.emplace_back(std::make_pair(u8Lang, u8Path + u8"/"s + u8Lang));
    }

    return result;
}

bool config::exportByLanguage()
{
    auto exportByLang = pImpl->getNested(pImpl->json,
        {key(JsonKey::Write), key(JsonKey::ExportByLang)}, false);
    auto enableLangPatch = pImpl->getNested(pImpl->json,
        {key(JsonKey::Write), key(JsonKey::EnableLanguagePatch)}, false);
    return exportByLang || enableLangPatch;
}

bool config::enableTranslationDefLang()
{
    return pImpl->getNested(pImpl->json, {key(JsonKey::Write), key(JsonKey::EnableTranslationDefLang)}, false);
}

bool config::overwriteLangscore()
{
    return pImpl->getNested(pImpl->json,
        {key(JsonKey::Write), key(JsonKey::OverwriteLangscore)}, false);
}

std::u8string config::outputTranslateFilePathForRPGMaker()
{
    return u8"Data/Translate"s;
}

bool config::overwriteLangscoreCustom()
{
    return pImpl->getNested(pImpl->json,
        {key(JsonKey::Write), key(JsonKey::OverwriteLangscoreCustom)}, false);
}

config::ProjectType config::projectType()
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

    const auto checkFile = [&](const std::filesystem::path& path)
    {
        auto ext = path.extension();
        if(ext == ".rvproj2") {
            return config::ProjectType::VXAce;
        }
        else if(ext == ".rmmzproject")
        {
            if(hasHeader(path, "RPGMZ")) {
                return config::ProjectType::MZ;
            }
        }
        else if(ext == ".rpgproject")
        {
            if(hasHeader(path, "RPGMV")) {
                return config::ProjectType::MV;
            }
        }

        return config::ProjectType::None;
    };

    auto projectPath = gameProjectPath();
    if(std::filesystem::is_directory(projectPath))
    {
        std::filesystem::directory_iterator it(projectPath);
        for(auto& file : it) {
            auto type = checkFile(file.path());
            if(type != config::ProjectType::None) {
                pImpl->projectType = type;
                break;
            }
        }
    }
    else {
        pImpl->projectType = checkFile(std::filesystem::path(projectPath));
    }

    return pImpl->projectType;
}


std::u8string config::packingInputDirectory()
{
    return utility::cnvStr<std::u8string>(
        pImpl->get(pImpl->json, key(JsonKey::PackingInputDir), ""s)
    );
}

bool config::packingEnablePerLang()
{
    return pImpl->get(pImpl->json, key(JsonKey::PackingEnablePerLang), false);
}

std::u8string config::packingPerLangOutputDir()
{
    return utility::cnvStr<std::u8string>(
        pImpl->get(pImpl->json, key(JsonKey::PackingPerLangOutputDir), ""s)
    );
}

std::vector<config::BasicData> config::rpgMakerBasicData()
{
    auto write = pImpl->get(pImpl->json, key(JsonKey::Write), nlohmann::json::object());
    auto dataList = pImpl->get(write, key(JsonKey::RPGMakerBasicData), nlohmann::json::array());
    std::vector<BasicData> result;
    for(auto s = dataList.begin(); s != dataList.end(); ++s)
    {
        auto& script = s.value();

        TextValidateTypeMap validateTextInfo;
        if(script.contains(key(JsonKey::ValidateTextCategory)))
        {
            const auto& infos = script[key(JsonKey::ValidateTextCategory)];
            for(auto validateItr = infos.begin(); validateItr != infos.end(); ++validateItr)
            {
                auto validateInfoObj = validateItr.value();
                std::string name = pImpl->get(validateInfoObj, key(JsonKey::Name), ""s);
                TextValidationLangMap langMap;
                for(auto& langEntry : validateInfoObj.items())
                {
                    if(langEntry.key() == key(JsonKey::Name)) {
                        continue;
                    }
                    auto& langInfo = langEntry.value();
                    config::ValidateTextInfo infosTemp;
                    infosTemp.mode = static_cast<ValidateTextMode>(
                        pImpl->get(langInfo, key(JsonKey::ValidateTextMode), 0)
                        );
                    auto sizeList = pImpl->get(langInfo, key(JsonKey::ValidateSizeList), nlohmann::json::object());
                    infosTemp.width = pImpl->get(sizeList, key(JsonKey::ValidateTextWidth), 0);
                    infosTemp.count = pImpl->get(sizeList, key(JsonKey::ValidateTextLength), 0);
                    langMap[utility::cnvStr<std::u8string>(langEntry.key())] = std::move(infosTemp);
                }
                validateTextInfo[utility::cnvStr<std::u8string>(name)] = std::move(langMap);
            }
        }

        BasicData data = {
            utility::cnvStr<std::u8string>(pImpl->get(script, key(JsonKey::Name), ""s)),
            pImpl->get(script, key(JsonKey::Ignore), false),
            0,	//writeMode
            std::move(validateTextInfo)  //textValidateSize
        };
        if(data.filename.empty() == false) {
            result.emplace_back(std::move(data));
        }
    }
    return result;
}


std::vector<config::ScriptData> config::rpgMakerScripts()
{
    auto write = pImpl->get(pImpl->json, key(JsonKey::Write), nlohmann::json::object());
    auto scripts = pImpl->get(write, key(JsonKey::RPGMakerScripts), nlohmann::json::array());
    std::vector<ScriptData> result;
    for(auto s = scripts.begin(); s != scripts.end(); ++s)
    {
        auto& script = s.value();
        ScriptData data = {
            utility::cnvStr<std::u8string>(pImpl->get(script, key(JsonKey::Name), ""s)),
            pImpl->get(script, key(JsonKey::Ignore), false),
            0,	//writeMode,
            {}, //textValidateSize
        };
        auto ignorePoints = pImpl->get(script, key(JsonKey::IgnorePoints), nlohmann::json::array());
        for(auto i = ignorePoints.begin(); i != ignorePoints.end(); ++i) {
            auto& rc = i.value();
            data.texts.emplace_back(
                pImpl->get(rc, key(JsonKey::Row), 0),
                pImpl->get(rc, key(JsonKey::Col), 0),
                pImpl->get(rc, key(JsonKey::Disable), false),
                pImpl->get(rc, key(JsonKey::Ignore), false),
                pImpl->get(rc, key(JsonKey::WriteType), -1),
                pImpl->get(script, key(JsonKey::AttachLsTransType), 0),
                utility::cnvStr<std::u8string>(pImpl->get(rc, key(JsonKey::Text), ""s))
            );
        }

        if(data.filename.empty() == false) {
            result.emplace_back(std::move(data));
        }
    }
    return result;
}

utility::u8stringlist config::ignorePictures()
{
    auto write = pImpl->get(pImpl->json, key(JsonKey::Write), nlohmann::json::object());
    auto pictures = pImpl->get(write, key(JsonKey::IgnorePictures), nlohmann::json::array());
    utility::u8stringlist result;
    for(auto s = pictures.begin(); s != pictures.end(); ++s) {
        result.emplace_back(utility::cnvStr<std::u8string>(pImpl->get(s.value(), ""s)));
    }
    return result;
}

utility::u8stringlist config::globalFontList()
{
    auto write = pImpl->get(pImpl->json, key(JsonKey::Write), nlohmann::json::object());
    auto fontPaths = pImpl->get(write, key(JsonKey::FontPath), nlohmann::json::object());
    auto globals = pImpl->get(fontPaths, key(JsonKey::Global), nlohmann::json::array());
    utility::u8stringlist result;
    for(auto s = globals.begin(); s != globals.end(); ++s) {
        result.emplace_back(utility::cnvStr<std::u8string>(pImpl->get(s.value(), ""s)));
    }
    return result;
}

utility::u8stringlist config::localFontList()
{
    auto write = pImpl->get(pImpl->json, key(JsonKey::Write), nlohmann::json::object());
    auto fontPaths = pImpl->get(write, key(JsonKey::FontPath), nlohmann::json::object());
    auto locals = pImpl->get(fontPaths, key(JsonKey::Local), nlohmann::json::array());
    utility::u8stringlist result;
    for(auto s = locals.begin(); s != locals.end(); ++s) {
        result.emplace_back(utility::cnvStr<std::u8string>(pImpl->get(s.value(), ""s)));
    }
    return result;
}

utility::u8stringlist config::extendControlCharList()
{
    auto validate = pImpl->get(pImpl->json, key(JsonKey::Validate), nlohmann::json::object());
    auto controlChars = pImpl->get(validate, key(JsonKey::ControlCharList), nlohmann::json::array());
    utility::u8stringlist result;
    for(auto s = controlChars.begin(); s != controlChars.end(); ++s) {
        result.emplace_back(utility::cnvStr<std::u8string>(pImpl->get(s.value(), ""s)));
    }
    return result;
}

char config::globalWriteMode()
{
    auto write = pImpl->get(pImpl->json, key(JsonKey::Write), nlohmann::json::object());
    return pImpl->get(write, key(JsonKey::WriteType), (char)0);
}

bool config::enableLanguagePatch()
{
    return pImpl->getNested(pImpl->json,
        {key(JsonKey::Write), key(JsonKey::EnableLanguagePatch)}, false);
}

//int config::validateTextMode()
//{
//    auto val = pImpl->get(pImpl->json[key(JsonKey::ValidateTextMode)], 0);
//    return val;
//}
//
//std::vector<int> config::validateSizeList()
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