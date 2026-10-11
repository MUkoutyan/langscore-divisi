# テスト

テストは全て CTest に登録されています。C++ (gtest)、Ruby、JavaScript (mocha)、Python が
同じ 1 つのテストリストに並びます。

```
cmake --preset x64-debug-test        # 構成 (DIVISI_BUILD_TESTS=ON)
cmake --build --preset x64-debug-test
ctest --preset x64-debug-test        # 全テスト
```

VSCode / Visual Studio からは、プリセットを選べば IDE のテストエクスプローラに一覧が出ます。

Markdown のレポートが必要な場合は、上記をまとめて実行するラッパーを使います。
スクリプトは**リポジトリのルート**にあり、ルートから実行します。

```
python run_test.py all       # 構成 → ビルド → ctest → test/test_log.md
python run_test.py cpp       # 個別 (cpp, lscsv, rvcnv, vxace, mv, mz, plugin, divisi)
python run_test.py --preset x64-release-test all
```

結果は `test/test_log.md` に書き出されます。失敗時は終了コード 1 を返します。
CI 等で JUnit XML がそのまま欲しい場合は `ctest --output-junit <path>` を使ってください。

| テスト名 | 対象 | 必要なもの |
|---------|------|-----------|
| `cpp` | divisi 本体 (gtest) | Visual Studio、CMake、Ninja、テストデータ |
| `divisi_ct` | divisi.exe のコマンドライン | `bin/divisi.exe`、Python、テストデータ |
| `rvcnv` | rvcnv (VXAce の rvdata2 変換) | Python、Ruby、テストデータ |
| `plugin_mv_*` / `plugin_mz_*` | Langscore.js (mocha + jsdom) | Node.js、テストデータ |
| `vxace_*` | langscore.rb (RGSS3 実機) | Python、テストデータ、GUI セッション |
| `lscsv_js` / `lscsv_rb` / `lscsv_py` | lscsv.{js,rb,py} の CSV パーサ | Node.js / Ruby / Python |

処理系が見つからない言語のテストは登録されません。テストデータが無い場合は
`lscsv_js` / `lscsv_py` だけが登録され、構成自体は成功します。

`plugin_sync` は mocha テストの前処理 (`sync_plugin.js`) で、CTest のフィクスチャとして
`plugin_mv_*` / `plugin_mz_*` の実行前に自動で走ります。個別に呼ぶ必要はありません。

`vxace_*` の 3 つは同じゲームプロジェクトの `Game.ini` を書き換えるため、
`RESOURCE_LOCK` で直列に実行されます。

Visual Studio は vswhere で自動検出するため、開発者コマンドプロンプトである必要はありません。

vxace はテストデータ内の `Game.exe` (RGSS3) を実際に起動して検証するため、GUI セッションが必要です。
詳細は [plugin/README.md](plugin/README.md) を参照してください。

## テストデータ

RPGツクールのゲームプロジェクトを含み、サイズと再配布の都合でリポジトリには含めません。
**リポジトリと同階層**の `langscore-divisi-test-data` を参照します。

```
Github/
  langscore-divisi/              <- このリポジトリ
  langscore-divisi-test-data/
    data/                        <- cpp / divisi_ct / rvcnv 用
    lscsv/                       <- lscsv_rb 用 (Skills.csv / CommonEvents.csv)
    plugin/
      mv_test/  mv_test_langscore/
      mz_test/  mz_test_langscore/
      vxace_test/  vxace_test_langscore/
```

別の場所に置く場合は環境変数 `LANGSCORE_TEST_DATA` でフォルダを指定してください
(`-DTEST_DATA_SRC` で `data` ディレクトリを直接指定することもできます)。
CMake が解決した場所は各テストに `LANGSCORE_TEST_DATA` として渡されるため、
CMake 側とランナースクリプト側で参照先がずれません。

`compress_data.ps1` でテストデータ一式を zip とその SHA256 にまとめられます。
別マシンへ持っていく場合やバックアップに使ってください。

`resource/` のテンプレートや翻訳CSVの出力内容を変えてフィクスチャ側を追従させたい場合は
`python test/prepare_test.py` を使います (divisi で各テストプロジェクトを作り直します)。
テストデータを直接書き換えて元に戻せないため、通常のテスト実行では不要です。

## 既知の不具合のテスト

`test_known_issues.cpp` の `Langscore_KnownIssue_*` は、報告済みの不具合に対して
**あるべき動作**を書いたテストです。不具合が残っている間は失敗し、直れば通ります。

| テスト | 対象 |
|--------|------|
| `Langscore_KnownIssue_MV.ActorNameFromMapEventIsAppended` | `divisi_mvmz.cpp:350` 既存の名前1件で処理を打ち切っている |
| `Langscore_KnownIssue_MV.ActorNameFromMapEventIsAppendedForEachLanguage` | `divisi_mvmz.cpp:712` リーダーが揃う前に呼んでいる |
| `Langscore_KnownIssue_CsvWriter.ColumnOrderFollowsHeader` | `csvwriter.cpp:189` 行のセルを unordered_map の列挙順で並べている |
| `Langscore_KnownIssue_MV.ValidateCSVNameListSelectsRealFile` | `divisi_mvmz.cpp:438` 検証対象の指定がファイル名として扱われていない |

これらを除いて実行する場合:

```
build/x64-debug-test/divisi_test.exe --gtest_filter=-Langscore_KnownIssue_*
```

ゲームプロジェクトのJSONやCSVを書き換えるテストが含まれますが、
`ScopedFilePatch` が終了時に必ず元へ戻します。

## 補足

- `test_config.h` と `test_log.md` はビルド・実行時に生成されるため git 管理外です。
- C++ テストは初回実行時にテストデータをビルドディレクトリへコピーします。
  `build/x64-debug-test` を消すとコピーからやり直しになります。
