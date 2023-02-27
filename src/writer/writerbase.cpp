#include "writerbase.h"
#include "utility.hpp"
#include <fstream>
#include "scripttextparser.hpp"

using namespace langscore;
using namespace std::string_literals;

writerbase::writerbase(std::vector<std::u8string> langs, const std::unique_ptr<jsonreaderbase>& json)
	: writerbase(std::move(langs), std::vector<TranslateText>{})
{
	json->json2tt(this->useLangs);
	this->texts = json->texts;

	//#ifdef _DEBUG
	//	ReplaceDebugTextByOrigin(this->texts);
	//#endif
}

writerbase::writerbase(std::vector<std::u8string> langs, std::vector<TranslateText> texts)
	: useLangs(std::move(langs))
	, texts(std::move(texts))
	, overwriteMode(MergeTextMode::AcceptSource)
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
		if(isContinue == ProgressNextStep::Next){
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

