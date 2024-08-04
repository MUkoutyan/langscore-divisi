#include "readerbase.h"
#include "scripttextparser.hpp"
#include <algorithm>

using namespace langscore;

readerbase::readerbase(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList)
	: pluginInfoList()
	, useLangList(std::move(langs)), scriptFileList(std::move(scriptFileList))
	, texts()
	, scriptTranslatesMap()
	, lineComment(u8"//"), rangeCommentBegin(u8"/*"), rangeCommentEnd(u8"*/")
{
}

readerbase::~readerbase()
{}

void readerbase::setComment(TextCodec line, TextCodec begin, TextCodec end)
{
	this->lineComment = std::move(line);
	this->rangeCommentBegin = std::move(begin);
	this->rangeCommentEnd = std::move(end);
}

readerbase::ProgressNextStep readerbase::replaceCommentToSpace(TextCodec& line, bool& inRangeComment) const 
{
	//コメント行かをチェック
	//空行しか無いので無視
	if(line.empty()){ return ProgressNextStep::NextLine; }

	auto lineCommentResult = this->checkLineComment(line);
	if(lineCommentResult != ProgressNextStep::Throught){
		return lineCommentResult;
	}

	size_t rangeCommentBeginPos = 0;
	auto rangeCommentResult = this->checkRangeComment(line, inRangeComment, rangeCommentBeginPos);
	if(rangeCommentResult != ProgressNextStep::Throught){
		return rangeCommentResult;
	}

	//langscore関数の無視
	ScriptTextParser textParser;
	const std::u8string_view transFuncName = u8".lstrans";
	auto pos = line.find(transFuncName);
	for(; pos != TextCodec::npos; pos = line.find(transFuncName))
	{
		auto quotePos = pos + transFuncName.length();
		auto quote = u8'\"';
		constexpr auto sq = u8'\'';
		//Rubyは(を書かなくてもいいので2通り見る
		bool useBracket = false;
		if(line[quotePos] == sq){ quote = sq; }
		else if(line[quotePos] == u8'('){
			quotePos++;
			useBracket = true;
			if(line[quotePos] == sq){
				quote = sq;
			}
		}
		auto endQuotePos = line.find_first_of(quote, quotePos + 1) + 1;
		if(useBracket){ endQuotePos++; }
		//assert(endQuotePos != TextCodec::npos && "There's a weird script!");

		//列数がズレるのが困るので、削除ではなく空白で置換
		//UTF8も考慮して、マルチバイトを考慮した文字数を置換する。
		const auto detectLength = endQuotePos - pos;
		auto detectStr = line.substr(pos, detectLength);
		auto numText = textParser.ConvertWordList(detectStr);
		TextCodec space(numText.size(), u8' ');
		line.replace(pos, detectLength, space);
	}
	if(line.empty()){ ProgressNextStep::NextLine; }

	//全て空白か制御文字で構成されている。文字列の検出なので次の行に移る。
	if(isValidProgramLine(line) == false){
		return ProgressNextStep::NextLine;
	}

	return ProgressNextStep::Throught;
}

readerbase::ProgressNextStep readerbase::checkRangeComment(TextCodec& line, bool& inRangeComment, size_t beginPos) const
{
	if(inRangeComment)
	{
		//範囲コメント内の場合の処理
		//コメント開始部分が検出済み。
		auto pos = line.find(this->rangeCommentEnd);
		if(pos == TextCodec::npos){
			return ProgressNextStep::NextLine;
		}

		inRangeComment = false;
		std::fill_n(line.begin() + beginPos, pos - beginPos + this->rangeCommentEnd.size(), TextCodecChar(' '));

		//line.replace(beginPos, pos - beginPos + this->rangeCommentEnd.size(), TextCodec{' '});
		return line.empty() ? ProgressNextStep::NextLine : this->checkRangeComment(line, inRangeComment, beginPos);
	}

	//範囲コメント外の場合の処理
	auto pos = line.find(this->rangeCommentBegin, beginPos);
	if(pos == TextCodec::npos){ return ProgressNextStep::Throught; }

	//文字列内の場合はコメントが無視されるため、文字列チェックも行う。
	TextCodecChar findQuote = findBeginEnclose(line, pos);

	//コメント文が文字列に含まれている
	if(findQuote != TextCodecChar{})
	{
		beginPos = pos + this->rangeCommentBegin.size();
		if(line.find(findQuote, beginPos) == TextCodec::npos){
			return this->checkRangeComment(line, inRangeComment, beginPos);
		}
		return ProgressNextStep::Throught;
	}

	auto endPos = line.find(this->rangeCommentEnd, pos);
	if(endPos == TextCodec::npos){
		inRangeComment = true;
		//行の途中で範囲コメントされていた場合の対応。
		std::fill_n(line.begin() + pos, line.size() - pos, TextCodecChar(' '));
		if(isValidProgramLine(line) == false) {
			return ProgressNextStep::NextLine;
		}
		return line.empty() ? ProgressNextStep::NextLine : ProgressNextStep::Throught;
	}

	std::fill_n(line.begin() + pos, endPos - pos + this->rangeCommentEnd.size(), TextCodecChar(' '));
	return line.empty() ? ProgressNextStep::NextLine : this->checkRangeComment(line, inRangeComment, beginPos);

}

readerbase::ProgressNextStep readerbase::checkLineComment(TextCodec& line) const
{
	size_t startTextPos = 0;
	for(size_t i = 0; const auto & c : line) {
		if(!std::iscntrl(c) && !std::isspace(c)) {
			startTextPos = i;
			break;
		}
		i++;
	}

	//先頭が行コメントなら無視する
	auto lineCommentBegin = line.find(this->lineComment);
	if(lineCommentBegin != TextCodec::npos){
		if(line.substr(startTextPos, this->lineComment.size()) == this->lineComment){
			return ProgressNextStep::NextLine;
		}
		else{
			std::fill_n(line.begin() + lineCommentBegin, line.size() - lineCommentBegin, TextCodecChar(' '));
		}
	}
	if(isValidProgramLine(line) == false){
		return ProgressNextStep::NextLine;
	}
	return line.empty() ? ProgressNextStep::NextLine : ProgressNextStep::Throught;
}

readerbase::TextCodecChar readerbase::findBeginEnclose(TextCodecView text, size_t endPos) const
{
	assert(endPos < text.size());
	TextCodecChar findQuote = {};
	for(size_t i = 0; i < endPos; ++i)
	{
		//文字列検出。空文字なら初めてヒットするクオートを検出
		if(findQuote == TextCodecChar{}){
			if(text[i] == u8'\"' || text[i] == u8'\''){ findQuote = text[i]; }
		}
		//クオート検出済みなら先頭と同じものが来るまで待機
		else if(findQuote == text[i]){ findQuote = TextCodecChar{}; }
	}
	return findQuote;
}

bool readerbase::isValidProgramLine(TextCodecView line) const
{
	if(line.empty()){ return false; }
	return std::ranges::all_of(line, [](auto ch) {
		return std::isspace(static_cast<unsigned char>(ch)) || std::iscntrl(static_cast<unsigned char>(ch));
	}) == false;
}

std::vector<TranslateText> readerbase::convertScriptToCSV(std::filesystem::path path) const
{
	using namespace std::string_literals;

	std::ifstream loadFile(path);
	if(loadFile.is_open() == false){ return {}; }
	auto fileName = path.filename().stem();

	std::u8string defaultLanguage;
	{
		config config;
		defaultLanguage = utility::cnvStr<std::u8string>(config.defaultLanguage());
	}

	ScriptTextParser scriptParser;
	std::vector<TranslateText> transTextList;
	bool rangeComment = false;
	size_t lineCount = 0;
	//スクリプト内の文字列を1行ずつ抜き出し
	while(loadFile.eof() == false)
	{
		lineCount++;
		std::string lineTemp;
		std::getline(loadFile, lineTemp);
		auto line = utility::cnvStr<std::u8string>(lineTemp);

		//※行頭の空白を削除すると識別用の列番号がズレるため何もしない。
		//同様に、コメント部分の処理は削除ではなく空白への置き換えにする。

		auto isContinue = this->replaceCommentToSpace(line, rangeComment);
		if(isContinue == ProgressNextStep::NextLine){
			continue;
		}
		else if(isContinue == ProgressNextStep::Break){
			break;
		}

		auto dqResult = this->findStrings(line);

		auto lineCountStr = std::to_string(lineCount);
		std::u8string u8lineCount(lineCountStr.begin(), lineCountStr.end());

		for(auto& str : dqResult)
		{
			//文字列内のクォーテーションは、ツクール側のlangscoreスクリプトで
			//エスケープ文字に誤認される可能性があるため削除する。
			auto original = utility::replace(std::get<0>(str), u8"\\\'"s, u8"\'"s);
			original = utility::replace(original, u8"\\\""s, u8"\""s);
			original = utility::replace(original, u8"\\n"s, u8"\n"s);
			original = utility::replace(original, u8"\\r"s, u8"\r"s);
			langscore::TranslateText t = {
				original,
				this->useLangList
			};
			auto colCountStr = std::to_string(std::get<1>(str));
			std::u8string u8ColCountStr(colCountStr.begin(), colCountStr.end());
			auto scriptPos = fileName.u8string() + u8":" + u8lineCount + u8":" + u8ColCountStr;
			t.scriptLineInfo = scriptPos;
			if(t.translates.find(defaultLanguage) != t.translates.end()) {
				t.translates[defaultLanguage] = original;
			}
			transTextList.emplace_back(std::move(t));
		}
	}
	return transTextList;

}
