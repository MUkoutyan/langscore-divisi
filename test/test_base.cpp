IUTEST(Langscore_Config, TmpDir)
{
	langscore::config config;

	auto expected = config.langscoreAnalyzeDirectorty();
	auto build_folder = utility::cnvStr<std::u8string>(std::string{CMAKE_BUILD_TYPE_STRING});
	auto actual = fs::path(u8"D:\\Programming\\Github\\langscore-divisi\\out\\build\\"s + build_folder + u8"\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\analyze"s);
	IUTEST_ASSERT_STREQ(expected, actual.u8string());
}

IUTEST(Langscore_Config, LoadLanguages)
{
	langscore::config config(".\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	auto expected = config.languages();
	utility::stringlist actual = {"en", "ja"};
	IUTEST_ASSERT(actual.size() == expected.size());
	for(int i = 0; i < actual.size(); ++i) {
		IUTEST_ASSERT_STREQ(expected[i].name, actual[i]);
	}
}

IUTEST(Langscore_Config, CheckFontName)
{
	langscore::config config(".\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	auto expected = config.languages();
	for(int i = 0; i < expected.size(); ++i)
	{
		if(expected[i].name == "ja") {
			IUTEST_ASSERT_STREQ(expected[i].font.name, u8"メイリオ"s);
		}
		else {
			IUTEST_ASSERT_STREQ(expected[i].font.name, u8"VL Gothic"s);
		}
	}
}

IUTEST(Langscore_Config, CheckProjectPath)
{
	langscore::config config(".\\data\\vxace\\ソソポァゼゾタダＡボマミ_langscore\\config.json");

	auto expected = config.gameProjectPath();

	auto build_folder = utility::cnvStr<std::u8string>(std::string{CMAKE_BUILD_TYPE_STRING});
	auto actual = fs::path(u8"D:\\Programming\\Github\\langscore-divisi\\out\\build\\"s + build_folder + u8"\\data\\vxace\\ソポァゼゾタダＡボマミ"s);
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
		IUTEST_EXPECT_EQ(6, targetCsvList.size());

		IUTEST_ASSERT_STREQ(targetCsvList[1][0], u8"\\c[sigo]テスト\n1");
		IUTEST_ASSERT_STREQ(targetCsvList[1][1], u8"\\c[sigo]テスト\n1");
		IUTEST_ASSERT_STREQ(targetCsvList[1][2], u8"\\c[sigo]测试\n1");

		IUTEST_ASSERT_STREQ(targetCsvList[2][0], u8"\\r[sigo]テスト\n2");
		IUTEST_ASSERT_STREQ(targetCsvList[2][1], u8"\\r[sigo]テスト\n2");
		IUTEST_ASSERT_STREQ(targetCsvList[2][2], u8"\\r[sigo]测试\n2");

		IUTEST_ASSERT_STREQ(targetCsvList[3][0], u8"\\n[sigo]テスト\n3");
		IUTEST_ASSERT_STREQ(targetCsvList[3][1], u8"\\n[sigo]テスト\n3");
		IUTEST_ASSERT_STREQ(targetCsvList[3][2], u8"\\n[sigo]测试\n3");

		IUTEST_ASSERT_STREQ(targetCsvList[4][0], u8"タフルクォーテーションを含むテキストです");
		IUTEST_ASSERT_STREQ(targetCsvList[5][0], u8"タ\"フ\"ルクォーテーションを含むテキストです");
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
