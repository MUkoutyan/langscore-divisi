# テスト

```
python run_test.py all      # 全スイート (lscsv / rvcnv / vxace / MV / MZ / C++ / divisi_ct)
python run_test.py cpp      # 個別 (cpp, lscsv, rvcnv, vxace, mv, mz, divisi_ct)
```

結果は `test_log.md` に書き出されます。失敗時は終了コード 1 を返します。

| スイート | 対象 | 必要なもの |
|---------|------|-----------|
| cpp | divisi 本体 (gtest) | Visual Studio、CMake、Ninja、テストデータ |
| divisi_ct | divisi.exe のコマンドライン | `bin/divisi.exe`、テストデータ |
| rvcnv | rvcnv (VXAce の rvdata2 変換) | Ruby、テストデータ |
| mv / mz | Langscore.js (mocha + jsdom) | Node.js、テストデータ |
| vxace | langscore.rb (RGSS3 実機) | Ruby、テストデータ、GUI セッション |
| lscsv | lscsv.{js,rb,py} の CSV パーサ | Node.js / Ruby / Python |

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
    plugin/
      mv_test/  mv_test_langscore/
      mz_test/  mz_test_langscore/
      vxace_test/  vxace_test_langscore/
```

別の場所に置く場合は環境変数 `LANGSCORE_TEST_DATA` でフォルダを指定してください。
見つからない場合は、各スクリプトが配置場所を示して停止します。

`compress_data.ps1` でテストデータ一式を zip とその SHA256 にまとめられます。
別マシンへ持っていく場合やバックアップに使ってください。

## 既知の不具合のテスト

`test_known_issues.cpp` の `Langscore_KnownIssue_*` は、**報告済みで未修正の不具合によって失敗します。**
あるべき動作を書いてあるので、不具合が直れば通ります。

| テスト | 対象 |
|--------|------|
| `Langscore_KnownIssue_MV.ActorNameFromMapEventIsAppended` | `divisi_mvmz.cpp:350` 既存の名前1件で処理を打ち切っている |
| `Langscore_KnownIssue_MV.ActorNameFromMapEventIsAppendedForEachLanguage` | `divisi_mvmz.cpp:712` リーダーが揃う前に呼んでいる |
| `Langscore_KnownIssue_CsvWriter.ColumnOrderFollowsHeader` | `csvwriter.cpp:189` 行のセルを unordered_map の列挙順で並べている |
| `Langscore_KnownIssue_MV.ValidateCSVNameListSelectsRealFile` | `divisi_mvmz.cpp:438` 検証対象の指定がファイル名として扱われていない |

これらを除いて実行する場合:

```
build/Test_Debug/divisi_test.exe --gtest_filter=-Langscore_KnownIssue_*
```

ゲームプロジェクトのJSONやCSVを書き換えるテストが含まれますが、
`ScopedFilePatch` が終了時に必ず元へ戻します。

## 補足

- `test_config.h` と `test_log.md` はビルド・実行時に生成されるため git 管理外です。
- C++ テストは初回実行時にテストデータをビルドディレクトリへコピーします。
  `build/Test_Debug` を消すとコピーからやり直しになります。
