#pragma once
#include <iutest.hpp>
#include "config.h"
#include "invoker.h"
#include "divisi.h"
#include "..\\src\\platform\\divisi_vxace.h"
#include "..\\src\\reader\\csvreader.h"
#include "..\\src\\writer\\rbscriptwriter.h"
#include "..\\src\\utility.hpp"
#include "..\\src\\writer\\scripttextparser.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <Windows.h>

using namespace std::string_literals;
using namespace std::string_view_literals;
namespace fs = std::filesystem;


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
}

IUTEST(Langscore_Writer, CheckRubyCommentLine)
{
	langscore::rbscriptwriter scriptWriter({}, {});

	{
		std::u8string original = u8"chstring";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, original);
	}
	{
		std::u8string original = u8"#Commentout";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Continue);
	}
	{
		std::u8string original = u8"=begin";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Continue);
	}
	{
		std::u8string original = u8"Valid Script Line";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Continue);
	}
	{
		std::u8string original = u8"=end";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Continue);
	}
	{
		std::u8string original = u8"Without Comment Block";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_EQ(text, original);
	}
	{
		std::u8string original = u8"Script~~~ #Comment";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"Script~~~ "s);
	}
	{
		std::u8string original = u8R"("Text".lstrans'Honi')";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"              )");
	}
	{
		std::u8string original = u8R"("Text".lstrans"64:28:1")";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"                 )");
	}
	{
		std::u8string original = u8R"("Text".lstrans(''))";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"            )");
	}
	{
		std::u8string original = u8R"(Honi:"Te'xt".lstrans('Hoge') "DetectText")";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"(Honi:"Te'xt"                 "DetectText")"s);
	}
	{
		std::u8string original = u8R"("Text".lstrans(""))";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"            )");
	}
	{
		std::u8string original = u8R"(Honi:"Text".lstrans("Hoge") "DetectText")";
		std::u8string text = original;
		auto result = scriptWriter.checkCommentLine(text);
		IUTEST_ASSERT_EQ(result, langscore::writerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"(Honi:"Text"                 "DetectText")"s);
	}
}

IUTEST(Langscore_Writer, DetectStringPositionFromFile)
{
	std::u8string fileName = u8"chstring";
	langscore::rbscriptwriter scriptWriter({}, {});

	auto result = scriptWriter.convertScriptToCSV(u8"./data/" + fileName + u8".rb");

	IUTEST_ASSERT_EQ(result.size(), 13);

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

IUTEST(Langscore_Config, TmpDir)
{
	langscore::config config;
	
	auto expected = config.langscoreAnalyzeDirectorty();
#ifdef _DEBUG
	auto actual = fs::path(u8"D:\\Programming\\Github\\langscore-divisi\\out\\build\\Test_Debug\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze"s);
#else
	auto actual = fs::path(u8"D:\\Programming\\Github\\langscore-divisi\\out\\build\\Test\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze"s);
#endif
	IUTEST_ASSERT_STREQ(expected, actual.u8string());
}

IUTEST(Langscore_Config, LoadLanguages)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");

	auto expected = config.languages();
	utility::stringlist actual = {"en", "ja"};
	for(int i = 0; i < actual.size(); ++i){
		IUTEST_ASSERT_STREQ(expected[i].name, actual[i]);
	}
	actual = {"es", "zh-tw"};
	for(int i = 0; i < actual.size(); ++i){
		IUTEST_ASSERT_STRNE(expected[i].name, actual[i]);
	}
}

IUTEST(Langscore_Config, CheckFontName)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");

	auto expected = config.languages();
	utility::u8stringlist actual = {u8"VL Gothic"s, u8"メイリオ"s};
	for(int i = 0; i < actual.size(); ++i)
	{
		IUTEST_ASSERT_STREQ(expected[i].font.name, actual[i]);
	}
}

IUTEST(Langscore_Config, CheckProjectPath)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");

	auto expected = config.gameProjectPath();
#ifdef _DEBUG
	auto actual = fs::path(u8"D:\\Programming\\Github\\langscore-divisi\\out\\build\\Test_Debug\\data\\ソポァゼゾタダＡボマミ"s);
#else
	auto actual = fs::path(u8"D:\\Programming\\Github\\langscore-divisi\\out\\build\\Test\\data\\ソポァゼゾタダＡボマミ"s);
#endif
	IUTEST_ASSERT_STREQ(expected, actual.u8string());
}

IUTEST(Langscore_Csv, read1)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");

	utility::stringlist actual = {"en", "ja"};
	auto expected = config.languages();
	for(int i = 0; i < actual.size(); ++i)
	{
		IUTEST_ASSERT_STREQ(expected[i].name, actual[i]);
	}
}

IUTEST(Langscore_Invoker, NoAssignProject)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");
	langscore::invoker invoker;
	auto result = invoker.analyze();
	IUTEST_ASSERT_EQ(result.moduleCode(), ErrorStatus::Module::INVOKER);
	IUTEST_ASSERT_EQ(result.code(), 1);
}

IUTEST(Langscore_Invoker, AnalyzeVXAceProject)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");
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
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	std::filesystem::remove_all(outputPath);
	langscore::invoker invoker;
	invoker.setProjectPath(langscore::invoker::VXAce, config.gameProjectPath());
	invoker.analyze();

	IUTEST_ASSERT(fs::exists(outputPath / "Scripts/_list.csv"));
	langscore::csvreader csvreader;
	auto scriptList = csvreader.parsePlain(outputPath / "Scripts/_list.csv");
	
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

IUTEST(Langscore_Divisi, CheckScriptCSV)
{	
	langscore::divisi divisi("./", ".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");

	IUTEST_ASSERT(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	IUTEST_ASSERT(fs::exists(outputPath / "Scripts.csv"));
	langscore::csvreader csvreader;
	auto scriptCsv = csvreader.parse(outputPath / "Scripts.csv");
	auto scriptList = csvreader.parsePlain(outputPath / "Scripts/_list.csv");

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
	langscore::divisi divisi("./", ".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");

	fs::path scriptDataSrc(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");

	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate");
	}

	if(fs::exists(scriptDataDest) == false){
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	IUTEST_ASSERT(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	IUTEST_ASSERT(fs::exists(outputPath / "Scripts.csv"));
	langscore::csvreader csvreader;
	auto scriptList = csvreader.parsePlain(outputPath / "Scripts/_list.csv");

	fs::path langscoreCustomFilename;
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x){
			return x[1] == u8"langscore_custom";
		});
		IUTEST_ASSERT(result != scriptList.cend());
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x){
			return x[1] == u8"langscore";
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

	std::u8string fileName = u8"57856563";
	langscore::rbscriptwriter scriptWriter({u8"en", u8"ja"}, {u8".\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze\\Scripts\\"s + fileName + u8".rb"s});

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
			if(line.find("Langscore.translate_" + utility::cnvStr<std::string>(fileName)) != line.npos) {
				numSucceed++;
			}
			if(line.find("Scripts/Cache#15,18") != line.npos) {
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
	langscore::divisi divisi("./", ".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");

	fs::path scriptDataSrc(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");

	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate");
	}

	if(fs::exists(scriptDataDest) == false){
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	divisi.write().valid();
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	langscore::csvreader csvreader;
	auto scriptList = csvreader.parsePlain(outputPath / "Scripts/_list.csv");

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
	fs::path scriptDataSrc(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");

	if(fs::exists(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate")){
		fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ\\Data\\Translate");
	}

	if(fs::exists(scriptDataDest) == false){
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	langscore::config config;
	langscore::divisi_vxace divisi_vxace;
	divisi_vxace.setAppPath("./");
	divisi_vxace.setProjectPath(config.gameProjectPath());

	divisi_vxace.fetchFilePathList();
	langscore::csvreader csvreader;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	auto scriptList = csvreader.parsePlain(outputPath / "Scripts/_list.csv");

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

	langscore::rbscriptwriter scriptWriter({u8"ja"s}, divisi_vxace.scriptFileList);
	
	const auto funcName = [](auto str)
	{
		using Str = decltype(str);
		using Char = Str::value_type;
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

	for(const auto& script : scriptWriter.scriptTranslates)
	{
		//文字列のないスクリプトファイルを無視する
		if(std::get<1>(script).empty()){ continue; }
		auto fileName = std::get<0>(script);
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [&fileName](const auto& x){
			return x[0] == fileName;
		});

		IUTEST_ASSERT(result != scriptList.cend());

		const auto& scriptName = (*result)[1];
		if(scriptName == u8"langscore"){ continue; }
		else if(scriptName == u8"langscore_custom"){ continue; }

		auto path = outputPath / "Scripts" / (fileName + u8".rb");

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
		auto [result1, result2] = divisi_vxace.findEscChars(text);
		IUTEST_ASSERT(result1.empty() && result2.empty());
	}
	{
		std::u8string text = u8"Honi\\V[0]Honi";
		auto [result1, result2] = divisi_vxace.findEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\V[0]");
	}
	{
		std::u8string text = u8"Honi\\N[funi]Honi";
		auto [result1, result2] = divisi_vxace.findEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\N[funi]");
	}
	{
		std::u8string text = u8"Honi\\P[58919047]Honi";
		auto [result1, result2] = divisi_vxace.findEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\P[58919047]");
	}
	{
		std::u8string text = u8"Honi\\C[16]Honi";
		auto [result1, result2] = divisi_vxace.findEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\C[16]");
	}
	{
		std::u8string text = u8"ほに\\l[16]ほに";
		auto [result1, result2] = divisi_vxace.findEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\l[16]");
	}
	{
		std::u8string text = u8"あいう\\{えお";
		auto [result1, result2] = divisi_vxace.findEscChars(text);
		IUTEST_ASSERT(result1.empty() && result2.empty() == false);
		IUTEST_ASSERT_STREQ(result2[0], u8"\\{");
	}
	{
		std::u8string text = u8"99999\\G 手に入れた！";
		auto [result1, result2] = divisi_vxace.findEscChars(text);
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
	fs::remove_all(".\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze");
	langscore::config::attachConfigFile(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");
	IUTEST_INIT(&argc, argv);
	return IUTEST_RUN_ALL_TESTS();
}