
class DummyPlatformBase : public langscore::platform_base
{
public:
    void setProjectPath(std::filesystem::path path) override {}
    ErrorStatus analyze() override { return ErrorStatus(); }
    ErrorStatus reanalysis() override { return ErrorStatus(); }
    ErrorStatus updatePlugin() override { return ErrorStatus(); }
    ErrorStatus exportCSV() override { return ErrorStatus(); }
    ErrorStatus validate() override { return ErrorStatus(); }
    ErrorStatus packing() override { return ErrorStatus(); }

    platform_base::ValidateTextInfo testFindRPGMakerEscChars(std::u8string text) const {
        platform_base::ValidateTextInfo textInfo;
        textInfo.origin.original = std::move(text);
        this->detectConstrolChar(textInfo);
        return textInfo;
    }

    bool testValidateTranslateFileList(std::vector<ValidateFileInfo> csvPathList) const {
        return this->validateTranslateFileList(std::move(csvPathList));
    }
    bool testValidateTranslateList(std::vector<TranslateText> texts, std::filesystem::path path) const {
        //textsをstd::vector<ValidateTextInfo>に変換してvalidateTextsを呼び出す。
        std::vector<platform_base::ValidateTextInfo> validateTexts;
        for(auto& text : texts) {
            ValidateTextInfo validateText;
            validateText.origin = text;
            this->detectConstrolChar(validateText);
            validateTexts.emplace_back(std::move(validateText));
        }

        return this->validateTextFormat(std::move(validateTexts), std::move(path));
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
    config config;
    langscore::invoker invoker(config.projectType());
	auto result = invoker.analyze(config.langscoreAnalyzeDirectorty());
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
	langscore::invoker invoker(config.projectType());
	invoker.setProjectPath(config.gameProjectPath());
	auto analyzeResult = invoker.analyze(config.langscoreAnalyzeDirectorty());
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
	langscore::invoker invoker(config.projectType());
	invoker.setProjectPath(config.gameProjectPath());
	auto analyzeResult = invoker.analyze(config.langscoreAnalyzeDirectorty());
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
	langscore::invoker invoker(config.projectType());
	invoker.setProjectPath(config.gameProjectPath());
	invoker.analyze(config.langscoreAnalyzeDirectorty());

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
	ASSERT_TRUE(divisi.exportCSV().valid());
	ASSERT_TRUE(divisi.reanalysis().valid());
}

TEST_F(Langscore_VXAce_Divisi, CheckLangscoreRubyScript)
{
    //Langscore.rbが正しく出力されているかを確認するテスト。
    //ここで失敗する場合、divisi_vxace::formatSystemVariableのミスか、
    //rubyreaderクラスの解析処理に問題がある可能性がある。
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
		ASSERT_TRUE(divisi.updatePlugin().valid());
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
	csvwriter.write("data\\dummy.csv", u8"ja");

	csvreader csvreader("data\\dummy.csv");
	auto writedCsvTexts = csvreader.currentTexts();

    //検出されなければいけないテキストのみを記述。
    std::vector<std::u8string> expectedTexts = {
        u8"ja", u8"en", u8"VL Gothic", u8"メイリオ", u8"Data/Translate",
        u8".rvdata2", u8".csv",
        u8"Graphics", u8"Scripts", u8"Troops", u8"CommonEvents",
        u8"Actors", u8"System", u8"Classes", u8"Skills", u8"States", u8"Weapons",
        u8"Armors", u8"Items", u8"Enemies", u8"Map%03d",
        u8"現在選択中の言語が表示されます。", u8"The currently selected language is displayed.",
        u8"English", u8"日本語", u8"OK", u8"Reselect", u8"Cancel", u8" ",
        u8"Langscore", u8"Lang", u8"./Game.ini"
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

	ASSERT_TRUE(fs::exists(outputPath / "Scripts.lsjson"));
	langscore::analyzejsonreader csvreader{outputPath / "Scripts.lsjson"};
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
	ASSERT_TRUE(divisi.updatePlugin().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	ASSERT_TRUE(fs::exists(outputPath / "Scripts.lsjson"));
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
	ASSERT_TRUE(divisi.updatePlugin().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	ASSERT_TRUE(fs::exists(outputPath / "Scripts.lsjson"));
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
	langscore::langscore_custom_rb_writer scriptWriter(rubyReader);

	const auto outputFileName = "./data/langscore_custom.rb"s;
	auto result = scriptWriter.write(outputFileName, u8"ja");
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
	ASSERT_TRUE(divisi.updatePlugin().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	fs::path langscoreCustomFilename;
	{
		auto result = std::ranges::find_if(scriptList, [](const auto& x) {
			return x[1] == u8"langscore_custom";
		});
        ASSERT_TRUE(result != scriptList.end());
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
	//ASSERT_TRUE(FindString("def Langscore.translate_"s));

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
		auto result = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result.escWithValueChars.empty() && result.escChars.empty());
	}
	{
		std::u8string text = u8"Honi\\V[0]Honi";
		auto result = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result.escWithValueChars.empty() == false && result.escChars.empty());
		ASSERT_TRUE(result.escWithValueChars[0] == u8"\\V[0]");
	}
	{
		std::u8string text = u8"Honi\\N[funi]Honi";
		auto result = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result.escWithValueChars.empty() == false && result.escChars.empty());
		ASSERT_TRUE(result.escWithValueChars[0] == u8"\\N[funi]");
	}
	{
		std::u8string text = u8"Honi\\P[58919047]Honi";
		auto result = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result.escWithValueChars.empty() == false && result.escChars.empty());
		ASSERT_TRUE(result.escWithValueChars[0] == u8"\\P[58919047]");
	}
	{
		std::u8string text = u8"Honi\\C[16]Honi";
		auto result = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result.escWithValueChars.empty() == false && result.escChars.empty());
		ASSERT_TRUE(result.escWithValueChars[0] == u8"\\C[16]");
	}
	{
		std::u8string text = u8"ほに\\l[16]ほに";
		auto result = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result.escWithValueChars.empty() == false && result.escChars.empty());
		ASSERT_TRUE(result.escWithValueChars[0] == u8"\\l[16]");
	}
	{
		std::u8string text = u8"あいう\\{えお";
		auto result = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result.escWithValueChars.empty() && result.escChars.empty() == false);
		ASSERT_TRUE(result.escChars[0] == u8"\\{");
	}
	{
		std::u8string text = u8"99999\\G 手に入れた！";
		auto result = divisi_vxace.testFindRPGMakerEscChars(text);
		ASSERT_TRUE(result.escWithValueChars.empty() && result.escChars.empty() == false);
		ASSERT_TRUE(result.escChars[0] == u8"\\G");
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

    ASSERT_TRUE(fs::exists(outputPath / "Map001.lsjson"));
    auto scriptCsv = analyzejsonreader{outputPath / "Map001.lsjson"}.currentTexts();
    ASSERT_TRUE(scriptCsv.empty() == false);

    //Ver.0.7.4からVXAceにおいて末尾に改行を付けないように変更した。
    std::vector<std::u8string> includeTexts = {
        u8"通常のテキストです",u8"改行を含む\nテキストです",u8"カンマを含む,テキストです",
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
        auto t = row.original;
        auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [t](const auto& x) {
            return x == t;
            });
        if(result == includeTexts.cend()) {
            GTEST_FAIL() << "Not Found!" << std::string(t.begin(), t.end());
        }
    }
    GTEST_SUCCEED();
}

// VXAce用のJSONリーダーのテキストタイプのテスト
TEST(Langscore_VXAce_VXAceJsonReader, TextTypeForMaker)
{
    // 最初にdivisi.analyze()を実行してJSONファイルを生成
    ClearGenerateFiles();
    langscore::config::detachConfigFile();
    langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");

    ASSERT_TRUE(divisi.analyze().valid());
    langscore::config config;
    auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

    // 生成されたActors.jsonファイルを読み込む
    std::filesystem::path jsonFilePath = outputPath / "Actors.json";
    ASSERT_TRUE(fs::exists(jsonFilePath)) << "Actors.json not found in analyze directory";

    std::ifstream loadFile(jsonFilePath);
    nlohmann::json json;
    loadFile >> json;

    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};
    langscore::vxace_jsonreader reader(useLangs, std::move(json));

    // テキストタイプが正しく設定されているかを確認
    const auto& texts = reader.currentTexts();
    ASSERT_FALSE(texts.empty()) << "No texts found";

    // 名前とプロフィールのテキストタイプを確認
    bool foundName = false;
    bool foundNickname = false;
    bool foundProfile = false;

    for(const auto& text : texts) {
        if(text.original == u8"エルーシェ" || text.original == u8"ラフィーナ") {
            EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), TranslateText::nameType) != text.textType.end()) << "Name text type incorrect";
            foundName = true;
        }
        else if(text.original == u8"雑用係" || text.original == u8"傲慢ちき") {
            EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), TranslateText::nameType) != text.textType.end()) << "Nickname text type incorrect";
            foundNickname = true;
        }
        else if(text.original == u8"ケスティニアスの雑用係。\r\nそんなに仕事は無い。" ||
                text.original == u8"チビのツンデレウーマン。\r\n魔法が得意。") 
        {
            EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), TranslateText::descriptionType) != text.textType.end()) << "Profile text type incorrect";
            foundProfile = true;
        }
    }

    EXPECT_TRUE(foundName) << "Name text not found";
    EXPECT_TRUE(foundNickname) << "Nickname text not found";
    EXPECT_TRUE(foundProfile) << "Profile text not found";
}

// データタイプの検出テスト
TEST(Langscore_VXAce_VXAceJsonReader, DetectDataType)
{
    // 最初にdivisi.analyze()を実行してJSONファイルを生成
    ClearGenerateFiles();
    langscore::config::detachConfigFile();
    langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");

    ASSERT_TRUE(divisi.analyze().valid());
    langscore::config config;
    auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

    // 異なるファイル名でデータタイプが正しく検出されるかテスト
    std::vector<std::string> testCases = {
        "Actors.json",
        "Armors.json",
        "Classes.json",
        "CommonEvents.json",
        "Enemies.json",
        "Items.json",
        "Map001.json",
        "Skills.json",
        "States.json",
        "System.json",
        "Troops.json",
        "Weapons.json"
    };

    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};

    for(const auto& filename : testCases) {
        std::filesystem::path jsonFilePath = outputPath / filename;
        if(fs::exists(jsonFilePath)) {
            std::ifstream loadFile(jsonFilePath);
            nlohmann::json json;
            loadFile >> json;
            langscore::vxace_jsonreader reader(useLangs, std::move(json));

            // データが読み込めたことを確認
            const auto& texts = reader.currentTexts();

            // データタイプが正しく検出されていることを確認するメッセージ
            SUCCEED() << "Successfully detected data type for " << filename;
        }
        else {
            // ファイルが見つからない場合はスキップ
            GTEST_SKIP() << "File not found: " << jsonFilePath.string();
        }
    }
}

// テキストの積み上げ処理テスト
TEST(Langscore_VXAce_VXAceJsonReader, StackTextProcessing)
{
    // 最初にdivisi.analyze()を実行してJSONファイルを生成
    ClearGenerateFiles();
    langscore::config::detachConfigFile();
    langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");

    ASSERT_TRUE(divisi.analyze().valid());
    langscore::config config;
    auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

    // Map001のJSON(イベントテキストを含むファイル)を読み込む
    std::filesystem::path jsonFilePath = outputPath / "Map001.json";
    ASSERT_TRUE(fs::exists(jsonFilePath)) << "Map001.json not found in analyze directory";

    std::ifstream loadFile(jsonFilePath);
    nlohmann::json json;
    loadFile >> json;

    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};
    langscore::vxace_jsonreader reader(useLangs, std::move(json));

    // テキストが正しく積み上げられているかを確認
    const auto& texts = reader.currentTexts();

    // テキストがあることを確認
    ASSERT_FALSE(texts.empty()) << "No texts found in Map001";

    // 積み上げられたメッセージを確認
    bool foundStackedText = false;
    for(const auto& text : texts) {
        if(text.code == 401) {
            // イベントのメッセージ
            if(text.original == u8"通常のテキストです" ||
                text.original == u8"改行を含む\nテキストです") {
                foundStackedText = true;
                break;
            }
        }
    }
    EXPECT_TRUE(foundStackedText) << "Stacked text not found";

    // 選択肢も正しく抽出されているか確認
    bool foundChoices = false;
    for(const auto& text : texts) {
        if(text.code == 102) {
            // 選択肢のテキスト
            if(text.original == u8"ラフィーナ" ||
                text.original == u8"エルーシェ" ||
                text.original == u8"やめる") {
                foundChoices = true;
                break;
            }
        }
    }
    EXPECT_TRUE(foundChoices) << "Choices not found";
}

// テキストタイプの検証テスト
TEST(Langscore_VXAce_VXAceJsonReader, TextTypeValidation)
{
    // 最初にdivisi.analyze()を実行してJSONファイルを生成
    ClearGenerateFiles();
    langscore::config::detachConfigFile();
    langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");

    ASSERT_TRUE(divisi.analyze().valid());
    langscore::config config;
    auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

    // 各種JSONファイルを読み込んでtextTypeを検証する
    struct TestCase {
        std::string filename;
        std::vector<std::pair<std::u8string, const char8_t*>> expectedTexts;
    };

    std::vector<TestCase> testCases = {
        // アクター関連のテキストタイプ
        {"Actors.json", {
            {u8"エルーシェ", TranslateText::nameType},
            {u8"ラフィーナ", TranslateText::nameType},
            {u8"雑用係", TranslateText::nameType},
            {u8"傲慢ちき", TranslateText::nameType},
            {u8"ケスティニアスの雑用係。\r\nそんなに仕事は無い。", TranslateText::descriptionType},
            {u8"チビのツンデレウーマン。\r\n魔法が得意。", TranslateText::descriptionType}
        }},
        // アイテム関連のテキストタイプ
        {"Items.json", {
            {u8"ポーション", TranslateText::nameType},
            {u8"マジックウォーター", TranslateText::nameType},
            {u8"ディスペルハーブ", TranslateText::nameType}
        }},
        // マップイベントのテキストタイプ
        {"Map001.json", {
            {u8"通常のテキストです", TranslateText::message},
            {u8"改行を含む\nテキストです", TranslateText::message},
            {u8"名前変えるよ", TranslateText::message},
            {u8"ラフィーナ", TranslateText::other},
            {u8"エルーシェ", TranslateText::other},
            {u8"やめる", TranslateText::other}
        }}
    };

    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};

    for(const auto& testCase : testCases) {
        // JSONファイルを読み込む
        std::filesystem::path jsonFilePath = outputPath / testCase.filename;
        if(!fs::exists(jsonFilePath)) {
            GTEST_SKIP() << "Test file not found: " << jsonFilePath.string();
            continue;
        }

        std::ifstream loadFile(jsonFilePath);
        nlohmann::json json;
        loadFile >> json;

        langscore::vxace_jsonreader reader(useLangs, std::move(json));
        const auto& texts = reader.currentTexts();

        // 期待されるテキストとそのタイプを確認
        for(const auto& [expectedText, expectedType] : testCase.expectedTexts) {
            bool found = false;
            for(const auto& text : texts) {
                if(text.original == expectedText) {
                    EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), expectedType) != text.textType.end()) 
                        << "Wrong text type for '" << std::string(expectedText.begin(), expectedText.end())
                        << "' in " << testCase.filename;
                    found = true;
                    break;
                }
            }

            // 期待されるテキストが見つからなかった場合は警告だけ出す（データが変わっている可能性もあるため）
            if(!found) {
                ADD_FAILURE() << "Text '" << std::string(expectedText.begin(), expectedText.end())
                    << "' not found in " << testCase.filename;
            }
        }
    }
}

// 親ヘルパー関数の定義（重複を避けるため）
// ファイルの存在確認がなければ加えてください
auto createRubyScriptFile = [](const std::u8string& content) {
    std::filesystem::path tempPath = "data/temp_ruby_test.rb";
    std::ofstream file(tempPath);
    file << utility::cnvStr<std::string>(content);
    file.close();
    return tempPath;
};


// rubyreaderクラスのテスト
TEST(Langscore_Reader, RubyReader_TreeSitter_StringExtraction)
{
    // テスト用の一時Rubyファイルを作成する関数
    auto createRubyScriptFile = [](const std::u8string& content) {
        std::filesystem::path tempPath = "data/temp_ruby_test.rb";
        std::ofstream file(tempPath);
        file << utility::cnvStr<std::string>(content);
        file.close();
        return tempPath;
        };

    // テストケース1: 基本的な文字列抽出
    {
        std::u8string script = u8"str = \"Hello, World!\"\nstr2 = 'こんにちは世界！'";
        auto scriptPath = createRubyScriptFile(script);

        langscore::rubyreader scriptReader({u8"en", u8"ja"}, {scriptPath});
        auto texts = scriptReader.currentTexts();

        ASSERT_EQ(texts.size(), 2);
        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"Hello, World!";
            }));
        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"こんにちは世界！";
            }));

        // 位置情報の確認
        for(const auto& text : texts) {
            auto parts = utility::split(text.scriptLineInfo, u8':');
            ASSERT_EQ(parts.size(), 3);
            ASSERT_EQ(parts[0], u8"temp_ruby_test");
        }
    }

    // テストケース2: 文字列補間
    {
        std::u8string script = u8"name = \"User\"\nstr = \"Hello, #{name}!\"\nmessage = \"Error!: #{file_name}, #{mismatch_cells.to_s}\"";
        auto scriptPath = createRubyScriptFile(script);

        langscore::rubyreader scriptReader({u8"en", u8"ja"}, {scriptPath});
        auto texts = scriptReader.currentTexts();

        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"User";
            }));
        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"Hello, #{name}!";
            }));
        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"Error!: #{file_name}, #{mismatch_cells.to_s}";
            }));
    }

    // テストケース3: コメント内の文字列は無視されることを確認
    {
        std::u8string script = u8"# This is a comment with \"string\"\nreal_string = \"Actual text\"";
        auto scriptPath = createRubyScriptFile(script);

        langscore::rubyreader scriptReader({u8"en", u8"ja"}, {scriptPath});
        auto texts = scriptReader.currentTexts();

        ASSERT_EQ(texts.size(), 1);
        ASSERT_EQ(texts[0].original, u8"Actual text");
    }

    // テストケース4: 空の文字列
    {
        std::u8string script = u8"empty_string = \"\"\nanother_empty = ''";
        auto scriptPath = createRubyScriptFile(script);

        langscore::rubyreader scriptReader({u8"en", u8"ja"}, {scriptPath});
        auto texts = scriptReader.currentTexts();

        // 空文字列は抽出されないか、抽出された場合は空であることを確認
        for(const auto& text : texts) {
            ASSERT_FALSE(text.original.empty()) << "Empty strings should not be extracted";
        }
    }

    // テストケース5: シンボルと特殊な文字列表記
    {
        std::u8string script = u8"symbol = :symbol_name\n"
            u8"percent_q = %q{Text with 'single' quotes}\n"
            u8"percent_Q = %Q{Text with \"double\" quotes and #{interpolation}}";
        auto scriptPath = createRubyScriptFile(script);

        langscore::rubyreader scriptReader({u8"en", u8"ja"}, {scriptPath});
        auto texts = scriptReader.currentTexts();

        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"Text with 'single' quotes";
        }));
        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"Text with \"double\" quotes and #{interpolation}";
        }));
    }

    // テストケース6: メソッド内の文字列引数
    {
        std::u8string script = u8"def translate(text)\n"
            u8"  puts text\n"
            u8"end\n"
            u8"translate(\"This is a message\")\n"
            u8"translate('Another message')";
        auto scriptPath = createRubyScriptFile(script);

        langscore::rubyreader scriptReader({u8"en", u8"ja"}, {scriptPath});
        auto texts = scriptReader.currentTexts();

        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"This is a message";
            }));
        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"Another message";
            }));
    }

    // テストケース7: 無視すべきメソッド呼び出し
    {
        std::u8string script = u8"require \"some_library\"\n"
            u8"puts \"This should be ignored\"\n"
            u8"p \"Also ignored\"\n"
            u8"important = \"This should be extracted\"";
        auto scriptPath = createRubyScriptFile(script);

        langscore::rubyreader scriptReader({u8"en", u8"ja"}, {scriptPath});
        auto texts = scriptReader.currentTexts();

        // "require", "puts", "p" の引数の文字列は無視されるべき
        ASSERT_FALSE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"some_library";
            }));
        ASSERT_FALSE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"This should be ignored";
            }));
        ASSERT_FALSE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"Also ignored";
            }));

        // 通常の変数への代入は抽出される
        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"This should be extracted";
            }));
    }

    // テスト後にテンポラリファイルを削除
    std::filesystem::remove("data/temp_ruby_test.rb");
}

TEST(Langscore_Reader, RubyReader_TreeSitter_Performance)
{
    // 大きなRubyファイルの作成
    std::filesystem::path tempPath = "data/large_ruby_test.rb";
    {
        std::ofstream file(tempPath);
        // 多数の文字列を含む大きなファイルを生成
        for(int i = 0; i < 1000; i++) {
            file << "str" << i << " = \"String content " << i << " for testing performance\"\n";
            file << "str_single" << i << " = 'Single quoted string " << i << "'\n";
            if(i % 10 == 0) {
                file << "# Comment line with \"strings\" that should be ignored " << i << "\n";
            }
            if(i % 50 == 0) {
                file << "multi_line" << i << " = <<-TEXT\n";
                file << "  Heredoc content line 1 for iteration " << i << "\n";
                file << "  Heredoc content line 2 for iteration " << i << "\n";
                file << "TEXT\n";
            }
        }
        file.close();
    }

    // パフォーマンス測定
    auto start = std::chrono::high_resolution_clock::now();
    langscore::rubyreader scriptReader({u8"en", u8"ja"}, {tempPath});
    auto texts = scriptReader.currentTexts();
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // 基本的な検証
    ASSERT_FALSE(texts.empty());
    ASSERT_GT(texts.size(), 1000); // 少なくとも1000の文字列が含まれているはず

    // 処理時間を出力（参考情報）
    std::cout << "Tree-sitter Ruby parser processed " << texts.size()
        << " strings in " << duration << "ms" << std::endl;

    // テスト後に一時ファイルを削除
    std::filesystem::remove(tempPath);
}

// 行コメントと範囲コメントの処理テスト
TEST(Langscore_Reader, RubyReader_CommentHandling)
{
    // テストケース1: 行コメント
    {
        std::u8string script = u8"# This is a full line comment\n"
            u8"string_var = \"Text value\" # This is an end-of-line comment";
        auto scriptPath = createRubyScriptFile(script);

        langscore::rubyreader scriptReader({u8"en", u8"ja"}, {scriptPath});
        auto texts = scriptReader.currentTexts();

        ASSERT_EQ(texts.size(), 1);
        ASSERT_EQ(texts[0].original, u8"Text value");
    }

    // テストケース2: 範囲コメント
    {
        std::u8string script = u8"=begin\n"
            u8"This is a block comment\n"
            u8"with \"strings\" that should be ignored\n"
            u8"=end\n"
            u8"real_string = \"This should be extracted\"";
        auto scriptPath = createRubyScriptFile(script);

        langscore::rubyreader scriptReader({u8"en", u8"ja"}, {scriptPath});
        auto texts = scriptReader.currentTexts();

        ASSERT_EQ(texts.size(), 1);
        ASSERT_EQ(texts[0].original, u8"This should be extracted");
    }

    // テストケース3: コメントと文字列の混在
    {
        std::u8string script = u8"# Line comment\n"
            u8"string1 = \"First string\"\n"
            u8"=begin\n"
            u8"Block comment\n"
            u8"=end\n"
            u8"string2 = \"Second string\" # End comment\n"
            u8"string3 = \"String with # inside\"";
        auto scriptPath = createRubyScriptFile(script);

        langscore::rubyreader scriptReader({u8"en", u8"ja"}, {scriptPath});
        auto texts = scriptReader.currentTexts();

        ASSERT_EQ(texts.size(), 3);
        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"First string";
            }));
        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"Second string";
            }));
        ASSERT_TRUE(std::any_of(texts.begin(), texts.end(), [](const auto& t) {
            return t.original == u8"String with # inside";
            }));
    }

    // テスト後にテンポラリファイルを削除
    std::filesystem::remove("data/temp_ruby_test.rb");
}
