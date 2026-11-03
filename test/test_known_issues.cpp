//-------------------------------------------------------------------
// 既知の不具合を検出するテスト
//
// ここのテストは「あるべき動作」を書いており、現在の src/ の実装では失敗する。
// 不具合が修正されたら通るようになる。
//
//   Langscore_KnownIssue_MV.ActorNameFromMapEventIsAppended
//       divisi_mvmz.cpp:350 fetchActorTextFromMap の return (VXAce は continue)
//   Langscore_KnownIssue_CsvWriter.ColumnOrderFollowsHeader
//       csvwriter.cpp:189,214 行のセルを unordered_map の列挙順で並べている
//   Langscore_KnownIssue_MV.ActorNameFromMapEventIsAppendedForEachLanguage
//       divisi_mvmz.cpp:712 jsonreader_map が揃う前に fetchActorTextFromMap を呼んでいる
//   Langscore_KnownIssue_MV.ValidateCSVNameListFilePath
//   Langscore_KnownIssue_MV.ValidateCSVNameListFolderPath
//       divisi_mvmz.cpp:449 ValidateCSVNameList の指定に .csv を付けて開いている
//   Langscore_KnownIssue_VXAce.GraphicsCsvKeepsFileName
//       divisi_vxace.cpp:517 画像のパスにファイル名ではなく拡張子を付けている
//   Langscore_KnownIssue_Script.IgnorePluginParameter
//       config.cpp:442 IgnorePoints の ParameterText を読んでいない
//   Langscore_KnownIssue_Script.IgnorePluginDoesNotAffectPrefixNamedPlugin
//       javascriptreader.hpp:85 無視するプラグインを名前の部分一致で探している
//   Langscore_KnownIssue_Config.FontPathKeepsNonAsciiCharacters
//       config.cpp:164 UTF-8 の std::string をそのまま std::filesystem::path にしている
//   Langscore_KnownIssue_MV_Reanalysis.PluginSubFolder
//       divisi_mvmz.cpp:250 Scripts のサブフォルダを、中身があるまま fs::remove している
//   Langscore_KnownIssue_MV_Reanalysis.ReadOnlyGameData
//       divisi_mvmz.cpp:239 読み取り専用の属性ごとコピーした解析ファイルを上書きできない
//   Langscore_KnownIssue_MV_Reanalysis.RemovedPluginIsRemovedFromAnalyze
//       divisi_mvmz.cpp:247 ゲームから削除したプラグインが解析フォルダに残る
//-------------------------------------------------------------------

#include <sstream>

namespace
{
    //テスト中だけゲームプロジェクトのファイルを書き換え、デストラクタで必ず元へ戻す。
    class ScopedFilePatch
    {
    public:
        explicit ScopedFilePatch(std::filesystem::path path)
            : path(std::move(path))
        {
            std::ifstream in(this->path, std::ios::binary);
            backup.assign(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
        }

        ~ScopedFilePatch()
        {
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            out.write(backup.data(), static_cast<std::streamsize>(backup.size()));
        }

        ScopedFilePatch(const ScopedFilePatch&) = delete;
        ScopedFilePatch& operator=(const ScopedFilePatch&) = delete;

        bool valid() const { return backup.empty() == false; }

        nlohmann::json loadJson() const { return nlohmann::json::parse(backup); }

        void saveJson(const nlohmann::json& json) const
        {
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            auto dump = json.dump();
            out.write(dump.data(), static_cast<std::streamsize>(dump.size()));
        }

        void saveText(std::u8string_view text) const
        {
            std::ofstream out(path, std::ios::binary | std::ios::trunc);
            const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
            out.write(reinterpret_cast<const char*>(bom), sizeof(bom));
            out.write(reinterpret_cast<const char*>(text.data()), static_cast<std::streamsize>(text.size()));
        }

    private:
        std::filesystem::path path;
        std::string backup;
    };

    //マップのイベントへ「名前の変更(320)」を1件追加する。
    //既存の320コマンドの直後に置くことで、
    //「既にActors.csvに有る名前」→「無い名前」の順で抽出されるようにする。
    bool InsertChangeActorNameCommand(nlohmann::json& mapJson, const std::string& newName)
    {
        if(mapJson.contains("events") == false) { return false; }
        for(auto& ev : mapJson["events"])
        {
            if(ev.is_object() == false) { continue; }
            if(ev.contains("pages") == false) { continue; }
            for(auto& page : ev["pages"])
            {
                if(page.contains("list") == false) { continue; }
                auto& list = page["list"];
                for(size_t i = 0; i < list.size(); ++i)
                {
                    if(list[i].value("code", 0) != 320) { continue; }
                    nlohmann::json command;
                    command["code"] = 320;
                    command["indent"] = list[i].value("indent", 0);
                    command["parameters"] = nlohmann::json::array({1, newName});
                    list.insert(list.begin() + i + 1, command);
                    return true;
                }
            }
        }
        return false;
    }

    bool CsvContainsOriginal(const std::filesystem::path& csvPath, const std::u8string& text)
    {
        auto csv = plaincsvreader{csvPath}.getPlainCsvTexts();
        return std::ranges::any_of(csv, [&text](const auto& row) {
            return row.empty() == false && row[0] == text;
        });
    }

    constexpr char8_t MapEventOnlyName[] = u8"マップイベントだけに存在する名前";
}

//-------------------------------------------------------------------
// divisi_mvmz.cpp:350
// マップイベントの「名前の変更」で使われる文字列は Actors.csv へ追記される。
// 抽出した名前の中に既にCSVへ有るものが含まれていても、残りは追記されなければならない。
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_MV, ActorNameFromMapEventIsAppended)
{
    ClearGenerateFiles();
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");

    const auto mapPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest/data/Map001.json";
    ASSERT_TRUE(fs::exists(mapPath)) << mapPath;

    ScopedFilePatch mapPatch(mapPath);
    ASSERT_TRUE(mapPatch.valid());
    {
        auto mapJson = mapPatch.loadJson();
        //Map001のイベントには既存のアクター名(エルーシェ/ラフィーナ)を設定する320が含まれている。
        //その直後にActors.csvへ存在しない名前を追加する。
        ASSERT_TRUE(InsertChangeActorNameCommand(mapJson, utility::cnvStr<std::string>(std::u8string(MapEventOnlyName))));
        mapPatch.saveJson(mapJson);
    }

    langscore::config::detachConfigFile();
    langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json");
    ASSERT_TRUE(divisi.analyze().valid());
    ASSERT_TRUE(divisi.exportCSV().valid());

    langscore::config config;
    std::u8string root;
    const auto exportDirs = config.exportDirectory(root);
    ASSERT_FALSE(exportDirs.empty());

    const auto actorsCsv = fs::path(exportDirs[0]) / "Actors.csv";
    ASSERT_TRUE(fs::exists(actorsCsv)) << actorsCsv;

    EXPECT_TRUE(CsvContainsOriginal(actorsCsv, MapEventOnlyName))
        << "マップイベントで指定された名前が Actors.csv に追記されていません : " << actorsCsv;
}

//-------------------------------------------------------------------
// divisi_mvmz.cpp:712
// 言語別出力(ExportByLang)でも、全ての言語のActors.csvへ同じように追記される。
// 上の不具合を直してもこちらは通らない。jsonreader_mapが揃う前に呼ばれているため。
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_MV, ActorNameFromMapEventIsAppendedForEachLanguage)
{
    ClearGenerateFiles();
    checkAndCreateConfigFile("data/mv/LangscoreTest_MultipleExport_langscore/config.json", "Game.rpgproject");

    const auto mapPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_MultipleExport/data/Map001.json";
    ASSERT_TRUE(fs::exists(mapPath)) << mapPath;

    ScopedFilePatch mapPatch(mapPath);
    ASSERT_TRUE(mapPatch.valid());
    {
        auto mapJson = mapPatch.loadJson();
        ASSERT_TRUE(InsertChangeActorNameCommand(mapJson, utility::cnvStr<std::string>(std::u8string(MapEventOnlyName))));
        mapPatch.saveJson(mapJson);
    }

    langscore::config::detachConfigFile();
    langscore::divisi divisi("./", fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_MultipleExport_langscore/config.json");
    ASSERT_TRUE(divisi.analyze().valid());
    ASSERT_TRUE(divisi.exportCSV().valid());

    langscore::config config;
    std::u8string root;
    const auto exportDirs = config.exportDirectory(root);
    ASSERT_FALSE(exportDirs.empty());
    ASSERT_TRUE(config.exportByLanguage()) << "このテストは言語別出力の設定を前提としています";

    for(const auto& dir : exportDirs)
    {
        const auto actorsCsv = fs::path(dir) / "Actors.csv";
        ASSERT_TRUE(fs::exists(actorsCsv)) << actorsCsv;

        EXPECT_TRUE(CsvContainsOriginal(actorsCsv, MapEventOnlyName))
            << "マップイベントで指定された名前が Actors.csv に追記されていません : " << actorsCsv;
    }
}

//-------------------------------------------------------------------
// csvwriter.cpp:189,214
// 各行のセルはヘッダーの言語順で並べなければならない。
// 行の生成が unordered_map の列挙順に依存しているため、
//  ・ヘッダーと値の対応がずれる
//  ・useLangsにあってtranslatesに無い言語があると列数が足りなくなる
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_CsvWriter, ColumnOrderFollowsHeader)
{
    const auto testDir = fs::path(BINARY_DIRECTORY) / "known_issue_csvwriter";
    std::error_code ec;
    fs::remove_all(testDir, ec);
    fs::create_directories(testDir);

    //2言語だと偶然一致してしまうため、許容されている言語を全て使う。
    const utility::u8stringlist langs = {
        u8"ja", u8"en", u8"zh-cn", u8"zh-tw", u8"ko", u8"ru", u8"fr", u8"de", u8"es", u8"it"
    };

    //各言語の欄に言語コードそのものを入れておくと、ヘッダーとの対応をそのまま検証できる。
    TranslateText text(u8"original text", langs);
    for(const auto& lang : langs) {
        text.translates[lang] = lang;
    }

    const auto path = testDir / "column_order.csv";
    langscore::csvwriter writer{speciftranstext{langs, {text}}};
    ASSERT_EQ(writer.write(path, u8"ja", MergeTextMode::AcceptTarget), Status_Success);

    auto csv = plaincsvreader{path}.getPlainCsvTexts();
    ASSERT_EQ(csv.size(), 2u);

    const auto& header = csv[0];
    const auto& row = csv[1];
    ASSERT_EQ(header.size(), langs.size() + 1);
    EXPECT_EQ(row.size(), header.size()) << "行の列数がヘッダーと一致していません";

    for(size_t i = 1; i < header.size() && i < row.size(); ++i)
    {
        SCOPED_TRACE(::testing::Message() << "column " << i);
        EXPECT_TRUE(header[i] == row[i])
            << "ヘッダー[" << i << "]=" << utility::toString(header[i])
            << " に対して値が " << utility::toString(row[i]) << " になっています";
    }

    //useLangsに含まれる言語がtranslatesに無い場合でも、列数は保たれなければならない。
    {
        TranslateText lackText(u8"lack of language", {u8"ja", u8"en"});
        lackText.translates[u8"ja"] = u8"ja";
        lackText.translates[u8"en"] = u8"en";

        const auto lackPath = testDir / "lack_language.csv";
        langscore::csvwriter lackWriter{speciftranstext{langs, {lackText}}};
        ASSERT_EQ(lackWriter.write(lackPath, u8"ja", MergeTextMode::AcceptTarget), Status_Success);

        auto lackCsv = plaincsvreader{lackPath}.getPlainCsvTexts();
        ASSERT_EQ(lackCsv.size(), 2u);
        EXPECT_EQ(lackCsv[1].size(), lackCsv[0].size())
            << "翻訳の無い言語があると列数がヘッダーと合わなくなります";
    }

    fs::remove_all(testDir, ec);
}

//-------------------------------------------------------------------
// divisi_mvmz.cpp:449
// Validate.ValidateCSVNameList には、検証する CSV ファイルかフォルダのパスを指定する。
// ファイルならそのファイルを、フォルダならその中の CSV を検証しなければならない。
// langscore-app は CSV エディターの検証で、editing フォルダの CSV のフルパスを指定する。
//-------------------------------------------------------------------
namespace
{
    //列数が合わない行を混ぜた CSV。正しく検証されていれば InvalidCSV が報告される。
    constexpr char8_t BrokenActorsCsv[] =
        u8"original,ja,en\n"
        u8"Hello,こんにちは,Hello\n"
        u8"Broken,足りない列\n"
        u8"Tail,末尾,tail\n";

    //ValidateCSVNameList に target を指定して検証し、標準出力を返す。
    //editing フォルダに BrokenActorsCsv を置いてから呼ぶ。
    std::string ValidateWithNameList(const fs::path& configPath, const fs::path& target)
    {
        ScopedFilePatch configPatch(configPath);
        if(configPatch.valid() == false) {
            ADD_FAILURE() << "config.json を読めません : " << configPath;
            return {};
        }
        {
            auto configJson = configPatch.loadJson();
            configJson["Validate"]["ValidateCSVNameList"] = nlohmann::json::array({utility::cnvStr<std::string>(target.u8string())});
            configPatch.saveJson(configJson);
        }

        langscore::config::detachConfigFile();
        langscore::config::attachConfigFile(configPath);

        langscore::divisi divisi("./", configPath);
        ::testing::internal::CaptureStdout();
        const auto status = divisi.validate();
        const auto output = ::testing::internal::GetCapturedStdout();
        EXPECT_TRUE(status.valid());
        return output;
    }

    //editing フォルダを作って壊れた Actors.csv を置き、終了時にフォルダごと消す。
    class ScopedEditingFolder
    {
    public:
        explicit ScopedEditingFolder(fs::path directory)
            : directory(std::move(directory))
        {
            fs::create_directories(this->directory);
            std::ofstream out(this->directory / "Actors.csv", std::ios::binary | std::ios::trunc);
            const std::u8string_view text = BrokenActorsCsv;
            out.write(reinterpret_cast<const char*>(text.data()), static_cast<std::streamsize>(text.size()));
        }

        ~ScopedEditingFolder()
        {
            std::error_code error;
            fs::remove_all(directory, error);
        }

        ScopedEditingFolder(const ScopedEditingFolder&) = delete;
        ScopedEditingFolder& operator=(const ScopedEditingFolder&) = delete;

        const fs::path& path() const { return directory; }

    private:
        fs::path directory;
    };

    //出力された検証結果のうち、指定した CSV に InvalidCSV が報告されているかを確認する。
    //".csv" を付けて別のパスを開いていると、File が一致しない。
    void ExpectInvalidCsvReported(const std::string& output, const fs::path& expectedCsv)
    {
        const auto invalidCsvSummary = static_cast<int>(platform_base::ValidateSummary::InvalidCSV);
        const auto expected = fs::path(expectedCsv).make_preferred().lexically_normal();
        bool found = false;
        std::istringstream lines(output);
        for(std::string line; std::getline(lines, line);)
        {
            if(line.starts_with("{") == false) { continue; }
            const auto json = nlohmann::json::parse(line, nullptr, false);
            if(json.is_discarded() || json.contains("File") == false) { continue; }
            auto file = fs::path(utility::cnvStr<std::u8string>(json["File"].get<std::string>())).make_preferred().lexically_normal();
            if(file == expected && json.value("Summary", -1) == invalidCsvSummary) {
                found = true;
                break;
            }
        }
        EXPECT_TRUE(found) << "指定した CSV で列数の不一致が報告されていません : " << expected << "\n出力:\n" << output;
    }
}

TEST(Langscore_KnownIssue_MV, ValidateCSVNameListFilePath)
{
    ClearGenerateFiles();
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");
    const auto configPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json";

    ScopedEditingFolder editing(configPath.parent_path() / "editing");
    const auto output = ValidateWithNameList(configPath, editing.path() / "Actors.csv");
    ExpectInvalidCsvReported(output, editing.path() / "Actors.csv");
}

TEST(Langscore_KnownIssue_MV, ValidateCSVNameListFolderPath)
{
    ClearGenerateFiles();
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");
    const auto configPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json";

    ScopedEditingFolder editing(configPath.parent_path() / "editing");
    const auto output = ValidateWithNameList(configPath, editing.path());
    ExpectInvalidCsvReported(output, editing.path() / "Actors.csv");
}

namespace
{
    //langscore-app が書き出す形式の config.json を一時フォルダに置き、読み込ませる。
    //終了時に config を外し、フォルダを消す。
    class ScopedConfig
    {
    public:
        explicit ScopedConfig(const nlohmann::json& json)
            : directory(fs::temp_directory_path() / "divisi_known_issue_config")
        {
            fs::create_directories(directory);
            const auto path = directory / "config.json";
            {
                std::ofstream out(path);
                out << json.dump();
            }
            langscore::config::detachConfigFile();
            langscore::config::attachConfigFile(path);
        }

        ~ScopedConfig()
        {
            langscore::config::detachConfigFile();
            std::error_code error;
            fs::remove_all(directory, error);
        }

        ScopedConfig(const ScopedConfig&) = delete;
        ScopedConfig& operator=(const ScopedConfig&) = delete;

    private:
        fs::path directory;
    };

    //プラグインのパラメータから抽出したテキストを作る（javascriptreader::parse と同じ行情報）
    TranslateText MakePluginParameterText(std::u8string value, std::u8string lineInfo)
    {
        TranslateText text{std::move(value), {u8"ja"}};
        text.scriptLineInfo = std::move(lineInfo);
        return text;
    }

    bool ContainsLineInfo(const std::vector<TranslateText>& texts, std::u8string_view lineInfo)
    {
        //applyIgnoreScriptsAndSwapLineInfo の後は original に行情報が入っている
        return std::ranges::any_of(texts, [lineInfo](const auto& text) { return text.original == lineInfo; });
    }
}

//-------------------------------------------------------------------
// divisi_vxace.cpp:517
// VXAce の Graphics.csv には、画像ごとにフォルダ付きのファイル名（拡張子なし）が出力されなければならない。
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_VXAce, GraphicsCsvKeepsFileName)
{
    ClearGenerateFiles();
    const auto configPath = fs::path(BINARY_DIRECTORY) / "data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json";
    checkAndCreateConfigFile("data/vxace/ソポァゼゾタダＡボマミ_langscore/config.json");

    {
        langscore::config::detachConfigFile();
        langscore::divisi divisi("./", configPath);
        ASSERT_TRUE(divisi.analyze().valid());
    }
    {
        //analyze と exportCSV は別のインスタンスで呼ぶ（CheckLangscoreRubyScript と同じ）
        langscore::config::detachConfigFile();
        langscore::divisi divisi("./", configPath);
        ASSERT_TRUE(divisi.exportCSV().valid());
    }

    langscore::config config;
    std::u8string root;
    const auto exportDirs = config.exportDirectory(root);
    ASSERT_FALSE(exportDirs.empty());
    const auto graphicsCsv = fs::path(exportDirs[0]) / "Graphics.csv";
    ASSERT_TRUE(fs::exists(graphicsCsv)) << graphicsCsv;

    //テストデータには Graphics/Pictures/Actor1-1.png がある
    EXPECT_TRUE(CsvContainsOriginal(graphicsCsv, u8"Graphics/Pictures/Actor1-1"))
        << "画像のファイル名が Graphics.csv に出力されていません : " << graphicsCsv;
}

//-------------------------------------------------------------------
// config.cpp:442
// langscore-app はプラグインのパラメータを無視するとき、IgnorePoints に ParameterText を書く。
// そのパラメータのテキストは Scripts.csv から除外されなければならない。
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_Script, IgnorePluginParameter)
{
    //langscore-app の settings_io.cpp serializeScripts と同じ形式
    ScopedConfig scopedConfig(nlohmann::json::parse(R"({"Write": {"RPGMakerScripts": [
        {"Name": "Foo.js", "Ignore": false, "IgnorePoints": [{"ParameterText": "Message", "Disable": false}]}
    ]}})"));
    langscore::config config;

    javascriptreader reader(u8"ja", {u8"ja"}, "", {});
    auto& texts = reader.currentTexts();
    texts.emplace_back(MakePluginParameterText(u8"こんにちは", u8"Foo:Message"));
    texts.emplace_back(MakePluginParameterText(u8"さようなら", u8"Foo:Other"));

    reader.applyIgnoreScriptsAndSwapLineInfo(config.rpgMakerScripts());

    EXPECT_FALSE(ContainsLineInfo(reader.currentTexts(), u8"Foo:Message")) << "無視したパラメータが残っています";
    EXPECT_TRUE(ContainsLineInfo(reader.currentTexts(), u8"Foo:Other")) << "無視していないパラメータが消えています";
}

//-------------------------------------------------------------------
// javascriptreader.hpp:85
// プラグイン Foo を無視しても、名前が Foo で始まる別のプラグイン FooBar のテキストは残らなければならない。
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_Script, IgnorePluginDoesNotAffectPrefixNamedPlugin)
{
    ScopedConfig scopedConfig(nlohmann::json::parse(R"({"Write": {"RPGMakerScripts": [
        {"Name": "Foo.js", "Ignore": true, "IgnorePoints": []}
    ]}})"));
    langscore::config config;

    javascriptreader reader(u8"ja", {u8"ja"}, "", {});
    auto& texts = reader.currentTexts();
    texts.emplace_back(MakePluginParameterText(u8"こんにちは", u8"Foo:Message"));
    texts.emplace_back(MakePluginParameterText(u8"さようなら", u8"FooBar:Message"));

    reader.applyIgnoreScriptsAndSwapLineInfo(config.rpgMakerScripts());

    EXPECT_FALSE(ContainsLineInfo(reader.currentTexts(), u8"Foo:Message")) << "無視したプラグインのテキストが残っています";
    EXPECT_TRUE(ContainsLineInfo(reader.currentTexts(), u8"FooBar:Message")) << "別のプラグインのテキストが消えています";
}

//-------------------------------------------------------------------
// config.cpp:164
// config.json の文字列は UTF-8。std::string のまま std::filesystem::path にすると
// Windows では ANSI コードページとして解釈され、日本語のフォントパスが化ける。
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_Config, FontPathKeepsNonAsciiCharacters)
{
    nlohmann::json json;
    json["Languages"] = nlohmann::json::array({
        {
            {"LanguageName", "ja"},
            {"Enable", true},
            {"FontName", "Test Font"},
            {"FontPath", utility::cnvStr<std::string>(std::u8string(u8"Fonts/日本語フォント.ttf"))},
            {"FontSize", 22}
        }
    });
    ScopedConfig scopedConfig(json);
    langscore::config config;

    const auto languages = config.allLanguages();
    ASSERT_EQ(languages.size(), 1u);
    EXPECT_EQ(languages[0].font.file.generic_u8string(), std::u8string(u8"Fonts/日本語フォント.ttf"))
        << "フォントのパスが変わっています";
}

//-------------------------------------------------------------------
// MV/MZ の再解析（プロジェクトの更新）
// テストの間だけゲームプロジェクトにファイルやフォルダを足し、終了時に消す。
//-------------------------------------------------------------------
namespace
{
    constexpr char MvReanalysisConfig[] = "data/mv/LangscoreTest_langscore/config.json";

    class ScopedGamePath
    {
    public:
        explicit ScopedGamePath(fs::path path)
            : path(std::move(path))
        {}

        ~ScopedGamePath()
        {
            std::error_code error;
            fs::remove_all(path, error);
        }

        ScopedGamePath(const ScopedGamePath&) = delete;
        ScopedGamePath& operator=(const ScopedGamePath&) = delete;

    private:
        fs::path path;
    };

    //ファイルを読み取り専用にし、終了時に書き込みできる状態へ戻す
    class ScopedReadOnly
    {
    public:
        explicit ScopedReadOnly(fs::path path)
            : path(std::move(path))
        {
            fs::permissions(this->path, fs::perms::owner_write | fs::perms::group_write | fs::perms::others_write, fs::perm_options::remove);
        }

        ~ScopedReadOnly()
        {
            std::error_code error;
            fs::permissions(path, fs::perms::owner_write, fs::perm_options::add, error);
        }

        ScopedReadOnly(const ScopedReadOnly&) = delete;
        ScopedReadOnly& operator=(const ScopedReadOnly&) = delete;

    private:
        fs::path path;
    };

    fs::path MvGameDirectory()
    {
        return fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest";
    }

    fs::path MvConfigPath()
    {
        return fs::path(BINARY_DIRECTORY) / MvReanalysisConfig;
    }

    //前回の解析結果を消してから解析する
    void AnalyzeMvProject()
    {
        ClearGenerateFiles();
        checkAndCreateConfigFile(MvReanalysisConfig, "Game.rpgproject");
        langscore::config::detachConfigFile();
        langscore::config::attachConfigFile(MvConfigPath());
        {
            langscore::config config;
            ClearGenerateFiles(config);
        }
        langscore::config::detachConfigFile();
        langscore::divisi divisi("./", MvConfigPath());
        ASSERT_TRUE(divisi.analyze().valid());
    }

    //再解析する。例外が出た場合はテストの失敗として報告する（divisi.exe では abort になる）
    void ReanalyzeMvProject()
    {
        langscore::config::detachConfigFile();
        langscore::divisi divisi("./", MvConfigPath());
        try {
            EXPECT_TRUE(divisi.reanalysis().valid());
        }
        catch(const std::exception& e) {
            ADD_FAILURE() << "再解析で例外が出ました（divisi.exe では abort になります）: " << e.what();
        }
    }

    fs::path MvAnalyzeDirectory()
    {
        langscore::config::detachConfigFile();
        langscore::config::attachConfigFile(MvConfigPath());
        langscore::config config;
        return fs::path(config.langscoreAnalyzeDirectorty());
    }
}

//-------------------------------------------------------------------
// divisi_mvmz.cpp:250
// js/plugins にサブフォルダがあっても、再解析は失敗せずに終わらなければならない。
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_MV_Reanalysis, PluginSubFolder)
{
    const auto subFolder = MvGameDirectory() / "js/plugins/SubFolder";
    ScopedGamePath scopedSubFolder(subFolder);

    AnalyzeMvProject();

    fs::create_directories(subFolder);
    {
        std::ofstream out(subFolder / "Inner.js");
        out << "var text = 'inner';";
    }
    ReanalyzeMvProject();
}

//-------------------------------------------------------------------
// divisi_mvmz.cpp:239
// ゲームのデータが読み取り専用でも、2回目以降の再解析は失敗せずに終わらなければならない。
// 解析で読み取り専用の属性ごとコピーされた解析ファイルを、再解析で上書きできないことが原因。
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_MV_Reanalysis, ReadOnlyGameData)
{
    const auto actorsJson = MvGameDirectory() / "data/Actors.json";
    ASSERT_TRUE(fs::exists(actorsJson)) << actorsJson;
    ScopedReadOnly readOnly(actorsJson);

    AnalyzeMvProject();
    ReanalyzeMvProject();

    //解析フォルダにコピーされた読み取り専用のファイルを、後片付けで消せるようにする
    std::error_code error;
    fs::permissions(MvAnalyzeDirectory() / "Actors.json", fs::perms::owner_write, fs::perm_options::add, error);
}

//-------------------------------------------------------------------
// divisi_mvmz.cpp:247
// ゲームから削除したプラグインは、再解析で解析フォルダからも消えなければならない。
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_MV_Reanalysis, RemovedPluginIsRemovedFromAnalyze)
{
    const auto extraPlugin = MvGameDirectory() / "js/plugins/Extra.js";
    ScopedGamePath scopedPlugin(extraPlugin);
    {
        std::ofstream out(extraPlugin);
        out << "var text = 'extra';";
    }

    AnalyzeMvProject();
    const auto analyzedPlugin = MvAnalyzeDirectory() / "Scripts/Extra.js";
    ASSERT_TRUE(fs::exists(analyzedPlugin)) << "解析フォルダにプラグインがコピーされていません: " << analyzedPlugin;

    fs::remove(extraPlugin);
    ReanalyzeMvProject();

    EXPECT_FALSE(fs::exists(analyzedPlugin)) << "削除したプラグインが解析フォルダに残っています: " << analyzedPlugin;
}
