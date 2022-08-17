#pragma once
#include <iutest.hpp>
#include "config.h"
#include "invoker.h"
#include "divisi.h"
#include "..\\src\\reader\\csvreader.h"
#include "..\\src\\utility.hpp"
#include <iostream>
#include <filesystem>
#include <Windows.h>

using namespace std::string_literals;
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

int main(int argc, char** argv)
{
	langscore::config::attachConfigFile(".\\data\\ソポァゼゾタダＡボマミ_langscore\\test_config_with.json");
	IUTEST_INIT(&argc, argv);
	return IUTEST_RUN_ALL_TESTS();
}