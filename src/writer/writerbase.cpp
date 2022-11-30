#include "writerbase.h"
#include "utility.hpp"
#include <fstream>
#include "scripttextparser.hpp"

using namespace langscore;
using namespace std::string_literals;
namespace
{
	//共通して無視するキー
	static const std::vector<std::u8string> ignoreKeys = {
		u8"class", u8"@note", u8"@character_name", 
		u8"@self_switch_ch", u8"@switches", u8"@title1_name", u8"@variables"
	};
	//特定のクラスに存在する、無視するキー
	static const std::map<std::u8string, std::vector<std::u8string>> ignoreForClassKeys = {
		{u8"RPG::UsableItem::Damage", {u8"@formula"}},
		{u8"RPG::Event",  {u8"@name"}},
		{u8"RPG::SE",     {u8"@name"}},
		{u8"RPG::ME",     {u8"@name"}},
		{u8"RPG::BGM",    {u8"@name"}},
		{u8"RPG::BGS",    {u8"@name"}},
		{u8"RPG::Map",    {u8"@battleback1_name", u8"@battleback2_name", u8"@parallax_name"}},
		{u8"RPG::Troop",  {u8"@name"}},
		{u8"RPG::Actor",  {u8"@face_name"}},
		{ u8"RPG::System",    {u8"@battleback1_name", u8"@battleback2_name"} },
	};


}

writerbase::writerbase(std::vector<std::u8string> langs, const nlohmann::json& json)
	: writerbase(std::move(langs), std::vector<TranslateText>{})
{
	json2tt(json);
//#ifdef _DEBUG
//	ReplaceDebugTextByOrigin(this->texts);
//#endif
}

writerbase::writerbase(std::vector<std::u8string> langs, std::vector<TranslateText> texts)
	: useLangs(std::move(langs))
	, texts(std::move(texts))
	, overwriteMode(MergeTextMode::AcceptSource)
	, stackText(false)
	, stackTextStr(u8"")
	, rangeComment(false)
{
}

writerbase::~writerbase()
{}

#ifdef _DEBUG
void writerbase::ReplaceDebugTextByOrigin(std::vector<TranslateText>& transTexts)
{
	for(auto& txt : transTexts)
	{
		auto origin = txt.original;
		for(auto& tl : txt.translates)
		{
			auto t = tl.second;
			t = origin;
			if(t[0] == u8'\"'){
				t.insert(1, tl.first + u8"-");
			}
			else {
				t = tl.first + u8"-" + t;
			}
			tl.second = t;
		}
	}
}

void writerbase::ReplaceDebugTextByLang(std::vector<TranslateText>& transTexts, std::u8string def_lang)
{
	for(auto& txt : transTexts)
	{
		auto origin = txt.translates[def_lang];
		for(auto& tl : txt.translates)
		{
			auto t = tl.second;
			if(def_lang != tl.first)
			{
				t = origin;
				if(t[0] == u8'\"'){
					t.insert(1, tl.first + u8"-");
				}
				else {
					t = tl.first + u8"-" + t;
				}
			}
			tl.second = t;
		}
	}
}
#endif

void writerbase::writeU8String(std::ofstream& out, std::u8string text)
{
	out.write(reinterpret_cast<const char*>(text.c_str()), text.size());
}

void writerbase::addText(const nlohmann::json& json, std::u8string note)
{
	std::string valStr;
	json.get_to(valStr);

	//手動の中央揃え等で空行を使用する場合があるため、空文字の検出は行わない。
	//if(valStr.empty()){ return; }

	std::u8string original(valStr.begin(), valStr.end());
	addText(std::move(original), std::move(note));
}

void writerbase::addText(std::u8string text, std::u8string note)
{
	if(stackText){
		//1行に付き必ず改行が挟まる。(VXAceのみの仕様？MV/MZは要確認)
		stackTextStr += text + u8'\n';
		return;
	}
	else {
		if(text.empty()){
			//ただの空文は無視する。
			return;
		}
	}

	bool wrapDq = false;
	if(text.find(u8'\n') != std::u8string::npos){
		wrapDq = true;
	}
	if(text.find(u8'\"') != std::u8string::npos){
		wrapDq = true;
		text = utility::replace<std::u8string>(text, u8"\"", u8"\"\"");
	}

	if(wrapDq){
		text = u8"\"" + text + u8"\"";
	}

	TranslateText t(std::move(text), useLangs);
	auto result = std::find_if(texts.begin(), texts.end(), [&t](const auto& x){
		return x.original == t.original;
	});

	if(result == texts.end()){
		texts.emplace_back(std::move(t));
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
	if(root.find("class") != root.end())
	{
		const std::string _currentClassName = root["class"];
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

std::tuple<bool, int> writerbase::checkEventCommandCode(const nlohmann::json& obj)
{
	bool result = false;
	int code = 0;
	for(auto s = obj.begin(); s != obj.end(); ++s)
	{
		if(s.key() == "@code"){
			//許可するコード
			s->get_to(code);
			switch(code){
				case 102: //選択肢
				case 401: //文章の表示
				case 405: //文章のスクロール表示
				//case 231: //画像の表示
					result = true;
				default:
					break;
			}
		}

		if(result){ break; }
	}
	return std::forward_as_tuple(result, code);
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
		auto [result, code] = checkEventCommandCode(root);
		if(stackText == false && (code == 401 || code == 405)){
			stackText = true;
		}
		else if(stackText && (code != 401 && code != 405)){
			stackText = false;
			addText(stackTextStr);
			stackTextStr.clear();
		}
		if(result == false){ return; }
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

std::vector<TranslateText> writerbase::convertScriptToCSV(std::filesystem::path path)
{
	size_t lineCount = 0;

	std::vector<TranslateText> transTextList;

	std::ifstream loadFile(path);
	if(loadFile.is_open() == false){ return {}; }
	auto fileName = path.filename().stem();

	//スクリプト内の文字列の抜き出し
	ScriptTextParser scriptParser;
	rangeComment = false;
	while(loadFile.eof() == false)
	{
		lineCount++;
		std::string lineTemp;
		std::getline(loadFile, lineTemp);
		auto line = utility::cnvStr<std::u8string>(lineTemp);

		auto isContinue = checkCommentLine(line);
		if(isContinue == ProgressNextStep::Continue){
			continue;
		}
		else if(isContinue == ProgressNextStep::Break){
			break;
		}

		auto dqResult = scriptParser.findStrings(line);

		auto lineCountStr = std::to_string(lineCount);
		std::u8string u8lineCount(lineCountStr.begin(), lineCountStr.end());

		for(auto& str : dqResult)
		{			
			auto colCountStr = std::to_string(std::get<1>(str));
			std::u8string u8ColCountStr(colCountStr.begin(), colCountStr.end());

			//文字列内のクォーテーションは、ツクール側のスクリプトで
			//エスケープ文字に誤認される可能性があるため削除する。
			auto original = utility::replace(std::get<0>(str), u8"\\\'"s, u8"\'"s);
			original = utility::replace(original, u8"\\\""s, u8"\""s);
			langscore::TranslateText t = {
				original,
				this->useLangs
			};
			auto scriptPos = fileName.u8string() + u8":" + u8lineCount + u8":" + u8ColCountStr;
			t.scriptLineInfo = scriptPos;
			transTextList.emplace_back(std::move(t));
		}
	}
	return transTextList;

}

