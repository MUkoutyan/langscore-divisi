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
#include "..\\src\\scripttextparser.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>


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

void ClearGenerateFiles()
{
	const auto Delete = [](auto path)
		{
			if(fs::exists(path))
			{
				if(fs::is_directory(path)) {
					fs::remove_all(path);
				}
				else {
					fs::remove(path);
				}
			}
		};
	Delete(".\\data\\mv\\LangscoreTest\\data\\translate");
	Delete(".\\data\\mv\\LangscoreTest\\js\\plugins\\Langscore.js");
	Delete(".\\data\\mv\\LangscoreTest_langscore\\analyze");
	Delete(".\\data\\vxace\\ソポァゼゾタダＡボマミ\\Data\\Translate");
	Delete(".\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\analyze");
	Delete(".\\data\\vxace\\ソポァゼゾタダＡボマミ_Invalid_langscore\\analyze");
	Delete(".\\data\\vxace\\Include WhiteSpacePath Project_langscore\\analyze");
	Delete(".\\data\\langscore_custom.rb"s);
}

void ClearGenerateFiles(langscore::config& config)
{
	const auto Delete = [](auto path) 
	{
		if(fs::exists(path))
		{
			if(fs::is_directory(path)) {
				fs::remove_all(path);
			}
			else {
				fs::remove(path);
			}
		}
	};
	ClearGenerateFiles();

	Delete(config.langscoreAnalyzeDirectorty());
	Delete(config.langscoreUpdateDirectorty());
}