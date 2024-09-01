
class DummyPlatformBase : public langscore::platform_base 
{
public:

    void setProjectPath(std::filesystem::path path) override {}
    ErrorStatus analyze() override { return Status_Success; }
    ErrorStatus update() override { return Status_Success; }
    ErrorStatus write() override { return Status_Success; }
    ErrorStatus validate() override { return Status_Success; }
    ErrorStatus packing() override { return Status_Success; }

    std::tuple<std::vector<std::u8string>, std::vector<std::u8string>> testFindRPGMakerEscChars(std::u8string text) const {
        return this->findRPGMakerEscChars(std::move(text));
    }


    bool testValidateTranslateFileList(utility::filelist csvPathList) const {
        return this->validateTranslateFileList(std::move(csvPathList));
    }
    bool testValidateTranslateList(std::vector<TranslateText> texts, std::filesystem::path path) const {
        return this->validateTranslateList(std::move(texts), std::move(path));
    }

};

class Langscore_VXAce_Invoker : public ::testing::Test {
protected:
    void SetUp() override {
        // テスト用の一時ディレクトリを作成
        //std::filesystem::create_directory(testDir);
    }

    void TearDown() override {
        // テスト用の一時ディレクトリを削除
        //std::filesystem::remove_all(testDir);
    }

    //void attachConfigFile(fs::path config_path) 
    //{
    //    if(fs::exists(fs::path(BINARYT_DIRECTORY) / config_path) == false) 
    //    {
    //        std::string suffix = "_langscore";

    //        // パスを分割
    //        std::filesystem::path parentPath = config_path.parent_path();
    //        std::filesystem::path filename = "Game.rvproj2";

    //        // フォルダ名からサフィックスを削除
    //        std::filesystem::path gamePath;
    //        for(auto& part : parentPath) {
    //            std::string partStr = part.string();
    //            if(partStr.size() >= suffix.size() && partStr.compare(partStr.size() - suffix.size(), suffix.size(), suffix) == 0) {
    //                partStr = partStr.substr(0, partStr.size() - suffix.size());
    //            }
    //            gamePath /= partStr;
    //        }

    //        gamePath /= filename;

    //        langscore::divisi divisi("", "");
    //        divisi.createConfig(fs::path(BINARYT_DIRECTORY) / gamePath);
    //    }

    //    langscore::config::detachConfigFile();
    //    langscore::config::attachConfigFile(fs::path(BINARYT_DIRECTORY) / config_path);

    //}
};

class Langscore_VXAce_Divisi : public ::testing::Test {
protected:
    void SetUp() override {
        // テスト用の一時ディレクトリを作成
        //std::filesystem::create_directory(testDir);
    }

    void TearDown() override {
        // テスト用の一時ディレクトリを削除
        //std::filesystem::remove_all(testDir);
    }
};



TEST_F(Langscore_VXAce_Invoker, NoAssignProject)
{
	attachConfigFile("data/vxace/ソポァゼゾタダＡボマミ_Invalid_langscore/config.json");
	langscore::invoker invoker;
	auto result = invoker.analyze();
	ASSERT_EQ(result.moduleCode(), ErrorStatus::Module::INVOKER);
	ASSERT_EQ(result.code(), 1);
}

TEST_F(Langscore_VXAce_Invoker, AnalyzeVXAceProject)
{
	attachConfigFile("data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");
	langscore::config config;
	ClearGenerateFiles(config);

	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	std::filesystem::remove_all(outputPath);
	langscore::invoker invoker;
	invoker.setProjectPath(config.gameProjectPath());
	auto analyzeResult = invoker.analyze();
	ASSERT_TRUE(analyzeResult.valid());

	auto itr = fs::recursive_directory_iterator(outputPath);
	auto numFiles = std::distance(itr, fs::recursive_directory_iterator{});
	ASSERT_LT(135, numFiles);

	ClearGenerateFiles(config);
}

TEST_F(Langscore_VXAce_Invoker, AnalyzeWhiteSpaceVXAceProject)
{
	attachConfigFile("data/vxace/Include WhiteSpacePath Project_langscore/config.json");
	langscore::config config;
	ClearGenerateFiles(config);

	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	std::filesystem::remove_all(outputPath);
	langscore::invoker invoker;
	invoker.setProjectPath(config.gameProjectPath());
	auto analyzeResult = invoker.analyze();
	ASSERT_TRUE(analyzeResult.valid());

	auto itr = fs::recursive_directory_iterator(outputPath);
	auto numFiles = std::distance(itr, fs::recursive_directory_iterator{});
	ASSERT_LT(135, numFiles);

	ClearGenerateFiles(config);
}

TEST_F(Langscore_VXAce_Invoker, CheckValidScriptList)
{
	attachConfigFile("data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");
	langscore::config config;
	ClearGenerateFiles(config);

	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	langscore::invoker invoker;
	invoker.setProjectPath(config.gameProjectPath());
	invoker.analyze();

	ASSERT_TRUE(fs::exists(outputPath / "Scripts/_list.csv"));
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	for(auto& file : fs::recursive_directory_iterator(outputPath / "Scripts"))
	{
		auto filename = file.path().filename();
		if(filename.extension() != ".rb") { continue; }
		filename = filename.stem();
		auto r = std::find_if(scriptList.cbegin(), scriptList.cend(), [&filename](const auto& x) {
			return x[0] == filename;
			});
		if(r == scriptList.cend()) {
			std::cout << "Not Found!" << filename << std::endl;
			GTEST_FAIL();
		}
	}

	std::filesystem::remove_all(outputPath);

	GTEST_SUCCEED();
}



TEST_F(Langscore_VXAce_Divisi, CheckIncludeEmptyPath)
{
	ClearGenerateFiles();
	//テキストが一致するかの整合性を確認するテスト
    langscore::config::detachConfigFile();
	langscore::divisi divisi("./", "data/vxace/Include WhiteSpacePath Project_langscore/config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	ASSERT_TRUE(divisi.write().valid());
	ASSERT_TRUE(divisi.update().valid());
}

TEST_F(Langscore_VXAce_Divisi, CheckLangscoreRubyScript)
{
	ClearGenerateFiles();
	{
		langscore::config::detachConfigFile();
		langscore::divisi divisi("./", "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");
		ASSERT_TRUE(divisi.analyze().valid());
	}
	{
		//analyzeとwriteを同時に呼び出すことを想定していない。
		//analyzeを呼び出すとコンストラクト時の言語リストが初期化されるため、
		//インスタンスは別に分ける。
		langscore::config::detachConfigFile();
		langscore::divisi divisi("./", "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");
		ASSERT_TRUE(divisi.write().valid());
	}
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();
	fs::path langscoreFilename;
	{
		auto result = std::ranges::find_if(scriptList, [](const auto& x) {
			return x[1] == platform_base::Script_File_Name;
		});
		ASSERT_TRUE(result != scriptList.cend());
		langscoreFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}

	std::cout << "MESSAGE : Langscore.rb " << langscoreFilename;

	ASSERT_TRUE(fs::exists(langscoreFilename));
	langscore::rubyreader rubyReader{{u8"ja"}, {langscoreFilename}};
	auto scriptCsv = rubyReader.currentTexts();

	csvwriter csvwriter{rubyReader};
	csvwriter.write("data\\dummy.csv");

	csvreader csvreader({u8"ja"}, "data\\dummy.csv");
	auto writedCsvTexts = csvreader.currentTexts();

    std::vector<std::u8string> expectedTexts = {
        u8"en", u8"ja", u8"VL Gothic", u8"メイリオ", u8"Data/Translate", u8"/",
        u8"Invalid CSV Data", 
        u8"Error! : 翻訳文の中に列数が一致しない箇所があります！ : #{mismatch_cells.first}",
        u8"File : #{file_name}, Header size : #{size}, Languages : #{rows[0]}",
        u8"Error! : 翻訳文の中に列数が一致しない箇所があります！ : #{mismatch_cells.first}",
        u8".rvdata2", u8"load_data #{file_name}", u8".csv", u8"rb:utf-8:utf-8",
        u8"open #{file_name}", u8"Warning : 翻訳ファイルが見つかりません。 #{file_name}",
        u8"\n", u8",", u8"\"", u8"Graphics", u8"Scripts", u8"Troops", u8"CommonEvents",
        u8"Actors", u8"System", u8"Classes", u8"Skills", u8"States", u8"Weapons",
        u8"Armors", u8"Items", u8"Enemies", u8"Map%03d", u8"_",
        u8"現在選択中の言語が表示されます。", u8"The currently selected language is displayed.",
        u8"English", u8"日本語", u8"OK", u8"Reselect", u8"Cancel", u8"kernel32",
        u8"GetPrivateProfileString", u8"L", u8"WritePrivateProfileString", u8"i", u8" ",
        u8"Langscore", u8"Lang", u8"./Game.ini", u8"Langscore Load ini : #{$langscore_current_language}"
    };

    volatile int index = 0;
    for(const auto& expectedText : expectedTexts) {
        ASSERT_TRUE(std::any_of(writedCsvTexts.begin(), writedCsvTexts.end(), [&](const auto& text) {
            return text.original == expectedText;
        })) << index << " : " << utility::cnvStr<std::string>(expectedText);
        index++;
    }
}

TEST_F(Langscore_VXAce_Divisi, CheckScriptCSV)
{
	ClearGenerateFiles();
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	ASSERT_TRUE(fs::exists(outputPath / "Scripts.csv"));
	langscore::csvreader csvreader{{}, outputPath / "Scripts.csv"};
	auto scriptCsv = csvreader.currentTexts();
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	ASSERT_TRUE(scriptCsv.empty() == false);

	for(auto& obj : scriptCsv)
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [&obj](const auto& x) {
			return obj.original.find(x[0]) == std::u8string::npos;
			});
		if(result == scriptList.cend()) {
			std::cout << "Not Found!" << std::string((*result)[0].begin(), (*result)[0].end()) << std::endl;
			GTEST_FAIL();
		}
	}

	GTEST_SUCCEED();
}

TEST_F(Langscore_VXAce_Divisi, WriteVXAceProject)
{
	ClearGenerateFiles();
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");

	fs::path scriptDataSrc("data/vxace/ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest("data/vxace/ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");


	if(fs::exists(scriptDataDest) == false) {
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	ASSERT_TRUE(divisi.analyze().valid());
	ASSERT_TRUE(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	ASSERT_TRUE(fs::exists(outputPath / "Scripts.csv"));
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	fs::path langscoreCustomFilename;
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x) {
			return x[1] == platform_base::Custom_Script_File_Name;
			});
		ASSERT_TRUE(result != scriptList.cend());
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x) {
			return x[1] == platform_base::Script_File_Name;
			});
		ASSERT_TRUE(result != scriptList.cend());
	}

	fs::copy(scriptDataDest, scriptDataSrc, fs::copy_options::overwrite_existing);

	GTEST_SUCCEED();
}

TEST_F(Langscore_VXAce_Divisi, WriteVocab)
{
	ClearGenerateFiles();
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");

	fs::path scriptDataSrc("data/vxace/ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest("data/vxace/ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");

	if(fs::exists(scriptDataDest) == false) {
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	ASSERT_TRUE(divisi.analyze().valid());
	ASSERT_TRUE(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	ASSERT_TRUE(fs::exists(outputPath / "Scripts.csv"));
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	fs::path langscoreCustomFilename;
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x) {
			return x[1] == platform_base::Custom_Script_File_Name;
			});
		ASSERT_TRUE(result != scriptList.cend());
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x) {
			return x[1] == platform_base::Script_File_Name;
			});
		ASSERT_TRUE(result != scriptList.cend());
	}

	fs::copy(scriptDataDest, scriptDataSrc, fs::copy_options::overwrite_existing);

	GTEST_SUCCEED();
}

TEST_F(Langscore_VXAce_Divisi, WriteLangscoreCustom)
{
	ClearGenerateFiles();
	langscore::config config;
	langscore::divisi_vxace divisi_vxace;
	divisi_vxace.setAppPath("./");
	divisi_vxace.setProjectPath(config.gameProjectPath());
	divisi_vxace.analyze();

	std::u8string fileName = u8"57856563";	//Cacheスクリプト
	langscore::rubyreader rubyReader({u8"en", u8"ja"}, {u8"data/vxace/ソポァゼゾタダＡボマミ_langscore\\analyze\\Scripts\\"s + fileName + u8".rb"s});
	langscore::rbscriptwriter scriptWriter(rubyReader);

	const auto outputFileName = "./data/langscore_custom.rb"s;
	auto result = scriptWriter.write(outputFileName);
	ASSERT_TRUE(result.valid());

	{
		std::ifstream outputFile(outputFileName);
		ASSERT_TRUE(outputFile.good());

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
		ASSERT_EQ(numSucceed, 3);
	}
}

TEST_F(Langscore_VXAce_Divisi, ValidateLangscoreCustom)
{
	ClearGenerateFiles();
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");

	fs::path scriptDataSrc("data/vxace/ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest("data/vxace/ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");

	if(fs::exists(scriptDataDest) == false) {
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	ASSERT_TRUE(divisi.analyze().valid());
	ASSERT_TRUE(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	fs::path langscoreCustomFilename;
	{
		auto result = std::ranges::find_if(scriptList, [](const auto& x) {
			return x[1] == u8"langscore_custom";
			});
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}

	std::ifstream customScript(langscoreCustomFilename);
	std::vector<std::string> lines;
	while(customScript.eof() == false) {
		std::string line;
		std::getline(customScript, line);
		lines.emplace_back(std::move(line));
	}

	const auto FindString = [&lines](std::string str) {
		auto result = std::find_if(lines.cbegin(), lines.cend(), [&str](const auto& x) {
			return x.find(str) != decltype(str)::npos;
			});
		return result != lines.cend();
		};
	ASSERT_TRUE(FindString("def Langscore.Translate_Script_Text"s));
	ASSERT_TRUE(FindString("def Langscore.translate_"s));

	fs::copy(scriptDataDest, scriptDataSrc, fs::copy_options::overwrite_existing);

	GTEST_SUCCEED();
}

TEST_F(Langscore_VXAce_Divisi, VXAce_WriteScriptCSV)
{
	ClearGenerateFiles();
	//Langscore_customに意図した通りに内容が書き込まれているかのテスト
	fs::path scriptDataSrc("data/vxace/ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest("data/vxace/ソポァゼゾタダＡボマミ\\Data\\Scripts_backup.rvdata2");


	if(fs::exists(scriptDataDest) == false) {
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

    /*
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
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x) {
			return x[1] == u8"langscore_custom";
			});
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}

	std::ifstream customScript(langscoreCustomFilename);
	std::vector<std::string> lines;
	while(customScript.eof() == false) {
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
			for(auto i = str.find(Char(" ")); i != decltype(str)::npos; i = str.find(Char(" "))) {
				str.replace(i, 1, (Char*)"_");
			}
			return Str(std::add_pointer_t<Char>("Langscore.translate_") + str);
		};

	const auto FindString = [&lines](std::string str) {
		auto result = std::find_if(lines.cbegin(), lines.cend(), [&str](const auto& x) {
			return x.find(str) != decltype(str)::npos;
			});
		return result != lines.cend();
		};


	for(const auto& script : scriptWriter.getScriptTexts())
	{
		//文字列のないスクリプトファイルを無視する
		if(std::get<1>(script).empty()) { continue; }
		auto fileName = std::get<0>(script);

		const auto& scriptName = scriptWriter.GetScriptName(fileName);
		if(scriptName.empty()) { continue; }
		ASSERT_FALSE(scriptName == platform_base::Script_File_Name);
		ASSERT_FALSE(scriptName == platform_base::Custom_Script_File_Name);

		auto path = outputPath / "Scripts" / fileName;

		if(fs::file_size(path) == 0) { continue; }

		auto scriptFuncName = "def "s + funcName(utility::cnvStr<std::string>(fileName));
		if(FindString(scriptFuncName) == false)
		{
			SCOPED_TRACE(::testing::Message() << "NotFound Line " << scriptFuncName);
			SCOPED_TRACE(::testing::Message() << "Script Name " << utility::cnvStr<std::string>(scriptName));
			GTEST_FAIL();
		}
	}
    */
}

TEST_F(Langscore_VXAce_Divisi, VXAce_FindEscChar)
{
	langscore::config config;
    DummyPlatformBase divisi_vxace;

	{
		std::u8string text = u8"HoniHoni";
		auto [result1, result2] = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result1.empty() && result2.empty());
	}
	{
		std::u8string text = u8"Honi\\V[0]Honi";
		auto [result1, result2] = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result1.empty() == false && result2.empty());
		ASSERT_TRUE(result1[0] == u8"\\V[0]");
	}
	{
		std::u8string text = u8"Honi\\N[funi]Honi";
		auto [result1, result2] = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result1.empty() == false && result2.empty());
		ASSERT_TRUE(result1[0] == u8"\\N[funi]");
	}
	{
		std::u8string text = u8"Honi\\P[58919047]Honi";
		auto [result1, result2] = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result1.empty() == false && result2.empty());
		ASSERT_TRUE(result1[0] == u8"\\P[58919047]");
	}
	{
		std::u8string text = u8"Honi\\C[16]Honi";
		auto [result1, result2] = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result1.empty() == false && result2.empty());
		ASSERT_TRUE(result1[0] == u8"\\C[16]");
	}
	{
		std::u8string text = u8"ほに\\l[16]ほに";
		auto [result1, result2] = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result1.empty() == false && result2.empty());
		ASSERT_TRUE(result1[0] == u8"\\l[16]");
	}
	{
		std::u8string text = u8"あいう\\{えお";
		auto [result1, result2] = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result1.empty() && result2.empty() == false);
		ASSERT_TRUE(result2[0] == u8"\\{");
	}
	{
		std::u8string text = u8"99999\\G 手に入れた！";
		auto [result1, result2] = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result1.empty() && result2.empty() == false);
		ASSERT_TRUE(result2[0] == u8"\\G");
	}
}

TEST_F(Langscore_VXAce_Divisi, VXAce_Validate)
{
	langscore::config config;
	DummyPlatformBase divisi_vxace;

	{
		langscore::TranslateText text;
		text.original = u8"HoniHoni";
		text.translates[u8"en"] = u8"HoniHoni";
		text.translates[u8"ja"] = u8"ほにほに";
		text.translates[u8"zh-tw"] = u8"深深地";
		text.translates[u8"zh-cn"] = u8"深深地";

		ASSERT_TRUE(divisi_vxace.testValidateTranslateList({text}, ""));
	}
	{
		langscore::TranslateText text;
		text.original = u8"Honi\\V[10]Honi";
		text.translates[u8"en"] = u8"Honi\\V[10]Honi";
		text.translates[u8"ja"] = u8"ほに\\V[10]ほに";
		text.translates[u8"zh-tw"] = u8"深深\\V[10]地";
		text.translates[u8"zh-cn"] = u8"深深\\V[10]地";

		ASSERT_TRUE(divisi_vxace.testValidateTranslateList({text}, ""));
	}
	{
		langscore::TranslateText text;
		text.original = u8"Honi\\V[10]Honi";
		text.translates[u8"en"] = u8"HoniHoni";
		text.translates[u8"ja"] = u8"ほにほに";
		text.translates[u8"zh-tw"] = u8"深深地";
		text.translates[u8"zh-cn"] = u8"深深地";

		ASSERT_FALSE(divisi_vxace.testValidateTranslateList({text}, ""));
	}
}


TEST(Langscore_VXAce_Divisi_Analyze, ValidateTexts)
{
    ClearGenerateFiles();
    //テキストが一致するかの整合性を確認するテスト
    langscore::config::detachConfigFile();
    langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");

    ASSERT_TRUE(divisi.analyze().valid());
    langscore::config config;
    auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

    ASSERT_TRUE(fs::exists(outputPath / "Map001.csv"));
    auto scriptCsv = plaincsvreader{outputPath / "Map001.csv"}.getPlainCsvTexts();
    ASSERT_TRUE(scriptCsv.empty() == false);

    //Ver.0.7.4からVXAceにおいて末尾に改行を付けないように変更した。
    std::vector<std::u8string> includeTexts = {
        u8"original",u8"通常のテキストです",u8"改行を含む\nテキストです",u8"カンマを含む,テキストです",
        u8"\"タ\"フ\"ルクォーテーションを含むテキストです\"",
        u8"\"\"\"Hello, World\"\",\nそれはプログラムを書く際の\",\"\"\"謎の呪文\"\"(Mystery spell)―――\"",
        u8"1番の変数の値は \\V[1] です。",u8"1番のアクターの名前は \\N[1] です。",
        u8"1番のパーティーメンバーの名前は \\P[1] です。",u8"現在の所持金は \\G です。",
        u8"この文字は \\C[2] 赤色 \\C[0] 通常色 です。",u8"これはアイコン \\I[64] の表示です。",
        u8"文字サイズを \\{ 大きく \\} 小さくします。",u8"\\$ 所持金ウィンドウを表示します。",
        u8"文章の途中で 1/4 秒間のウェイトを \\. します。",u8"文章の途中で 1 秒間のウェイトを \\| します。",
        u8"文章の途中でボタンの入力待ちを \\! します。",u8"\\>この行の文字を一瞬で表示します。\\<",
        u8"\\^文章表示後の入力待ちを行いません。",u8"バックスラッシュの表示は \\\\ です。",
        u8"複合させます\n\\{\\C[2]\\N[2]\\I[22]",u8"勝ち",u8"逃げ犬",u8"負け犬",u8"\\{言語\\}変える\\}よ",
        u8"日本語",u8"英語",u8"中国語",u8"やっぱやめる",u8"フィールドを移動します",u8"行って帰る",
        u8"行ったきり",u8"やめる",u8"移動後のメッセージです。",u8"更に移動した際のメッセージです",
        u8"名前変えるよ",u8"ラフィーナ",u8"エルーシェ",u8"二つ名変えるよ",u8"雑用係",u8"傲慢ちき",u8"無くす",
        u8"０１２３４５６７８９０１２３４５６７８９０１２３４５６７８９\n012345678901234567890123456789012345678901234567890123456789\nＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺＡＢＣＤＥＦＧ\nABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ",
        u8R"(何があったのかよく思い出せない。

辺りを包む木々。天を覆う新緑の葉。

小鳥のせせらぎと、風の音だけが聴こえる。

彼方には、見慣れたような、しかしそうでないような、
懐かしさを僅かに感じる家並み———

ふと、目の前の景色が遠ざかる。
何かに後ろに引っ張られるように、
辺りを俯瞰するような景色が広がっていく。

"アタシ"は後ろを振り向いた。

肩まで行かない赤髪を垂らし、
見開く緑目は"私"を見つめ———)",
    };

    for(auto& row : scriptCsv)
    {
        for(auto& t : row) {
            auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [&t](const auto& x) {
                return x == t;
                });
            if(result == includeTexts.cend()) {
                GTEST_FAIL() << "Not Found!" << std::string(t.begin(), t.end());
            }
        }
    }
    GTEST_SUCCEED();
}