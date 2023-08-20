#pragma once
#include <iutest.hpp>
#include "config.h"
#include "invoker.h"
#include "divisi.h"
#include "..\\src\\platform\\divisi_vxace.h"
#include "..\\src\\writer\\csvwriter.h"
#include "..\\src\\writer\\rbscriptwriter.h"
#include "..\\src\\reader\\csvreader.h"
#include "..\\src\\reader\\speciftranstext.hpp"
#include "..\\src\\reader\\rubyreader.hpp"
#include "..\\src\\reader\\javascriptreader.hpp"
#include "..\\src\\utility.hpp"
#include "..\\src\\writer\\scripttextparser.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <Windows.h> 


using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace langscore;
namespace fs = std::filesystem;

class test_reader : public readerbase {
public:
	test_reader(std::vector<std::u8string> langs)
		: readerbase(std::move(langs), {}) {
	}

	void setTexts(std::vector<TranslateText> t) {
		this->texts = std::move(t);
	}
	void addTexts(TranslateText t) {
		this->texts.emplace_back(std::move(t));
	}
};

class test_editable_csv_reader : public csvreader {
public:
	using csvreader::csvreader;

	void setTexts(std::vector<TranslateText> t) {
		this->texts = std::move(t);
	}
	void addTexts(TranslateText t) {
		this->texts.emplace_back(std::move(t));
	}
	std::vector<TranslateText>& getTexts() { return this->texts; }
};

IUTEST(Langscore_Writer, UTF8WordCount)
{
	ScriptTextParser scriptRegex;
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"Honi"s), 4);		//ASCII
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"©ÀʸΦ"s), 4);		//2Byte
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"あいうえお"s), 5);	//3Byte
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"𦹀𧃴𧚄𨉷𨏍𪆐"s), 6);	//4Byte
}

IUTEST(Langscore_Writer, ConvertWorkList)
{
	using namespace std::string_view_literals;
	ScriptTextParser scriptRegex;
	{
		auto baseStr = u8"HoniHoni"s;
		auto result = scriptRegex.ConvertWordList(baseStr);
		IUTEST_ASSERT(result.size() == 8);
		size_t i = 0;
		IUTEST_ASSERT_EQ(std::get<0>(result[i++]), u8"H"sv);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"n"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"i"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"H"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"n"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"i"sv) == 0);
	}

	{
		auto baseStr = u8"あいうえおoooかきくけこ"s;
		auto result = scriptRegex.ConvertWordList(baseStr);
		IUTEST_ASSERT(result.size() == 13);
		size_t i = 0;
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"あ"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"い"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"う"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"え"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"お"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"か"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"き"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"く"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"け"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"こ"sv) == 0);
	}

	{
		auto baseStr = u8"𦹀𧃴𧚄𨉷𨏍𪆐🙁😇"s;
		auto result = scriptRegex.ConvertWordList(baseStr);
		IUTEST_ASSERT(result.size() == 8);
		size_t i = 0;
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𦹀"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𧃴"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𧚄"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𨉷"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𨏍"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𪆐"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"🙁"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"😇"sv) == 0);
	}

}

IUTEST(Langscore_Writer, DetectStringPosition)
{
	ScriptTextParser scriptRegex;
	{
		auto result = scriptRegex.findStrings(u8"Test Line Script");
		IUTEST_ASSERT(result.empty());
	}
	{
		auto result = scriptRegex.findStrings(u8"");
		IUTEST_ASSERT(result.empty());
	}
	{
		auto result = scriptRegex.findStrings(u8"hoge = \"\";");
		IUTEST_ASSERT(result.empty());
	}
	{
		auto result = scriptRegex.findStrings(u8"Test \"Line\" Script");
		IUTEST_ASSERT_EQ(result.size(), 1);
		IUTEST_ASSERT_EQ(std::get<0>(result[0]), u8"Line"s);
		IUTEST_ASSERT_EQ(std::get<1>(result[0]), 7);
	}
	{
		auto result = scriptRegex.findStrings(u8"\"Test\" Line S\"cript\"");
		IUTEST_ASSERT_EQ(result.size(), 2);
		IUTEST_ASSERT_EQ(std::get<0>(result[0]), u8"Test"s);
		IUTEST_ASSERT_EQ(std::get<1>(result[0]), 2);
		IUTEST_ASSERT_EQ(std::get<0>(result[1]), u8"cript"s);
		IUTEST_ASSERT_EQ(std::get<1>(result[1]), 15);
	}
	{
		auto result = scriptRegex.findStrings(u8"auto text = \"あいうえお\"; auto text2 = \"aiueo\""s);
		IUTEST_ASSERT_EQ(result.size(), 2);
		IUTEST_ASSERT_EQ(std::get<0>(result[0]), u8"あいうえお"s);
		IUTEST_ASSERT_EQ(std::get<1>(result[0]), 14);
		IUTEST_ASSERT_EQ(std::get<0>(result[1]), u8"aiueo"s);
		IUTEST_ASSERT_EQ(std::get<1>(result[1]), 36);
	}
	{
		auto result = scriptRegex.findStrings(u8"text = \"\\C[16]プレイ時間\\X[104]\\C[0]\\T[%4$3d\\\"%3$02d]\""s);
		IUTEST_ASSERT_EQ(result.size(), 1);
		IUTEST_ASSERT_EQ(std::get<0>(result[0]), u8"\\C[16]プレイ時間\\X[104]\\C[0]\\T[%4$3d\\\"%3$02d]"s);
	}
	//langscore.rbで正しくCSVが出力されない問題の確認テスト。
	//""括りかつ文字列内に,が存在する場合、""が括られたまま結果が出力される。(CSV用の変換)
	//CSV用の変換はCSVの書き出し時点で行うため、findStringsで行うのは不適切。
	{
		auto result = scriptRegex.findStrings(u8"      raise \"Error!: Missmatch Num Cells : #{file_name}, #{mismatch_cells.to_s}\" "s);
		IUTEST_ASSERT_EQ(result.size(), 1);
		IUTEST_ASSERT_EQ(std::get<0>(result[0]), u8"Error!: Missmatch Num Cells : #{file_name}, #{mismatch_cells.to_s}"s);
	}
}

IUTEST(Langscore_Writer, CheckRubyCommentLine)
{
	langscore::rubyreader scriptWriter({}, {});

	{
		bool isRangeComment = false;
		std::u8string original = u8"chstring";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, original);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"#Commentout";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"=begin";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
		IUTEST_ASSERT(isRangeComment);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"\"Hoge\" =begin";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"\"Hoge\"       "s);
		IUTEST_ASSERT(isRangeComment);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"\"Hoge\" \"=begin\"Honi";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"\"Hoge\" \"=begin\"Honi"s);
		IUTEST_ASSERT(isRangeComment == false);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"Valid Script Line";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"=end";
		std::u8string text = original;
		isRangeComment = true;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
		IUTEST_ASSERT(isRangeComment == false);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"Without Comment Block";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_EQ(text, original);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"Script~~~ #Comment";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"Script~~~         "s);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"\"Key:#{Value}\"";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"\"Key:#{Value}\""s);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"("Text".lstrans'Honi')";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"              )");
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"("Text".lstrans"64:28:1")";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"                 )");
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"("Text".lstrans(''))";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"            )");
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"(Honi:"Te'xt".lstrans('Hoge') "DetectText")";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"(Honi:"Te'xt"                 "DetectText")"s);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"("Text".lstrans(""))";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"            )");
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"(Honi:"Text".lstrans("Hoge") "DetectText")";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"(Honi:"Text"                 "DetectText")"s);
	}
}


IUTEST(Langscore_Writer, CheckJavaScriptCommentLine)
{
	langscore::javascriptreader scriptWriter({}, {});

	bool isRangeComment = false;
	{
		std::u8string original = u8"chstring";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, original);
	}
	{
		std::u8string original = u8"//Commentout";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
	}
	{
		std::u8string original = u8"/*Hoge";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
		IUTEST_ASSERT_STREQ(text, u8"      ");
		IUTEST_ASSERT(isRangeComment);
	}
	{
		std::u8string original = u8"Hoge*/";
		std::u8string text = original;
		isRangeComment = true;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
		IUTEST_ASSERT_STREQ(text, u8"      ");
		IUTEST_ASSERT(!isRangeComment);
	}
	{
		std::u8string original = u8"Hoge*/";
		std::u8string text = original;
		isRangeComment = false;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"Hoge*/");
		IUTEST_ASSERT(!isRangeComment);
	}
	{
		std::u8string original = u8"\"Hoge\"/*Comment*/\"Honi\"";
		std::u8string text = original;
		isRangeComment = false;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"\"Hoge\"           \"Honi\"");
		IUTEST_ASSERT(!isRangeComment);
	}
	{
		std::u8string original = u8"\"Hoge\"//Comment*/\"Honi\"";
		std::u8string text = original;
		isRangeComment = false;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"\"Hoge\"                 ");
		IUTEST_ASSERT(!isRangeComment);
	}
}

IUTEST(Langscore_Writer, DetectRubyString)
{
	std::u8string fileName = u8"detectstring";
	langscore::rubyreader scriptWriter({}, {});

	auto extracted_strings = scriptWriter.convertScriptToCSV(u8"./data/" + fileName + u8".rb");

	IUTEST_ASSERT_EQ(10, extracted_strings.size());
	IUTEST_ASSERT_STREQ(u8"Hello, world!", extracted_strings[0].original);
	IUTEST_ASSERT_STREQ(u8"This is a \"quote\" character.", extracted_strings[1].original);
	IUTEST_ASSERT_STREQ(u8"This string has a \n newline character.", extracted_strings[2].original);
	IUTEST_ASSERT_STREQ(u8"This one has a , comma character.", extracted_strings[3].original);
	IUTEST_ASSERT_STREQ(u8"This string has both \"quote\" and \n newline characters.", extracted_strings[4].original);
	IUTEST_ASSERT_STREQ(u8"This is a 'single quote' character.", extracted_strings[5].original);
	IUTEST_ASSERT_STREQ(u8"This string has a \"quote\", a 'single quote' and a \n newline character.", extracted_strings[6].original);
	IUTEST_ASSERT_STREQ(u8"\"\"", extracted_strings[7].original);
	IUTEST_ASSERT_STREQ(u8"\n", extracted_strings[8].original);
	IUTEST_ASSERT_STREQ(u8",", extracted_strings[9].original);
}

IUTEST(Langscore_Writer, DetectStringPositionFromFile)
{
	std::u8string fileName = u8"chstring";
	langscore::rubyreader scriptWriter({}, {});

	auto result = scriptWriter.convertScriptToCSV(u8"./data/" + fileName + u8".rb");

	IUTEST_ASSERT_EQ(result.size(), 14);

	size_t i = 0;
	IUTEST_ASSERT_STREQ(result[i++].original, u8"あHoniい"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"𦹀𧃴𧚄𨉷𨏍𪆐🙁😇"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"HoniHoni"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"trans #{text}"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"call initialize_clone"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"call initialize_clone"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"call to_str"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"あいうえお"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"A"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"B"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"A"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"翻訳テキスト"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"翻訳テキスト2"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"\\C[16]プレイ時間\\X[104]\\C[0]\\T[%4$3d'%3$02d]"s);

	i = 0;
	//"を含まない単語の開始位置
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":8:4");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":8:16");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":11:2");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":16:10");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":23:8");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":28:8");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":33:8");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":38:6");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":43:13");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":44:4");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":45:4");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":47:2");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":48:2");
}

IUTEST(Langscore_Writer, ConvertCsvText_ASCII)
{
	langscore::csvwriter writer(speciftranstext{{}, {}});

	{
		auto input = u8"Hello, World!";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"Hello, World!\"");
	}
	{
		auto input = u8"First line\nSecond line";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"First line\nSecond line\"");
	}
	{
		auto input = u8"Quote \" in the text";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"Quote \"\" in the text\"");
	}
	{
		auto input = u8"Comma, and quote \"";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"Comma, and quote \"\"\"");
	}
}

IUTEST(Langscore_Writer, ConvertCsvText_Multibyte)
{
	langscore::csvwriter writer(speciftranstext{{}, {}});

	{
		auto input = u8"こんにちは、世界！";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"こんにちは、世界！");
	}
	{
		auto input = u8"こんにちは,世界！";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"こんにちは,世界！\"");
	}
	{
		auto input = u8"最初の行\n次の行";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"最初の行\n次の行\"");
	}
	{
		auto input = u8"文章中の\"記号";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"文章中の\"\"記号\"");
	}
	{
		auto input = u8"コンマ,\"と引用符";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"コンマ,\"\"と引用符\"");
	}
}

IUTEST(Langscore_Config, TmpDir)
{
	langscore::config config;
	
	auto expected = config.langscoreAnalyzeDirectorty();
	auto build_folder = utility::cnvStr<std::u8string>(std::string{CMAKE_BUILD_TYPE_STRING});
	auto actual = fs::path(u8"D:\\Programming\\Github\\langscore-divisi\\out\\build\\"s + build_folder + u8"\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze"s);
	IUTEST_ASSERT_STREQ(expected, actual.u8string());
}

IUTEST(Langscore_Config, LoadLanguages)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	auto expected = config.languages();
	utility::stringlist actual = {"en", "ja"};
	IUTEST_ASSERT(actual.size() == expected.size());
	for(int i = 0; i < actual.size(); ++i){
		IUTEST_ASSERT_STREQ(expected[i].name, actual[i]);
	}
}

IUTEST(Langscore_Config, CheckFontName)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	auto expected = config.languages();
	for(int i = 0; i < expected.size(); ++i)
	{
		if(expected[i].name == "ja"){
			IUTEST_ASSERT_STREQ(expected[i].font.name, u8"メイリオ"s);
		}
		else{
			IUTEST_ASSERT_STREQ(expected[i].font.name, u8"VL Gothic"s);
		}
	}
}

IUTEST(Langscore_Config, CheckProjectPath)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	auto expected = config.gameProjectPath();

	auto build_folder = utility::cnvStr<std::u8string>(std::string{CMAKE_BUILD_TYPE_STRING});
	auto actual = fs::path(u8"D:\\Programming\\Github\\langscore-divisi\\out\\build\\"s + build_folder + u8"\\data\\ソポァゼゾタダＡボマミ"s);
	IUTEST_ASSERT_STREQ(
		expected, 
		actual.u8string()
	);
}

IUTEST(Langscore_Csv, parsePlain)
{
	{
		auto targetCsvList = plaincsvreader{".\\data\\csv\\parsePlain.csv"}.getPlainCsvTexts();
		IUTEST_ASSERT(targetCsvList.empty() == false);

		// 期待される解析結果を定義
		std::vector<std::vector<std::u8string>> expected = {
			{u8"ID", u8"Name", u8"Age", u8"Address", u8"Quote", u8"Description"},
			{u8"1", u8"Yamada, Taro", u8"30", u8"Tokyo, Japan", u8"I love \"sushi\" and \"ramen\"", u8"Hobby: Basketball\nFavorite color: Blue"},
			{u8"2", u8"Suzuki, Hanako", u8"25", u8"Osaka, Japan", u8"Life is like a box of chocolates", u8"Hobby: Painting\nFavorite color: Green"},
			{u8"3", u8"Tanaka, Ken", u8"28", u8"Kyoto, Japan", u8"One,Two,Three,Four", u8"Hobby: Running\nFavorite color: Yellow"},
			{u8"4", u8"Kato, Yuki", u8"22", u8"Hokkaido, Japan", u8"Hello, world! This is a test", u8"Hobby: Swimming\nFavorite color: Red"},
			{u8"5", u8"Watanabe, Rika", u8"35", u8"Fukuoka, Japan", u8"こんにちは、元気ですか？", u8"趣味：読書\n好きな色：ピンク"},
		};

		// 解析結果が期待通りであることを確認    
		for(size_t i = 0; i < expected.size(); ++i)
		{
			for(size_t j = 0; j < expected[i].size(); ++j)
			{
				if(expected[i][j] != targetCsvList[i][j]) {
					IUTEST_SCOPED_TRACE(::iutest::Message() << "No match Text. : " << expected[i][j] << ":" << targetCsvList[i][j]);
					IUTEST_SCOPED_TRACE(::iutest::Message() << "i : " << i << " j : " << j);
					IUTEST_FAIL();
				}
			}
		}
	}
	{
		auto targetCsvList = plaincsvreader{".\\data\\csv\\plaincsvreader.csv"}.getPlainCsvTexts();
		IUTEST_ASSERT(targetCsvList.empty() == false);
		IUTEST_EXPECT_EQ(5, targetCsvList.size());

		IUTEST_ASSERT_STREQ(targetCsvList[1][0], u8"\\c[sigo]テスト\n1");
		IUTEST_ASSERT_STREQ(targetCsvList[1][1], u8"\\c[sigo]テスト\n1");
		IUTEST_ASSERT_STREQ(targetCsvList[1][2], u8"\\c[sigo]测试\n1");

		IUTEST_ASSERT_STREQ(targetCsvList[2][0], u8"\\r[sigo]テスト\n2");
		IUTEST_ASSERT_STREQ(targetCsvList[2][1], u8"\\r[sigo]テスト\n2");
		IUTEST_ASSERT_STREQ(targetCsvList[2][2], u8"\\r[sigo]测试\n2");

		IUTEST_ASSERT_STREQ(targetCsvList[3][0], u8"\\n[sigo]テスト\n3");
		IUTEST_ASSERT_STREQ(targetCsvList[3][1], u8"\\n[sigo]テスト\n3");
		IUTEST_ASSERT_STREQ(targetCsvList[3][2], u8"\\n[sigo]测试\n3");

		IUTEST_ASSERT_STREQ(targetCsvList[4][0], u8"\"タ\"フ\"ルクォーテーションを含むテキストです\"");
	}
}

IUTEST(Langscore_Csv, merge)
{
	{
		langscore::csvreader reader{{}, ".\\data\\csv\\after.csv"};
		auto& targetCsv = reader.curerntTexts();
		IUTEST_ASSERT(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::AcceptSource;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		IUTEST_ASSERT(writer.merge(".\\data\\csv\\before.csv"));

		auto& merged = writer.curerntTexts();

		IUTEST_ASSERT(merged.size() == 2);
		IUTEST_ASSERT_STREQ(merged[0].original, u8"あいうえお");
		IUTEST_ASSERT_STREQ(merged[0].translates[u8"ja"], u8"あいうえお");
		IUTEST_ASSERT_STREQ(merged[1].original, u8"さしすせそ");
		IUTEST_ASSERT_STREQ(merged[1].translates[u8"ja"], u8"");
	}
	{
		langscore::csvreader reader{{}, ".\\data\\csv\\after.csv"};
		auto& targetCsv = reader.curerntTexts();
		IUTEST_ASSERT(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::AcceptTarget;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		IUTEST_ASSERT(writer.merge(".\\data\\csv\\before.csv"));

		auto& merged = writer.curerntTexts();

		IUTEST_ASSERT(merged.size() == 3);
		IUTEST_ASSERT_STREQ(merged[0].original, u8"あいうえお");
		IUTEST_ASSERT_STREQ(merged[0].translates[u8"ja"], u8"たちつてと");
		IUTEST_ASSERT_STREQ(merged[1].original, u8"かきくけこ");
		IUTEST_ASSERT_STREQ(merged[1].translates[u8"ja"], u8"かきくけこ");
		IUTEST_ASSERT_STREQ(merged[2].original, u8"さしすせそ");
		IUTEST_ASSERT_STREQ(merged[2].translates[u8"ja"], u8"さしすせそ");
	}
	{
		langscore::csvreader reader{{}, ".\\data\\csv\\after.csv"};
		auto& targetCsv = reader.curerntTexts();
		IUTEST_ASSERT(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::MergeKeepTarget;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		IUTEST_ASSERT(writer.merge(".\\data\\csv\\before.csv"));

		auto& merged = writer.curerntTexts();

		IUTEST_ASSERT(merged.size() == 3);
		IUTEST_ASSERT_STREQ(merged[0].original, u8"あいうえお");
		IUTEST_ASSERT_STREQ(merged[0].translates[u8"ja"], u8"たちつてと");
		IUTEST_ASSERT_STREQ(merged[1].original, u8"かきくけこ");
		IUTEST_ASSERT_STREQ(merged[1].translates[u8"ja"], u8"かきくけこ");
		IUTEST_ASSERT_STREQ(merged[2].original, u8"さしすせそ");
		IUTEST_ASSERT_STREQ(merged[2].translates[u8"ja"], u8"さしすせそ");
	}
	{
		langscore::csvreader reader{{}, ".\\data\\csv\\before.csv"};
		auto& targetCsv = reader.curerntTexts();
		IUTEST_ASSERT(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::MergeKeepTarget;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		IUTEST_ASSERT(writer.merge(".\\data\\csv\\after.csv"));

		auto& merged = writer.curerntTexts();

		IUTEST_ASSERT(merged.size() == 3);
		IUTEST_ASSERT_STREQ(merged[0].original, u8"あいうえお");
		IUTEST_ASSERT_STREQ(merged[0].translates[u8"ja"], u8"あいうえお");
		IUTEST_ASSERT_STREQ(merged[1].original, u8"かきくけこ");
		IUTEST_ASSERT_STREQ(merged[1].translates[u8"ja"], u8"かきくけこ");
		IUTEST_ASSERT_STREQ(merged[2].original, u8"さしすせそ");
		IUTEST_ASSERT_STREQ(merged[2].translates[u8"ja"], u8"");
	}
	{
		langscore::csvreader reader{{}, ".\\data\\csv\\after.csv"};
		auto& targetCsv = reader.curerntTexts();
		IUTEST_ASSERT(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::MergeKeepSource;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		IUTEST_ASSERT(writer.merge(".\\data\\csv\\before.csv"));

		auto& merged = writer.curerntTexts();

		IUTEST_ASSERT(merged.size() == 3);
		IUTEST_ASSERT_STREQ(merged[0].original, u8"あいうえお");
		IUTEST_ASSERT_STREQ(merged[0].translates[u8"ja"], u8"あいうえお");
		IUTEST_ASSERT_STREQ(merged[1].original, u8"かきくけこ");
		IUTEST_ASSERT_STREQ(merged[1].translates[u8"ja"], u8"かきくけこ");
		IUTEST_ASSERT_STREQ(merged[2].original, u8"さしすせそ");
		IUTEST_ASSERT_STREQ(merged[2].translates[u8"ja"], u8"");
	}
	{
		langscore::csvreader reader{{}, ".\\data\\csv\\before.csv"};
		auto& targetCsv = reader.curerntTexts();
		IUTEST_ASSERT(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::MergeKeepSource;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		IUTEST_ASSERT(writer.merge(".\\data\\csv\\after.csv"));

		auto& merged = writer.curerntTexts();

		IUTEST_ASSERT(merged.size() == 3);
		IUTEST_ASSERT_STREQ(merged[0].original, u8"あいうえお");
		IUTEST_ASSERT_STREQ(merged[0].translates[u8"ja"], u8"たちつてと");
		IUTEST_ASSERT_STREQ(merged[1].original, u8"かきくけこ");
		IUTEST_ASSERT_STREQ(merged[1].translates[u8"ja"], u8"かきくけこ");
		IUTEST_ASSERT_STREQ(merged[2].original, u8"さしすせそ");
		IUTEST_ASSERT_STREQ(merged[2].translates[u8"ja"], u8"さしすせそ");
	}
	{
		std::vector<std::u8string> langs{u8"en"s, u8"ja"s, u8"zh-cn"s};
		test_reader reader(langs);
		{
			TranslateText t{u8"こんにちは"s, langs};
			t.translates[u8"ja"] = u8"こんにちは";
			t.translates[u8"zh-cn"] = u8"下午好";
			reader.addTexts(std::move(t));
		}
		{
			TranslateText t{u8"こん\nばんは"s, langs};
			t.translates[u8"ja"] = u8"こん\nばんは";
			t.translates[u8"zh-cn"] = u8"晚上\n好";
			reader.addTexts(std::move(t));
		}

		auto mode = langscore::MergeTextMode::MergeKeepSource;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);

		IUTEST_ASSERT(writer.merge(".\\data\\csv\\add_lang.csv"));

		auto& merged = writer.curerntTexts();

		IUTEST_ASSERT(merged.size() == 2);
		IUTEST_ASSERT_STREQ(merged[0].original, u8"こんにちは");
		IUTEST_ASSERT_STREQ(merged[0].translates[u8"en"], u8"");
		IUTEST_ASSERT_STREQ(merged[0].translates[u8"ja"], u8"こんにちは");
		IUTEST_ASSERT_STREQ(merged[0].translates[u8"zh-cn"], u8"下午好");
		IUTEST_ASSERT_STREQ(merged[1].original, u8"こん\nばんは");
		IUTEST_ASSERT_STREQ(merged[1].translates[u8"en"], u8"");
		IUTEST_ASSERT_STREQ(merged[1].translates[u8"ja"], u8"こん\nばんは");
		IUTEST_ASSERT_STREQ(merged[1].translates[u8"zh-cn"], u8"晚上\n好");
	}
}


IUTEST(Langscore_Invoker, NoAssignProject)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");
	langscore::invoker invoker;
	auto result = invoker.analyze();
	IUTEST_ASSERT_EQ(result.moduleCode(), ErrorStatus::Module::INVOKER);
	IUTEST_ASSERT_EQ(result.code(), 1);
}

IUTEST(Langscore_Invoker, AnalyzeVXAceProject)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	std::filesystem::remove_all(outputPath);
	langscore::invoker invoker;
	invoker.setProjectPath(langscore::invoker::VXAce, config.gameProjectPath());
	auto analyzeResult = invoker.analyze();
	IUTEST_ASSERT(analyzeResult.valid());

	auto itr = fs::recursive_directory_iterator(outputPath);
	auto numFiles = std::distance(itr, fs::recursive_directory_iterator{});
	IUTEST_ASSERT_LT(135, numFiles);

	fs::remove_all(outputPath);
}

IUTEST(Langscore_Invoker, CheckValidScriptList)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	std::filesystem::remove_all(outputPath);
	langscore::invoker invoker;
	invoker.setProjectPath(langscore::invoker::VXAce, config.gameProjectPath());
	invoker.analyze();

	IUTEST_ASSERT(fs::exists(outputPath / "Scripts/_list.csv"));
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();
	
	for(auto& file : fs::recursive_directory_iterator(outputPath/"Scripts"))
	{
		auto filename = file.path().filename();
		if(filename.extension() != ".rb"){ continue; }
		filename = filename.stem();
		auto r = std::find_if(scriptList.cbegin(), scriptList.cend(), [&filename](const auto& x){
			return x[0] == filename;
		});
		if(r == scriptList.cend()){ 
			std::cout << "Not Found!" << filename << std::endl;
			IUTEST_FAIL(); 
		}
	}

	std::filesystem::remove_all(outputPath);

	IUTEST_SUCCEED();
}

IUTEST(Langscore_Divisi_Analyze, ValidateTexts)
{
	//テキストが一致するかの整合性を確認するテスト
	langscore::divisi divisi("./", ".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	IUTEST_ASSERT(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	IUTEST_ASSERT(fs::exists(outputPath / "Map001.csv"));
	langscore::csvreader csvreader({}, outputPath / "Map001.csv");
	auto scriptCsv = csvreader.curerntTexts();
	
	std::vector<std::u8string> includeTexts = {
		u8"マップ名"s,
		u8"\"12345こんにちは世界 HelloWorld\n\""s,
		u8"\"12345HHEELLOO\n\""s,
		u8"\"これは追加テキストです\n\""s,
		u8"\"言語　切り替えるよ\n\""s,
		u8"\"\n手動で中央揃え\n\""s,
		u8"\"\n\\{中央揃え+フォントサイズ大\n\""s,
		u8"日本語"s,
		u8"英語"s,
		u8"中国語"s,
	};

	for(auto& t : scriptCsv)
	{
		auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [&t](const auto& x){
			return x == t.original;
		});
		if(result == includeTexts.cend()){
			std::cout << "Not Found!" << std::string(t.original.begin(), t.original.end()) << std::endl;
			IUTEST_FAIL();
		}
	}
	IUTEST_SUCCEED();
}

IUTEST(Langscore_Divisi, CheckLangscoreRubyScript)
{	
	//念のため一時キャッシュ側も削除
	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ_langscore\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ_langscore\\Data\\Translate");
	}
	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze");
	}
	{
		langscore::divisi divisi("./", ".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");
		IUTEST_ASSERT(divisi.analyze().valid());
	}
	{
		//analyzeとwriteを同時に呼び出すことを想定していない。
		//analyzeを呼び出すとコンストラクト時の言語リストが初期化されるため、
		//インスタンスは別に分ける。
		langscore::divisi divisi("./", ".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");
		IUTEST_ASSERT(divisi.write().valid());
	}
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();
	fs::path langscoreFilename;
	{
		auto result = std::ranges::find_if(scriptList, [](const auto& x){
			return x[1] == platform_base::Script_File_Name;
		});
		IUTEST_ASSERT(result != scriptList.cend());
		langscoreFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}

	std::cout << "MESSAGE : Langscore.rb " << langscoreFilename;

	IUTEST_ASSERT(fs::exists(langscoreFilename));
	langscore::rubyreader rubyReader{{u8"ja"}, {langscoreFilename}};
	auto scriptCsv = rubyReader.curerntTexts();
	IUTEST_ASSERT(scriptCsv.size() == 75);

	csvwriter csvwriter{rubyReader};
	csvwriter.write(".\\data\\dummy.csv");

	csvreader csvreader({u8"ja"}, ".\\data\\dummy.csv");
	auto writedCsvTexts = csvreader.curerntTexts();
	IUTEST_ASSERT(writedCsvTexts.size() == 75);

	int i = 0;
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"en");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"ja");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"ja");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"en");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"VL Gothic");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"ja");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"メイリオ");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Data/Translate");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"/");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Invalid CSV Data");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Error! : Missmatch Num Cells : #{mismatch_cells.first}");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"File : #{file_name}, Header size : #{size}, Languages : #{rows[0]}");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Error! : Missmatch Num Cells : #{mismatch_cells.first}");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8".rvdata2");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"load_data #{file_name}");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8".csv");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"rb:utf-8:utf-8");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"open #{file_name}");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Warning : Not Found Transcript File #{file_name}");

	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"\n");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8",");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"\"");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"\"");


	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"\"\"");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"\"");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"\"");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8",");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"\n");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Graphics");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Scripts");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Troops");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"CommonEvents");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Actors");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"System");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Classes");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Skills");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"States");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Weapons");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Armors");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Items");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Enemies");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Map%03d");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Graphics");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Scripts");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Troops");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"CommonEvents");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"_");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"現在選択中の言語が表示されます。");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"en");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"The currently selected language is displayed.");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"ja");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"現在選択中の言語が表示されます。");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"en");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"English");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"ja");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"日本語");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"OK");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Reselect");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Cancel");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"kernel32");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"GetPrivateProfileString");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"L");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"kernel32");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"WritePrivateProfileString");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"i");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8" ");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Langscore");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Lang");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"ja");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"./Game.ini");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"ja");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Langscore Load ini : #{$langscore_current_language}");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Langscore");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"Lang");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"./Game.ini");

	if(i != 75){
		IUTEST_SCOPED_TRACE(::iutest::Message() << "The number of tests in the string does not match. : " << i);
		IUTEST_FAIL();
	}
}

IUTEST(Langscore_Divisi, CheckScriptCSV)
{	
	langscore::divisi divisi("./", ".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	IUTEST_ASSERT(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	IUTEST_ASSERT(fs::exists(outputPath / "Scripts.csv"));
	langscore::csvreader csvreader{{}, outputPath / "Scripts.csv"};
	auto scriptCsv = csvreader.curerntTexts();
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	IUTEST_ASSERT(scriptCsv.empty() == false);

	for(auto& obj : scriptCsv)
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [&obj](const auto& x){
			return obj.original.find(x[0]) == std::u8string::npos;
		});
		if(result == scriptList.cend()){
			std::cout << "Not Found!" << std::string((*result)[0].begin(), (*result)[0].end()) << std::endl;
			IUTEST_FAIL();
		}
	}

	IUTEST_SUCCEED();
}

IUTEST(Langscore_Divisi, WriteVXAceProject)
{
	langscore::divisi divisi("./", ".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	fs::path scriptDataSrc(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");


	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate");
	}
	//念のため一時キャッシュ側も削除
	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ_langscore\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ_langscore\\Data\\Translate");
	}

	if(fs::exists(scriptDataDest) == false){
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	IUTEST_ASSERT(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	IUTEST_ASSERT(fs::exists(outputPath / "Scripts.csv"));
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	fs::path langscoreCustomFilename;
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x){
			return x[1] == platform_base::Custom_Script_File_Name;
		});
		IUTEST_ASSERT(result != scriptList.cend());
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x){
			return x[1] == platform_base::Script_File_Name;
		});
		IUTEST_ASSERT(result != scriptList.cend());
	}

	fs::copy(scriptDataDest, scriptDataSrc, fs::copy_options::overwrite_existing);

	IUTEST_SUCCEED();
}

IUTEST(Langscore_Divisi, WriteVocab)
{
	langscore::divisi divisi("./", ".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	fs::path scriptDataSrc(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");

	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate");
	}
	//念のため一時キャッシュ側も削除
	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ_langscore\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ_langscore\\Data\\Translate");
	}

	if(fs::exists(scriptDataDest) == false){
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	IUTEST_ASSERT(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	IUTEST_ASSERT(fs::exists(outputPath / "Scripts.csv"));
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	fs::path langscoreCustomFilename;
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x){
			return x[1] == platform_base::Custom_Script_File_Name;
		});
		IUTEST_ASSERT(result != scriptList.cend());
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x){
			return x[1] == platform_base::Script_File_Name;
		});
		IUTEST_ASSERT(result != scriptList.cend());
	}

	fs::copy(scriptDataDest, scriptDataSrc, fs::copy_options::overwrite_existing);

	IUTEST_SUCCEED();
}

IUTEST(Langscore_Divisi, WriteLangscoreCustom)
{
	langscore::config config;
	langscore::divisi_vxace divisi_vxace;
	divisi_vxace.setAppPath("./");
	divisi_vxace.setProjectPath(config.gameProjectPath());

	std::u8string fileName = u8"57856563";	//Cacheスクリプト
	langscore::rubyreader rubyReader({u8"en", u8"ja"}, {u8".\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze\\Scripts\\"s + fileName + u8".rb"s});
	langscore::rbscriptwriter scriptWriter(rubyReader);

	const auto outputFileName = "./data/langscore_custom.rb"s;
	if(fs::exists(outputFileName)){
		fs::remove(outputFileName);
	}

	auto result = scriptWriter.write(outputFileName);
	IUTEST_ASSERT(result.valid());

	{
		std::ifstream outputFile(outputFileName);
		IUTEST_ASSERT(outputFile.good());

		int numSucceed = 0;
		while(outputFile.eof() == false)
		{
			std::string line;
			std::getline(outputFile, line);
			if(line.find("\tLangscore.translate_" + utility::cnvStr<std::string>(fileName)) != line.npos) {
				numSucceed++;
			}
			if(line.find("Scripts/57856563#15,18") != line.npos) {
				numSucceed++;
			}
			if(line.find("Langscore.translate_for_script(\"57856563:15:18\")") != line.npos) {
				numSucceed++;
			}

		}
		IUTEST_ASSERT_EQ(numSucceed, 3);
	}
}

IUTEST(Langscore_Divisi, ValidateLangscoreCustom)
{
	langscore::divisi divisi("./", ".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	fs::path scriptDataSrc(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");

	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate");
	}
	//念のため一時キャッシュ側も削除
	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ_langscore\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ_langscore\\Data\\Translate");
	}

	if(fs::exists(scriptDataDest) == false){
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	divisi.write();
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	fs::path langscoreCustomFilename;
	{
		auto result = std::ranges::find_if(scriptList, [](const auto& x){
			return x[1] == u8"langscore_custom";
		});
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}

	std::ifstream customScript(langscoreCustomFilename);
	std::vector<std::string> lines;
	while(customScript.eof() == false){
		std::string line;
		std::getline(customScript, line);
		lines.emplace_back(std::move(line));
	}

	const auto FindString = [&lines](std::string str){
		auto result = std::find_if(lines.cbegin(), lines.cend(), [&str](const auto& x){
			return x.find(str) != decltype(str)::npos;
		});
		return result != lines.cend();
	};
	IUTEST_ASSERT(FindString("def Langscore.Translate_Script_Text"s));
	IUTEST_ASSERT(FindString("def Langscore.translate_"s));

	fs::copy(scriptDataDest, scriptDataSrc, fs::copy_options::overwrite_existing);

	IUTEST_SUCCEED();
}

IUTEST(Langscore_Divisi, VXAce_WriteScriptCSV)
{
	//Langscore_customに意図した通りに内容が書き込まれているかのテスト
	fs::path scriptDataSrc(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");

	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate");
	}
	//念のため一時キャッシュ側も削除
	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ_langscore\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ_langscore\\Data\\Translate");
	}
	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze");
	}

	if(fs::exists(scriptDataDest) == false){
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	langscore::config config;
	langscore::divisi_vxace divisi_vxace;
	divisi_vxace.setAppPath("./");
	divisi_vxace.setProjectPath(config.gameProjectPath());
	divisi_vxace.analyze();
	divisi_vxace.write();


	auto [scripts, dataList, graphics] = divisi_vxace.fetchFilePathList(config.langscoreAnalyzeDirectorty());
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	fs::path langscoreCustomFilename;
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x){
			return x[1] == u8"langscore_custom";
		});
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}

	std::ifstream customScript(langscoreCustomFilename);
	std::vector<std::string> lines;
	while(customScript.eof() == false){
		std::string line;
		std::getline(customScript, line);
		lines.emplace_back(std::move(line));
	}

	rbscriptwriter scriptWriter(rubyreader{{u8"ja"s}, scripts});
	
	const auto funcName = [](auto str)
	{
		using Str = decltype(str);
		using Char = Str::value_type;
		str = utility::removeExtension(str);
		for(auto i = str.find(Char(" ")); i != decltype(str)::npos; i = str.find(Char(" "))){
			str.replace(i, 1, (Char*)"_");
		}
		return Str(std::add_pointer_t<Char>("Langscore.translate_") + str);
	};

	const auto FindString = [&lines](std::string str){
		auto result = std::find_if(lines.cbegin(), lines.cend(), [&str](const auto& x){
			return x.find(str) != decltype(str)::npos;
		});
		return result != lines.cend();
	};


	for(const auto& script : scriptWriter.scriptTranslatesMap)
	{
		//文字列のないスクリプトファイルを無視する
		if(std::get<1>(script).empty()){ continue; }
		auto fileName = std::get<0>(script);

		const auto& scriptName = scriptWriter.GetScriptName(fileName);
		if(scriptName.empty()){ continue; }
		IUTEST_ASSERT_STRNE(scriptName, platform_base::Script_File_Name);
		IUTEST_ASSERT_STRNE(scriptName, platform_base::Custom_Script_File_Name);

		auto path = outputPath / "Scripts" / fileName;

		if(fs::file_size(path) == 0){ continue; }

		auto scriptFuncName = "def "s + funcName(utility::cnvStr<std::string>(fileName));
		if(FindString(scriptFuncName) == false)
		{
			IUTEST_SCOPED_TRACE(::iutest::Message() << "NotFound Line " << scriptFuncName);
			IUTEST_SCOPED_TRACE(::iutest::Message() << "Script Name " << utility::cnvStr<std::string>(scriptName));
			IUTEST_FAIL();
		}
	}

}

IUTEST(Langscore_Divisi, VXAce_FindEscChar)
{
	langscore::config config;
	langscore::divisi_vxace divisi_vxace;

	{
		std::u8string text = u8"HoniHoni";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() && result2.empty());
	}
	{
		std::u8string text = u8"Honi\\V[0]Honi";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\V[0]");
	}
	{
		std::u8string text = u8"Honi\\N[funi]Honi";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\N[funi]");
	}
	{
		std::u8string text = u8"Honi\\P[58919047]Honi";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\P[58919047]");
	}
	{
		std::u8string text = u8"Honi\\C[16]Honi";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\C[16]");
	}
	{
		std::u8string text = u8"ほに\\l[16]ほに";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\l[16]");
	}
	{
		std::u8string text = u8"あいう\\{えお";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() && result2.empty() == false);
		IUTEST_ASSERT_STREQ(result2[0], u8"\\{");
	}
	{
		std::u8string text = u8"99999\\G 手に入れた！";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() && result2.empty() == false);
		IUTEST_ASSERT_STREQ(result2[0], u8"\\G");
	}
}

IUTEST(Langscore_Divisi, VXAce_Validate)
{
	langscore::config config;
	langscore::divisi_vxace divisi_vxace;

	{
		langscore::TranslateText text;
		text.original = u8"HoniHoni";
		text.translates[u8"en"] = u8"HoniHoni";
		text.translates[u8"ja"] = u8"ほにほに";
		text.translates[u8"zh-tw"] = u8"深深地";
		text.translates[u8"zh-cn"] = u8"深深地";

		IUTEST_ASSERT(divisi_vxace.validateTranslateList({text}, ""));
	}
	{
		langscore::TranslateText text;
		text.original = u8"Honi\\V[10]Honi";
		text.translates[u8"en"] = u8"Honi\\V[10]Honi";
		text.translates[u8"ja"] = u8"ほに\\V[10]ほに";
		text.translates[u8"zh-tw"] = u8"深深\\V[10]地";
		text.translates[u8"zh-cn"] = u8"深深\\V[10]地";

		IUTEST_ASSERT(divisi_vxace.validateTranslateList({text}, ""));
	}
	{
		langscore::TranslateText text;
		text.original = u8"Honi\\V[10]Honi";
		text.translates[u8"en"] = u8"HoniHoni";
		text.translates[u8"ja"] = u8"ほにほに";
		text.translates[u8"zh-tw"] = u8"深深地";
		text.translates[u8"zh-cn"] = u8"深深地";

		IUTEST_ASSERT_NOT(divisi_vxace.validateTranslateList({text}, ""));
	}
}


int main(int argc, char** argv)
{
	if(std::is_move_constructible<TranslateText>::value) {
		std::cout << "MyClass has a move constructor." << std::endl;
	}
	else {
		std::cout << "MyClass does not have a move constructor." << std::endl;
	}

	if(std::is_trivially_move_constructible<TranslateText>::value) {
		std::cout << "MyClass has a trivial move constructor." << std::endl;
	}
	else {
		std::cout << "MyClass does not have a trivial move constructor." << std::endl;
	}

	langscore::config::attachConfigFile(".\\data\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	IUTEST_INIT(&argc, argv);
	return IUTEST_RUN_ALL_TESTS();
}