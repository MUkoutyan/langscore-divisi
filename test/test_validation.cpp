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
    std::u8string invalidFontPath = u8"Invalid/Path/To/Font.ttf";
    int fontSize = 12;
    auto result = this->measureTextWidth(text, invalidFontPath, fontSize);
    EXPECT_TRUE(result.empty()); // フォントが無効な場合、空の結果が返ることを期待
}