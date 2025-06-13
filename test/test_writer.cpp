
class Langscore_Writer : public ::testing::Test {
protected:
    std::filesystem::path testDir;
    void SetUp() override {
        // テスト用の一時ディレクトリを作成
        testDir = fs::path(BINARY_DIRECTORY) / "test_rubyreader";
        std::filesystem::create_directory(testDir);
    }

    void TearDown() override {
        // テスト用の一時ディレクトリを削除
        std::filesystem::remove_all(testDir);
    }

    std::filesystem::path createScriptFile(const std::u8string& content) {
        std::filesystem::path filePath = testDir / "test_script.rb";
        std::ofstream outFile(filePath, std::ios::binary);
        outFile.write(reinterpret_cast<const char*>(content.c_str()), content.size());
        outFile.close();
        return filePath;
    }

    std::filesystem::path createDummyJavaScriptFile(const std::u8string& content) 
    {
        std::filesystem::path resultFilePath = testDir / "js/plugins/test_script.js";
        if(fs::exists(resultFilePath.parent_path()) == false) {
            fs::create_directories(resultFilePath.parent_path());
        }
        std::ofstream outFile(resultFilePath, std::ios::binary);
        outFile.write(reinterpret_cast<const char*>(content.c_str()), content.size());
        outFile.close();

        {
            std::filesystem::path filePath = testDir / "js/plugins.js";
            std::ofstream pluginsFile(filePath, std::ios::trunc);
            nlohmann::json emptyArray;
            emptyArray.emplace_back("");
            nlohmann::json pluginElem = {
                {"name", "test_script"},
                {"description", "Hoge"},
                {"status", true},
                {"parameters", emptyArray}
            };
            nlohmann::json plugin;
            plugin.emplace_back(pluginElem);
            pluginsFile << plugin.dump();
        }
        
        return resultFilePath;
    }

};

TEST_F(Langscore_Writer, UTF8WordCount)
{
	ScriptTextParser scriptRegex;
	ASSERT_EQ(scriptRegex.wordCountUTF8(u8"Honi"s), 4);		//ASCII
	ASSERT_EQ(scriptRegex.wordCountUTF8(u8"©ÀʸΦ"s), 4);		//2Byte
	ASSERT_EQ(scriptRegex.wordCountUTF8(u8"あいうえお"s), 5);	//3Byte
	ASSERT_EQ(scriptRegex.wordCountUTF8(u8"𦹀𧃴𧚄𨉷𨏍𪆐"s), 6);	//4Byte
}

TEST_F(Langscore_Writer, ConvertWorkList)
{
	using namespace std::string_view_literals;
	ScriptTextParser scriptRegex;
	{
		auto baseStr = u8"HoniHoni"s;
		auto result = scriptRegex.ConvertWordList(baseStr);
		ASSERT_TRUE(result.size() == 8);
		size_t i = 0;
		ASSERT_EQ(std::get<0>(result[i++]), u8"H"sv);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"n"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"i"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"H"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"n"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"i"sv) == 0);
	}

	{
		auto baseStr = u8"あいうえおoooかきくけこ"s;
		auto result = scriptRegex.ConvertWordList(baseStr);
		ASSERT_TRUE(result.size() == 13);
		size_t i = 0;
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"あ"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"い"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"う"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"え"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"お"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"か"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"き"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"く"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"け"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"こ"sv) == 0);
	}

	{
		auto baseStr = u8"𦹀𧃴𧚄𨉷𨏍𪆐🙁😇"s;
		auto result = scriptRegex.ConvertWordList(baseStr);
		ASSERT_TRUE(result.size() == 8);
		size_t i = 0;
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"𦹀"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"𧃴"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"𧚄"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"𨉷"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"𨏍"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"𪆐"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"🙁"sv) == 0);
		ASSERT_TRUE(std::get<0>(result[i++]).compare(u8"😇"sv) == 0);
	}

}

TEST_F(Langscore_Writer, DetectStringPosition)
{
	ScriptTextParser scriptRegex;
	{
		auto result = scriptRegex.findStrings(u8"Test Line Script");
		ASSERT_TRUE(result.empty());
	}
	{
		auto result = scriptRegex.findStrings(u8"");
		ASSERT_TRUE(result.empty());
	}
	{
		auto result = scriptRegex.findStrings(u8"hoge = \"\";");
		ASSERT_TRUE(result.empty());
	}
	{
		auto result = scriptRegex.findStrings(u8"Test \"Line\" Script");
		ASSERT_EQ(result.size(), 1);
		ASSERT_EQ(std::get<0>(result[0]), u8"Line"s);
		ASSERT_EQ(std::get<1>(result[0]), 7);
	}
	{
		auto result = scriptRegex.findStrings(u8"\"Test\" Line S\"cript\"");
		ASSERT_EQ(result.size(), 2);
		ASSERT_EQ(std::get<0>(result[0]), u8"Test"s);
		ASSERT_EQ(std::get<1>(result[0]), 2);
		ASSERT_EQ(std::get<0>(result[1]), u8"cript"s);
		ASSERT_EQ(std::get<1>(result[1]), 15);
	}
	{
		auto result = scriptRegex.findStrings(u8"auto text = \"あいうえお\"; auto text2 = \"aiueo\""s);
		ASSERT_EQ(result.size(), 2);
		ASSERT_EQ(std::get<0>(result[0]), u8"あいうえお"s);
		ASSERT_EQ(std::get<1>(result[0]), 14);
		ASSERT_EQ(std::get<0>(result[1]), u8"aiueo"s);
		ASSERT_EQ(std::get<1>(result[1]), 36);
	}
	{
		auto result = scriptRegex.findStrings(u8"text = \"\\C[16]プレイ時間\\X[104]\\C[0]\\T[%4$3d\\\"%3$02d]\""s);
		ASSERT_EQ(result.size(), 1);
		ASSERT_EQ(std::get<0>(result[0]), u8"\\C[16]プレイ時間\\X[104]\\C[0]\\T[%4$3d\\\"%3$02d]"s);
	}
	//langscore.rbで正しくCSVが出力されない問題の確認テスト。
	//""括りかつ文字列内に,が存在する場合、""が括られたまま結果が出力される。(CSV用の変換)
	//CSV用の変換はCSVの書き出し時点で行うため、findStringsで行うのは不適切。
	{
		auto result = scriptRegex.findStrings(u8"      raise \"Error!: Missmatch Num Cells : #{file_name}, #{mismatch_cells.to_s}\" "s);
		ASSERT_EQ(result.size(), 1);
		ASSERT_EQ(std::get<0>(result[0]), u8"Error!: Missmatch Num Cells : #{file_name}, #{mismatch_cells.to_s}"s);
	}
}
TEST_F(Langscore_Writer, CheckRubyCommentLine)
{
    //文字列の抽出の結果をテストする
    // テストケース 1: 通常のスクリプト
    {
        std::u8string script = u8"'Hello, world!'";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts(); // 取得したテキストを検証

        ASSERT_EQ(texts.size(), 1);
        ASSERT_EQ(texts[0].original, u8"Hello, world!");
    }

    // テストケース 2: コメントアウトされた行
    {
        std::u8string script = u8"# これはコメントです";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0); // コメント行は無視されるのでテキストは空
    }

    // テストケース 3: マルチバイト文字を含む通常のスクリプト
    {
        std::u8string script = u8"'こんにちは、世界！'";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1);
        ASSERT_EQ(texts[0].original, u8"こんにちは、世界！");
    }

    // テストケース 4: マルチバイト文字を含む式展開
    {
        std::u8string script = u8"\"キー:#{値}\"";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1);
        ASSERT_EQ(texts[0].original, u8"キー:#{値}");
    }

    // テストケース 5: マルチバイト文字を含むコメントブロック
    {
        std::u8string script = u8"=begin\nこれはコメントブロックです\n=end";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0); // コメントブロックなのでテキストは空
    }

    // テストケース 6: マルチバイト文字を含む通常の行とコメントブロックの混在
    {
        std::u8string script = u8"'こんにちは'\n=begin\nコメントブロック\n=end";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1);
        ASSERT_EQ(texts[0].original, u8"こんにちは");
    }
}
TEST_F(Langscore_Writer, CheckJavaScriptCommentLine) 
{
    // テストケース 1: 通常のスクリプト
    {
        std::u8string script = u8"chstring";
        auto scriptPath = createDummyJavaScriptFile(script);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts(); // 取得したテキストを検証

        ASSERT_TRUE(texts.empty());
    }

    // テストケース 2: コメントアウトされた行
    {
        std::u8string script = u8"//Commentout";
        auto scriptPath = createDummyJavaScriptFile(script);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0); // コメント行は無視されるのでテキストは空
    }

    // テストケース 3: /* コメントブロックの開始
    {
        std::u8string script = u8"/*Hoge";
        auto scriptPath = createDummyJavaScriptFile(script);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0);
    }

    // テストケース 4: コメントブロックの終了 */
    {
        std::u8string script = u8"Hoge*/";
        auto scriptPath = createDummyJavaScriptFile(script);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0);
    }

    // テストケース 5: コメントブロックを含むが無視する場合
    {
        std::u8string script = u8"Hoge*/";
        auto scriptPath = createDummyJavaScriptFile(script);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0);
    }

    // テストケース 6: コメントを含む行の置換
    {
        std::u8string script = u8"\"Hoge\";/*Comment*/\"Honi\";";
        auto scriptPath = createDummyJavaScriptFile(script);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 2); // コメントがスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"Hoge");
        ASSERT_EQ(texts[1].original, u8"Honi");
    }

    // テストケース 7: コメントアウトと文字列が含まれる行
    {
        std::u8string script = u8"/*\"Hoge\"//Comment*/\"Honi\"";
        auto scriptPath = createDummyJavaScriptFile(script);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"Honi");
    }
}

TEST_F(Langscore_Writer, DetectRubyString)
{
	std::u8string fileName = u8"detectstring";
	langscore::rubyreader scriptWriter({}, {u8"./data/vxace/" + fileName + u8".rb"});


    {
        auto extracted_strings = scriptWriter.currentTexts();

        ASSERT_EQ(9, extracted_strings.size());
        ASSERT_TRUE(u8"Hello, world!" == extracted_strings[0].original);
        ASSERT_TRUE(u8"This is a \\\"quote\\\" character." == extracted_strings[1].original);
        ASSERT_TRUE(u8"This string has a \\n newline character." == extracted_strings[2].original);
        ASSERT_TRUE(u8"This one has a , comma character." == extracted_strings[3].original);
        ASSERT_TRUE(u8"This string has both \\\"quote\\\" and \\n newline characters." == extracted_strings[4].original);
        ASSERT_TRUE(u8"This is a 'single quote' character." == extracted_strings[5].original);
        ASSERT_TRUE(u8"This string has a \\\"quote\\\", a 'single quote' and a \\n newline character." == extracted_strings[6].original);
        ASSERT_TRUE(u8"\\\"\\\"" == extracted_strings[7].original);
        ASSERT_TRUE(u8"," == extracted_strings[8].original);
    }

}

TEST_F(Langscore_Writer, DetectStringPositionFromFile)
{
    //注)このテストが失敗する場合、vxaceのrubyスクリプトの解析において過去のバージョンとの互換性が取れていない。
    //   langscore_customの出力に影響が出るため、変更する場合は注意書きを載せる必要がある。
	std::u8string fileName = u8"chstring";

    langscore::rubyreader scriptWriter({}, {u8"./data/vxace/" + fileName + u8".rb"});
    auto result = scriptWriter.currentTexts();

	ASSERT_EQ(result.size(), 5);

	size_t i = 0;
	ASSERT_TRUE(result[i++].original == u8"𦹀𧃴𧚄𨉷𨏍𪆐🙁😇"s);
	ASSERT_TRUE(result[i++].original == u8"HoniHoni"s);
	ASSERT_TRUE(result[i++].original == u8"あいうえお"s);
	ASSERT_TRUE(result[i++].original == u8"chstring:14:2"s);
	ASSERT_TRUE(result[i++].original == u8"\\C[16]プレイ時間\\X[104]\\C[0]\\T[%4$3d\\'%3$02d]"s);

	i = 0;
	//"を含まない単語の開始位置
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":8:16");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":11:2");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":38:6");
    ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":49:17");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":51:2");

}

TEST_F(Langscore_Writer, ConvertCsvText_ASCII)
{
	langscore::csvwriter writer(speciftranstext{{}, {}});

    const fs::path filePath = "./langscore_write_test.csv";

	{
		auto input = u8"Hello, World!"s;
        auto result = writer.writePlain(filePath, {{input}});
        ASSERT_EQ(result, Status_Success);

        std::ifstream file(filePath);
        std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        ASSERT_TRUE(fs::exists(filePath));
        ASSERT_STREQ(contents.c_str(), "\"Hello, World!\"");

	}
	{
		auto input = u8"First line\nSecond line";
        auto result = writer.writePlain(filePath, {{input}});
        ASSERT_EQ(result, Status_Success);

        std::ifstream file(filePath);
        std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        ASSERT_TRUE(fs::exists(filePath));
        ASSERT_STREQ(contents.c_str(), utility::cnvStr<std::string>(u8"\"First line\nSecond line\""s).c_str());
	}
	{
		auto input = u8"Quote \" in the text";
        auto result = writer.writePlain(filePath, {{input}});
        ASSERT_EQ(result, Status_Success);

        std::ifstream file(filePath);
        std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        ASSERT_TRUE(fs::exists(filePath));
        ASSERT_STREQ(contents.c_str(), utility::cnvStr<std::string>(u8"\"Quote \"\" in the text\""s).c_str());
	}
	{
		auto input = u8"Comma, and quote \"";
        auto result = writer.writePlain(filePath, {{input}});
        ASSERT_EQ(result, Status_Success);

        std::ifstream file(filePath);
        std::string contents((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

        ASSERT_TRUE(fs::exists(filePath));
        ASSERT_STREQ(contents.c_str(), utility::cnvStr<std::string>(u8"\"Comma, and quote \"\"\""s).c_str());
	}
}

TEST_F(Langscore_Writer, ConvertCsvText_Multibyte)
{
	langscore::csvwriter writer(speciftranstext{{}, {}});

	{
		auto input = u8"\"こんにちは、世界！\"";
		auto scriptPath = createDummyJavaScriptFile(input);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"こんにちは、世界！");
	}
    {
        auto input = u8"//\"こんにちは,世界！\"";
        auto scriptPath = createDummyJavaScriptFile(input);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0);
    }
    {
        //右辺値のみの文字列は意味が無く解析をほぼサポートしていない状態なので、
        //代入式など意味のあるものにしてテストする。
        auto input = u8"\"最初の行\n次の行\"";
        auto scriptPath = createDummyJavaScriptFile(input);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"最初の行\n次の行");
    }
    {
        auto input = u8"var hoge = \"文章中の\\\"記号\"";
        auto scriptPath = createDummyJavaScriptFile(input);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"文章中の\\\"記号");
    }
    {
        auto input = u8"var hoge = \"コンマ,\\\"と引用符\"";
        auto scriptPath = createDummyJavaScriptFile(input);
        langscore::javascriptreader scriptWriter(u8"ja", {}, testDir / u8"js/plugins.js"s, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"コンマ,\\\"と引用符");
    }
}