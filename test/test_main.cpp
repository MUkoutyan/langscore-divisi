#pragma once
#include <iutest.hpp>
#include "config.h"
#include "invoker.h"
#include "divisi.h"
#include "..\\src\\reader\\csvreader.h"
#include "..\\src\\writer\\rbscriptwriter.h"
#include "..\\src\\utility.hpp"
#include "..\\src\\writer\\scriptRegex.hpp"
#include <iostream>
#include <filesystem>
#include <Windows.h>

using namespace std::string_literals;
using namespace std::string_view_literals;
namespace fs = std::filesystem;

IUTEST(Langscore_Config, TmpDir)
{
	langscore::config config;
	
	auto expected = config.langscoreAnalyzeDirectorty();
	auto actual = fs::path(u8"D:\\Programming\\Github\\langscore-divisi\\out\\build\\Test\\data\\ソポァゼゾタダＡボマミ_langscore\\analyze"s);
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

	auto expected = config.projectPath();
	auto actual = fs::path(u8"D:\\Programming\\Github\\langscore-divisi\\out\\build\\Test\\data\\ソポァゼゾタダＡボマミ"s);
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
	IUTEST_ASSERT_EQ(result.val(), 1);
}

IUTEST(Langscore_Invoker, AnalyzeVXAceProject)
{
	langscore::config config(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	std::filesystem::remove_all(outputPath);
	langscore::invoker invoker;
	invoker.setProjectPath(langscore::invoker::VXAce, config.projectPath());
	auto analyzeResult = invoker.analyze();
	IUTEST_ASSERT_EQ(analyzeResult.val(), 0);

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
	invoker.setProjectPath(langscore::invoker::VXAce, config.projectPath());
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

	IUTEST_ASSERT(divisi.analyze());
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

IUTEST(Langscore_Writer, UTF8WordCount)
{
	ScriptRegex scriptRegex;
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"Honi"s), 4);		//ASCII
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"©ÀʸΦ"s), 4);		//2Byte
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"あいうえお"s), 5);	//3Byte
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"𦹀𧃴𧚄𨉷𨏍𪆐"s), 6);	//4Byte
}

IUTEST(Langscore_Writer, ConvertWorkList)
{
	using namespace std::string_view_literals;
	ScriptRegex scriptRegex;
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
	ScriptRegex scriptRegex;
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

IUTEST(Langscore_Writer, DetectStringPositionFromFile)
{
	std::u8string fileName = u8"chstring";
	//IUTEST_ASSERT(fs::exists(u8"./data/"s + fileName));
	langscore::rbscriptwriter scriptWriter({}, {});

	auto result = scriptWriter.convertScriptToCSV(u8"./data/" + fileName + u8".rb");

	IUTEST_ASSERT_EQ(result.size(), 10);
	
	size_t i = 0;
	IUTEST_ASSERT_EQ(result[i++].original, u8"あHoniい"s);
	IUTEST_ASSERT_EQ(result[i++].original, u8"𦹀𧃴𧚄𨉷𨏍𪆐🙁😇"s);
	IUTEST_ASSERT_EQ(result[i++].original, u8"trans #{text}"s);
	IUTEST_ASSERT_EQ(result[i++].original, u8"call initialize_clone"s);
	IUTEST_ASSERT_EQ(result[i++].original, u8"call initialize_clone"s);
	IUTEST_ASSERT_EQ(result[i++].original, u8"call to_str"s);
	IUTEST_ASSERT_EQ(result[i++].original, u8"あいうえお"s);
	IUTEST_ASSERT_EQ(result[i++].original, u8"A"s);
	IUTEST_ASSERT_EQ(result[i++].original, u8"B"s);
	IUTEST_ASSERT_EQ(result[i++].original, u8"A"s);

	i = 0;
	IUTEST_ASSERT_EQ(result[i++].memo, fileName+u8":8:4");
	IUTEST_ASSERT_EQ(result[i++].memo, fileName+u8":8:16");
	IUTEST_ASSERT_EQ(result[i++].memo, fileName+u8":14:10");
	IUTEST_ASSERT_EQ(result[i++].memo, fileName+u8":21:8");
	IUTEST_ASSERT_EQ(result[i++].memo, fileName+u8":26:8");
	IUTEST_ASSERT_EQ(result[i++].memo, fileName+u8":31:8");
	IUTEST_ASSERT_EQ(result[i++].memo, fileName+u8":36:6");
	IUTEST_ASSERT_EQ(result[i++].memo, fileName+u8":41:13");
	IUTEST_ASSERT_EQ(result[i++].memo, fileName+u8":42:4");
	IUTEST_ASSERT_EQ(result[i++].memo, fileName+u8":43:4");

}

int main(int argc, char** argv)
{
	langscore::config::attachConfigFile(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");
	IUTEST_INIT(&argc, argv);
	return IUTEST_RUN_ALL_TESTS();
}