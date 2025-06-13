#include <gtest/gtest.h>
#include "platform/platform_base.h"


class PlatformBaseTest : public platform_base, public ::testing::Test {
protected:

    void setProjectPath(std::filesystem::path path) override {}
    ErrorStatus analyze() override { return Status_Success; }
    ErrorStatus reanalysis() override { return Status_Success; }
    ErrorStatus updatePlugin() override { return Status_Success; }
    ErrorStatus exportCSV() override { return Status_Success; }
    ErrorStatus validate() override { return Status_Success; }
    ErrorStatus packing() override { return Status_Success; }

    // テスト用のヘルパーメソッド
    bool TestValidateTexts(std::vector<TranslateText> translateList, const config::TextValidateTypeMap& validateInfoList, std::filesystem::path path) {
        std::vector<ValidateTextInfo> validateTexts;
        for(auto& text : translateList) {
            ValidateTextInfo validateText;
            validateText.origin = text;
            validateTexts.emplace_back(std::move(validateText));
        }
        return this->validateTexts(std::move(validateTexts), validateInfoList, std::move(path));
    }
};

TEST_F(PlatformBaseTest, CountNumTexts)
{
    //platform_base::countNumTextsのテストを複数のパターン用意する。
    //このとき、日本語・英語・中国語や、制御文字が含まれる場合などを考慮する。
    //また、文字長も様々な長さのものを用意する。

    {
        auto result = countNumTexts(u8"あいうえお");
        EXPECT_EQ(result, 5);
    }
    {
        auto result = countNumTexts(u8"abcde");
        EXPECT_EQ(result, 5);
    }
    {
        auto result = countNumTexts(u8"あいうえお\nかきくけこ");
        EXPECT_EQ(result, 5);
    }
    {
        auto result = countNumTexts(u8"abcde\nfghij");
        EXPECT_EQ(result, 5);
    }
    {
        auto result = countNumTexts(u8"あいうえお\nかきくけこ\nさしすせそ");
        EXPECT_EQ(result, 5);
    }
    {
        auto result = countNumTexts(u8"abcde\nfghij\nklmno");
        EXPECT_EQ(result, 5);
    }
    {
        auto result = countNumTexts(u8"あいうえお\nかきくけこ\nさしす\nたと");
        EXPECT_EQ(result, 5);
    }
    {
        auto result = countNumTexts(u8"abcde\nfghijkl\nmno\npqrst");
        EXPECT_EQ(result, 7);
    }
    // エラーとなりそうなデータのテストケース
    {
        auto result = countNumTexts(u8"");
        EXPECT_EQ(result, 0);
    }
    {
        auto result = countNumTexts(u8"\n\n\n");
        EXPECT_EQ(result, 0);
    }
    {
        auto result = countNumTexts(u8"あいうえお\0かきくけこ");
        EXPECT_EQ(result, 5); // Null文字以降は無視されることを期待
    }
    {
        auto result = countNumTexts(u8"abcde\0fghij");
        EXPECT_EQ(result, 5); // Null文字以降は無視されることを期待
    }
    {
        auto result = countNumTexts(u8"あいうえお\nかきくけこ\n\0さしすせそ");
        EXPECT_EQ(result, 5); // Null文字以降は無視されることを期待
    }
    {
        auto result = countNumTexts(u8"abcde\nfghij\n\0klmno");
        EXPECT_EQ(result, 5); // Null文字以降は無視されることを期待
    }
    // 複雑な文章のテストケース
    {
        auto result = countNumTexts(u8"こんにちは、世界！🌏✨\ntest.\n测试文本");
        EXPECT_EQ(result, 11); // "こんにちは、世界！🌏✨" の文字数
    }
    {
        auto result = countNumTexts(u8"Line with emojis 😊🚀🔥\nAnother line\n短い");
        EXPECT_EQ(result, 20); // "Line with emojis 😊🚀🔥" の文字数
    }
    {
        auto result = countNumTexts(u8"Combining characters: a\u0300a\u0301a\u0302\nNormal text");
        EXPECT_EQ(result, 28); // "Combining characters: àáâ" の文字数
    }
    {
        auto result = countNumTexts(u8"非常に長い行をテストするための文字列です。これは複数の文字を含み、特定の制限を確認するために使用されます。\n短い行");
        EXPECT_EQ(result, 53); // "非常に長い行をテストするための文字列です。これは複数の文字を含み、特定の制限を確認するために使用されます。" の文字数
    }
    {
        auto result = countNumTexts(u8"Special characters !@#$%^&*()_+-=[]{}|;':\",./<>?\nAnother line");
        EXPECT_EQ(result, 48); // "Special characters !@#$%^&*()_+-=[]{}|;':\",./<>?" の文字数
    }
}


const std::u8string kTestFontPath = (std::filesystem::path(TEST_DATA_SRC) / u8"fonts\\VL-Gothic-Regular.ttf").u8string(); // フォントファイルのパスを指定
TEST_F(PlatformBaseTest, MeasureTextWidth_EmptyString) {
    std::u8string text = u8"";
    int fontSize = 12;
    auto result = this->measureTextWidth(text, kTestFontPath, fontSize);
    EXPECT_TRUE(result.empty());
}

TEST_F(PlatformBaseTest, MeasureTextWidth_SingleASCII) {
    std::u8string text = u8"A";
    int fontSize = 12;
    auto result = this->measureTextWidth(text, kTestFontPath, fontSize);
    ASSERT_EQ(result.size(), 1u);
    EXPECT_GT(result[0].second.right, 0);
}

TEST_F(PlatformBaseTest, MeasureTextWidth_MultipleASCII) {
    std::u8string text = u8"Hello";
    int fontSize = 12;
    auto result = this->measureTextWidth(text, kTestFontPath, fontSize);
    ASSERT_EQ(result.size(), 5u);
    for(const auto& pair : result) {
        EXPECT_GT(pair.second.right, 0);
    }
}

TEST_F(PlatformBaseTest, MeasureTextWidth_UnicodeCharacters) {
    std::u8string text = u8"こんにちは"; // "Hello" in Japanese
    int fontSize = 12;
    auto result = this->measureTextWidth(text, kTestFontPath, fontSize);
    ASSERT_EQ(result.size(), 5u);
    for(const auto& pair : result) {
        EXPECT_GT(pair.second.right, 0);
    }
}

TEST_F(PlatformBaseTest, MeasureTextWidth_Emojis) {
    std::u8string text = u8"Hello 🌏✨";
    int fontSize = 12;
    auto result = this->measureTextWidth(text, kTestFontPath, fontSize);
    ASSERT_EQ(result.size(), 8u); // "Hello " + 🌏 + ✨
    for(const auto& pair : result) {
        EXPECT_GT(pair.second.right, 0);
    }
}

TEST_F(PlatformBaseTest, MeasureTextWidth_ControlCharacters) {
    std::u8string text = u8"Line1\nLine2\tEnd";
    int fontSize = 12;
    auto result = this->measureTextWidth(text, kTestFontPath, fontSize);
    ASSERT_EQ(result.size(), 14u); // 各文字の数に応じて調整
    for(const auto& pair : result) {
        EXPECT_GE(pair.second.right, 0);
    }
}

TEST_F(PlatformBaseTest, MeasureTextWidth_VariousFontSizes) {
    std::u8string text = u8"Test";
    for(int fontSize = 8; fontSize <= 72; fontSize += 8) {
        auto result = this->measureTextWidth(text, kTestFontPath, fontSize);
        ASSERT_EQ(result.size(), 4u);
        for(const auto& pair : result) {
            EXPECT_GT(pair.second.right, 0);
        }
    }
}

TEST_F(PlatformBaseTest, MeasureTextWidth_LongString) {
    std::u8string text = u8"The quick brown fox jumps over the lazy dog.";
    int fontSize = 12;
    auto result = this->measureTextWidth(text, kTestFontPath, fontSize);
    ASSERT_EQ(result.size(), text.length());
    for(const auto& pair : result) {
        EXPECT_GT(pair.second.right, 0);
    }
}

TEST_F(PlatformBaseTest, MeasureTextWidth_UnsupportedCharacters) {
    std::u8string text = u8"Test 🚀🔥";
    int fontSize = 12;
    // 非対応文字が含まれる場合でも処理が継続するか確認
    auto result = this->measureTextWidth(text, kTestFontPath, fontSize);
    ASSERT_EQ(result.size(), 7u); // "Test " + 🚀 + 🔥
    for(const auto& pair : result) {
        // 幅の計測ができない文字でも処理が止まらないこと
        EXPECT_GE(pair.second.right, 0);
    }
}

TEST_F(PlatformBaseTest, MeasureTextWidth_InvalidFontPath) {
    std::u8string text = u8"Test";
    std::u8string invalidFontPath = u8"fonts/expected/missing-is-normal.ttf";
    int fontSize = 12;
    auto result = this->measureTextWidth(text, invalidFontPath, fontSize);
    EXPECT_TRUE(result.empty()); // フォントが無効な場合、空の結果が返ることを期待
}

// テスト用のモッククラス
class TestPlatformBase : public platform_base {
public:
    void setProjectPath(std::filesystem::path path) override {}
    ErrorStatus analyze() override { return ErrorStatus(); }
    ErrorStatus reanalysis() override { return ErrorStatus(); }
    ErrorStatus updatePlugin() override { return ErrorStatus(); }
    ErrorStatus exportCSV() override { return ErrorStatus(); }
    ErrorStatus validate() override { return ErrorStatus(); }
    ErrorStatus packing() override { return ErrorStatus(); }


    // テスト用にprotectedメソッドを公開
    using platform_base::convertCopyOption;
    using platform_base::exportFolderPath;
    using platform_base::writeFixedGraphFileNameData;
    using platform_base::validateCsvFormat;
    using platform_base::validateTextFormat;
    using platform_base::validateTexts;
    using platform_base::countNumTexts;
    using platform_base::measureTextWidth;
    using platform_base::convertValidateTextInfo;
    using platform_base::detectConstrolChar;
};

// std::coutの出力をキャプチャするためのヘルパークラス
class CoutCapture {
public:
    CoutCapture() {
        oldBuffer = std::cout.rdbuf();
        std::cout.rdbuf(captureStream.rdbuf());
    }

    ~CoutCapture() {
        std::cout.rdbuf(oldBuffer);
    }

    std::string getCapturedOutput() {
        return captureStream.str();
    }

private:
    std::ostringstream captureStream;
    std::streambuf* oldBuffer;
};

// csvreaderとValidateFileInfoのテスト
TEST_F(PlatformBaseTest, ValidateCsvFormat_MissingFile) {
    // ValidateFileInfoの正しい構造を使用
    ValidateFileInfo fileInfo;
    fileInfo.csvPath = std::filesystem::path("non_existent_file.csv");

    CoutCapture capture;

    // csvreaderはnullptrとして渡す（ファイルが存在しない場合のテスト）
    bool result = this->validateCsvFormat(fileInfo, {u8"ja", u8"en"});

    // 結果の検証
    std::string output = capture.getCapturedOutput();
    EXPECT_TRUE(output.find("\"Type\":1") != std::string::npos);
    // 出力にファイルパスの情報が含まれているか確認
    EXPECT_TRUE(output.find("non_existent_file.csv") != std::string::npos);
}

// テキスト検証のテスト
TEST_F(PlatformBaseTest, ValidateTextFormat_Test) {
    // ValidateTextInfoの正しい構造でオブジェクトを作成
    std::vector<ValidateTextInfo> texts;
    ValidateTextInfo validateText;

    // TranslateTextの正しい構造を使用
    validateText.display.original = u8"テスト原文";
    validateText.display.translates[u8"ja"] = u8"テスト文字列";
    validateText.display.translates[u8"en"] = u8"Test string with \r\n line breaks";
    validateText.origin.textType = {u8"message"};
    texts.push_back(validateText);

    // 検証条件設定（TextValidateTypeMapの正しい構造を使用）
    config::TextValidateTypeMap validateInfoList;
    // テキストタイプ -> 言語 -> 検証情報のマッピング
    validateInfoList[u8"message"][u8"en"].mode = config::ValidateTextMode::TextCount;
    validateInfoList[u8"message"][u8"en"].count = 10; // 文字数制限を10に設定

    CoutCapture capture;

    // validateTextFormatを呼び出し
    bool result = this->validateTexts(texts, validateInfoList, std::filesystem::path("test_path"));

    // 結果の検証
    std::string output = capture.getCapturedOutput();
    EXPECT_TRUE(output.find("\"Type\":1") != std::string::npos);
}

// validateTextsのテスト
TEST_F(PlatformBaseTest, ValidateTexts_WithErrors) {
    // 複数のテキストを含むテストケース
    std::vector<ValidateTextInfo> translateList;

    // 最初のテキスト（長すぎるテキスト）
    ValidateTextInfo text1;
    text1.display.original = u8"原文1";
    text1.display.translates[u8"ja"] = u8"日本語テキスト";
    text1.display.translates[u8"en"] = u8"Very long English text that exceeds the maximum length limit";
    // テキストタイプを設定
    text1.origin.textType.push_back(u8"message");
    translateList.push_back(text1);

    // 2つ目のテキスト（制御文字を含む）
    ValidateTextInfo text2;
    text2.display.original = u8"原文2";
    text2.display.translates[u8"ja"] = u8"制御文字\r\n含む";
    text2.display.translates[u8"en"] = u8"Contains\tcontrol\rchars";
    text2.origin.textType.push_back(u8"message");
    translateList.push_back(text2);

    // 検証条件設定
    config::TextValidateTypeMap validateInfoList;
    // 文字数による検証モード
    validateInfoList[u8"message"][u8"en"].mode = config::ValidateTextMode::TextCount;
    validateInfoList[u8"message"][u8"en"].count = 20;  // 英語の最大文字数

    // 幅による検証モード（制御文字は自動的に検出される）
    validateInfoList[u8"message"][u8"ja"].mode = config::ValidateTextMode::TextWidth;
    validateInfoList[u8"message"][u8"ja"].width = 200;  // 日本語の最大幅

    CoutCapture capture;

    // TestValidateTextsを通してテスト
    bool result = this->validateTexts(translateList, validateInfoList, "test_path");

    // 結果の検証
    std::string output = capture.getCapturedOutput();
    EXPECT_TRUE(output.find("\"Type\":1") != std::string::npos);
    // 長さエラーの検証
    EXPECT_TRUE(output.find("\"Language\":\"en\"") != std::string::npos);
    EXPECT_TRUE(output.find("\"Summary\":7") != std::string::npos);
}

// 空のテキストリストでのvalidateTextsのテスト
TEST_F(PlatformBaseTest, ValidateTexts_EmptyList) {
    // 空のテキストリスト
    std::vector<ValidateTextInfo> emptyList;

    // TextValidateTypeMapの正しい構造
    config::TextValidateTypeMap validateInfoList;
    // いくつかのデフォルト設定を追加
    validateInfoList[u8"message"][u8"en"].mode = config::ValidateTextMode::TextCount;
    validateInfoList[u8"message"][u8"en"].count = 100;

    std::filesystem::path path("test_path");

    CoutCapture capture;

    // validateTextsを呼び出し
    bool result = this->validateTexts(std::move(emptyList), validateInfoList, path);

    std::string output = capture.getCapturedOutput();
    // 空リストでも何かしらの出力がある場合はそれを検証
    if(!output.empty()) {
        EXPECT_TRUE(output.find("\"Type\":1") != std::string::npos ||
            output.find("empty") != std::string::npos);
    }
}
