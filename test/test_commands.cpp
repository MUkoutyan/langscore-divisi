#include <functional>
//-------------------------------------------------------------------
// divisi.exe の各コマンドを、ゲームプロジェクト単位で通して確認するテスト。
// もとは divisi_ct (run_divisi_test.py) が divisi.exe を起動して確認していたもの。
// divisi.exe は langscore::divisi を呼ぶだけなので、ここでは langscore::divisi を直接呼ぶ。
// 引数の解釈や終了コードのように、divisi.exe でしか確認できないものは divisi_ct に残している。
//
// テストごとに、テストデータのゲームプロジェクトを command_work へコピーして使う。
//-------------------------------------------------------------------
namespace
{
    struct CommandProject
    {
        //テスト名に使う
        const char* name;
        //テストデータ（data フォルダ）からの相対パス
        const char* source;
        //書き出した CSV に含まれる文章の最小数
        size_t minimumTextCount;
    };

    //失敗したときに、パラメーターをテスト名で表示する
    void PrintTo(const CommandProject& project, std::ostream* stream)
    {
        *stream << project.name;
    }

    const std::vector<std::string> AnalyzeExpectedFiles = {
        "System.lsjson", "Troops.lsjson", "Weapons.lsjson", "Actors.lsjson",
        "Armors.lsjson", "Classes.lsjson", "CommonEvents.lsjson",
        "Enemies.lsjson", "Items.lsjson", "Map001.lsjson", "Map002.lsjson", "Map003.lsjson",
        "Skills.lsjson", "States.lsjson"
    };

    const std::vector<std::string> WriteExpectedFiles = {
        "System.csv", "Troops.csv", "Weapons.csv", "Actors.csv",
        "Armors.csv", "Classes.csv", "CommonEvents.csv",
        "Enemies.csv", "Items.csv", "Map001.csv", "Map002.csv", "Map003.csv",
        "Skills.csv", "States.csv"
    };

    //コピーしたテストデータの読み取り専用を外す
    void RemoveReadOnlyFiles(const fs::path& directory)
    {
        for(const auto& entry : fs::recursive_directory_iterator{directory})
        {
            if(entry.is_directory()) { continue; }
            fs::permissions(entry.path(), fs::perms::owner_write, fs::perm_options::add);
        }
    }

    void CopyProject(const fs::path& source, const fs::path& destination)
    {
        DeleteGeneratedPath(destination);
        fs::create_directories(destination.parent_path());
        fs::copy(source, destination, fs::copy_options::recursive);
        RemoveReadOnlyFiles(destination);
    }

    nlohmann::json ReadJson(const fs::path& path)
    {
        nlohmann::json json;
        std::ifstream file(path);
        file >> json;
        return json;
    }

    void EditConfig(const fs::path& configPath, const std::function<void(nlohmann::json&)>& edit)
    {
        auto json = ReadJson(configPath);
        edit(json);
        std::ofstream file(configPath);
        file << json.dump(4);
    }

    //CSV の1行目（ヘッダー）を BOM を除いて返す
    std::string ReadCsvHeader(const fs::path& path)
    {
        std::ifstream file(path);
        std::string header;
        std::getline(file, header);
        constexpr std::string_view Bom = "\xEF\xBB\xBF";
        if(header.starts_with(Bom)) {
            header.erase(0, Bom.size());
        }
        return header;
    }

    size_t CountCsvTexts(const fs::path& path)
    {
        langscore::csvreader reader{path};
        const auto& texts = reader.currentTexts();
        return std::ranges::count_if(texts, [](const auto& text) { return text.original.empty() == false; });
    }
}

class Langscore_Divisi_Command : public ::testing::TestWithParam<CommandProject>
{
protected:
    fs::path workDirectory;
    fs::path sourcePath;
    fs::path gamePath;
    fs::path langscorePath;
    fs::path configPath;

    void SetUp() override
    {
        const auto& project = GetParam();
        workDirectory = fs::path(BINARY_DIRECTORY) / "command_work";
        sourcePath = fs::path(BINARY_DIRECTORY) / "data" / project.source;
        gamePath = workDirectory / sourcePath.filename();
        langscorePath = gamePath;
        langscorePath += "_langscore";
        configPath = langscorePath / "config.json";

        ASSERT_TRUE(fs::exists(sourcePath)) << sourcePath;
        DeleteGeneratedPath(workDirectory);
        CopyProject(sourcePath, gamePath);
    }

    void TearDown() override
    {
        langscore::config::detachConfigFile();
        DeleteGeneratedPath(workDirectory);
    }

    //divisi.exe の -c <config> --<command> と同じ処理を行う
    ErrorStatus Run(ErrorStatus (langscore::divisi::*command)())
    {
        //config は読み込んだ内容を保持するため、書き換えた内容を読ませるために外してから作る
        langscore::config::detachConfigFile();
        langscore::divisi divisi("./", configPath);
        return (divisi.*command)();
    }

    //divisi.exe の --createConfigFile と同じ処理を行い、日本語を既定、英語を有効にする
    void CreateConfig()
    {
        langscore::config::detachConfigFile();
        langscore::divisi divisi("./", "");
        divisi.createConfig(gamePath);
        ASSERT_TRUE(fs::exists(configPath)) << configPath;

        EditConfig(configPath, [](nlohmann::json& json)
        {
            json["DefaultLanguage"] = "ja";
            for(auto& language : json["Languages"])
            {
                if(language["LanguageName"] == "en") {
                    language["Enable"] = true;
                }
            }
        });
    }

    void ExpectValid(ErrorStatus result, const char* command)
    {
        EXPECT_TRUE(result.valid()) << command << " : " << result.toStr();
    }

    //設定の作成・解析・CSV の書き出しまでを行う
    void PrepareExportedProject()
    {
        ASSERT_NO_FATAL_FAILURE(CreateConfig());
        ASSERT_TRUE(Run(&langscore::divisi::analyze).valid());
        ASSERT_TRUE(Run(&langscore::divisi::exportCSV).valid());
    }

    fs::path TranslateDirectory() const
    {
        return langscorePath / "data" / "translate";
    }
};

TEST_P(Langscore_Divisi_Command, CreateConfig)
{
    ASSERT_FALSE(fs::exists(langscorePath)) << langscorePath;
    ASSERT_NO_FATAL_FAILURE(CreateConfig());

    const auto json = ReadJson(configPath);
    for(const auto& field : {"Project", "DefaultLanguage", "Languages", "Analyze", "Write"}) {
        EXPECT_TRUE(json.contains(field)) << field;
    }

    //Project はゲームプロジェクトのフォルダを指す
    const auto project = fs::path(utility::cnvStr<std::u8string>(json.value("Project", ""s)));
    EXPECT_TRUE(fs::equivalent(project, gamePath)) << project << " / " << gamePath;

    const auto& languages = json["Languages"];
    ASSERT_TRUE(languages.is_array());
    EXPECT_FALSE(languages.empty());
    for(const auto& language : languages)
    {
        EXPECT_TRUE(language.contains("LanguageName"));
        EXPECT_TRUE(language.contains("Enable"));
    }
}

TEST_P(Langscore_Divisi_Command, Analyze)
{
    ASSERT_NO_FATAL_FAILURE(CreateConfig());
    ExpectValid(Run(&langscore::divisi::analyze), "analyze");

    const auto analyzeDirectory = langscorePath / "analyze";
    for(const auto& file : AnalyzeExpectedFiles)
    {
        SCOPED_TRACE(file);
        const auto path = analyzeDirectory / file;
        ASSERT_TRUE(fs::exists(path)) << path;
        EXPECT_LT(0u, fs::file_size(path));

        const auto json = ReadJson(path);
        ASSERT_TRUE(json.is_array());
        if(json.empty()) { continue; }
        EXPECT_TRUE(json[0].contains("original"));
    }
}

TEST_P(Langscore_Divisi_Command, ExportCSV)
{
    ASSERT_NO_FATAL_FAILURE(CreateConfig());
    ASSERT_TRUE(Run(&langscore::divisi::analyze).valid());
    ExpectValid(Run(&langscore::divisi::exportCSV), "exportCSV");

    const auto translateDirectory = TranslateDirectory();
    ASSERT_TRUE(fs::exists(translateDirectory)) << translateDirectory;

    std::vector<std::uintmax_t> firstSizes;
    size_t totalTextCount = 0;
    for(const auto& file : WriteExpectedFiles)
    {
        SCOPED_TRACE(file);
        const auto path = translateDirectory / file;
        ASSERT_TRUE(fs::exists(path)) << path;
        firstSizes.emplace_back(fs::file_size(path));

        //ヘッダーは original + 有効な言語名（例: original,ja,en）
        const auto header = ReadCsvHeader(path);
        EXPECT_TRUE(header.starts_with("original")) << header;
        EXPECT_NE(header.find(",ja"), std::string::npos) << header;

        totalTextCount += CountCsvTexts(path);
    }
    EXPECT_LE(GetParam().minimumTextCount, totalTextCount);

    //2回目の書き出しで内容が変わらない
    ExpectValid(Run(&langscore::divisi::exportCSV), "exportCSV (2nd)");
    for(size_t i = 0; i < WriteExpectedFiles.size(); ++i)
    {
        const auto path = translateDirectory / WriteExpectedFiles[i];
        EXPECT_EQ(firstSizes[i], fs::file_size(path)) << path;
    }
}

TEST_P(Langscore_Divisi_Command, Reanalysis)
{
    ASSERT_NO_FATAL_FAILURE(PrepareExportedProject());

    //ゲームプロジェクトを、文章を変更した版に置き換えてから再解析する
    auto updatedSourcePath = sourcePath;
    updatedSourcePath += "_updated";
    ASSERT_TRUE(fs::exists(updatedSourcePath)) << updatedSourcePath;
    CopyProject(updatedSourcePath, gamePath);

    ExpectValid(Run(&langscore::divisi::reanalysis), "reanalysis");
    ASSERT_TRUE(fs::exists(langscorePath / "update"));

    for(const auto& file : WriteExpectedFiles) {
        EXPECT_TRUE(fs::exists(TranslateDirectory() / file)) << file;
    }

    const std::vector<std::u8string> expectedTexts = {
        u8"通常のテキストです2",
        u8"改行を含む\nテキストです2",
        u8"\"タ\"フ\"ルクォーテーションを含むテキストです2\"",
        u8"\"\"\"Hello, World\"\",\nそれはプログラムを書く際の\",\"\"\"謎の呪文\"\"(Mystery spell)―――2\"",
        u8"複合させます\n\\{\\C[2]\\N[2]\\I[22]\n2",
    };
    const auto updatedMap = langscorePath / "update" / "Map001.lsjson";
    ASSERT_TRUE(fs::exists(updatedMap)) << updatedMap;
    const auto json = ReadJson(updatedMap);
    for(const auto& expected : expectedTexts)
    {
        const auto text = utility::cnvStr<std::string>(expected);
        const bool found = std::ranges::any_of(json, [&text](const auto& item)
        {
            return item.contains("original") && item["original"].template get<std::string>().find(text) != std::string::npos;
        });
        EXPECT_TRUE(found) << "更新後の Map001.lsjson にありません: " << text;
    }
}

TEST_P(Langscore_Divisi_Command, UpdatePlugin)
{
    ASSERT_NO_FATAL_FAILURE(CreateConfig());
    ASSERT_TRUE(Run(&langscore::divisi::analyze).valid());
    ExpectValid(Run(&langscore::divisi::updatePlugin), "updatePlugin");

    //VXAce のスクリプトは Scripts.rvdata2 に書き込まれるため、ファイルとしては確認しない
    if(fs::exists(gamePath / "js" / "plugins") == false) { return; }
    for(const auto& file : {"Langscore.js", "Langscore_ObserverBridge.js"})
    {
        const auto path = gamePath / "js" / "plugins" / file;
        ASSERT_TRUE(fs::exists(path)) << path;
        std::ifstream stream(path);
        const std::string content{std::istreambuf_iterator<char>(stream), std::istreambuf_iterator<char>()};
        EXPECT_NE(content.find("Langscore"), std::string::npos) << path;
    }
}

TEST_P(Langscore_Divisi_Command, Validate)
{
    ASSERT_NO_FATAL_FAILURE(PrepareExportedProject());
    //--validate / --packing は PackingInputDir 以下の CSV を読むため、書き出し先を設定する
    const auto translateDirectory = utility::cnvStr<std::string>(TranslateDirectory().generic_u8string());
    EditConfig(configPath, [&translateDirectory](nlohmann::json& json) { json["PackingInputDir"] = translateDirectory; });

    ExpectValid(Run(&langscore::divisi::validate), "validate");
}

TEST_P(Langscore_Divisi_Command, Packing)
{
    ASSERT_NO_FATAL_FAILURE(PrepareExportedProject());
    const auto translateDirectory = utility::cnvStr<std::string>(TranslateDirectory().generic_u8string());
    EditConfig(configPath, [&translateDirectory](nlohmann::json& json) { json["PackingInputDir"] = translateDirectory; });

    ExpectValid(Run(&langscore::divisi::packing), "packing");
}

INSTANTIATE_TEST_SUITE_P(Projects, Langscore_Divisi_Command,
    ::testing::Values(
        CommandProject{"MV", "LangscoreTestMV", 11},
        CommandProject{"MZ", "LangscoreTestMZ", 11},
        CommandProject{"VXAce", "LangscoreTestVXAce", 6},
        CommandProject{"VXAceWhiteSpacePath", "vxace/Include WhiteSpacePath Project", 0}
    ),
    [](const ::testing::TestParamInfo<CommandProject>& info) { return std::string(info.param.name); }
);
