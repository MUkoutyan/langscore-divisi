# Langscore プラグインテスト

`resource/` にあるプラグインをゲームプロジェクトに組み込んで動作確認する結合テストです。

| 対象 | テスト | 実行 | 必要なもの |
|------|--------|------|-----------|
| Langscore.js (MV/MZ) | `Langscore_mvmz.test.js` (mocha + jsdom) | `npm test` | Node.js、`mv_test/` `mz_test/` |
| langscore.rb (VXAce) | `vxace/ls_realtest.rb` (RGSS3 実機) | `python run_vxace_test.py` | Ruby (圧縮用)、`vxace_test/` |

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
python run_vxace_test.py            # 全設定
python run_vxace_test.py default    # default / deflang / patch のいずれか
```

**テストは実際の `Game.exe` (RGSS3) の中で動きます。** Bitmap・Font・Cache・Marshal はすべて本物で、
RGSS3 のエミュレーションは行いません。手順は次の通りです。

1. `sync_vxace.js` が `resource/langscore.rb` (テンプレート) を divisi と同じ規則で展開し、
   `vxace_test/Scripts/langscore.rb` へ書き出す (MV/MZ の `sync_plugin.js` と同じ役割)。
2. `vxace/compress.rb` が `Scripts/_list.csv` から `Data/Scripts.rvdata2` を組み立てる。
   このとき `vxace/ls_realtest.rb` を **Main の直前** に挿入する。
   `Main` の `rgss_main` は制御を返さないため、その前で実行して `exit` する必要があります。
3. `Game.exe test` を起動する。`test` 引数は `$TEST` を true にするために必要です
   (`Langscore.translate_list_reset` が参照します)。
4. テストは結果を `ls_test_result.txt` に書いて終了し、runner がそれを読む。
   最終行の `DONE` が無い場合は途中で異常終了したものとして失敗扱いにします。

`langscore.rb` の設定 (`ENABLE_PATCH_MODE` など) はスクリプト内の定数のため、
設定を変えるテストは `Game.exe` を起動し直します (`VARIANTS`)。

`Game.exe` は `System/RGSS301.dll` を使うため、VX Ace のインストールは不要です。
ただし GUI セッションが必要なので、この1件だけ CI では動きません。

テスト中に書き換わるもの (`Game.ini` の言語設定、`Data/Scripts.rvdata2`、テスト用セーブ) は
runner が終了時に元へ戻します。

## 全体

`test/run_test.py` から `python run_test.py plugin` (または `mv` / `mz` / `vxace` / `lscsv`) で
まとめて実行でき、結果は `test/test_log.md` に書き出されます。
