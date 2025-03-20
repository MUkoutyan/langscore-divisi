
TEST(Langscore_MV_Divisi, CheckIncludeEmptyPath)
{
	ClearGenerateFiles();
    checkAndCreateConfigFile("data/mv/Include WhiteSpacePath Project_langscore/config.json", "Game.rpgproject");
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/mv/Include WhiteSpacePath Project_langscore/config.json");

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
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");

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
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");
	//テキストが一致するかの整合性を確認するテスト
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	ASSERT_TRUE(fs::exists(outputPath / "Map001.csv"));
	auto scriptCsv = plaincsvreader{outputPath / "Map001.csv"}.getPlainCsvTexts();
	ASSERT_TRUE(scriptCsv.empty() == false);

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
        u8"プロフィール変えるよ", u8"チビのツンデレウーマン。\n魔法が得意。", u8"ケスティニアスの雑用係。\nそんなに仕事は無い。",
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
        //for(auto& t : row) 
        auto& t = row[0];
        {
            auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [&t](const auto& x) {
                return x == t;
                });
            if(result == includeTexts.cend()) {
                GTEST_FAIL() << "Not Found!" << std::string(t.begin(), t.end());
            }
        }
    }
}


TEST(Langscore_MV_Divisi_Write, ValidateFiles)
{
	ClearGenerateFiles();
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");
	//テキストが一致するかの整合性を確認するテスト
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	
	ASSERT_TRUE(divisi.exportCSV().valid());
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
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");
	langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");

	ASSERT_TRUE(divisi.analyze().valid());

	ASSERT_TRUE(divisi.updatePlugin().valid());
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
        GTEST_FAIL() << "Failure to load plugins.js" << std::endl;
	}
	
	auto item = jsonStruct.begin();
	auto firstScriptName = (*item)["name"].get<std::string>();
	ASSERT_STREQ(firstScriptName.c_str(), "Langscore");

	GTEST_SUCCEED();
}


TEST(Langscore_MV_Divisi_Write, WritePluginJS_WhenNoPlugins)
{
	ClearGenerateFiles();
    checkAndCreateConfigFile("data/mv/LangscoreTest_NoPlugins_langscore/config.json", "Game.rpgproject");
	//テキストが一致するかの整合性を確認するテスト
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/mv/LangscoreTest_NoPlugins_langscore/config.json");

	ASSERT_TRUE(divisi.analyze().valid());

	ASSERT_TRUE(divisi.updatePlugin().valid());
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
        GTEST_FAIL() << "Not Found Backet" << std::endl;
	}

	auto jsonStr = utility::cnvStr<std::u8string>(content.substr(startPos, endPos - startPos + 1));

	nlohmann::json jsonStruct = nlohmann::json::parse(jsonStr, nullptr);
	//plugins.jsの読み込み失敗
	if(jsonStruct.is_discarded()) {
        GTEST_FAIL() << "Failure to load plugins.js" << std::endl;
	}

	auto item = jsonStruct.begin();
	auto firstScriptName = (*item)["name"].get<std::string>();
	ASSERT_STREQ(firstScriptName.c_str(), "Langscore");

	GTEST_SUCCEED();
}



TEST(Langscore_MV_Divisi_Write, Multiple_Export)
{
    ClearGenerateFiles();
    checkAndCreateConfigFile("data/mv/LangscoreTest_MultipleExport_langscore/config.json", "Game.rpgproject");
    //テキストが一致するかの整合性を確認するテスト
    langscore::config::detachConfigFile();
    langscore::divisi divisi("./", fs::path(BINARYT_DIRECTORY) / "data/mv/LangscoreTest_MultipleExport_langscore/config.json");

    ASSERT_TRUE(divisi.analyze().valid());

    ASSERT_TRUE(divisi.exportCSV().valid());

    fs::path root("data/mv/LangscoreTest_MultipleExport_langscore/data/translate");
    std::vector<std::string> checkLangs = {"en", "ja", "zh-cn", "zh-tw"};
    for(auto lang : checkLangs)
    {
        auto checkDir = root / lang;
        ASSERT_TRUE(fs::exists(checkDir)) << "Failed : " << lang;
        int numFiles = 0;
        fs::path firstFile;
        for(auto i : fs::directory_iterator{checkDir}) { 
            if(firstFile.empty()) { firstFile = i; }
            numFiles++; 
        }
        ASSERT_EQ(numFiles, 15) << "Failed : " << lang;

        auto texts = plaincsvreader{firstFile}.getPlainCsvTexts();
        ASSERT_FALSE(texts.empty()) << "Failed : " << lang;
        ASSERT_EQ(texts[0].size(), 2) << "Failed : " << lang;
    }

    GTEST_SUCCEED();
}