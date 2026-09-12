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
//   Langscore_KnownIssue_MV.ValidateCSVNameListSelectsRealFile
//       divisi_mvmz.cpp:438 ValidateCSVNameList をファイル名ではなくパスとして扱っている
//-------------------------------------------------------------------

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
// divisi_mvmz.cpp:438
// Validate.ValidateCSVNameList はファイル名の指定。
// 指定した場合、そのCSVが実際に検証されなければならない。
//-------------------------------------------------------------------
TEST(Langscore_KnownIssue_MV, ValidateCSVNameListSelectsRealFile)
{
    ClearGenerateFiles();
    checkAndCreateConfigFile("data/mv/LangscoreTest_langscore/config.json", "Game.rpgproject");

    const auto configPath = fs::path(BINARY_DIRECTORY) / "data/mv/LangscoreTest_langscore/config.json";

    //まずCSVを出力する。
    {
        langscore::config::detachConfigFile();
        langscore::divisi divisi("./", configPath);
        ASSERT_TRUE(divisi.analyze().valid());
        ASSERT_TRUE(divisi.exportCSV().valid());
    }

    fs::path actorsCsv;
    {
        langscore::config config;
        std::u8string root;
        const auto exportDirs = config.exportDirectory(root);
        ASSERT_FALSE(exportDirs.empty());
        actorsCsv = fs::path(exportDirs[0]) / "Actors.csv";
    }
    ASSERT_TRUE(fs::exists(actorsCsv)) << actorsCsv;

    ScopedFilePatch configPatch(configPath);
    ScopedFilePatch csvPatch(actorsCsv);
    ASSERT_TRUE(configPatch.valid());
    ASSERT_TRUE(csvPatch.valid());

    //Actors.csvだけを検証対象に指定する。
    {
        auto configJson = configPatch.loadJson();
        configJson["Validate"]["ValidateCSVNameList"] = nlohmann::json::array({"Actors"});
        configPatch.saveJson(configJson);
    }

    //列数が合わない行を混ぜる。正しく検証されていれば InvalidCSV が報告される。
    csvPatch.saveText(
        u8"original,ja,en\n"
        u8"Hello,こんにちは,Hello\n"
        u8"Broken,足りない列\n"
        u8"Tail,末尾,tail\n"
    );

    langscore::config::detachConfigFile();
    langscore::config::attachConfigFile(configPath);

    langscore::divisi divisi("./", configPath);
    ::testing::internal::CaptureStdout();
    const auto status = divisi.validate();
    const auto output = ::testing::internal::GetCapturedStdout();

    EXPECT_TRUE(status.valid());

    //出力されたJSONのうち、Actors.csvを対象としたものがあるかを確認する。
    //ValidateCSVNameListがパスとして扱われていると、拡張子の無い "Actors" が報告される。
    EXPECT_NE(output.find("Actors.csv"), std::string::npos)
        << "ValidateCSVNameList で指定した Actors.csv が検証されていません。出力:\n" << output;

    //列数が合わない行があるため、InvalidCSV (ValidateSummary の 8) が報告されるはず。
    const auto invalidCsvSummary = "\"Summary\":" + std::to_string(static_cast<int>(platform_base::ValidateSummary::InvalidCSV));
    EXPECT_NE(output.find(invalidCsvSummary), std::string::npos)
        << "列数が一致しない行が検出されていません。出力:\n" << output;
}
