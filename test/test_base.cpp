
#include <gmock/gmock.h> 
#include <gmock/gmock-matchers.h>
using ::testing::Contains;

TEST(Langscore_Utility_Join, HandlesEmptyList) {
	std::vector<std::string> list;
	std::string separator = ", ";
	std::string expected = "";
	ASSERT_TRUE(expected == utility::join(list, separator));
}

TEST(Langscore_Utility_Join, HandlesSingleElement) {
	std::vector<std::string> list = {"Hello"};
	std::string separator = ", ";
	std::string expected = "Hello";
	ASSERT_TRUE(expected == utility::join(list, separator));
}

TEST(Langscore_Utility_Join, HandlesMultipleElements) {
	std::vector<std::string> list = {"Hello", "World"};
	std::string separator = ", ";
	std::string expected = "Hello, World";
	ASSERT_TRUE(expected == utility::join(list, separator));
}

TEST(Langscore_Utility_Join, HandlesMixedEmptyAndNonEmptyElements) {
	std::vector<std::string> list = {"Hello", "", "World"};
	std::string separator = ", ";
	std::string expected = "Hello, , World";
	ASSERT_TRUE(expected == utility::join(list, separator));
}

class Langscore_Config : public ::testing::Test
{
public:
    std::u8string build_folder = utility::cnvStr<std::u8string>(std::string{BINARYT_DIRECTORY});
protected:
    void SetUp() override {
        //langscore::config::attachConfigFile(this->build_folder + u8".\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\config.json");
    }

    void TearDown() override {
        // テスト用の一時ディレクトリを削除
        langscore::config::detachConfigFile();
    }
};

TEST_F(Langscore_Config, TmpDir)
{
	langscore::config config;

	auto expected = fs::path(config.langscoreAnalyzeDirectorty()).generic_string();
	auto actual = fs::path(this->build_folder + u8"\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\analyze"s).generic_string();
	ASSERT_TRUE(expected == actual);
}

TEST_F(Langscore_Config, LoadLanguages)
{
	langscore::config config(fs::path(this->build_folder) / u8"data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	auto expected = config.languages();
	utility::stringlist actual = {"en", "ja"};
	ASSERT_TRUE(actual.size() == expected.size());
	for(auto& elem : actual) {
        auto result = std::find_if(expected.begin(), expected.end(), [&](auto& x) {
            return x.name == elem;
        });
        ASSERT_TRUE(result != expected.end());
	}
}

TEST_F(Langscore_Config, CheckFontName)
{
	langscore::config config(fs::path(this->build_folder) / u8"data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\config.json");

	auto expected = config.languages();
	for(int i = 0; i < expected.size(); ++i)
	{
		if(expected[i].name == "ja") {
			ASSERT_TRUE(expected[i].font.name == u8"メイリオ"s);
		}
		else {
			ASSERT_TRUE(expected[i].font.name == u8"VL Gothic"s);
		}
	}
}

TEST_F(Langscore_Config, CheckProjectPath)
{
	langscore::config config(fs::path(this->build_folder) / u8"data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\config.json");

    auto expected = fs::path(config.gameProjectPath()).generic_string();
    auto build_folder = utility::cnvStr<std::u8string>(std::string{BINARYT_DIRECTORY});
    auto actual = fs::path(build_folder + u8"\\data\\vxace\\ソポァゼゾタダＡボマミ"s).generic_string();
    ASSERT_TRUE(expected == actual);
}

TEST(Langscore_Csv, parsePlain)
{
	{
		auto targetCsvList = plaincsvreader{fs::path(BINARYT_DIRECTORY) / "data\\csv\\parsePlain.csv"}.getPlainCsvTexts();
		ASSERT_TRUE(targetCsvList.empty() == false);

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
					SCOPED_TRACE(::testing::Message() << "No match Text. : " << utility::cnvStr<std::string>(expected[i][j]) << ":" << utility::cnvStr<std::string>(targetCsvList[i][j]));
					SCOPED_TRACE(::testing::Message() << "i : " << i << " j : " << j);
					GTEST_FAIL();
				}
			}
		}
	}
	{
		auto targetCsvList = plaincsvreader{fs::path(BINARYT_DIRECTORY) / "data\\csv\\plaincsvreader.csv"}.getPlainCsvTexts();
		ASSERT_TRUE(targetCsvList.empty() == false);
        EXPECT_EQ(6, targetCsvList.size());

		ASSERT_TRUE(targetCsvList[1][0] == u8"\\c[sigo]テスト\n1");
		ASSERT_TRUE(targetCsvList[1][1] == u8"\\c[sigo]テスト\n1");
		ASSERT_TRUE(targetCsvList[1][2] == u8"\\c[sigo]测试\n1");

		ASSERT_TRUE(targetCsvList[2][0] == u8"\\r[sigo]テスト\n2");
		ASSERT_TRUE(targetCsvList[2][1] == u8"\\r[sigo]テスト\n2");
		ASSERT_TRUE(targetCsvList[2][2] == u8"\\r[sigo]测试\n2");

		ASSERT_TRUE(targetCsvList[3][0] == u8"\\n[sigo]テスト\n3");
		ASSERT_TRUE(targetCsvList[3][1] == u8"\\n[sigo]テスト\n3");
		ASSERT_TRUE(targetCsvList[3][2] == u8"\\n[sigo]测试\n3");

		ASSERT_TRUE(targetCsvList[4][0] == u8"タフルクォーテーションを含むテキストです");
		ASSERT_TRUE(targetCsvList[5][0] == u8"タ\"フ\"ルクォーテーションを含むテキストです");
	}
}

TEST(Langscore_Csv, merge)
{
	{
		langscore::csvreader reader{{}, fs::path(BINARYT_DIRECTORY) / "data\\csv\\after.csv"};
		auto& targetCsv = reader.currentTexts();
		ASSERT_TRUE(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::AcceptSource;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		ASSERT_TRUE(writer.merge(fs::path(BINARYT_DIRECTORY) / u8"data\\csv\\before.csv"));

		auto& merged = writer.currentTexts();

		ASSERT_TRUE(merged.size() == 2);
		ASSERT_TRUE(merged[0].original == u8"あいうえお");
		ASSERT_TRUE(merged[0].translates[u8"ja"] == u8"あいうえお");
		ASSERT_TRUE(merged[1].original == u8"さしすせそ");
		ASSERT_TRUE(merged[1].translates[u8"ja"] == u8"");
	}
	{
		langscore::csvreader reader{{}, fs::path(BINARYT_DIRECTORY) / "data\\csv\\after.csv"};
		auto& targetCsv = reader.currentTexts();
		ASSERT_TRUE(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::AcceptTarget;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		ASSERT_TRUE(writer.merge(fs::path(BINARYT_DIRECTORY) / u8"data\\csv\\before.csv"));

		auto& merged = writer.currentTexts();

		ASSERT_TRUE(merged.size() == 3);
		ASSERT_TRUE(merged[0].original == u8"あいうえお");
		ASSERT_TRUE(merged[0].translates[u8"ja"] == u8"たちつてと");
		ASSERT_TRUE(merged[1].original == u8"かきくけこ");
		ASSERT_TRUE(merged[1].translates[u8"ja"] == u8"かきくけこ");
		ASSERT_TRUE(merged[2].original == u8"さしすせそ");
		ASSERT_TRUE(merged[2].translates[u8"ja"] == u8"さしすせそ");
	}
	{
		langscore::csvreader reader{{}, fs::path(BINARYT_DIRECTORY) / "data\\csv\\after.csv"};
		auto& targetCsv = reader.currentTexts();
		ASSERT_TRUE(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::MergeKeepTarget;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		ASSERT_TRUE(writer.merge(fs::path(BINARYT_DIRECTORY) / u8"data\\csv\\before.csv"));

		auto& merged = writer.currentTexts();

		ASSERT_TRUE(merged.size() == 3);
		ASSERT_TRUE(merged[0].original == u8"あいうえお");
		ASSERT_TRUE(merged[0].translates[u8"ja"] == u8"たちつてと");
		ASSERT_TRUE(merged[1].original == u8"かきくけこ");
		ASSERT_TRUE(merged[1].translates[u8"ja"] == u8"かきくけこ");
		ASSERT_TRUE(merged[2].original == u8"さしすせそ");
		ASSERT_TRUE(merged[2].translates[u8"ja"] == u8"さしすせそ");
	}
	{
		langscore::csvreader reader{{}, fs::path(BINARYT_DIRECTORY) / "data\\csv\\before.csv"};
		auto& targetCsv = reader.currentTexts();
		ASSERT_TRUE(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::MergeKeepTarget;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		ASSERT_TRUE(writer.merge(fs::path(BINARYT_DIRECTORY) / u8"data\\csv\\after.csv"));

		auto& merged = writer.currentTexts();

        std::vector<TranslateText> actuals;
        {
            TranslateText a;
            a.original = u8"あいうえお";
            a.translates[u8"ja"] = u8"あいうえお";
            actuals.emplace_back(std::move(a));

            TranslateText b;
            b.original = u8"かきくけこ";
            b.translates[u8"ja"] = u8"かきくけこ";
            actuals.emplace_back(std::move(b));

            TranslateText c;
            c.original = u8"さしすせそ";
            c.translates[u8"ja"] = u8"";
            actuals.emplace_back(std::move(c));
        }

		ASSERT_EQ(merged.size(), 3);

        for(auto& actual : actuals) {
            auto r = std::find(merged.begin(), merged.end(), actual);
            ASSERT_TRUE(r != merged.end());
        }

		//ASSERT_TRUE(merged[0].original == u8"あいうえお");
		//ASSERT_TRUE(merged[0].translates[u8"ja"] == u8"あいうえお");
		//ASSERT_TRUE(merged[1].original == u8"かきくけこ");
		//ASSERT_TRUE(merged[1].translates[u8"ja"] == u8"かきくけこ");
		//ASSERT_TRUE(merged[2].original == u8"さしすせそ");
		//ASSERT_TRUE(merged[2].translates[u8"ja"] == u8"");
	}
	{
		langscore::csvreader reader{{}, fs::path(BINARYT_DIRECTORY) / "data\\csv\\after.csv"};
		auto& targetCsv = reader.currentTexts();
		ASSERT_TRUE(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::MergeKeepSource;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		ASSERT_TRUE(writer.merge(fs::path(BINARYT_DIRECTORY) / u8"data\\csv\\before.csv"));

		auto& merged = writer.currentTexts();

		ASSERT_TRUE(merged.size() == 3);
		ASSERT_TRUE(merged[0].original == u8"あいうえお");
		ASSERT_TRUE(merged[0].translates[u8"ja"] == u8"あいうえお");
		ASSERT_TRUE(merged[1].original == u8"かきくけこ");
		ASSERT_TRUE(merged[1].translates[u8"ja"] == u8"かきくけこ");
		ASSERT_TRUE(merged[2].original == u8"さしすせそ");
		ASSERT_TRUE(merged[2].translates[u8"ja"] == u8"");
	}
	{
		langscore::csvreader reader{{}, fs::path(BINARYT_DIRECTORY) / "data\\csv\\before.csv"};
		auto& targetCsv = reader.currentTexts();
		ASSERT_TRUE(targetCsv.empty() == false);

		auto mode = langscore::MergeTextMode::MergeKeepSource;
		langscore::csvwriter writer(reader);
		writer.setOverwriteMode(mode);
		ASSERT_TRUE(writer.merge(fs::path(BINARYT_DIRECTORY) / u8"data\\csv\\after.csv"));

        std::vector<TranslateText> actuals;
        {
            TranslateText a;
            a.original = u8"あいうえお";
            a.translates[u8"ja"] = u8"たちつてと";
            actuals.emplace_back(std::move(a));

            TranslateText b;
            b.original = u8"かきくけこ";
            b.translates[u8"ja"] = u8"かきくけこ";
            actuals.emplace_back(std::move(b));

            TranslateText c;
            c.original = u8"さしすせそ";
            c.translates[u8"ja"] = u8"さしすせそ";
            actuals.emplace_back(std::move(c));
        }

		auto& merged = writer.currentTexts();

		ASSERT_TRUE(merged.size() == 3);
        for(auto& actual : actuals) {
            auto r = std::find(merged.begin(), merged.end(), actual);
            ASSERT_TRUE(r != merged.end());
        }
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

		ASSERT_TRUE(writer.merge(fs::path(BINARYT_DIRECTORY) / u8"data\\csv\\add_lang.csv"));

		auto& merged = writer.currentTexts();

		ASSERT_TRUE(merged.size() == 2);
		ASSERT_TRUE(merged[0].original == u8"こんにちは");
		ASSERT_TRUE(merged[0].translates[u8"en"] == u8"");
		ASSERT_TRUE(merged[0].translates[u8"ja"] == u8"こんにちは");
		ASSERT_TRUE(merged[0].translates[u8"zh-cn"] == u8"下午好");
		ASSERT_TRUE(merged[1].original == u8"こん\nばんは");
		ASSERT_TRUE(merged[1].translates[u8"en"] == u8"");
		ASSERT_TRUE(merged[1].translates[u8"ja"] == u8"こん\nばんは");
		ASSERT_TRUE(merged[1].translates[u8"zh-cn"] == u8"晚上\n好");
	}
}
