
IUTEST(Langscore_VXAce_Invoker, NoAssignProject)
{
	langscore::config::detachConfigFile();
	langscore::config::attachConfigFile(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_Invalid_langscore\\config.json");
	langscore::invoker invoker;
	auto result = invoker.analyze();
	IUTEST_ASSERT_EQ(result.moduleCode(), ErrorStatus::Module::INVOKER);
	IUTEST_ASSERT_EQ(result.code(), 1);
}

IUTEST(Langscore_VXAce_Invoker, AnalyzeVXAceProject)
{
	langscore::config::detachConfigFile();
	langscore::config::attachConfigFile(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_langscore\\config.json");
	langscore::config config;
	ClearGenerateFiles(config);

	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	std::filesystem::remove_all(outputPath);
	langscore::invoker invoker;
	invoker.setProjectPath(config.gameProjectPath());
	auto analyzeResult = invoker.analyze();
	IUTEST_ASSERT(analyzeResult.valid());

	auto itr = fs::recursive_directory_iterator(outputPath);
	auto numFiles = std::distance(itr, fs::recursive_directory_iterator{});
	IUTEST_ASSERT_LT(135, numFiles);

	ClearGenerateFiles(config);
}

IUTEST(Langscore_VXAce_Invoker, AnalyzeWhiteSpaceVXAceProject)
{
	langscore::config::detachConfigFile();
	langscore::config::attachConfigFile(".\\data\\vxace\\Include WhiteSpacePath Project_langscore\\config.json");
	langscore::config config;
	ClearGenerateFiles(config);

	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	std::filesystem::remove_all(outputPath);
	langscore::invoker invoker;
	invoker.setProjectPath(config.gameProjectPath());
	auto analyzeResult = invoker.analyze();
	IUTEST_ASSERT(analyzeResult.valid());

	auto itr = fs::recursive_directory_iterator(outputPath);
	auto numFiles = std::distance(itr, fs::recursive_directory_iterator{});
	IUTEST_ASSERT_LT(135, numFiles);

	ClearGenerateFiles(config);
}

IUTEST(Langscore_VXAce_Invoker, CheckValidScriptList)
{
	langscore::config::detachConfigFile();
	langscore::config::attachConfigFile(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_langscore\\config.json");
	langscore::config config;
	ClearGenerateFiles(config);

	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	langscore::invoker invoker;
	invoker.setProjectPath(config.gameProjectPath());
	invoker.analyze();

	IUTEST_ASSERT(fs::exists(outputPath / "Scripts/_list.csv"));
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
			IUTEST_FAIL();
		}
	}

	std::filesystem::remove_all(outputPath);

	IUTEST_SUCCEED();
}


IUTEST(Langscore_VXAce_Divisi, IUTEST_JAPANESE_NAME(�󔒂��܂ރp�X�̉��߃`�F�b�N))
{
	ClearGenerateFiles();
	//�e�L�X�g����v���邩�̐��������m�F����e�X�g
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\vxace\\Include WhiteSpacePath Project_langscore\\config.json");

	IUTEST_ASSERT(divisi.analyze().valid());
	IUTEST_ASSERT(divisi.write().valid());
	IUTEST_ASSERT(divisi.update().valid());
}

IUTEST(Langscore_VXAce_Divisi_Analyze, ValidateTexts)
{
	ClearGenerateFiles();
	//�e�L�X�g����v���邩�̐��������m�F����e�X�g
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_langscore\\config.json");

	IUTEST_ASSERT(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	IUTEST_ASSERT(fs::exists(outputPath / "Map001.csv"));
	auto scriptCsv = plaincsvreader{outputPath / "Map001.csv"}.getPlainCsvTexts();
	IUTEST_ASSERT(scriptCsv.empty() == false);

	std::vector<std::u8string> includeTexts = {
		u8"original",
		u8"�}�b�v��"s,
		u8"12345����ɂ��͐��E HelloWorld\n"s,
		u8"12345HHEELLOO\n"s,
		u8"����͒ǉ��e�L�X�g�ł�\n"s,
		u8"����@�؂�ւ����\n"s,
		u8"\n�蓮�Œ�������\n"s,
		u8"\n\\{��������+�t�H���g�T�C�Y��\n"s,
		u8"���{��"s,
		u8"�p��"s,
		u8"������"s,
	};

	for(auto& row : scriptCsv)
	{
		for(auto& t : row) {
			auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [&t](const auto& x) {
				return x == t;
			});
			if(result == includeTexts.cend()) {
				std::cout << "Not Found!" << std::string(t.begin(), t.end()) << std::endl;
				IUTEST_FAIL();
			}
		}
	}
	IUTEST_SUCCEED();
}

IUTEST(Langscore_VXAce_Divisi, CheckLangscoreRubyScript)
{
	ClearGenerateFiles();
	{
		langscore::config::detachConfigFile();
		langscore::divisi divisi("./", ".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_langscore\\config.json");
		IUTEST_ASSERT(divisi.analyze().valid());
	}
	{
		//analyze��write�𓯎��ɌĂяo�����Ƃ�z�肵�Ă��Ȃ��B
		//analyze���Ăяo���ƃR���X�g���N�g���̌��ꃊ�X�g������������邽�߁A
		//�C���X�^���X�͕ʂɕ�����B
		langscore::config::detachConfigFile();
		langscore::divisi divisi("./", ".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_langscore\\config.json");
		IUTEST_ASSERT(divisi.write().valid());
	}
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();
	fs::path langscoreFilename;
	{
		auto result = std::ranges::find_if(scriptList, [](const auto& x) {
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
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"���C���I");
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
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"���ݑI�𒆂̌��ꂪ�\������܂��B");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"en");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"The currently selected language is displayed.");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"ja");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"���ݑI�𒆂̌��ꂪ�\������܂��B");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"en");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"English");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"ja");
	IUTEST_ASSERT_STREQ(writedCsvTexts[i++].original, u8"���{��");
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

	if(i != 75) {
		IUTEST_SCOPED_TRACE(::iutest::Message() << "The number of tests in the string does not match. : " << i);
		IUTEST_FAIL();
	}
}

IUTEST(Langscore_VXAce_Divisi, CheckScriptCSV)
{
	ClearGenerateFiles();
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_langscore\\config.json");

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
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [&obj](const auto& x) {
			return obj.original.find(x[0]) == std::u8string::npos;
			});
		if(result == scriptList.cend()) {
			std::cout << "Not Found!" << std::string((*result)[0].begin(), (*result)[0].end()) << std::endl;
			IUTEST_FAIL();
		}
	}

	IUTEST_SUCCEED();
}

IUTEST(Langscore_VXAce_Divisi, WriteVXAceProject)
{
	ClearGenerateFiles();
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_langscore\\config.json");

	fs::path scriptDataSrc(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~\\Data\\Scripts_backup.rvdata2");


	if(fs::exists(scriptDataDest) == false) {
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	IUTEST_ASSERT(divisi.analyze().valid());
	IUTEST_ASSERT(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	IUTEST_ASSERT(fs::exists(outputPath / "Scripts.csv"));
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	fs::path langscoreCustomFilename;
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x) {
			return x[1] == platform_base::Custom_Script_File_Name;
			});
		IUTEST_ASSERT(result != scriptList.cend());
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x) {
			return x[1] == platform_base::Script_File_Name;
			});
		IUTEST_ASSERT(result != scriptList.cend());
	}

	fs::copy(scriptDataDest, scriptDataSrc, fs::copy_options::overwrite_existing);

	IUTEST_SUCCEED();
}

IUTEST(Langscore_VXAce_Divisi, WriteVocab)
{
	ClearGenerateFiles();
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_langscore\\config.json");

	fs::path scriptDataSrc(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~\\Data\\Scripts_backup.rvdata2");

	if(fs::exists(scriptDataDest) == false) {
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	IUTEST_ASSERT(divisi.analyze().valid());
	IUTEST_ASSERT(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	IUTEST_ASSERT(fs::exists(outputPath / "Scripts.csv"));
	auto scriptList = plaincsvreader{outputPath / "Scripts/_list.csv"}.getPlainCsvTexts();

	fs::path langscoreCustomFilename;
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x) {
			return x[1] == platform_base::Custom_Script_File_Name;
			});
		IUTEST_ASSERT(result != scriptList.cend());
		langscoreCustomFilename = outputPath / "Scripts" / (std::u8string((*result)[0]) + u8".rb"s);
	}
	{
		auto result = std::find_if(scriptList.cbegin(), scriptList.cend(), [](const auto& x) {
			return x[1] == platform_base::Script_File_Name;
			});
		IUTEST_ASSERT(result != scriptList.cend());
	}

	fs::copy(scriptDataDest, scriptDataSrc, fs::copy_options::overwrite_existing);

	IUTEST_SUCCEED();
}

IUTEST(Langscore_VXAce_Divisi, WriteLangscoreCustom)
{
	ClearGenerateFiles();
	langscore::config config;
	langscore::divisi_vxace divisi_vxace;
	divisi_vxace.setAppPath("./");
	divisi_vxace.setProjectPath(config.gameProjectPath());
	divisi_vxace.analyze();

	std::u8string fileName = u8"57856563";	//Cache�X�N���v�g
	langscore::rubyreader rubyReader({u8"en", u8"ja"}, {u8".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_langscore\\analyze\\Scripts\\"s + fileName + u8".rb"s});
	langscore::rbscriptwriter scriptWriter(rubyReader);

	const auto outputFileName = "./data/langscore_custom.rb"s;
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

IUTEST(Langscore_VXAce_Divisi, ValidateLangscoreCustom)
{
	ClearGenerateFiles();
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~_langscore\\config.json");

	fs::path scriptDataSrc(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~\\Data\\Scripts_backup.rvdata2");

	if(fs::exists(scriptDataDest) == false) {
		fs::copy(scriptDataSrc, scriptDataDest, fs::copy_options::overwrite_existing);
	}

	IUTEST_ASSERT(divisi.analyze().valid());
	IUTEST_ASSERT(divisi.write().valid());
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
	IUTEST_ASSERT(FindString("def Langscore.Translate_Script_Text"s));
	IUTEST_ASSERT(FindString("def Langscore.translate_"s));

	fs::copy(scriptDataDest, scriptDataSrc, fs::copy_options::overwrite_existing);

	IUTEST_SUCCEED();
}

IUTEST(Langscore_VXAce_Divisi, VXAce_WriteScriptCSV)
{
	ClearGenerateFiles();
	//Langscore_custom�ɈӐ}�����ʂ�ɓ��e���������܂�Ă��邩�̃e�X�g
	fs::path scriptDataSrc(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~\\Data\\Scripts.rvdata2");
	fs::path scriptDataDest(".\\data\\vxace\\�\�|�@�[�]�^�_�`�{�}�~\\Data\\Scripts_backup.rvdata2");


	if(fs::exists(scriptDataDest) == false) {
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


	for(const auto& script : scriptWriter.scriptTranslatesMap)
	{
		//������̂Ȃ��X�N���v�g�t�@�C���𖳎�����
		if(std::get<1>(script).empty()) { continue; }
		auto fileName = std::get<0>(script);

		const auto& scriptName = scriptWriter.GetScriptName(fileName);
		if(scriptName.empty()) { continue; }
		IUTEST_ASSERT_STRNE(scriptName, platform_base::Script_File_Name);
		IUTEST_ASSERT_STRNE(scriptName, platform_base::Custom_Script_File_Name);

		auto path = outputPath / "Scripts" / fileName;

		if(fs::file_size(path) == 0) { continue; }

		auto scriptFuncName = "def "s + funcName(utility::cnvStr<std::string>(fileName));
		if(FindString(scriptFuncName) == false)
		{
			IUTEST_SCOPED_TRACE(::iutest::Message() << "NotFound Line " << scriptFuncName);
			IUTEST_SCOPED_TRACE(::iutest::Message() << "Script Name " << utility::cnvStr<std::string>(scriptName));
			IUTEST_FAIL();
		}
	}

}

IUTEST(Langscore_VXAce_Divisi, VXAce_FindEscChar)
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
		std::u8string text = u8"�ق�\\l[16]�ق�";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() == false && result2.empty());
		IUTEST_ASSERT_STREQ(result1[0], u8"\\l[16]");
	}
	{
		std::u8string text = u8"������\\{����";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() && result2.empty() == false);
		IUTEST_ASSERT_STREQ(result2[0], u8"\\{");
	}
	{
		std::u8string text = u8"99999\\G ��ɓ��ꂽ�I";
		auto [result1, result2] = divisi_vxace.findRPGMakerEscChars(text);
		IUTEST_ASSERT(result1.empty() && result2.empty() == false);
		IUTEST_ASSERT_STREQ(result2[0], u8"\\G");
	}
}

IUTEST(Langscore_VXAce_Divisi, VXAce_Validate)
{
	langscore::config config;
	langscore::divisi_vxace divisi_vxace;

	{
		langscore::TranslateText text;
		text.original = u8"HoniHoni";
		text.translates[u8"en"] = u8"HoniHoni";
		text.translates[u8"ja"] = u8"�قɂق�";
		text.translates[u8"zh-tw"] = u8"�[�[�n";
		text.translates[u8"zh-cn"] = u8"�[�[�n";

		IUTEST_ASSERT(divisi_vxace.validateTranslateList({text}, ""));
	}
	{
		langscore::TranslateText text;
		text.original = u8"Honi\\V[10]Honi";
		text.translates[u8"en"] = u8"Honi\\V[10]Honi";
		text.translates[u8"ja"] = u8"�ق�\\V[10]�ق�";
		text.translates[u8"zh-tw"] = u8"�[�[\\V[10]�n";
		text.translates[u8"zh-cn"] = u8"�[�[\\V[10]�n";

		IUTEST_ASSERT(divisi_vxace.validateTranslateList({text}, ""));
	}
	{
		langscore::TranslateText text;
		text.original = u8"Honi\\V[10]Honi";
		text.translates[u8"en"] = u8"HoniHoni";
		text.translates[u8"ja"] = u8"�قɂق�";
		text.translates[u8"zh-tw"] = u8"�[�[�n";
		text.translates[u8"zh-cn"] = u8"�[�[�n";

		IUTEST_ASSERT_NOT(divisi_vxace.validateTranslateList({text}, ""));
	}
}
