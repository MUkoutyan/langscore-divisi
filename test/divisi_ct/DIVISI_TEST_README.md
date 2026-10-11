# Divisi Test Documentation

このドキュメントは、`run_divisi_test.py`スクリプトの使用方法と、main.cppで実装されたdivisi.exeプログラムのテスト内容について説明します。

## テスト概要

このテストスイートは、Langscore Divisiプログラムの全機能を包括的にテストします。

### テスト対象のオプション

main.cppで実装されている以下のコマンドラインオプションをテストします：

1. **--createConfigFile <path>**: 設定ファイルの作成
2. **--analyze**: プロジェクトの解析
3. **--reanalysis**: プロジェクトの再解析  
4. **--updatePlugin**: プラグインの更新
5. **--exportCSV**: CSVファイルのエクスポート
6. **--validate**: プロジェクトの検証
7. **--packing**: プロジェクトのパッキング
8. **--convertPatchCSV <path>**: パッチCSVの変換
9. **--convertBoundCSV <path>**: バウンドCSVの変換
10. **-c <config_file>**: 設定ファイルの指定

## テストクラス

### TestCreateProject
- 設定ファイル作成機能のテスト
- 複数のプロジェクトタイプ（MV, MZ, VXAce）での検証
- 空白を含むパス名のテスト
- **新機能**: 設定ファイルのJSON構造検証、必須フィールド確認、プロジェクトタイプ別の詳細チェック

### TestAnalyze
- プロジェクト解析機能のテスト
- 解析結果ファイルの存在確認
- 期待されるファイル一覧の検証
- **新機能**: lsjsonファイルの内容検証、テキストパターン分析、プロジェクト固有の解析結果チェック

### TestWrite
- CSVエクスポート機能のテスト
- 出力ファイルサイズの一貫性確認
- 複数回実行時の結果確認
- **新機能**: CSVファイルの詳細分析、ヘッダー構造検証、テキストエントリ数カウント、サンプルテキスト収集

### TestUpdate
- プロジェクト更新機能のテスト
- 更新ファイルの生成確認
- 特定のキーワードの存在確認

### TestPluginOperations
- プラグイン更新機能のテスト
- --updatePluginオプションの動作確認
- **新機能**: プラグインファイルの存在確認、langscoreファイルの内容検証、プロジェクトタイプ別のファイルチェック

### TestValidationAndPacking
- --validateオプションのテスト
- --packingオプションのテスト
- 各プロジェクトタイプでの検証
- **新機能**: CSVファイル検証前チェック、検証ログファイル確認、パッキング結果の詳細検証

### TestCSVConversion
- --convertPatchCSVオプションのテスト
- --convertBoundCSVオプションのテスト
- テスト用CSVファイルでの変換確認
- **新機能**: 複数タイプのテストCSVファイル作成、変換結果ファイル検証、エラーハンドリングテスト

### TestErrorHandling
- エラー条件での動作テスト
- 存在しない設定ファイルの処理
- 引数なし実行時の動作
- 無効なオプションの処理

### TestVersionInfo
- バージョン情報表示のテスト
- ビルド情報の確認

## 使用方法

### 基本的な実行

```bash
# 全テストを実行
python run_divisi_test.py

# 詳細出力付きで全テストを実行
python run_divisi_test.py --verbose
```

### 個別テストの実行

```bash
# 特定のテストのみを実行
python run_divisi_test.py --test create
python run_divisi_test.py --test analyze
python run_divisi_test.py --test plugin
python run_divisi_test.py --test validation
python run_divisi_test.py --test csv
python run_divisi_test.py --test error
python run_divisi_test.py --test version

# 個別テストを詳細出力で実行
python run_divisi_test.py --test analyze --verbose
```

### 利用可能なテストオプション

- `all`: 全テストを実行（デフォルト）
- `create`: 設定ファイル作成テスト
- `analyze`: プロジェクト解析テスト
- `write`: CSVエクスポートテスト
- `update`: プロジェクト更新テスト
- `plugin`: プラグイン操作テスト
- `validation`: 検証とパッキングテスト
- `csv`: CSV変換テスト
- `error`: エラーハンドリングテスト
- `version`: バージョン情報テスト

## テスト環境の準備

テストを実行する前に、以下の条件が満たされている必要があります：

1. `divisi.exe`が`bin/`ディレクトリに存在すること
2. テストデータが適切なディレクトリに配置されていること
3. 必要な依存関係（test_core、lscsv等）が利用可能であること

## テスト結果の読み方

- **PASSED**: テストが成功
- **FAILED**: テストが失敗
- 失敗したテストについては詳細な出力とエラーメッセージが表示されます

## トラブルシューティング

### よくある問題

1. **divisi.exeが見つからない**
   - `bin/divisi.exe`が存在することを確認してください
   - パスが正しく設定されていることを確認してください

2. **テストデータが見つからない**
   - `test/data/`ディレクトリにテストプロジェクトが存在することを確認してください

3. **権限エラー**
   - テストディレクトリへの書き込み権限があることを確認してください
   - 読み取り専用ファイルがある場合は解除してください

### デバッグ方法

```bash
# 詳細出力でデバッグ
python run_divisi_test.py --verbose

# 特定のテストのみをデバッグ
python run_divisi_test.py --test analyze --verbose
```

## 拡張方法

新しいテストを追加する場合：

1. 新しいテストクラスを作成
2. `test_cases`リストに追加
3. 個別実行関数を作成（必要に応じて）
4. `test_map`に追加（個別実行する場合）

## 注意事項

- テストは一時的なワークディレクトリを作成し、テスト終了時に削除します
- 既存のプロジェクトファイルには影響しません
- テスト実行には時間がかかる場合があります（特に大きなプロジェクトの場合）
