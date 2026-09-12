#include "..\\src\\converter\\to_patch_csv.h"
#include "..\\src\\writer\\uniquerowcsvwriter.hpp"

//============================================================
// utility.hpp
//============================================================

TEST(Langscore_Utility, SplitByChar)
{
    auto result = utility::split(u8"a,b,,c"s, u8',');
    ASSERT_EQ(result.size(), 4u);
    EXPECT_TRUE(result[0] == u8"a");
    EXPECT_TRUE(result[1] == u8"b");
    EXPECT_TRUE(result[2] == u8"");
    EXPECT_TRUE(result[3] == u8"c");

    //区切り文字が無い場合は元の文字列がそのまま1要素で返る
    auto single = utility::split(u8"abc"s, u8',');
    ASSERT_EQ(single.size(), 1u);
    EXPECT_TRUE(single[0] == u8"abc");
}

TEST(Langscore_Utility, SplitByString)
{
    auto result = utility::split(u8"a::b::c"s, u8"::"s);
    ASSERT_EQ(result.size(), 3u);
    EXPECT_TRUE(result[0] == u8"a");
    EXPECT_TRUE(result[1] == u8"b");
    EXPECT_TRUE(result[2] == u8"c");

    //末尾が区切り文字の場合は空要素が付く
    auto trailing = utility::split(u8"a::"s, u8"::"s);
    ASSERT_EQ(trailing.size(), 2u);
    EXPECT_TRUE(trailing[1] == u8"");
}

TEST(Langscore_Utility, Replace)
{
    EXPECT_TRUE(utility::replace(u8"aXbXc"s, u8"X"s, u8"YY"s) == u8"aYYbYYc");
    //置換後の文字列が置換前を含んでいても無限ループにならないこと
    EXPECT_TRUE(utility::replace(u8"aa"s, u8"a"s, u8"aa"s) == u8"aaaa");
    //見つからない場合は変化しない
    EXPECT_TRUE(utility::replace(u8"abc"s, u8"X"s, u8"Y"s) == u8"abc");
}

TEST(Langscore_Utility, CnvStr)
{
    EXPECT_EQ(utility::cnvStr<std::string>(u8"abc"s), "abc"s);
    EXPECT_TRUE(utility::cnvStr<std::u8string>("abc"s) == u8"abc");

    auto list = utility::cnvStr<std::string>(utility::u8stringlist{u8"a", u8"b"});
    ASSERT_EQ(list.size(), 2u);
    EXPECT_EQ(list[0], "a"s);
    EXPECT_EQ(list[1], "b"s);
}

TEST(Langscore_Utility, Includes)
{
    EXPECT_TRUE(utility::includes(u8"abcdef"s, u8"cd"s));
    EXPECT_FALSE(utility::includes(u8"abcdef"s, u8"xy"s));
    //空文字は常に含まれる
    EXPECT_TRUE(utility::includes(u8"abcdef"s, u8""s));
}

TEST(Langscore_Utility, Trim)
{
    EXPECT_TRUE(utility::left_trim(u8"  ab  "s, u8" "s) == u8"ab  ");
    EXPECT_TRUE(utility::right_trim(u8"  ab  "s, u8" "s) == u8"  ab");
    //全て空白の場合は空文字になる
    EXPECT_TRUE(utility::left_trim(u8"   "s, u8" "s) == u8"");
    EXPECT_TRUE(utility::right_trim(u8"   "s, u8" "s) == u8"");
}

TEST(Langscore_Utility, RemoveExtension)
{
    EXPECT_TRUE(utility::removeExtension(u8"foo.csv"s) == u8"foo");
    //最後のドットのみを対象にする
    EXPECT_TRUE(utility::removeExtension(u8"foo.bar.csv"s) == u8"foo.bar");
    //拡張子が無い場合は変化しない
    EXPECT_TRUE(utility::removeExtension(u8"foo"s) == u8"foo");
}

TEST(Langscore_Utility, GetUTF8ByteLength)
{
    EXPECT_EQ(utility::getUTF8ByteLength(u8'a'), 1);
    EXPECT_EQ(utility::getUTF8ByteLength(u8"\u00E9"[0]), 2);     // 2バイト文字
    EXPECT_EQ(utility::getUTF8ByteLength(u8"あ"[0]), 3);         // ひらがな (3バイト)
    EXPECT_EQ(utility::getUTF8ByteLength(u8"\U0001F600"[0]), 4); // 絵文字 (4バイト)
}

TEST(Langscore_Utility, GetFileData)
{
    const auto path = fs::path(BINARY_DIRECTORY) / "utility_getfiledata_test.txt";
    {
        std::ofstream f(path, std::ios::binary);
        f << "abc";
    }

    auto data = utility::getFileData(path);
    ASSERT_EQ(data.size(), 3u);
    EXPECT_EQ(data[0], 'a');
    EXPECT_EQ(data[1], 'b');
    EXPECT_EQ(data[2], 'c');

    fs::remove(path);
}

//============================================================
// errorstatus.hpp
//============================================================

TEST(Langscore_ErrorStatus, DefaultIsSuccess)
{
    ErrorStatus status;
    EXPECT_TRUE(status.valid());
    EXPECT_FALSE(status.invalid());
    EXPECT_EQ(status.val(), ErrorStatus::NoError);
    EXPECT_TRUE(status.toStr().empty());
    EXPECT_TRUE(status == Status_Success);
}

TEST(Langscore_ErrorStatus, ModuleAndCode)
{
    ErrorStatus status(ErrorStatus::Module::INVOKER, 3);
    EXPECT_TRUE(status.invalid());
    EXPECT_EQ(status.moduleCode(), ErrorStatus::Module::INVOKER);
    EXPECT_EQ(status.code(), 3);
    //valはモジュール番号を4bit左シフトしてコードと合成する
    EXPECT_EQ(status.val(), (int(ErrorStatus::Module::INVOKER) << 4) | 3);
}

TEST(Langscore_ErrorStatus, ToStr)
{
    EXPECT_EQ(ErrorStatus(ErrorStatus::Module::INVOKER, 3).toStr(), "error code 3 : Not Found Convert file."s);
    EXPECT_EQ(ErrorStatus(ErrorStatus::Module::DIVISI, 1).toStr(), "error code 1 : Not Found Project Path."s);
    EXPECT_EQ(ErrorStatus(ErrorStatus::Module::DIVISI, 2).toStr(), "error code 2 : Unsupport Project Type"s);
    EXPECT_EQ(ErrorStatus(ErrorStatus::Module::PLATFORM_BASE, 1).toStr(), "error code 1 : Failed to Marge"s);
    //定義の無いコードは空文字
    EXPECT_TRUE(ErrorStatus(ErrorStatus::Module::INVOKER, 99).toStr().empty());
}

TEST(Langscore_ErrorStatus, ToStrWithSpecMsg)
{
    //invokerのコード255は指定メッセージをそのまま返す
    EXPECT_EQ(ErrorStatus(ErrorStatus::Module::INVOKER, 255, "custom message").toStr(), "custom message"s);

    //VXAce/MVMZは指定メッセージがあれば括弧書きで付加する
    EXPECT_EQ(ErrorStatus(ErrorStatus::Module::DIVISI_VXACE, 1).toStr(),
              "error code 1 : \"analyze\" has not been executed"s);
    EXPECT_EQ(ErrorStatus(ErrorStatus::Module::DIVISI_VXACE, 1, "detail").toStr(),
              "error code 1 : \"analyze\" has not been executed (detail)"s);
    EXPECT_EQ(ErrorStatus(ErrorStatus::Module::DIVISI_MVMZ, 1).toStr(),
              "error code 1 : validate error. Not Found Packing Input Directory."s);
}

//============================================================
// csvreader / csvwriter
//============================================================

class Langscore_Core : public ::testing::Test
{
protected:
    fs::path testDir;

    void SetUp() override
    {
        testDir = fs::path(BINARY_DIRECTORY) / "core_test" /
                  ::testing::UnitTest::GetInstance()->current_test_info()->name();
        std::error_code ec;
        fs::remove_all(testDir, ec);
        fs::create_directories(testDir);
    }

    void TearDown() override
    {
        std::error_code ec;
        fs::remove_all(testDir, ec);
    }

    //テスト対象のフィクスチャはテスト内で生成する。(test/dataはgitignoreされているため)
    //ナロー文字列リテラルは実行時文字セット (cp932) になるため、内容はu8リテラルで受け取る。
    fs::path writeTextFile(const std::string& name, std::u8string_view contents, bool withBom = false) const
    {
        auto path = testDir / name;
        std::ofstream f(path, std::ios::binary);
        if(withBom) {
            const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
            f.write(reinterpret_cast<const char*>(bom), sizeof(bom));
        }
        f.write(reinterpret_cast<const char*>(contents.data()), contents.size());
        return path;
    }

    static TranslateText makeText(std::u8string original, std::u8string ja, std::u8string en)
    {
        TranslateText t{std::move(original), {u8"ja", u8"en"}};
        t.translates[u8"ja"] = std::move(ja);
        t.translates[u8"en"] = std::move(en);
        return t;
    }
};

TEST_F(Langscore_Core, CsvReader_SkipsUtf8Bom)
{
    auto path = writeTextFile("bom.csv", u8"original,ja,en\nHello,こんにちは,Hello\n", true);

    auto plain = plaincsvreader{path}.getPlainCsvTexts();
    ASSERT_EQ(plain.size(), 2u);
    //BOMが先頭セルに混入していないこと
    EXPECT_TRUE(plain[0][0] == u8"original");

    langscore::csvreader reader{path};
    const auto& texts = reader.currentTexts();
    ASSERT_EQ(texts.size(), 1u);
    EXPECT_TRUE(texts[0].original == u8"Hello");
    EXPECT_TRUE(texts[0].translates.at(u8"ja") == u8"こんにちは");
    EXPECT_TRUE(texts[0].translates.at(u8"en") == u8"Hello");

    const auto& langs = reader.curerntUseLangList();
    ASSERT_EQ(langs.size(), 2u);
    EXPECT_TRUE(langs[0] == u8"ja");
    EXPECT_TRUE(langs[1] == u8"en");
}

TEST_F(Langscore_Core, CsvReader_QuotedCell)
{
    //カンマ・改行・エスケープされたクオートを含むセル
    auto path = writeTextFile("quote.csv",
                              u8"original,ja\n"
                              u8"\"a,b\",\"line1\nline2\"\n"
                              u8"\"say \"\"hi\"\"\",x\n");

    auto plain = plaincsvreader{path}.getPlainCsvTexts();
    ASSERT_EQ(plain.size(), 3u);
    EXPECT_TRUE(plain[1][0] == u8"a,b");
    EXPECT_TRUE(plain[1][1] == u8"line1\nline2");
    EXPECT_TRUE(plain[2][0] == u8"say \"hi\"");
    EXPECT_TRUE(plain[2][1] == u8"x");
}

TEST_F(Langscore_Core, CsvReader_RemovesCarriageReturn)
{
    //CRLF改行のファイル。セル内のCRも取り除かれる。
    auto path = writeTextFile("crlf.csv",
                              u8"original,ja\r\n"
                              u8"\"line1\r\nline2\",x\r\n");

    auto plain = plaincsvreader{path}.getPlainCsvTexts();
    ASSERT_EQ(plain.size(), 2u);
    EXPECT_TRUE(plain[0][0] == u8"original");
    EXPECT_TRUE(plain[0][1] == u8"ja");
    EXPECT_TRUE(plain[1][0] == u8"line1\nline2");
}

TEST_F(Langscore_Core, CsvReader_EscapeSequenceOnlyOutsideQuotes)
{
    //クオートで括られていないセルのバックスラッシュnは改行として解釈される。
    //ツクールの制御文字を壊さないよう、括られたセルでは解釈しない。
    auto path = writeTextFile("escape.csv",
                              u8"original,ja\n"
                              u8"a\\nb,\"c\\nd\"\n"
                              u8"\\C[1]red,\"\\C[1]red\"\n");

    auto plain = plaincsvreader{path}.getPlainCsvTexts();
    ASSERT_EQ(plain.size(), 3u);
    EXPECT_TRUE(plain[1][0] == u8"a\nb");
    EXPECT_TRUE(plain[1][1] == u8"c\\nd");
    //定義に無いエスケープはバックスラッシュごと残る
    EXPECT_TRUE(plain[2][0] == u8"\\C[1]red");
    EXPECT_TRUE(plain[2][1] == u8"\\C[1]red");
}

TEST_F(Langscore_Core, CsvReader_TrailingCommaAddsEmptyColumn)
{
    //改行で終わらず、行末がカンマの場合も列数を揃える
    auto path = writeTextFile("trailing.csv", u8"original,ja\na,");

    auto plain = plaincsvreader{path}.getPlainCsvTexts();
    ASSERT_EQ(plain.size(), 2u);
    ASSERT_EQ(plain[1].size(), 2u);
    EXPECT_TRUE(plain[1][0] == u8"a");
    EXPECT_TRUE(plain[1][1] == u8"");
}

TEST_F(Langscore_Core, CsvReader_InvalidInput)
{
    //存在しないファイル
    EXPECT_TRUE(plaincsvreader{testDir / "notfound.csv"}.getPlainCsvTexts().empty());
    EXPECT_TRUE(langscore::csvreader{testDir / "notfound.csv"}.currentTexts().empty());

    //ヘッダーのみ (データ行が無い)
    auto headerOnly = writeTextFile("header_only.csv", u8"original,ja\n");
    EXPECT_TRUE(langscore::csvreader{headerOnly}.currentTexts().empty());

    //列が1つしかないCSVは翻訳テキストとして扱えない
    auto oneColumn = writeTextFile("one_column.csv", u8"original\nHello\n");
    EXPECT_TRUE(langscore::csvreader{oneColumn}.currentTexts().empty());

    //空ファイル
    auto empty = writeTextFile("empty.csv", u8"");
    EXPECT_TRUE(plaincsvreader{empty}.getPlainCsvTexts().empty());
}

TEST_F(Langscore_Core, CsvWriter_RoundTrip)
{
    //書き出したCSVを読み直して同じ内容になること
    std::vector<TranslateText> texts = {
        makeText(u8"Hello", u8"こんにちは", u8"Hello"),
        makeText(u8"a,b", u8"あ,い", u8"a,b"),
        makeText(u8"line1\nline2", u8"1行目\n2行目", u8"line1\nline2"),
        makeText(u8"say \"hi\"", u8"「やあ」", u8"say \"hi\""),
    };

    const auto path = testDir / "roundtrip.csv";
    langscore::csvwriter writer{speciftranstext{{u8"ja", u8"en"}, texts}};
    ASSERT_EQ(writer.write(path, u8"ja", MergeTextMode::AcceptTarget), Status_Success);

    langscore::csvreader reader{path};
    const auto& actual = reader.currentTexts();
    ASSERT_EQ(actual.size(), texts.size());
    for(size_t i = 0; i < texts.size(); ++i)
    {
        SCOPED_TRACE(::testing::Message() << "row " << i);
        EXPECT_TRUE(actual[i].original == texts[i].original);
        EXPECT_TRUE(actual[i].translates.at(u8"ja") == texts[i].translates.at(u8"ja"));
        EXPECT_TRUE(actual[i].translates.at(u8"en") == texts[i].translates.at(u8"en"));
    }
}

TEST_F(Langscore_Core, CsvWriter_EmptyOriginalIsSkipped)
{
    std::vector<TranslateText> texts = {
        makeText(u8"", u8"空", u8"empty"),
        makeText(u8"Hello", u8"こんにちは", u8"Hello"),
    };

    const auto path = testDir / "skip_empty.csv";
    langscore::csvwriter writer{speciftranstext{{u8"ja", u8"en"}, texts}};
    ASSERT_EQ(writer.write(path, u8"ja", MergeTextMode::AcceptTarget), Status_Success);

    langscore::csvreader reader{path};
    ASSERT_EQ(reader.currentTexts().size(), 1u);
    EXPECT_TRUE(reader.currentTexts()[0].original == u8"Hello");
}

TEST_F(Langscore_Core, CsvWriter_FillDefaultLanguageColumn)
{
    //デフォルト言語の翻訳が空の場合、原文で埋める
    std::vector<TranslateText> texts = { makeText(u8"Hello", u8"", u8"") };

    const auto path = testDir / "filldef.csv";
    langscore::csvwriter writer{speciftranstext{{u8"ja", u8"en"}, texts}};
    writer.setFillDefLangCol(true);
    ASSERT_EQ(writer.write(path, u8"ja", MergeTextMode::AcceptTarget), Status_Success);

    langscore::csvreader reader{path};
    ASSERT_EQ(reader.currentTexts().size(), 1u);
    EXPECT_TRUE(reader.currentTexts()[0].translates.at(u8"ja") == u8"Hello");
    EXPECT_TRUE(reader.currentTexts()[0].translates.at(u8"en") == u8"");
}

//============================================================
// uniquerowcsvwriter
//============================================================

class Langscore_UniqueRowCsvWriter : public Langscore_Core
{
protected:
    //マージ元になるCSVを作る
    fs::path writeSourceCsv()
    {
        return writeTextFile("source.csv",
                             u8"original,ja,en\n"
                             u8"Hello,ソース日本語,source english\n"
                             u8"OnlyInSource,のみ,only\n");
    }

    //マージ先 (現在の解析結果) にあたるテキスト
    std::vector<TranslateText> targetTexts()
    {
        return {
            makeText(u8"Hello", u8"", u8"target english"),
            makeText(u8"OnlyInTarget", u8"対象のみ", u8""),
        };
    }
};

TEST_F(Langscore_UniqueRowCsvWriter, Merge_AcceptTargetKeepsTarget)
{
    auto source = writeSourceCsv();
    langscore::uniquerowcsvwriter writer{speciftranstext{{u8"ja", u8"en"}, targetTexts()}};
    writer.setOverwriteMode(MergeTextMode::AcceptTarget);
    ASSERT_TRUE(writer.merge(source));

    const auto& texts = writer.currentTexts();
    ASSERT_EQ(texts.size(), 2u);
    EXPECT_TRUE(texts[0].original == u8"Hello");
    EXPECT_TRUE(texts[1].original == u8"OnlyInTarget");
}

TEST_F(Langscore_UniqueRowCsvWriter, Merge_AcceptSourceReplacesWithSource)
{
    auto source = writeSourceCsv();
    langscore::uniquerowcsvwriter writer{speciftranstext{{u8"ja", u8"en"}, targetTexts()}};
    writer.setOverwriteMode(MergeTextMode::AcceptSource);
    ASSERT_TRUE(writer.merge(source));

    const auto& texts = writer.currentTexts();
    ASSERT_EQ(texts.size(), 2u);
    EXPECT_TRUE(texts[0].original == u8"Hello");
    EXPECT_TRUE(texts[0].translates.at(u8"ja") == u8"ソース日本語");
    EXPECT_TRUE(texts[1].original == u8"OnlyInSource");
}

TEST_F(Langscore_UniqueRowCsvWriter, Merge_KeepTargetFillsEmptyFromSource)
{
    //解析結果 (マージ先) の行を維持したまま、空欄だけをソースの翻訳で埋める。
    auto source = writeSourceCsv();
    langscore::uniquerowcsvwriter writer{speciftranstext{{u8"ja", u8"en"}, targetTexts()}};
    writer.setOverwriteMode(MergeTextMode::MergeKeepTarget);
    ASSERT_TRUE(writer.merge(source));

    const auto& texts = writer.currentTexts();
    ASSERT_EQ(texts.size(), 2u);

    EXPECT_TRUE(texts[0].original == u8"Hello");
    //空欄はソースから補完される
    EXPECT_TRUE(texts[0].translates.at(u8"ja") == u8"ソース日本語");
    //既に値がある場合はマージ先を優先する
    EXPECT_TRUE(texts[0].translates.at(u8"en") == u8"target english");

    //ソースに無い行も消えない
    EXPECT_TRUE(texts[1].original == u8"OnlyInTarget");
    EXPECT_TRUE(texts[1].translates.at(u8"ja") == u8"対象のみ");
}

TEST_F(Langscore_UniqueRowCsvWriter, Merge_EmptySourceKeepsTarget)
{
    auto source = writeTextFile("empty_source.csv", u8"original,ja,en\n");
    langscore::uniquerowcsvwriter writer{speciftranstext{{u8"ja", u8"en"}, targetTexts()}};
    writer.setOverwriteMode(MergeTextMode::MergeKeepTarget);
    ASSERT_TRUE(writer.merge(source));

    ASSERT_EQ(writer.currentTexts().size(), 2u);
}

//============================================================
// converter/to_patch_csv
//============================================================

TEST_F(Langscore_Core, ConvertPatchCsv_SplitsByLanguage)
{
    writeTextFile("Items.csv",
                  u8"original,ja,en\n"
                  u8"Potion,ポーション,Potion\n"
                  u8"Ether,エーテル,Ether\n");

    langscore::convert_patch_csv{testDir};

    for(const auto& lang : {u8"ja"s, u8"en"s})
    {
        SCOPED_TRACE(::testing::Message() << "lang " << utility::cnvStr<std::string>(lang));
        const auto path = testDir / lang / "Items.csv";
        ASSERT_TRUE(fs::exists(path)) << path.string();

        langscore::csvreader reader{path};
        //言語列は該当言語の1つだけになる
        const auto& langs = reader.curerntUseLangList();
        ASSERT_EQ(langs.size(), 1u);
        EXPECT_TRUE(langs[0] == lang);

        const auto& texts = reader.currentTexts();
        ASSERT_EQ(texts.size(), 2u);
        EXPECT_TRUE(texts[0].original == u8"Potion");
        EXPECT_EQ(texts[0].translates.size(), 1u);
    }
}

TEST_F(Langscore_Core, ConvertBoundCsv_RestoresSingleCsv)
{
    //言語毎に分割されたCSVを1つに戻す
    fs::create_directories(testDir / "ja");
    fs::create_directories(testDir / "en");
    writeTextFile("ja/Items.csv", u8"original,ja\nPotion,ポーション\nEther,エーテル\n");
    writeTextFile("en/Items.csv", u8"original,en\nPotion,Potion\nEther,Ether\n");

    langscore::convert_bound_csv{testDir};

    const auto path = testDir / "Items.csv";
    ASSERT_TRUE(fs::exists(path));

    langscore::csvreader reader{path};
    const auto& texts = reader.currentTexts();
    ASSERT_EQ(texts.size(), 2u);
    EXPECT_TRUE(texts[0].original == u8"Potion");
    EXPECT_TRUE(texts[0].translates.at(u8"ja") == u8"ポーション");
    EXPECT_TRUE(texts[0].translates.at(u8"en") == u8"Potion");
    EXPECT_TRUE(texts[1].translates.at(u8"ja") == u8"エーテル");
    EXPECT_TRUE(texts[1].translates.at(u8"en") == u8"Ether");
}

TEST_F(Langscore_Core, ConvertPatchCsv_RoundTrip)
{
    writeTextFile("Items.csv",
                  u8"original,ja,en\n"
                  u8"Potion,ポーション,Potion\n"
                  u8"\"a,b\",\"あ,い\",\"a,b\"\n");

    langscore::convert_patch_csv{testDir};
    langscore::convert_bound_csv{testDir};

    langscore::csvreader reader{testDir / "Items.csv"};
    const auto& texts = reader.currentTexts();
    ASSERT_EQ(texts.size(), 2u);
    EXPECT_TRUE(texts[0].original == u8"Potion");
    EXPECT_TRUE(texts[0].translates.at(u8"ja") == u8"ポーション");
    EXPECT_TRUE(texts[0].translates.at(u8"en") == u8"Potion");
    EXPECT_TRUE(texts[1].original == u8"a,b");
    EXPECT_TRUE(texts[1].translates.at(u8"ja") == u8"あ,い");
    EXPECT_TRUE(texts[1].translates.at(u8"en") == u8"a,b");
}

//============================================================
// config
//============================================================

class Langscore_ConfigAccessor : public Langscore_Core
{
protected:
    void TearDown() override
    {
        langscore::config::detachConfigFile();
        //test_main.cppで設定しているグローバルな設定ファイルに戻す
        langscore::config::attachConfigFile(".\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore\\config.json");
        Langscore_Core::TearDown();
    }

    //設定ファイルはグローバルにアタッチされるため、テスト毎に生成して差し替える
    void writeConfig(nlohmann::json json)
    {
        const auto path = testDir / "config.json";
        {
            std::ofstream f(path);
            f << json.dump(2);
        }
        langscore::config::detachConfigFile();
        langscore::config::attachConfigFile(path);
    }

    static nlohmann::json baseConfig(fs::path projectPath)
    {
        return {
            {"Project", projectPath.generic_string()},
            {"DefaultLanguage", "ja"},
            {"Languages", nlohmann::json::array({
                {{"Enable", true},  {"LanguageName", "ja"}, {"FontName", "M+ 1m regular"}, {"FontPath", "resources/fonts/mplus-1m-regular.ttf"}, {"FontSize", 22}},
                {{"Enable", true},  {"LanguageName", "en"}, {"FontName", "M+ 1m regular"}, {"FontPath", "resources/fonts/mplus-1m-regular.ttf"}, {"FontSize", 22}},
                {{"Enable", false}, {"LanguageName", "ko"}, {"FontName", "M+ 1m regular"}, {"FontPath", "resources/fonts/mplus-1m-regular.ttf"}, {"FontSize", 22}},
            })},
            {"Analyze", {{"TmpDir", "./analyze"}}},
            {"Write", {
                {"ExportDirectory", "./data/translate"},
                {"ExportByLang", false},
                {"EnableLanguagePatch", false},
                {"EnableTranslationDefLang", false},
            }},
            {"PackingInputDir", ""},
        };
    }
};

TEST_F(Langscore_ConfigAccessor, DetectsProjectTypeByExtension)
{
    //MV/MZは拡張子に加えて先頭5バイトのヘッダーも判定に使う。VXAceは拡張子のみ。
    const std::vector<std::tuple<std::string, std::string, langscore::config::ProjectType>> cases = {
        {"Game.rpgproject",  "RPGMV", langscore::config::MV},
        {"Game.rmmzproject", "RPGMZ", langscore::config::MZ},
        {"Game.rvproj2",     "",      langscore::config::VXAce},
        //ヘッダーが違うMV/MZプロジェクトは判別できない
        {"Game.rpgproject",  "XXXXX", langscore::config::None},
        {"Game.rmmzproject", "XXXXX", langscore::config::None},
    };

    int caseIndex = 0;
    for(const auto& [projectFile, header, expected] : cases)
    {
        SCOPED_TRACE(::testing::Message() << "project file " << projectFile << " header " << header);

        const auto projectDir = testDir / ("project" + std::to_string(caseIndex++));
        fs::create_directories(projectDir);
        { std::ofstream f(projectDir / projectFile, std::ios::binary); f << header; }

        writeConfig(baseConfig(projectDir));
        EXPECT_EQ(langscore::config{}.projectType(), expected);
    }

    //プロジェクトファイルが無い場合はNone
    const auto emptyDir = testDir / "no_project";
    fs::create_directories(emptyDir);
    writeConfig(baseConfig(emptyDir));
    EXPECT_EQ(langscore::config{}.projectType(), langscore::config::None);
}

TEST_F(Langscore_ConfigAccessor, EnableLanguagesAndDefault)
{
    writeConfig(baseConfig(testDir));
    langscore::config config;

    EXPECT_EQ(config.defaultLanguage(), "ja"s);
    EXPECT_EQ(config.allLanguages().size(), 3u);

    auto enables = config.enableLanguages();
    ASSERT_EQ(enables.size(), 2u);
    EXPECT_EQ(enables[0].name, "ja"s);
    EXPECT_EQ(enables[1].name, "en"s);
}

TEST_F(Langscore_ConfigAccessor, ExportDirectory_NormalMode)
{
    writeConfig(baseConfig(testDir));
    langscore::config config;

    EXPECT_FALSE(config.exportByLanguage());
    EXPECT_FALSE(config.enableLanguagePatch());

    std::u8string root;
    auto dirs = config.exportDirectory(root);
    //通常モードでは1つのフォルダにまとめて出力する
    ASSERT_EQ(dirs.size(), 1u);
    EXPECT_TRUE(dirs[0] == root);
    EXPECT_TRUE(config.exportDirectoryWithLang(root).empty());
}

TEST_F(Langscore_ConfigAccessor, ExportDirectory_LanguagePatchMode)
{
    auto json = baseConfig(testDir);
    json["Write"]["EnableLanguagePatch"] = true;
    writeConfig(std::move(json));
    langscore::config config;

    EXPECT_TRUE(config.enableLanguagePatch());
    //EnableLanguagePatchが有効なら言語別出力も有効になる
    EXPECT_TRUE(config.exportByLanguage());

    std::u8string root;
    auto dirs = config.exportDirectory(root);
    //有効な言語の数だけフォルダが作られる
    ASSERT_EQ(dirs.size(), 2u);
    EXPECT_TRUE(dirs[0] == root + u8"/ja");
    EXPECT_TRUE(dirs[1] == root + u8"/en");

    auto pairs = config.exportDirectoryWithLang(root);
    ASSERT_EQ(pairs.size(), 2u);
    EXPECT_TRUE(pairs[0].first == u8"ja");
    EXPECT_TRUE(pairs[0].second == root + u8"/ja");
    EXPECT_TRUE(pairs[1].first == u8"en");
    EXPECT_TRUE(pairs[1].second == root + u8"/en");
}

TEST_F(Langscore_ConfigAccessor, EnableTranslationDefLang)
{
    writeConfig(baseConfig(testDir));
    EXPECT_FALSE(langscore::config{}.enableTranslationDefLang());

    auto json = baseConfig(testDir);
    json["Write"]["EnableTranslationDefLang"] = true;
    writeConfig(std::move(json));
    EXPECT_TRUE(langscore::config{}.enableTranslationDefLang());
}
