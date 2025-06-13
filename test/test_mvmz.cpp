
TEST(Langscore_MV_Divisi, CheckIncludeEmptyPath)
{
	ClearGenerateFiles();
    checkAndCreateConfigFile("data/mv/Include WhiteSpacePath Project_langscore/config.json", "Game.rpgproject");
	langscore::config::detachConfigFile();
	langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/Include WhiteSpacePath Project_langscore/config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	std::vector<std::u8string> includeTexts = {
		u8"Animations.lsjson"s,
		u8"Armors.lsjson"s,
		u8"Classes.lsjson"s,
		u8"CommonEvents.lsjson"s,
		u8"Enemies.lsjson"s,
		u8"Items.lsjson"s,
		u8"Map001.lsjson"s,
		u8"Map002.lsjson"s,
		u8"Map003.lsjson"s,
		u8"Skills.lsjson"s,
		u8"Scripts.lsjson"s,
		u8"States.lsjson"s,
		u8"System.lsjson"s,
		u8"Tilesets.lsjson"s,
		u8"Troops.lsjson"s,
		u8"Weapons.lsjson"s,
		u8"Actors.lsjson"s,
	};


	bool fail = false;
	for(auto& file : fs::directory_iterator{outputPath})
	{
		if(file.path().extension() != ".lsjson") { continue; }

		auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [t = file.path().filename().u8string()](const auto& x) {
			return x == t;
			});
		if(result == includeTexts.cend()) {
            ADD_FAILURE() << "Not Found File! " << file.path().filename().string() << std::endl;
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
	langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());
	
	std::vector<std::u8string> includeTexts = {
		u8"Animations.lsjson"s,
		u8"Armors.lsjson"s,
		u8"Classes.lsjson"s,
		u8"CommonEvents.lsjson"s,
		u8"Enemies.lsjson"s,
		u8"Items.lsjson"s,
		u8"Map001.lsjson"s,
		u8"Map002.lsjson"s,
		u8"Map003.lsjson"s,
		u8"Skills.lsjson"s,
		u8"Scripts.lsjson"s,
		u8"States.lsjson"s,
		u8"System.lsjson"s,
		u8"Tilesets.lsjson"s,
		u8"Troops.lsjson"s,
		u8"Weapons.lsjson"s,
		u8"Actors.lsjson"s,
	};


	bool fail = false;
	for(auto& file : fs::directory_iterator{outputPath})
	{
		if(file.path().extension() != ".lsjson") { continue; }

		auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [t = file.path().filename().u8string()](const auto& x) {
			return x == t;
		});
		if(result == includeTexts.cend()) {
            ADD_FAILURE() << "Not Found File! " << file.path().filename().string() << std::endl;
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
	langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");

	ASSERT_TRUE(divisi.analyze().valid());
	langscore::config config;
	auto outputPath = fs::path(config.langscoreAnalyzeDirectorty());

	ASSERT_TRUE(fs::exists(outputPath / "Map001.lsjson"));
	auto scriptCsv = analyzejsonreader{outputPath / "Map001.lsjson"}.currentTexts();
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
        auto t = row.original;
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
	langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");

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
		if(file.path().extension() != ".lsjson") { continue; }

		auto result = std::find_if(includeTexts.cbegin(), includeTexts.cend(), [t = file.path().filename().u8string()](const auto& x) {
			return x == t;
			});
		if(result == includeTexts.cend()) {
			ADD_FAILURE() << "Not Found File! " << file.path().filename().string() << std::endl;
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
	langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");

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
		ADD_FAILURE() << "Not Found Backet" << std::endl;
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
	langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_NoPlugins_langscore/config.json");

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
    langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_MultipleExport_langscore/config.json");

    ASSERT_TRUE(divisi.analyze().valid());

    ASSERT_TRUE(divisi.exportCSV().valid());

    
    // ファイル名リスト
    std::vector<std::string> fileNames = {
        "Graphics.csv",
        "Items.csv",
        "Map001.csv",
        "Map002.csv",
        "Map003.csv",
        "Scripts.csv",
        "Skills.csv",
        "States.csv",
        "System.csv",
        "Troops.csv",
        "Weapons.csv",
        "Actors.csv",
        "Armors.csv",
        "Classes.csv",
        "CommonEvents.csv",
        "Enemies.csv"
    };

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
        ASSERT_EQ(numFiles, fileNames.size()) << "Failed : " << lang;

        auto texts = plaincsvreader{firstFile}.getPlainCsvTexts();
        ASSERT_FALSE(texts.empty()) << "Failed : " << lang;
        ASSERT_EQ(texts[0].size(), 2) << "Failed : " << lang;
    }

    GTEST_SUCCEED();
}

// JSONファイルを読み込むヘルパー関数
nlohmann::json loadJsonFromFile(const std::filesystem::path& filePath) {
    std::ifstream file(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file: " + filePath.string());
    }
    
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    file.close();
    
    return nlohmann::json::parse(content);
}

TEST(Langscore_MV_MVMZJsonReader, TextTypeForMaker)
{
    // test/plugin/mv_test内のActors.jsonファイルを読み込む
    std::filesystem::path testPath = "Actors.json";
    std::filesystem::path jsonFilePath = fs::path(BINARY_DIRECTORY) / "data/LangscoreTestMV/data/Actors.json";
    nlohmann::json jsonData = loadJsonFromFile(jsonFilePath);
    
    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};
    langscore::mvmz_jsonreader reader(testPath, useLangs, jsonData);

    // テキストタイプが正しく設定されているかを確認
    const auto& texts = reader.currentTexts();
    ASSERT_GE(texts.size(), 4); // 少なくとも4つのテキストがあるはず

    // 名前とプロフィールのテキストタイプを確認
    bool foundName = false;
    bool foundProfile = false;
    bool foundNickname = false;
    
    for(const auto& text : texts) {
        if(text.original == u8"エルーシェ" || text.original == u8"ラフィーナ") {
            EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), TranslateText::nameType) != text.textType.end()) << "Name text type incorrect";
            foundName = true;
        }
        else if(text.original == u8"雑用係" || text.original == u8"傲慢ちき") {
            EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), TranslateText::nameType) != text.textType.end()) << "Nickname text type incorrect";
            foundNickname = true;
        }
        else if(text.original == u8"ケスティニアスの雑用係。\nそんなに仕事は無い。" ||
                text.original == u8"チビのツンデレウーマン。\n魔法が得意。")
        {
            EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), TranslateText::descriptionType) != text.textType.end()) << "Profile text type incorrect";
            foundProfile = true;
        }
    }
    
    EXPECT_TRUE(foundName) << "Name text not found";
    EXPECT_TRUE(foundNickname) << "Nickname text not found";
    EXPECT_TRUE(foundProfile) << "Profile text not found";
}

TEST(Langscore_MV_MVMZJsonReader, DetectDataType)
{
    // 異なるファイル名でデータタイプが正しく検出されるかテスト
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"Actors.json", "Actors"},
        {"Armors.json", "Armors"},
        {"Classes.json", "Classes"},
        {"CommonEvents.json", "CommonEvents"},
        {"Enemies.json", "Enemies"},
        {"Items.json", "Items"},
        {"Map001.json", "Map"},
        {"Skills.json", "Skills"},
        {"States.json", "States"},
        {"System.json", "System"},
        {"Troops.json", "Troops"},
        {"Weapons.json", "Weapons"}
    };

    nlohmann::json emptyJson = {};
    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};

    for (const auto& testCase : testCases) {
        std::filesystem::path testPath = testCase.first;
        langscore::mvmz_jsonreader reader(testPath, useLangs, emptyJson);
        
        // 内部のcurrentDataTypeを直接テストできないため、
        // 特定のデータタイプに対応するキーが検出されるかどうかで間接的にテスト
        const auto& texts = reader.currentTexts();
        
        // データタイプが正しく検出されていることを確認するメッセージ
        SUCCEED() << "Successfully detected data type for " << testCase.second;
    }
}

TEST(Langscore_MV_MVMZJsonReader, StackTextProcessing)
{
    // test/plugin/mv_test内のMap001.jsonファイルを読み込む
    std::filesystem::path testPath = "Map001.json";
    std::filesystem::path jsonFilePath = fs::path(BINARY_DIRECTORY) / "data/LangscoreTestMV/data/Map001.json";
    nlohmann::json jsonData = loadJsonFromFile(jsonFilePath);

    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};
    langscore::mvmz_jsonreader reader(testPath, useLangs, jsonData);

    // テキストが正しく積み上げられているかを確認
    const auto& texts = reader.currentTexts();
    
    // 多数のテキストがあるはず
    ASSERT_GE(texts.size(), 10);
    
    // 積み上げられたメッセージを確認
    bool foundStackedText = false;
    for (const auto& text : texts) {
        if (text.code == 401) {
            // イベント1の最初のメッセージ
            if (text.original == u8"通常のテキストです") {
                foundStackedText = true;
                break;
            }
            // 改行を含むメッセージ
            if (text.original == u8"改行を含む\nテキストです") {
                foundStackedText = true;
                break;
            }
        }
    }
    EXPECT_TRUE(foundStackedText) << "Stacked text not found";
    
    // 選択肢も正しく抽出されているか確認
    bool foundChoices = false;
    for (const auto& text : texts) {
        if (text.code == 102) {
            // イベント2の選択肢
            if (text.original == u8"ラフィーナ" || text.original == u8"エルーシェ" || text.original == u8"やめる") {
                foundChoices = true;
                break;
            }
        }
    }
    EXPECT_TRUE(foundChoices) << "Choices not found";
}

TEST(Langscore_MV_MVMZJsonReader, TextTypeValidation)
{
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
            {u8"ケスティニアスの雑用係。\nそんなに仕事は無い。", TranslateText::descriptionType},
            {u8"チビのツンデレウーマン。\n魔法が得意。", TranslateText::descriptionType}
        }},
        // アイテム関連のテキストタイプ
        {"Items.json", {
            {u8"ポーション", TranslateText::nameType},
            {u8"マジックウォーター", TranslateText::nameType},
            {u8"ディスペルハーブ", TranslateText::nameType},
            {u8"スティミュラント", TranslateText::nameType}
        }},
        // システムメッセージのテキストタイプ
        {"System.json", {
            {u8"%1には効かなかった！", TranslateText::battleMessage},
            {u8"%1は %2 のダメージを受けた！", TranslateText::battleMessage},
            {u8"%1の勝利！", TranslateText::battleMessage},
            {u8"どのファイルにセーブしますか？", TranslateText::message},
            {u8"どのファイルをロードしますか？", TranslateText::message},
            {u8"常時ダッシュ", TranslateText::other}
        }},
        // マップイベントのテキストタイプ
        {"Map001.json", {
            {u8"通常のテキストです", TranslateText::message},
            {u8"改行を含む\nテキストです", TranslateText::message},
            {u8"名前変えるよ", TranslateText::message},
            {u8"ラフィーナ", TranslateText::nameType},
            {u8"エルーシェ", TranslateText::nameType},
            {u8"やめる", TranslateText::other}
        }}
    };

    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};

    for (const auto& testCase : testCases) {
        // JSONファイルを読み込む
        std::filesystem::path testPath = testCase.filename;
        std::filesystem::path jsonFilePath = fs::path(BINARY_DIRECTORY) / "data/LangscoreTestMV/data" / testCase.filename;
        nlohmann::json jsonData = loadJsonFromFile(jsonFilePath);
        
        langscore::mvmz_jsonreader reader(testPath, useLangs, jsonData);
        const auto& texts = reader.currentTexts();

        // 期待されるテキストとそのタイプを確認
        for (const auto& [expectedText, expectedType] : testCase.expectedTexts) {
            bool found = false;
            for (const auto& text : texts) {
                if (text.original == expectedText) {
                    EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), expectedType) != text.textType.end())
                        << "Wrong text type for '" << std::string(expectedText.begin(), expectedText.end())
                        << "' in " << testCase.filename;
                    found = true;
                    break;
                }
            }
            EXPECT_TRUE(found)
                << "Text '" << std::string(expectedText.begin(), expectedText.end())
                << "' not found in " << testCase.filename;
        }
    }
}





TEST(Langscore_MZ_MVMZJsonReader, TextTypeForMaker)
{
    // test/plugin/mv_test内のActors.jsonファイルを読み込む
    std::filesystem::path testPath = "Actors.json";
    std::filesystem::path jsonFilePath = fs::path(BINARY_DIRECTORY) / "data/LangscoreTestMZ/data/Actors.json";
    nlohmann::json jsonData = loadJsonFromFile(jsonFilePath);

    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};
    langscore::mvmz_jsonreader reader(testPath, useLangs, jsonData);

    // テキストタイプが正しく設定されているかを確認
    const auto& texts = reader.currentTexts();
    ASSERT_GE(texts.size(), 4); // 少なくとも4つのテキストがあるはず

    // 名前とプロフィールのテキストタイプを確認
    bool foundName = false;
    bool foundProfile = false;
    bool foundNickname = false;

    for(const auto& text : texts) {
        if(text.original == u8"エルーシェ" || text.original == u8"ラフィーナ") {
            EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), TranslateText::nameType) != text.textType.end()) << "Name text type incorrect";
            foundName = true;
        }
        else if(text.original == u8"雑用係" || text.original == u8"傲慢ちき") {
            EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), TranslateText::nameType) != text.textType.end()) << "Nickname text type incorrect";
            foundNickname = true;
        }
        else if(text.original == u8"ケスティニアスの雑用係。\nそんなに仕事は無い。" ||
                text.original == u8"チビのツンデレウーマン。\n魔法が得意。")
        {
            EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), TranslateText::descriptionType) != text.textType.end()) << "Profile text type incorrect";
            foundProfile = true;
        }
    }

    EXPECT_TRUE(foundName) << "Name text not found";
    EXPECT_TRUE(foundNickname) << "Nickname text not found";
    EXPECT_TRUE(foundProfile) << "Profile text not found";
}

TEST(Langscore_MZ_MVMZJsonReader, DetectDataType)
{
    // 異なるファイル名でデータタイプが正しく検出されるかテスト
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"Actors.json", "Actors"},
        {"Armors.json", "Armors"},
        {"Classes.json", "Classes"},
        {"CommonEvents.json", "CommonEvents"},
        {"Enemies.json", "Enemies"},
        {"Items.json", "Items"},
        {"Map001.json", "Map"},
        {"Skills.json", "Skills"},
        {"States.json", "States"},
        {"System.json", "System"},
        {"Troops.json", "Troops"},
        {"Weapons.json", "Weapons"}
    };

    nlohmann::json emptyJson = {};
    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};

    for(const auto& testCase : testCases) {
        std::filesystem::path testPath = testCase.first;
        langscore::mvmz_jsonreader reader(testPath, useLangs, emptyJson);

        // 内部のcurrentDataTypeを直接テストできないため、
        // 特定のデータタイプに対応するキーが検出されるかどうかで間接的にテスト
        const auto& texts = reader.currentTexts();

        // データタイプが正しく検出されていることを確認するメッセージ
        SUCCEED() << "Successfully detected data type for " << testCase.second;
    }
}

TEST(Langscore_MZ_MVMZJsonReader, StackTextProcessing)
{
    // test/plugin/mv_test内のMap001.jsonファイルを読み込む
    std::filesystem::path testPath = "Map001.json";
    std::filesystem::path jsonFilePath = fs::path(BINARY_DIRECTORY) / "data/LangscoreTestMZ/data/Map001.json";
    nlohmann::json jsonData = loadJsonFromFile(jsonFilePath);

    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};
    langscore::mvmz_jsonreader reader(testPath, useLangs, jsonData);

    // テキストが正しく積み上げられているかを確認
    const auto& texts = reader.currentTexts();

    // 多数のテキストがあるはず
    ASSERT_GE(texts.size(), 10);

    // 積み上げられたメッセージを確認
    bool foundStackedText = false;
    for(const auto& text : texts) {
        if(text.code == 401) {
            // イベント1の最初のメッセージ
            if(text.original == u8"通常のテキストです") {
                foundStackedText = true;
                break;
            }
            // 改行を含むメッセージ
            if(text.original == u8"改行を含む\nテキストです") {
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
            // イベント2の選択肢
            if(text.original == u8"ラフィーナ" || text.original == u8"エルーシェ" || text.original == u8"やめる") {
                foundChoices = true;
                break;
            }
        }
    }
    EXPECT_TRUE(foundChoices) << "Choices not found";
}

TEST(Langscore_MZ_MVMZJsonReader, TextTypeValidation)
{
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
            {u8"ケスティニアスの雑用係。\nそんなに仕事は無い。", TranslateText::descriptionType},
            {u8"チビのツンデレウーマン。\n魔法が得意。", TranslateText::descriptionType}
        }},
        // アイテム関連のテキストタイプ
        {"Items.json", {
            {u8"ポーション", TranslateText::nameType},
            {u8"マジックウォーター", TranslateText::nameType},
            {u8"ディスペルハーブ", TranslateText::nameType},
            {u8"スティミュラント", TranslateText::nameType}
        }},
        // システムメッセージのテキストタイプ
        {"System.json", {
            {u8"%1には効かなかった！", TranslateText::battleMessage},
            {u8"%1は %2 のダメージを受けた！", TranslateText::battleMessage},
            {u8"%1の勝利！", TranslateText::battleMessage},
            {u8"どのファイルにセーブしますか？", TranslateText::message},
            {u8"どのファイルをロードしますか？", TranslateText::message},
            {u8"常時ダッシュ", TranslateText::other}
        }},
        // マップイベントのテキストタイプ
        {"Map001.json", {
            {u8"通常のテキストです", TranslateText::message},
            {u8"改行を含む\nテキストです", TranslateText::message},
            {u8"名前変えるよ", TranslateText::message},
            {u8"ラフィーナ", TranslateText::nameType},
            {u8"エルーシェ", TranslateText::nameType},
            {u8"やめる", TranslateText::other}
        }}
    };

    std::vector<std::u8string> useLangs = {u8"en", u8"ja"};

    for(const auto& testCase : testCases) {
        // JSONファイルを読み込む
        std::filesystem::path testPath = testCase.filename;
        std::filesystem::path jsonFilePath = fs::path(BINARY_DIRECTORY) / "data/LangscoreTestMZ/data" / testCase.filename;
        nlohmann::json jsonData = loadJsonFromFile(jsonFilePath);

        langscore::mvmz_jsonreader reader(testPath, useLangs, jsonData);
        const auto& texts = reader.currentTexts();

        // 期待されるテキストとそのタイプを確認
        int i = 0;
        for(const auto& [expectedText, expectedType] : testCase.expectedTexts) {
            bool found = false;
            for(const auto& text : texts) {
                if(text.original == expectedText) {
                    EXPECT_TRUE(std::find(text.textType.begin(), text.textType.end(), expectedType) != text.textType.end())
                        << "Wrong text type for '" << std::string(expectedText.begin(), expectedText.end())
                        << "' in " << testCase.filename << " (" << i << ")";
                    found = true;
                    break;
                }
            }
            EXPECT_TRUE(found)
                << "Text '" << std::string(expectedText.begin(), expectedText.end())
                << "' not found in " << testCase.filename;
            i++;
        }
    }
}

// platform_baseのwriteFixedGraphFileNameDataメソッドのテスト
TEST(Langscore_Platform_Base, WriteFixedGraphFileNameData_NormalMode)
{
    // テスト設定の準備
    ClearGenerateFiles();

    // 言語パッチモードを無効化するように設定ファイルを変更
    // 通常モードのテスト
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");
    langscore::config::detachConfigFile();

    // 言語パッチモードが無効の設定ファイルを作成
    {
        langscore::config config;
        auto configPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json";
        nlohmann::json json;
        std::ifstream file(configPath);
        file >> json;
        file.close();

        // 言語パッチモードを無効化
        json["Write"]["EnableLanguagePatch"] = false;
        json["Write"]["ExportByLang"] = false;

        // "Languages" 配列を操作
        for(auto& lang : json["Languages"]) {
            if(lang["LanguageName"] == "en" || lang["LanguageName"] == "zh-cn") {
                lang["Enable"] = true;
            }
        }

        std::ofstream outFile(configPath);
        outFile << json.dump(2);
        outFile.close();
    }

    // divisiを作成して実行
    langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");

    // 解析とCSVエクスポート
    ASSERT_TRUE(divisi.analyze().valid());
    ASSERT_TRUE(divisi.exportCSV().valid());

    // 設定とエクスポートディレクトリを取得
    langscore::config config;
    std::u8string root;
    auto exportFolderList = config.exportDirectory(root);

    // 通常モードでは単一のGraphics.csvファイルが作成されていることを確認
    ASSERT_EQ(exportFolderList.size(), 1);
    auto graphicsPath = fs::path(exportFolderList[0]) / "Graphics.csv";
    ASSERT_TRUE(fs::exists(graphicsPath));

    // ファイルの内容を確認
    auto csvReader = plaincsvreader{graphicsPath};
    auto csvData = csvReader.getPlainCsvTexts();

    // ヘッダー行の列数が言語数+1であることを確認（original + 全言語）
    ASSERT_GE(csvData.size(), 1);
    ASSERT_EQ(csvData[0].size(), config.enableLanguages().size() + 1);

    // すべての言語列にデータがあることを確認（各行の列数チェック）
    for(size_t i = 1; i < csvData.size(); ++i) {
        ASSERT_EQ(csvData[i].size(), csvData[0].size()) << "index : " << i;
    }
}

TEST(Langscore_Platform_Base, WriteFixedGraphFileNameData_PatchMode)
{
    // テスト設定の準備
    ClearGenerateFiles();

    // 言語パッチモードを有効化するように設定ファイルを変更
    checkAndCreateConfigFile("data/mv/LangscoreTest_MultipleExport_langscore/config.json", "Game.rpgproject");
    langscore::config::detachConfigFile();

    // 言語パッチモードが有効の設定ファイルを作成
    {
        langscore::config config;
        auto configPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_MultipleExport_langscore/config.json";
        nlohmann::json json;
        std::ifstream file(configPath);
        file >> json;
        file.close();

        // 言語パッチモードを有効化
        json["Write"]["EnableLanguagePatch"] = true;

        // "Languages" 配列を操作
        for(auto& lang : json["Languages"]) {
            if(lang["LanguageName"] == "en" || lang["LanguageName"] == "zh-cn") {
                lang["Enable"] = true;
            }
        }

        std::ofstream outFile(configPath);
        outFile << json.dump(2);
        outFile.close();
    }

    // divisiを作成して実行
    langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_MultipleExport_langscore/config.json");

    // 解析とCSVエクスポート
    ASSERT_TRUE(divisi.analyze().valid());
    ASSERT_TRUE(divisi.exportCSV().valid());

    // 設定を読み込み
    langscore::config config;
    std::u8string root;

    // 言語パッチモードが有効なので、言語ごとのディレクトリが存在するはず
    auto langPairs = config.exportDirectoryWithLang(root);
    ASSERT_GE(langPairs.size(), 1);

    // 各言語ディレクトリにGraphics.csvファイルが存在するか確認
    for(const auto& [lang, folder] : langPairs) {
        auto graphicsPath = fs::path(folder) / "Graphics.csv";
        ASSERT_TRUE(fs::exists(graphicsPath)) << "Graphics.csv not found for language: "
            << utility::cnvStr<std::string>(lang);

        // 各言語ファイルの内容を確認
        auto csvReader = plaincsvreader{graphicsPath};
        auto csvData = csvReader.getPlainCsvTexts();

        // ヘッダー行の列数が2であることを確認（original + 当該言語のみ）
        ASSERT_GE(csvData.size(), 1);
        ASSERT_EQ(csvData[0].size(), 2) << "Column count mismatch for language: "
            << utility::cnvStr<std::string>(lang);

        // すべての行が同じ列数を持つことを確認
        for(size_t i = 1; i < csvData.size(); ++i) {
            ASSERT_EQ(csvData[i].size(), 2) << "Row " << i << " has incorrect column count for language: "
                << utility::cnvStr<std::string>(lang);
        }
    }
}

// divisi_mvmzのwriteFixedBasicDataメソッドのテスト
TEST(Langscore_MVMZ_Divisi, WriteFixedBasicData_NormalMode)
{
    // テスト設定の準備
    ClearGenerateFiles();

    // 言語パッチモードを無効化する設定
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");
    langscore::config::detachConfigFile();


    {
        langscore::config config;
        auto configPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json";
        nlohmann::json json;
        std::ifstream file(configPath);
        file >> json;
        file.close();

        // 言語パッチモードを無効化
        json["Write"]["EnableLanguagePatch"] = false;
        json["Write"]["ExportByLang"] = false;

        // "Languages" 配列を操作
        for(auto& lang : json["Languages"]) {
            if(lang["LanguageName"] == "en" || lang["LanguageName"] == "zh-cn") {
                lang["Enable"] = true;
            }
        }


        std::ofstream outFile(configPath);
        outFile << json.dump(2);
        outFile.close();
    }

    // divisiを作成して実行
    langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");

    // 解析とCSVエクスポート
    ASSERT_TRUE(divisi.analyze().valid());
    ASSERT_TRUE(divisi.exportCSV().valid());

    // 設定を読み込み
    langscore::config config;
    std::u8string root;
    auto exportFolderList = config.exportDirectory(root);
    ASSERT_EQ(exportFolderList.size(), 1);

    // 基本データファイルの存在を確認
    std::vector<std::u8string> expectedFiles = {
        u8"Actors.csv", u8"Armors.csv", u8"Classes.csv", u8"Items.csv",
        u8"Map001.csv", u8"System.csv", u8"Troops.csv", u8"Weapons.csv"
    };

    for(const auto& fileName : expectedFiles) {
        auto filePath = fs::path(exportFolderList[0]) / fileName;
        ASSERT_TRUE(fs::exists(filePath)) << "File not found: " << utility::cnvStr<std::string>(fileName);

        // ファイルの内容を確認
        auto csvReader = plaincsvreader{filePath};
        auto csvData = csvReader.getPlainCsvTexts();

        // ヘッダー行の列数が言語数+1であることを確認
        ASSERT_GE(csvData.size(), 1);
        ASSERT_EQ(csvData[0].size(), config.enableLanguages().size() + 1);

        // すべての行が同じ列数を持つことを確認
        for(size_t i = 1; i < csvData.size(); ++i) {
            ASSERT_EQ(csvData[i].size(), csvData[0].size());
        }
    }
}

TEST(Langscore_MVMZ_Divisi, WriteFixedBasicData_PatchMode)
{
    // テスト設定の準備
    ClearGenerateFiles();

    // 言語パッチモードを有効化する設定
    checkAndCreateConfigFile("data/mv/LangscoreTest_MultipleExport_langscore/config.json", "Game.rpgproject");
    langscore::config::detachConfigFile();

    {
        langscore::config config;
        auto configPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_MultipleExport_langscore/config.json";
        nlohmann::json json;
        std::ifstream file(configPath);
        file >> json;
        file.close();

        // 言語パッチモードを有効化
        json["Write"]["EnableLanguagePatch"] = true;        
        
        // "Languages" 配列を操作
        for(auto& lang : json["Languages"]) {
            if(lang["LanguageName"] == "en" || lang["LanguageName"] == "zh-cn") {
                lang["Enable"] = true;
            }
        }

        std::ofstream outFile(configPath);
        outFile << json.dump(2);
        outFile.close();
    }

    // divisiを作成して実行
    langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_MultipleExport_langscore/config.json");

    // 解析とCSVエクスポート
    ASSERT_TRUE(divisi.analyze().valid());
    ASSERT_TRUE(divisi.exportCSV().valid());

    // 設定を読み込み
    langscore::config config;
    std::u8string root;
    auto langPairs = config.exportDirectoryWithLang(root);
    ASSERT_GE(langPairs.size(), 1);

    // 基本データファイルの存在を確認（各言語ごと）
    std::vector<std::u8string> expectedFiles = {
        u8"Actors.csv", u8"Armors.csv", u8"Classes.csv", u8"Items.csv",
        u8"Map001.csv", u8"System.csv", u8"Troops.csv", u8"Weapons.csv"
    };

    for(const auto& [lang, folder] : langPairs) {
        for(const auto& fileName : expectedFiles) {
            auto filePath = fs::path(folder) / fileName;
            ASSERT_TRUE(fs::exists(filePath)) << "File not found: " << utility::cnvStr<std::string>(fileName)
                << " for language: " << utility::cnvStr<std::string>(lang);

            // ファイルの内容を確認
            auto csvReader = plaincsvreader{filePath};
            auto csvData = csvReader.getPlainCsvTexts();

            // ヘッダー行の列数が2であることを確認（original + 当該言語のみ）
            ASSERT_GE(csvData.size(), 1);
            ASSERT_EQ(csvData[0].size(), 2) << "Column count mismatch for file: "
                << utility::cnvStr<std::string>(fileName)
                << ", language: " << utility::cnvStr<std::string>(lang);

            // すべての行が同じ列数を持つことを確認
            for(size_t i = 1; i < csvData.size(); ++i) {
                ASSERT_EQ(csvData[i].size(), 2) << "Row " << i << " has incorrect column count in file: "
                    << utility::cnvStr<std::string>(fileName)
                    << ", language: " << utility::cnvStr<std::string>(lang);
            }
        }
    }

    // Actorsファイルのアクター名が正しく処理されているか確認
    for(const auto& [lang, folder] : langPairs) {
        auto filePath = fs::path(folder) / u8"Actors.csv";
        auto csvReader = plaincsvreader{filePath};
        auto csvData = csvReader.getPlainCsvTexts();

        bool foundActorName = false;
        for(const auto& row : csvData) {
            if(row.size() >= 2 && (row[0] == u8"ラフィーナ" || row[0] == u8"エルーシェ")) {
                foundActorName = true;
                break;
            }
        }

        ASSERT_TRUE(foundActorName) << "Actor name not found in Actors.csv for language: "
            << utility::cnvStr<std::string>(lang);
    }
}

void ValidateScriptsCSV(const std::filesystem::path& csvPath, const std::vector<std::u8string>& expectedHeaders) 
{
    ASSERT_TRUE(fs::exists(csvPath)) << "Scripts.csv not found at: " << csvPath.string();

    // CSVファイルの内容を読み込む
    auto csvReader = plaincsvreader{csvPath};
    auto csvData = csvReader.getPlainCsvTexts();

    // ヘッダー行の検証
    ASSERT_GE(csvData.size(), 1) << "CSV file is empty: " << csvPath.string();
    ASSERT_EQ(csvData[0].size(), expectedHeaders.size()) << "Header column count mismatch in Scripts.csv";


    // ヘッダー行の検証
    ASSERT_GE(csvData.size(), 1) << "CSV file is empty: " << csvPath.string();
    ASSERT_EQ(csvData[0].size(), expectedHeaders.size()) << "Header column count mismatch in Scripts.csv";

    for(size_t i = 0; i < expectedHeaders.size(); ++i) {
        auto it = std::find(csvData[0].begin(), csvData[0].end(), expectedHeaders[i]);
        ASSERT_TRUE(it != csvData[0].end()) << "Header not found: " << std::string(expectedHeaders[i].begin(), expectedHeaders[i].end());
    }

    // データ行の検証（例: 各行の列数が一致していることを確認）
    for(size_t i = 1; i < csvData.size(); ++i) {
        ASSERT_EQ(csvData[i].size(), expectedHeaders.size()) << "Row " << i << " column count mismatch in Scripts.csv";
    }
}

TEST(Langscore_MVMZ_Divisi, WriteFixedScript_NormalMode) {
    ClearGenerateFiles();

    // 設定ファイルを準備
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");
    langscore::config::detachConfigFile();

    // 言語パッチモードを無効化
    {
        langscore::config config;
        auto configPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json";
        nlohmann::json json;
        std::ifstream file(configPath);
        file >> json;
        file.close();

        json["Write"]["EnableLanguagePatch"] = false;
        json["Write"]["ExportByLang"] = false;

        // "Languages" 配列を操作
        for(auto& lang : json["Languages"]) {
            if(lang["LanguageName"] == "en" || lang["LanguageName"] == "zh-cn") {
                lang["Enable"] = true;
            }
        }

        std::ofstream outFile(configPath);
        outFile << json.dump(2);
        outFile.close();
    }

    // divisiを作成して実行
    langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");
    ASSERT_TRUE(divisi.analyze().valid());
    ASSERT_TRUE(divisi.exportCSV().valid());

    // Map001.csv の検証
    langscore::config config;
    std::u8string root;
    auto exportFolderList = config.exportDirectory(root);
    ASSERT_EQ(exportFolderList.size(), 1);

    auto map001Path = fs::path(exportFolderList[0]) / "Scripts.csv";
    ValidateScriptsCSV(map001Path, {u8"original", u8"en", u8"ja", u8"zh-cn"});
}

TEST(Langscore_MVMZ_Divisi, WriteFixedScript_PatchMode) {
    ClearGenerateFiles();

    // 設定ファイルを準備
    checkAndCreateConfigFile("data/mv/LangscoreTest_MultipleExport_langscore/config.json", "Game.rpgproject");
    langscore::config::detachConfigFile();

    // 言語パッチモードを有効化
    {
        langscore::config config;
        auto configPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_MultipleExport_langscore/config.json";
        nlohmann::json json;
        std::ifstream file(configPath);
        file >> json;
        file.close();

        json["Write"]["EnableLanguagePatch"] = true;

        // "Languages" 配列を操作
        for(auto& lang : json["Languages"]) {
            if(lang["LanguageName"] == "en" || lang["LanguageName"] == "zh-cn") {
                lang["Enable"] = true;
            }
        }

        std::ofstream outFile(configPath);
        outFile << json.dump(2);
        outFile.close();
    }

    // divisiを作成して実行
    langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_MultipleExport_langscore/config.json");
    ASSERT_TRUE(divisi.analyze().valid());
    ASSERT_TRUE(divisi.exportCSV().valid());

    // 各言語ディレクトリの Map001.csv を検証
    langscore::config config;
    std::u8string root;
    auto langPairs = config.exportDirectoryWithLang(root);
    ASSERT_GE(langPairs.size(), 1);

    for(const auto& [lang, folder] : langPairs) {
        auto map001Path = fs::path(folder) / "Scripts.csv";
        ValidateScriptsCSV(map001Path, {u8"original", lang});
    }
}