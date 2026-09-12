# Langscore プラグインテスト

`resource/` にあるプラグインをゲームプロジェクトに組み込んで動作確認する結合テストです。

| 対象 | テスト | 実行 | 必要なもの |
|------|--------|------|-----------|
| Langscore.js (MV/MZ) | `Langscore_mvmz.test.js` (mocha + jsdom) | `npm test` | Node.js、`mv_test/` `mz_test/` |
| langscore.rb (VXAce) | `Langscore_vxace_test.rb` (test/unit) | `run_vxace_test.ps1` | Ruby 1.9.2 (RGSS3 相当)、`vxace_test/` |

## 準備

1. テストデータ (RPGツクールのゲームプロジェクト) を配置する。
   サイズと再配布の都合でリポジトリには含めず、**リポジトリと同階層**に置きます。

   ```
   Github/
     langscore-divisi/              <- このリポジトリ
     langscore-divisi-test-data/
       data/                        <- C++ / divisi_ct / rvcnv 用
       plugin/
         mv_test/  mv_test_langscore/
         mz_test/  mz_test_langscore/
         vxace_test/  vxace_test_langscore/
   ```

   別の場所に置く場合は環境変数 `LANGSCORE_TEST_DATA` でフォルダを指定してください。
2. `npm install`

## MV/MZ

```
npm test                 # MV/MZ × NW.js/ブラウザ の4パターン
npm run test-mv-nwjs     # 個別
```

- `pretest` で `sync_plugin.js` が `resource/Langscore.js` (テンプレート) を divisi と同じ規則で展開し、
  `<proj>/js/plugins/Langscore.js` へ書き出します。テスト対象は常に `resource/` の最新版です。
- `Langscore_mvmz.test.js` は Node の `http` でプロジェクトを配信し、jsdom で `index.html` を起動します。
  NW.js モード (`IS_NWJS=true`) では `process` / `require` を window に与え、ローカル実行を模倣します。
- プラグインパラメータを変えて起動したいテストは `initializeRPGMaker({ pluginParams: {...} })` を使います
  (配信時に `js/plugins.js` を書き換えます)。
- `<proj>/js/plugins/Langscore_test.js` は画像・音声・フォントのロードを止め、
  タイトル画面到達で `SceneManager` を停止させるテスト用スタブです。

## VXAce

```
$env:LANGSCORE_RUBY19 = "C:\ruby-1.9.2\bin\ruby.exe"   # 既定値と異なる場合
.\run_vxace_test.ps1
```

- 実行時に `sync_vxace.js` が `resource/langscore.rb` (テンプレート) を divisi と同じ規則で展開し、
  `vxace_test/Scripts/langscore.rb` へ書き出します (MV/MZ の `sync_plugin.js` と同じ役割)。
  その後 `vxace_test/compress.rb` が `Data/Scripts.rvdata2` へ固めてから実行されます。

## 全体

`test/run_test.py` から `python run_test.py plugin` (または `mv` / `mz` / `vxace` / `lscsv`) で
まとめて実行でき、結果は `test/test_log.md` に書き出されます。
