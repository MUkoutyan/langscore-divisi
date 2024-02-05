
IUTEST(Langscore_Writer, UTF8WordCount)
{
	ScriptTextParser scriptRegex;
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"Honi"s), 4);		//ASCII
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"©ÀʸΦ"s), 4);		//2Byte
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"あいうえお"s), 5);	//3Byte
	IUTEST_ASSERT_EQ(scriptRegex.wordCountUTF8(u8"𦹀𧃴𧚄𨉷𨏍𪆐"s), 6);	//4Byte
}

IUTEST(Langscore_Writer, ConvertWorkList)
{
	using namespace std::string_view_literals;
	ScriptTextParser scriptRegex;
	{
		auto baseStr = u8"HoniHoni"s;
		auto result = scriptRegex.ConvertWordList(baseStr);
		IUTEST_ASSERT(result.size() == 8);
		size_t i = 0;
		IUTEST_ASSERT_EQ(std::get<0>(result[i++]), u8"H"sv);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"n"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"i"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"H"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"n"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"i"sv) == 0);
	}

	{
		auto baseStr = u8"あいうえおoooかきくけこ"s;
		auto result = scriptRegex.ConvertWordList(baseStr);
		IUTEST_ASSERT(result.size() == 13);
		size_t i = 0;
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"あ"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"い"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"う"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"え"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"お"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"o"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"か"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"き"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"く"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"け"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"こ"sv) == 0);
	}

	{
		auto baseStr = u8"𦹀𧃴𧚄𨉷𨏍𪆐🙁😇"s;
		auto result = scriptRegex.ConvertWordList(baseStr);
		IUTEST_ASSERT(result.size() == 8);
		size_t i = 0;
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𦹀"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𧃴"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𧚄"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𨉷"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𨏍"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"𪆐"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"🙁"sv) == 0);
		IUTEST_ASSERT(std::get<0>(result[i++]).compare(u8"😇"sv) == 0);
	}

}

IUTEST(Langscore_Writer, DetectStringPosition)
{
	ScriptTextParser scriptRegex;
	{
		auto result = scriptRegex.findStrings(u8"Test Line Script");
		IUTEST_ASSERT(result.empty());
	}
	{
		auto result = scriptRegex.findStrings(u8"");
		IUTEST_ASSERT(result.empty());
	}
	{
		auto result = scriptRegex.findStrings(u8"hoge = \"\";");
		IUTEST_ASSERT(result.empty());
	}
	{
		auto result = scriptRegex.findStrings(u8"Test \"Line\" Script");
		IUTEST_ASSERT_EQ(result.size(), 1);
		IUTEST_ASSERT_EQ(std::get<0>(result[0]), u8"Line"s);
		IUTEST_ASSERT_EQ(std::get<1>(result[0]), 7);
	}
	{
		auto result = scriptRegex.findStrings(u8"\"Test\" Line S\"cript\"");
		IUTEST_ASSERT_EQ(result.size(), 2);
		IUTEST_ASSERT_EQ(std::get<0>(result[0]), u8"Test"s);
		IUTEST_ASSERT_EQ(std::get<1>(result[0]), 2);
		IUTEST_ASSERT_EQ(std::get<0>(result[1]), u8"cript"s);
		IUTEST_ASSERT_EQ(std::get<1>(result[1]), 15);
	}
	{
		auto result = scriptRegex.findStrings(u8"auto text = \"あいうえお\"; auto text2 = \"aiueo\""s);
		IUTEST_ASSERT_EQ(result.size(), 2);
		IUTEST_ASSERT_EQ(std::get<0>(result[0]), u8"あいうえお"s);
		IUTEST_ASSERT_EQ(std::get<1>(result[0]), 14);
		IUTEST_ASSERT_EQ(std::get<0>(result[1]), u8"aiueo"s);
		IUTEST_ASSERT_EQ(std::get<1>(result[1]), 36);
	}
	{
		auto result = scriptRegex.findStrings(u8"text = \"\\C[16]プレイ時間\\X[104]\\C[0]\\T[%4$3d\\\"%3$02d]\""s);
		IUTEST_ASSERT_EQ(result.size(), 1);
		IUTEST_ASSERT_EQ(std::get<0>(result[0]), u8"\\C[16]プレイ時間\\X[104]\\C[0]\\T[%4$3d\\\"%3$02d]"s);
	}
	//langscore.rbで正しくCSVが出力されない問題の確認テスト。
	//""括りかつ文字列内に,が存在する場合、""が括られたまま結果が出力される。(CSV用の変換)
	//CSV用の変換はCSVの書き出し時点で行うため、findStringsで行うのは不適切。
	{
		auto result = scriptRegex.findStrings(u8"      raise \"Error!: Missmatch Num Cells : #{file_name}, #{mismatch_cells.to_s}\" "s);
		IUTEST_ASSERT_EQ(result.size(), 1);
		IUTEST_ASSERT_EQ(std::get<0>(result[0]), u8"Error!: Missmatch Num Cells : #{file_name}, #{mismatch_cells.to_s}"s);
	}
}

IUTEST(Langscore_Writer, CheckRubyCommentLine)
{
	langscore::rubyreader scriptWriter({}, {});

	{
		bool isRangeComment = false;
		std::u8string original = u8"chstring";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, original);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"#Commentout";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"=begin";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
		IUTEST_ASSERT(isRangeComment);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"\"Hoge\" =begin";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"\"Hoge\"       "s);
		IUTEST_ASSERT(isRangeComment);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"\"Hoge\" \"=begin\"Honi";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"\"Hoge\" \"=begin\"Honi"s);
		IUTEST_ASSERT(isRangeComment == false);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"Valid Script Line";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"=end";
		std::u8string text = original;
		isRangeComment = true;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
		IUTEST_ASSERT(isRangeComment == false);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"Without Comment Block";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_EQ(text, original);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"Script~~~ #Comment";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"Script~~~         "s);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8"\"Key:#{Value}\"";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"\"Key:#{Value}\""s);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"("Text".lstrans'Honi')";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"              )");
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"("Text".lstrans"64:28:1")";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"                 )");
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"("Text".lstrans(''))";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"            )");
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"(Honi:"Te'xt".lstrans('Hoge') "DetectText")";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"(Honi:"Te'xt"                 "DetectText")"s);
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"("Text".lstrans(""))";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"("Text"            )");
	}
	{
		bool isRangeComment = false;
		std::u8string original = u8R"(Honi:"Text".lstrans("Hoge") "DetectText")";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8R"(Honi:"Text"                 "DetectText")"s);
	}
}


IUTEST(Langscore_Writer, CheckJavaScriptCommentLine)
{
	langscore::javascriptreader scriptWriter({}, {});

	bool isRangeComment = false;
	{
		std::u8string original = u8"chstring";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, original);
	}
	{
		std::u8string original = u8"//Commentout";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
	}
	{
		std::u8string original = u8"/*Hoge";
		std::u8string text = original;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
		IUTEST_ASSERT_STREQ(text, u8"      ");
		IUTEST_ASSERT(isRangeComment);
	}
	{
		std::u8string original = u8"Hoge*/";
		std::u8string text = original;
		isRangeComment = true;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::NextLine);
		IUTEST_ASSERT_STREQ(text, u8"      ");
		IUTEST_ASSERT(!isRangeComment);
	}
	{
		std::u8string original = u8"Hoge*/";
		std::u8string text = original;
		isRangeComment = false;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"Hoge*/");
		IUTEST_ASSERT(!isRangeComment);
	}
	{
		std::u8string original = u8"\"Hoge\"/*Comment*/\"Honi\"";
		std::u8string text = original;
		isRangeComment = false;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"\"Hoge\"           \"Honi\"");
		IUTEST_ASSERT(!isRangeComment);
	}
	{
		std::u8string original = u8"\"Hoge\"//Comment*/\"Honi\"";
		std::u8string text = original;
		isRangeComment = false;
		auto result = scriptWriter.replaceCommentToSpace(text, isRangeComment);
		IUTEST_ASSERT_EQ(result, langscore::readerbase::ProgressNextStep::Throught);
		IUTEST_ASSERT_STREQ(text, u8"\"Hoge\"                 ");
		IUTEST_ASSERT(!isRangeComment);
	}
}

IUTEST(Langscore_Writer, DetectRubyString)
{
	std::u8string fileName = u8"detectstring";
	langscore::rubyreader scriptWriter({}, {});

	auto extracted_strings = scriptWriter.convertScriptToCSV(u8"./data/vxace/" + fileName + u8".rb");

	IUTEST_ASSERT_EQ(10, extracted_strings.size());
	IUTEST_ASSERT_STREQ(u8"Hello, world!", extracted_strings[0].original);
	IUTEST_ASSERT_STREQ(u8"This is a \"quote\" character.", extracted_strings[1].original);
	IUTEST_ASSERT_STREQ(u8"This string has a \n newline character.", extracted_strings[2].original);
	IUTEST_ASSERT_STREQ(u8"This one has a , comma character.", extracted_strings[3].original);
	IUTEST_ASSERT_STREQ(u8"This string has both \"quote\" and \n newline characters.", extracted_strings[4].original);
	IUTEST_ASSERT_STREQ(u8"This is a 'single quote' character.", extracted_strings[5].original);
	IUTEST_ASSERT_STREQ(u8"This string has a \"quote\", a 'single quote' and a \n newline character.", extracted_strings[6].original);
	IUTEST_ASSERT_STREQ(u8"\"\"", extracted_strings[7].original);
	IUTEST_ASSERT_STREQ(u8"\n", extracted_strings[8].original);
	IUTEST_ASSERT_STREQ(u8",", extracted_strings[9].original);
}

IUTEST(Langscore_Writer, DetectStringPositionFromFile)
{
	std::u8string fileName = u8"chstring";
	langscore::rubyreader scriptWriter({}, {});

	auto result = scriptWriter.convertScriptToCSV(u8"./data/vxace/" + fileName + u8".rb");

	IUTEST_ASSERT_EQ(result.size(), 14);

	size_t i = 0;
	IUTEST_ASSERT_STREQ(result[i++].original, u8"あHoniい"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"𦹀𧃴𧚄𨉷𨏍𪆐🙁😇"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"HoniHoni"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"trans #{text}"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"call initialize_clone"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"call initialize_clone"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"call to_str"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"あいうえお"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"A"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"B"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"A"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"翻訳テキスト"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"翻訳テキスト2"s);
	IUTEST_ASSERT_STREQ(result[i++].original, u8"\\C[16]プレイ時間\\X[104]\\C[0]\\T[%4$3d'%3$02d]"s);

	i = 0;
	//"を含まない単語の開始位置
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":8:4");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":8:16");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":11:2");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":16:10");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":23:8");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":28:8");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":33:8");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":38:6");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":43:13");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":44:4");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":45:4");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":47:2");
	IUTEST_ASSERT_STREQ(result[i++].scriptLineInfo, fileName + u8":48:2");
}

IUTEST(Langscore_Writer, ConvertCsvText_ASCII)
{
	langscore::csvwriter writer(speciftranstext{{}, {}});

	{
		auto input = u8"Hello, World!";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"Hello, World!\"");
	}
	{
		auto input = u8"First line\nSecond line";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"First line\nSecond line\"");
	}
	{
		auto input = u8"Quote \" in the text";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"Quote \"\" in the text\"");
	}
	{
		auto input = u8"Comma, and quote \"";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"Comma, and quote \"\"\"");
	}
}

IUTEST(Langscore_Writer, ConvertCsvText_Multibyte)
{
	langscore::csvwriter writer(speciftranstext{{}, {}});

	{
		auto input = u8"こんにちは、世界！";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"こんにちは、世界！");
	}
	{
		auto input = u8"こんにちは,世界！";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"こんにちは,世界！\"");
	}
	{
		auto input = u8"最初の行\n次の行";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"最初の行\n次の行\"");
	}
	{
		auto input = u8"文章中の\"記号";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"文章中の\"\"記号\"");
	}
	{
		auto input = u8"コンマ,\"と引用符";
		auto result = writer.convertCsvText(input);
		IUTEST_ASSERT_STREQ(result, u8"\"コンマ,\"\"と引用符\"");
	}
}