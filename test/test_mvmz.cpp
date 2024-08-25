
TEST(Langscore_MV_Divisi, CheckIncludeEmptyPath)
{
	ClearGenerateFiles();
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\mv\\Include WhiteSpacePath Project_langscore\\config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	std::vector<std::u8string> includeTexts = {
		u8"Animations.csv"s,
		u8"Armors.csv"s,
		u8"Classes.csv"s,
		u8"CommonEvents.csv"s,
		u8"Enemies.csv"s,
		u8"Items.csv"s,
		u8"Map001.csv"s,
		u8"Map002.csv"s,
		u8"Map003.csv"s,
		u8"Skills.csv"s,
		u8"Scripts.csv"s,
		u8"States.csv"s,
		u8"System.csv"s,
		u8"Tilesets.csv"s,
		u8"Troops.csv"s,
		u8"Weapons.csv"s,
		u8"Actors.csv"s,
	};


	bool fail = false;
	for(auto& file : fs::directory_iterator{outputPath})
	{
		if(file.path().extension() != ".csv") { continue; }

		auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [t = file.path().filename().u8string()](const auto& x) {
			return x == t;
			});
		if(result == includeTexts.cend()) {
			GTEST_LOG_(FATAL) << "Not Found File! " << file.path().filename().string() << std::endl;
		}
	}

	if(fail) {
		GTEST_FAIL();
	}


	GTEST_SUCCEED();
}

TEST(Langscore_MV_Divisi_Analyze, ValidateFiles)
{
	ClearGenerateFiles();
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\mv\\LangscoreTest_langscore\\config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	
	std::vector<std::u8string> includeTexts = {
		u8"Animations.csv"s,
		u8"Armors.csv"s,
		u8"Classes.csv"s,
		u8"CommonEvents.csv"s,
		u8"Enemies.csv"s,
		u8"Items.csv"s,
		u8"Map001.csv"s,
		u8"Map002.csv"s,
		u8"Map003.csv"s,
		u8"Skills.csv"s,
		u8"Scripts.csv"s,
		u8"States.csv"s,
		u8"System.csv"s,
		u8"Tilesets.csv"s,
		u8"Troops.csv"s,
		u8"Weapons.csv"s,
		u8"Actors.csv"s,
	};


	bool fail = false;
	for(auto& file : fs::directory_iterator{outputPath})
	{
		if(file.path().extension() != ".csv") { continue; }

		auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [t = file.path().filename().u8string()](const auto& x) {
			return x == t;
		});
		if(result == includeTexts.cend()) {
			GTEST_LOG_(FATAL) << "Not Found File! " << file.path().filename().string() << std::endl;
		}
	}

	if(fail) {
		GTEST_FAIL();
	}


	GTEST_SUCCEED();
}

TEST(Langscore_MV_Divisi_Analyze, ValidateTexts)
{
	ClearGenerateFiles();
	//テキストが一致するかの整合性を確認するテスト
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\mv\\LangscoreTest_langscore\\config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	ASSERT_TRUE(fs::exists(outputPath / "Map001.csv"));
	auto scriptCsv = plaincsvreader{outputPath / "Map001.csv"}.getPlainCsvTexts();
	ASSERT_TRUE(scriptCsv.empty() == false);

	std::vector<std::u8string> includeTexts = {
		u8"original",
		u8"通常のテキストです"s,
		u8"改行を含む\nテキストです"s,
		u8"カンマを含む,テキストです"s,
		u8"\"タ\"フ\"ルクォーテーションを含むテキストです\""s,
		u8"\"\"\"Hello, World\"\",\nそれはプログラムを書く際の\",\"\"\"謎の呪文\"\"(Mystery spell)―――\"",
		u8"名前変えるよ"s,
		u8"\\{言語\\}変える\\}よ"s,
		u8"日本語"s,
		u8"英語"s,
		u8"中国語"s,
		u8"勝ち"s,
		u8"逃げ犬"s,
		u8"負け犬"s,
		u8"プロフィール変えるよ"s,
		u8"ラフィーナ"s,
		u8"エルーシェ"s,
		u8"やめる"s,
		u8"二つ名変えるよ"s,
		u8"雑用係"s,
		u8"傲慢ちき"s,
		u8"無くす"s,
		u8"フィールドを移動します"s,
		u8"行って帰る"s,
		u8"行ったきり"s,
		u8"やめる"s,
		u8"やっぱやめる"s,
		u8"移動後のメッセージです。"s,
		u8"更に移動した際のメッセージです"s,
		u8"チビのツンデレウーマン。\n魔法が得意。"s,
		u8"ケスティニアスの雑用係。\nそんなに仕事は無い。"s,
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
見開く緑目は"私"を見つめ———)"s,
		u8R"(０１２３４５６７８９０１２３４５６７８９０１２３４５６７８９
012345678901234567890123456789012345678901234567890123456789
ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺＡＢＣＤＥＦＧ
ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ)",
	};


	for(auto& row : scriptCsv)
	{
		for(auto& t : row) {
			auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [&t](const auto& x) {
				return x == t;
				});
			if(result == includeTexts.cend()) {
				GTEST_LOG_(FATAL) << "Not Found!" << std::string(t.begin(), t.end()) << std::endl;
				GTEST_FAIL();
			}
		}
	}
	GTEST_SUCCEED();
}


TEST(Langscore_MV_Divisi_Write, ValidateFiles)
{
	ClearGenerateFiles();
	//テキストが一致するかの整合性を確認するテスト
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\mv\\LangscoreTest_langscore\\config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	
	ASSERT_TRUE(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreProjectPath()) / "data" / "translate";

	std::vector<std::u8string> includeTexts = {
		u8"Animations.csv"s,
		u8"Armors.csv"s,
		u8"Classes.csv"s,
		u8"CommonEvents.csv"s,
		u8"Enemies.csv"s,
		u8"Items.csv"s,
		u8"Graphics.csv"s,
		u8"Map001.csv"s,
		u8"Map002.csv"s,
		u8"Map003.csv"s,
		u8"Skills.csv"s,
		u8"Scripts.csv"s,
		u8"States.csv"s,
		u8"System.csv"s,
		u8"Tilesets.csv"s,
		u8"Troops.csv"s,
		u8"Weapons.csv"s,
		u8"Actors.csv"s,
	};


	bool fail = false;
	for(auto& file : fs::directory_iterator{outputPath})
	{
		if(file.path().extension() != ".csv") { continue; }

		auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [t = file.path().filename().u8string()](const auto& x) {
			return x == t;
			});
		if(result == includeTexts.cend()) {
			GTEST_LOG_(FATAL) << "Not Found File! " << file.path().filename().string() << std::endl;
		}
	}

	if(fail) {
		GTEST_FAIL();
	}


	GTEST_SUCCEED();
}


TEST(Langscore_MV_Divisi_Write, WritePluginJS)
{
	ClearGenerateFiles();
	//テキストが一致するかの整合性を確認するテスト
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\mv\\LangscoreTest_langscore\\config.json");

	ASSERT_TRUE(divisi.analyze().valid());

	ASSERT_TRUE(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.gameProjectPath());

	auto pluginJs = outputPath / "js" / "plugins.js";
	ASSERT_TRUE(fs::exists(pluginJs));

	std::ifstream loadFile(pluginJs);
	std::string content((std::istreambuf_iterator<char>(loadFile)), std::istreambuf_iterator<char>());
	loadFile.close();

	std::size_t startPos = content.find('[');
	std::size_t endPos = content.rfind(']');

	if(startPos == std::string::npos || endPos == std::string::npos) {
		GTEST_LOG_(FATAL) << "Not Found Backet" << std::endl;
	}

	auto jsonStr = utility::cnvStr<std::u8string>(content.substr(startPos, endPos - startPos + 1));

	nlohmann::json jsonStruct = nlohmann::json::parse(jsonStr, nullptr);
	//plugins.jsの読み込み失敗
	if(jsonStruct.is_discarded()) {
		GTEST_LOG_(FATAL) << "Failure to load plugins.js" << std::endl;
		GTEST_FAIL();
	}
	
	auto item = jsonStruct.begin();
	auto firstScriptName = (*item)["name"].get<std::string>();
	ASSERT_STREQ(firstScriptName.c_str(), "Langscore");

	GTEST_SUCCEED();
}


TEST(Langscore_MV_Divisi_Write, WritePluginJS_WhenNoPlugins)
{
	ClearGenerateFiles();
	//テキストが一致するかの整合性を確認するテスト
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", ".\\data\\mv\\LangscoreTest_NoPlugins_langscore\\config.json");

	ASSERT_TRUE(divisi.analyze().valid());

	ASSERT_TRUE(divisi.write().valid());
	langscore::config config;
	auto outputPath = fs::path(config.gameProjectPath());

	auto pluginJs = outputPath / "js" / "plugins.js";
	ASSERT_TRUE(fs::exists(pluginJs));

	std::ifstream loadFile(pluginJs);
	std::string content((std::istreambuf_iterator<char>(loadFile)), std::istreambuf_iterator<char>());
	loadFile.close();

	std::size_t startPos = content.find('[');
	std::size_t endPos = content.rfind(']');

	if(startPos == std::string::npos || endPos == std::string::npos) {
		GTEST_LOG_(FATAL) << "Not Found Backet" << std::endl;
	}

	auto jsonStr = utility::cnvStr<std::u8string>(content.substr(startPos, endPos - startPos + 1));

	nlohmann::json jsonStruct = nlohmann::json::parse(jsonStr, nullptr);
	//plugins.jsの読み込み失敗
	if(jsonStruct.is_discarded()) {
		GTEST_LOG_(FATAL) << "Failure to load plugins.js" << std::endl;
		GTEST_FAIL();
	}

	auto item = jsonStruct.begin();
	auto firstScriptName = (*item)["name"].get<std::string>();
	ASSERT_STREQ(firstScriptName.c_str(), "Langscore");

	GTEST_SUCCEED();
}


