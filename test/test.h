#pragma once

#include <gtest/gtest.h>
#include "config.h"
#include "invoker.h"
#include "divisi.h"

#include "..\\src\\platform\\divisi_vxace.h"
#include "..\\src\\writer\\csvwriter.h"
#include "..\\src\\writer\\langscore_custom_rb_writer.h"
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
#include <thread>
#include <chrono>

#include "test_config.h"


using namespace std::string_literals;
using namespace std::string_view_literals;
using namespace langscore;
namespace fs = std::filesystem;

class test_reader : public readerbase {
public:
	test_reader(std::vector<std::u8string> langs)
		: readerbase({}) {
        this->useLangList = std::move(langs);
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
//生成物の削除。
//テストがクラッシュしてファイルハンドルを漏らすと削除に失敗するが、
//ここで例外を投げると後続のテストが全部「使用中」で失敗し、
//本来の失敗原因が見えなくなる。そこで例外にせず、警告だけを出す。
static void DeleteGeneratedPath(const std::filesystem::path& path)
{
	std::error_code ec;
	if(fs::exists(path, ec) == false) { return; }

	//ハンドルが閉じるまで少し待つ。
	for(int i = 0; i < 10; ++i)
	{
		ec.clear();
		if(fs::is_directory(path, ec)) { fs::remove_all(path, ec); }
		else                           { fs::remove(path, ec); }
		if(ec == std::error_code{}) { return; }
		std::this_thread::sleep_for(std::chrono::milliseconds(50));
	}

	std::cerr << "Warning: 生成ファイルを削除できません: "
	          << path.string() << " (" << ec.message() << ")" << std::endl;
}

void ClearGenerateFiles()
{
	const auto Delete = [](const auto& path) { DeleteGeneratedPath(path); };
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
	const auto Delete = [](const auto& path) { DeleteGeneratedPath(path); };
	ClearGenerateFiles();

	Delete(config.langscoreAnalyzeDirectorty());
	Delete(config.langscoreUpdateDirectorty());
}

static void checkAndCreateConfigFile(fs::path config_path, std::filesystem::path gameProjectFilename = "Game.rvproj2")
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

    const auto configFullPath = fs::path(BINARY_DIRECTORY) / config_path;
    if(fs::exists(configFullPath) == false)
    {
        langscore::divisi divisi("", "");
        divisi.createConfig(fs::path(BINARY_DIRECTORY) / gamePath / gameProjectFilename);
        return;
    }

    //テストデータに同梱されているconfigは、作成した時のビルドディレクトリを絶対パスで持っている。
    //そのままだと別のビルドディレクトリのゲームプロジェクトを読んでしまうため、
    //今のビルドディレクトリの外を指している場合は書き換える。
    nlohmann::json json;
    {
        std::ifstream file(configFullPath);
        file >> json;
    }
    const auto project = fs::path(utility::cnvStr<std::u8string>(json.value("Project", ""s)));
    const auto relative = project.lexically_relative(fs::path(BINARY_DIRECTORY));
    if(relative.empty() || *relative.begin() == "..")
    {
        json["Project"] = utility::cnvStr<std::string>((fs::path(BINARY_DIRECTORY) / gamePath).generic_u8string());
        std::ofstream outFile(configFullPath);
        outFile << json.dump(2);
    }
}

static void attachConfigFile(fs::path config_path)
{
    checkAndCreateConfigFile(config_path);

    langscore::config::detachConfigFile();
    langscore::config::attachConfigFile(fs::path(BINARY_DIRECTORY) / config_path);
}