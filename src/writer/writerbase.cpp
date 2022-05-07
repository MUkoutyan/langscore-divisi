#include "writerbase.h"
#include <fstream>

namespace
{

	static const std::vector<std::u8string> ignoreKeys = {
		u8"json_class", u8"@note", u8"@character_name", 
		u8"@self_switch_ch", u8"@switches", u8"@title1_name", u8"@variables"
	};
	static const std::map<std::u8string, std::vector<std::u8string>> ignoreForClassKeys = {
		{u8"RPG::UsableItem::Damage", {u8"@formula"}},
		{u8"RPG::Event",  {u8"@name"}},
		{u8"RPG::SE",     {u8"@name"}},
		{u8"RPG::ME",     {u8"@name"}},
		{u8"RPG::BGM",    {u8"@name"}},
		{u8"RPG::Map",    {u8"@battleback1_name", u8"@battleback2_name"}},
		{u8"RPG::Troop",  {u8"@name"}},
		{u8"RPG::Actor",  {u8"@face_name"}},
		{ u8"RPG::System",    {u8"@battleback1_name", u8"@battleback2_name"} },
	};

}

writerbase::writerbase(std::vector<std::string> locales, const nlohmann::json& json)
	: useLangList(std::move(locales))
{
	json2tt(json);
}

writerbase::~writerbase()
{}

void writerbase::writeU8String(std::ofstream& out, std::u8string text)
{
	out.write(reinterpret_cast<const char*>(text.c_str()), text.size());
}

void writerbase::addText(const nlohmann::json& json, std::u8string note)
{
	std::string valStr;
	json.get_to(valStr);
	if(valStr.empty()){ return; }

	if(valStr.find('\n') != std::string::npos){
		valStr = "\"" + valStr + "\"";
	}
	std::u8string original(valStr.begin(), valStr.end());

	TranslateText t;
	t.original = std::move(original);
	t.note = std::move(note);
	for(auto& lang : useLangList){
		t.translates[lang] = u8"";
	}
	auto result = std::find_if(texts.begin(), texts.end(), [&t](const auto& x){
		return x.original == t.original;
	});

	if(result == texts.end()){
		texts.emplace_back(std::move(t));
	}
	else{
		//        result->note += std::string("同じテキストが複数で使用されています");
	}
}

void writerbase::json2tt(const nlohmann::json& json)
{
	if(json.is_array())
	{
		convertJArray(json);
	}
	else if(json.is_object())
	{
		convertJObject(json);
	}
}

std::tuple<std::u8string, bool> writerbase::getObjectClass(const nlohmann::json& root)
{
	bool hasSpecIgnoreKeys = false;
	std::u8string currentClassName = u8"";
	if(root.find("json_class") != root.end())
	{
		const std::string _currentClassName = root["json_class"];
		currentClassName = std::u8string(_currentClassName.begin(), _currentClassName.end());
		if(currentClassName != u8"")
		{
			hasSpecIgnoreKeys = ignoreForClassKeys.find(currentClassName) != ignoreForClassKeys.end();
		}
	}

	return std::forward_as_tuple(currentClassName, hasSpecIgnoreKeys);
}

bool writerbase::checkIgnoreKey(const std::u8string& currentClassName, const std::u8string& key, bool hasSpecIgnoreKeys)
{
	if(std::find(ignoreKeys.cbegin(), ignoreKeys.cend(), key) != ignoreKeys.cend()){ return true; }
	if(hasSpecIgnoreKeys){
		const auto& fields = ignoreForClassKeys.at(currentClassName);
		if(std::find(fields.cbegin(), fields.cend(), key) != fields.cend()){
			return true;
		}
	}
	return false;
}

bool writerbase::checkEventCommandCode(const nlohmann::json& obj)
{
	for(auto s = obj.begin(); s != obj.end(); ++s)
	{
		if(s.key() == "@code"){
			//許可するコード
			int v = 0;
			s->get_to(v);
			switch(v){
				case 102: //選択肢
				case 401: //文章の表示
				case 231: //CGの表示
					return true;
				default:
					break;
			}
			if(v == 401){
				return true;
			}
			else if(v == 231){
				return true;
			}
		}
	}
	return false;
}

void writerbase::convertJArray(const nlohmann::json& arr, std::u8string parentClass, std::u8string arrayinKey)
{
	bool hasSpecIgnoreKeys = ignoreForClassKeys.find(parentClass) != ignoreForClassKeys.end();
	for(auto s = arr.begin(); s != arr.end(); ++s)
	{
		if(s->is_array()){
			convertJArray(*s, parentClass, arrayinKey);
			continue;
		}
		else if(s->is_object()){
			convertJObject(*s);
		}
		else if(s->is_string())
		{
			if(checkIgnoreKey(parentClass, arrayinKey, hasSpecIgnoreKeys)){ continue; }
			addText(*s, parentClass + u8"=>" + arrayinKey);
		}
	}
}

void writerbase::convertJObject(const nlohmann::json& root)
{
	if(root.empty()){ return; }

	auto [currentClassName, hasSpecIgnoreKeys] = getObjectClass(root);

	if(currentClassName == u8"RPG::EventCommand"){
		if(checkEventCommandCode(root) == false){ return; }
	}

	for(auto s = root.begin(); s != root.end(); ++s)
	{
		const auto& _key = s.key();
		std::u8string key(_key.begin(), _key.end());
		const auto& val = s.value();
		if(val.is_array()){
			convertJArray(val, currentClassName, key);
			continue;
		}
		else if(val.is_object()){
			convertJObject(val);
			continue;
		}
		else if(val.is_string() == false){ continue; }

		if(checkIgnoreKey(currentClassName, key, hasSpecIgnoreKeys)){
			continue;
		}

		addText(*s, currentClassName + u8":" + key);
	}
}
