#pragma once

#include <gtest/gtest.h>
#include "config.h"
#include "invoker.h"
#include "divisi.h"

#include "..\\src\\platform\\divisi_vxace.h"
#include "..\\src\\writer\\csvwriter.h"
#include "..\\src\\writer\\rbscriptwriter.h"
#include "..\\src\\writer\\jsonwriter.h"
#include "..\\src\\reader\\csvreader.h"
#include "..\\src\\reader\\speciftranstext.hpp"
#include "..\\src\\reader\\rubyreader.hpp"
#include "..\\src\\reader\\mvmzjsonreader.hpp"
#include "..\\src\\reader\\javascriptreader.hpp"
#include "..\\src\\reader\\vxacejsonreader.hpp"
#include "..\\src\\reader\\jsonreader.hpp"
#include "..\\src\\utility.hpp"
#include "..\\src\\scripttextparser.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>

#include "test_config.h"


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
    Delete(".\\data\\mv\\LangscoreTest_langscore\\data");
    Delete(".\\data\\mv\\LangscoreTest_MultipleExport_langscore\\analyze");
    Delete(".\\data\\mv\\LangscoreTest_MultipleExport_langscore\\data");
	Delete(".\\data\\vxace\\ソポァゼゾタダＡボマミ\\Data\\Translate");
	Delete(".\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\analyze");
	Delete(".\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\data");
	Delete(".\\data\\vxace\\ソポァゼゾタダＡボマミ_Invalid_langscore\\analyze");
	Delete(".\\data\\vxace\\ソポァゼゾタダＡボマミ_Invalid_langscore\\data");
	Delete(".\\data\\vxace\\Include WhiteSpacePath Project_langscore\\analyze");
	Delete(".\\data\\vxace\\Include WhiteSpacePath Project_langscore\\data");
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

static void checkAndCreateConfigFile(fs::path config_path, std::filesystem::path gameProjectFilename = "Game.rvproj2")
{
    if(fs::exists(fs::path(BINARYT_DIRECTORY) / config_path) == false)
    {
        std::string suffix = "_langscore";

        // パスを分割
        std::filesystem::path parentPath = config_path.parent_path();

        // フォルダ名からサフィックスを削除
        std::filesystem::path gamePath;
        for(auto& part : parentPath) {
            std::string partStr = part.string();
            if(partStr.size() >= suffix.size() && partStr.compare(partStr.size() - suffix.size(), suffix.size(), suffix) == 0) {
                partStr = partStr.substr(0, partStr.size() - suffix.size());
            }
            gamePath /= partStr;
        }

        gamePath /= gameProjectFilename;

        langscore::divisi divisi("", "");
        divisi.createConfig(fs::path(BINARYT_DIRECTORY) / gamePath);
    }
}

static void attachConfigFile(fs::path config_path)
{
    checkAndCreateConfigFile(config_path);

    langscore::config::detachConfigFile();
    langscore::config::attachConfigFile(fs::path(BINARYT_DIRECTORY) / config_path);
}