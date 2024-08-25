﻿
class Langscore_Writer : public ::testing::Test {
protected:
    void SetUp() override {
        // テスト用の一時ディレクトリを作成
        testDir = std::filesystem::temp_directory_path() / "test_rubyreader";
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

    std::filesystem::path testDir;
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

TEST_F(Langscore_Writer, CheckRubyCommentLine) {
    // テストケース 1: 通常のスクリプト
    {
        std::u8string script = u8"chstring";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts(); // 取得したテキストを検証
        
        ASSERT_TRUE(texts.empty());
    }

    // テストケース 2: コメントアウトされた行
    {
        std::u8string script = u8"#Commentout";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0); // コメント行は無視されるのでテキストは空
    }

    // テストケース 3: =begin コメントブロックの開始
    {
        std::u8string script = u8"=begin";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0); // コメントブロックなのでテキストは空
    }

    // テストケース 4: 通常の行と =begin コメントブロックの混在
    {
        std::u8string script = u8"\"Hoge\" =begin";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントブロックが無視され、通常の行のみ残る
        ASSERT_EQ(texts[0].original, u8"\"Hoge\"       ");
    }

    // テストケース 5: =begin が含まれるがコメントブロックではない行
    {
        std::u8string script = u8"\"Hoge\" \"=begin\"Honi";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // 全行が有効なスクリプトとして残る
        ASSERT_EQ(texts[0].original, u8"\"Hoge\" \"=begin\"Honi");
    }

    // テストケース 6: 有効なスクリプト行
    {
        std::u8string script = u8"Valid Script Line";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // 有効なスクリプト行がそのまま残る
        ASSERT_EQ(texts[0].original, u8"Valid Script Line");
    }

    // テストケース 7: =end コメントブロックの終了
    {
        std::u8string script = u8"=end";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0); // コメントブロックの終了なのでテキストは空
    }

    // テストケース 8: コメントブロックがない有効なスクリプト行
    {
        std::u8string script = u8"Without Comment Block";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントブロックがないのでそのまま残る
        ASSERT_EQ(texts[0].original, u8"Without Comment Block");
    }

    // テストケース 9: コメントが含まれるスクリプト行
    {
        std::u8string script = u8"Script~~~ #Comment";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメント部分はスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"Script~~~         ");
    }

    // テストケース 10: 式展開が含まれるスクリプト行
    {
        std::u8string script = u8"\"Key:#{Value}\"";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // 式展開はそのまま残る
        ASSERT_EQ(texts[0].original, u8"\"Key:#{Value}\"");
    }

    // テストケース 11: 式展開と変換メソッドが含まれるスクリプト行
    {
        std::u8string script = u8R"("Text".lstrans'Honi')";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // 式展開と変換メソッドの呼び出しがスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8R"("Text"              )");
    }

    // テストケース 12: 文字列を変換するメソッドが含まれるスクリプト行
    {
        std::u8string script = u8R"("Text".lstrans"64:28:1")";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // 変換メソッドの引数がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8R"("Text"                 )");
    }

    // テストケース 13: 空の引数を持つ変換メソッドが含まれるスクリプト行
    {
        std::u8string script = u8R"("Text".lstrans(''))";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // 空の引数がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8R"("Text"            )");
    }

    // テストケース 14: 式展開と文字列を変換するメソッドが含まれるスクリプト行
    {
        std::u8string script = u8R"(Honi:"Te'xt".lstrans('Hoge') "DetectText")";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // 式展開と変換メソッドが部分的にスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8R"(Honi:"Te'xt"                 "DetectText")");
    }

    // テストケース 15: 式展開を含まない変換メソッドが含まれるスクリプト行
    {
        std::u8string script = u8R"("Text".lstrans(""))";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // 空の引数がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8R"("Text"            )");
    }

    // テストケース 16: 式展開と文字列を変換するメソッドが含まれるスクリプト行
    {
        std::u8string script = u8R"(Honi:"Text".lstrans("Hoge") "DetectText")";
        auto scriptPath = createScriptFile(script);
        langscore::rubyreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // 式展開と変換メソッドが部分的にスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8R"(Honi:"Text"                 "DetectText")");
    }
}

TEST_F(Langscore_Writer, CheckJavaScriptCommentLine) 
{
    // テストケース 1: 通常のスクリプト
    {
        std::u8string script = u8"chstring";
        auto scriptPath = createScriptFile(script);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts(); // 取得したテキストを検証

        ASSERT_TRUE(texts.empty());
    }

    // テストケース 2: コメントアウトされた行
    {
        std::u8string script = u8"//Commentout";
        auto scriptPath = createScriptFile(script);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 0); // コメント行は無視されるのでテキストは空
    }

    // テストケース 3: /* コメントブロックの開始
    {
        std::u8string script = u8"/*Hoge";
        auto scriptPath = createScriptFile(script);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントブロックがスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"      ");
    }

    // テストケース 4: コメントブロックの終了 */
    {
        std::u8string script = u8"Hoge*/";
        auto scriptPath = createScriptFile(script);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントブロック終了時のテキストがスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"      ");
    }

    // テストケース 5: コメントブロックを含むが無視する場合
    {
        std::u8string script = u8"Hoge*/";
        auto scriptPath = createScriptFile(script);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントブロックが無視され、テキストはそのまま残る
        ASSERT_EQ(texts[0].original, u8"Hoge*/");
    }

    // テストケース 6: コメントを含む行の置換
    {
        std::u8string script = u8"\"Hoge\"/*Comment*/\"Honi\"";
        auto scriptPath = createScriptFile(script);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントがスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"\"Hoge\"           \"Honi\"");
    }

    // テストケース 7: コメントアウトと文字列が含まれる行
    {
        std::u8string script = u8"\"Hoge\"//Comment*/\"Honi\"";
        auto scriptPath = createScriptFile(script);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"\"Hoge\"                 ");
    }
}

TEST_F(Langscore_Writer, DetectRubyString)
{
	std::u8string fileName = u8"detectstring";
	langscore::rubyreader scriptWriter({}, {u8"./data/vxace/" + fileName + u8".rb"});


    {
        auto extracted_strings = scriptWriter.currentTexts();

        ASSERT_EQ(10, extracted_strings.size());
        ASSERT_TRUE(u8"Hello, world!" == extracted_strings[0].original);
        ASSERT_TRUE(u8"This is a \"quote\" character." == extracted_strings[1].original);
        ASSERT_TRUE(u8"This string has a \n newline character." == extracted_strings[2].original);
        ASSERT_TRUE(u8"This one has a , comma character." == extracted_strings[3].original);
        ASSERT_TRUE(u8"This string has both \"quote\" and \n newline characters." == extracted_strings[4].original);
        ASSERT_TRUE(u8"This is a 'single quote' character." == extracted_strings[5].original);
        ASSERT_TRUE(u8"This string has a \"quote\", a 'single quote' and a \n newline character." == extracted_strings[6].original);
        ASSERT_TRUE(u8"\"\"" == extracted_strings[7].original);
        ASSERT_TRUE(u8"\n" == extracted_strings[8].original);
        ASSERT_TRUE(u8"," == extracted_strings[9].original);
    }

}

TEST_F(Langscore_Writer, DetectStringPositionFromFile)
{
	std::u8string fileName = u8"chstring";

    langscore::rubyreader scriptWriter({}, {u8"./data/vxace/" + fileName + u8".rb"});
    auto result = scriptWriter.currentTexts();

	ASSERT_EQ(result.size(), 14);

	size_t i = 0;
	ASSERT_TRUE(result[i++].original == u8"あHoniい"s);
	ASSERT_TRUE(result[i++].original == u8"𦹀𧃴𧚄𨉷𨏍𪆐🙁😇"s);
	ASSERT_TRUE(result[i++].original == u8"HoniHoni"s);
	ASSERT_TRUE(result[i++].original == u8"trans #{text}"s);
	ASSERT_TRUE(result[i++].original == u8"call initialize_clone"s);
	ASSERT_TRUE(result[i++].original == u8"call initialize_clone"s);
	ASSERT_TRUE(result[i++].original == u8"call to_str"s);
	ASSERT_TRUE(result[i++].original == u8"あいうえお"s);
	ASSERT_TRUE(result[i++].original == u8"A"s);
	ASSERT_TRUE(result[i++].original == u8"B"s);
	ASSERT_TRUE(result[i++].original == u8"A"s);
	ASSERT_TRUE(result[i++].original == u8"翻訳テキスト"s);
	ASSERT_TRUE(result[i++].original == u8"翻訳テキスト2"s);
	ASSERT_TRUE(result[i++].original == u8"\\C[16]プレイ時間\\X[104]\\C[0]\\T[%4$3d'%3$02d]"s);

	i = 0;
	//"を含まない単語の開始位置
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":8:4");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":8:16");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":11:2");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":16:10");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":23:8");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":28:8");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":33:8");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":38:6");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":43:13");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":44:4");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":45:4");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":47:2");
	ASSERT_TRUE(result[i++].scriptLineInfo == fileName + u8":48:2");
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
        ASSERT_STREQ(contents.c_str(), utility::cnvStr<std::string>(input).c_str());

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
		auto input = u8"こんにちは、世界！";
		auto scriptPath = createScriptFile(input);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"こんにちは、世界！");
	}
    {
        auto input = u8"こんにちは,世界！";
        auto scriptPath = createScriptFile(input);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"\"こんにちは,世界！\"");
    }
    {
        auto input = u8"最初の行\n次の行";
        auto scriptPath = createScriptFile(input);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"\"最初の行\n次の行\"");
    }
    {
        auto input = u8"文章中の\"記号";
        auto scriptPath = createScriptFile(input);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"\"文章中の\"\"記号\"");
    }
    {
        auto input = u8"コンマ,\"と引用符";
        auto scriptPath = createScriptFile(input);
        langscore::javascriptreader scriptWriter({}, {scriptPath});
        auto texts = scriptWriter.currentTexts();

        ASSERT_EQ(texts.size(), 1); // コメントアウト部分がスペースに置き換えられる
        ASSERT_EQ(texts[0].original, u8"\"コンマ,\"\"と引用符\"");
    }
}