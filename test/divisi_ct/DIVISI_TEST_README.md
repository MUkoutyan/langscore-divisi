# divisi_ct

`bin/divisi.exe` を起動して、divisi.exe でしか確認できない動作をテストします。テストデータは使いません。

| テストクラス | 内容 |
|------|------|
| `TestErrorHandling` | 存在しない設定ファイル・引数なし・不明なオプションでエラー終了すること、ANSI コードページで表せない文字を含むパスの config.json を読めること、存在しないフォルダを `--convertPatchCSV` / `--convertBoundCSV` に渡したときに abort せず -1 で終わること |
| `TestVersionInfo` | 起動時にバージョン情報を表示すること |

各コマンド（`--createConfigFile` / `--analyze` / `--reanalysis` / `--updatePlugin` / `--exportCSV` / `--validate` / `--packing`）の処理は、gtest の `Langscore_Divisi_Command`（`test/test_commands.cpp`）で `langscore::divisi` を直接呼んで確認します。CSV 変換は `test/test_core.cpp` の `ConvertPatchCsv_*` / `ConvertBoundCsv_*` で確認します。gtest は Visual Studio からそのままデバッグ実行できます。

## 実行方法

先に divisi をビルドして `bin/divisi.exe` を用意してから実行します。

```bash
python run_divisi_test.py
```
