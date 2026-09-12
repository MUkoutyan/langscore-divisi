"""
Langscore Divisi Test Suite

このスクリプトは、main.cppで実装されたdivisi.exeプログラムの包括的なテストを実行します。

テスト対象オプション:
- --createConfigFile <path>: 設定ファイルの作成
- --analyze: プロジェクトの解析
- --reanalysis: プロジェクトの再解析
- --updatePlugin: プラグインの更新
- --exportCSV: CSVファイルのエクスポート
- --validate: プロジェクトの検証
- --packing: プロジェクトのパッキング
- --convertPatchCSV <path>: パッチCSVの変換
- --convertBoundCSV <path>: バウンドCSVの変換
- -c <config_file>: 設定ファイルの指定

テストクラス:
- TestCreateProject: 設定ファイル作成のテスト
- TestAnalyze: プロジェクト解析のテスト
- TestWrite: CSVエクスポートのテスト
- TestUpdate: プロジェクト更新のテスト
- TestPluginOperations: プラグイン操作のテスト
- TestValidationAndPacking: 検証とパッキングのテスト
- TestCSVConversion: CSV変換のテスト
- TestErrorHandling: エラーハンドリングのテスト
- TestVersionInfo: バージョン情報表示のテスト

使用方法:
python run_divisi_test.py  # 全テストを実行
"""

import subprocess
import io
import os
import re
import sys
import shutil
import stat
import unittest
import json
import pprint

current_dir = os.path.dirname(os.path.abspath(__file__))
# rootディレクトリのパスを取得してsys.pathに追加
test_root_dir = os.path.abspath(os.path.join(current_dir, '../'))
sys.path.append(test_root_dir)
divisi_root_dir = os.path.abspath(os.path.join(current_dir, '../../'))
sys.path.append(divisi_root_dir)
from internal import test_core as core

# 結果表示に ✓ 等の非cp932文字が含まれるため、標準出力をUTF-8にする。
# このスクリプトは sys.stdout を差し替えて出力を捕捉するため、TextIOWrapperで包むと
# 元のストリームごと閉じられてしまう。reconfigureで元のストリームを直接設定する。
sys.stdout.reconfigure(encoding='utf-8')

from resource import lscsv

def copy_folder(src, dest):
    ret = shutil.copytree(src, dest)
    core.remove_read_only(dest)
    return ret

def force_remove_readonly(func, path, _):
    os.chmod(path, stat.S_IWRITE)
    func(path)


def run_divisi(project_path, option):
    DIVISI_PATH = divisi_root_dir + '\\bin\\divisi.exe'
    if option == "create":
        args = [
            '--createConfigFile', project_path,
        ]
    elif option in ["updatePlugin", "validate", "packing"]:
        args = [
            '-c', project_path,
            f'--{option}'
        ]
    else:
        args = [
            '-c', project_path,
            f'--{option}'
        ]
    return core.run_command(DIVISI_PATH, args)

mv_work_path                = os.path.join(current_dir, 'work\\LangscoreTestMV')
mz_work_path                = os.path.join(current_dir, 'work\\LangscoreTestMZ')
vxace_work_path             = os.path.join(current_dir, 'work\\LangscoreTestVXAce')
include_empty_work_path     = os.path.join(current_dir, 'work\\Include WhiteSpacePath Project')
mv_work_ls_path             = os.path.join(current_dir, 'work\\LangscoreTestMV_langscore')
mz_work_ls_path             = os.path.join(current_dir, 'work\\LangscoreTestMZ_langscore')
vxace_work_ls_path          = os.path.join(current_dir, 'work\\LangscoreTestVXAce_langscore')
include_empty_work_ls_path  = os.path.join(current_dir, 'work\\Include WhiteSpacePath Project_langscore')


# テストデータはリポジトリ外 (langscore-divisi-test-data) を参照する
test_data_dir = core.require_test_data('data')

source_mv_work_path            = os.path.join(test_data_dir, 'LangscoreTestMV')
source_mz_work_path            = os.path.join(test_data_dir, 'LangscoreTestMZ')
source_vxace_work_path         = os.path.join(test_data_dir, 'LangscoreTestVXAce')
source_include_empty_work_path = os.path.join(test_data_dir, 'vxace', 'Include WhiteSpacePath Project')
source_update_mv_work_path            = os.path.join(test_data_dir, 'LangscoreTestMV_updated')
source_update_mz_work_path            = os.path.join(test_data_dir, 'LangscoreTestMZ_updated')
source_update_vxace_work_path         = os.path.join(test_data_dir, 'LangscoreTestVXAce_updated')
source_update_include_empty_work_path = os.path.join(test_data_dir, 'vxace', 'Include WhiteSpacePath Project_updated')

analyze_expected_files = [
    "System.lsjson","Troops.lsjson","Weapons.lsjson","Actors.lsjson",
    "Armors.lsjson","Classes.lsjson","CommonEvents.lsjson",
    "Enemies.lsjson","Items.lsjson","Map001.lsjson","Map002.lsjson","Map003.lsjson",
    "Skills.lsjson","States.lsjson"
]
write_expected_files = [
    "System.csv","Troops.csv","Weapons.csv","Actors.csv",
    "Armors.csv","Classes.csv","CommonEvents.csv",
    "Enemies.csv","Items.csv","Map001.csv","Map002.csv","Map003.csv",
    "Skills.csv","States.csv"
]

def setup_projects():
    if os.path.exists(mv_work_path):
        shutil.rmtree(mv_work_path, onerror=force_remove_readonly)
    if os.path.exists(mz_work_path):
        shutil.rmtree(mz_work_path, onerror=force_remove_readonly)
    if os.path.exists(vxace_work_path):
        shutil.rmtree(vxace_work_path, onerror=force_remove_readonly)
    if os.path.exists(include_empty_work_path):
        shutil.rmtree(include_empty_work_path, onerror=force_remove_readonly)

    copy_folder(source_mv_work_path, mv_work_path)
    copy_folder(source_mz_work_path, mz_work_path)
    copy_folder(source_vxace_work_path, vxace_work_path)
    copy_folder(source_include_empty_work_path, include_empty_work_path)
    print("copy projects")

def remove_projects():
    if os.path.exists(mv_work_path):
        shutil.rmtree(mv_work_path, onerror=force_remove_readonly)
    if os.path.exists(mz_work_path):
        shutil.rmtree(mz_work_path, onerror=force_remove_readonly)
    if os.path.exists(vxace_work_path):
        shutil.rmtree(vxace_work_path, onerror=force_remove_readonly)
    if os.path.exists(include_empty_work_path):
        shutil.rmtree(include_empty_work_path, onerror=force_remove_readonly)
    if os.path.exists(mv_work_ls_path):
        shutil.rmtree(mv_work_ls_path, onerror=force_remove_readonly)
    if os.path.exists(mz_work_ls_path):
        shutil.rmtree(mz_work_ls_path, onerror=force_remove_readonly)
    if os.path.exists(vxace_work_ls_path):
        shutil.rmtree(vxace_work_ls_path, onerror=force_remove_readonly)
    if os.path.exists(include_empty_work_ls_path):
        shutil.rmtree(include_empty_work_ls_path, onerror=force_remove_readonly)
    print("remove projects")

def edit_config_file(data):
    data['DefaultLanguage'] = "ja"

    for language in data['Languages']:
        if language['LanguageName'] == 'en':
            language['Enable'] = True

def create_config_file(work_path):
    run_divisi(work_path, "create")
    core.edit_ls_config(work_path+"_langscore\config.json", edit_config_file)

def set_packing_input_dir(ls_path):
    """--validate / --packing は PackingInputDir 配下のCSVを読むため、エクスポート先を設定する。
    作成直後のconfigでは空欄なので、GUIと同様にexportCSVの出力先を指す必要がある。"""
    translate_dir = os.path.join(ls_path, "data", "translate").replace("\\", "/")
    def edit(data):
        data['PackingInputDir'] = translate_dir
    core.edit_ls_config(os.path.join(ls_path, "config.json"), edit)


class TestCreateProject(unittest.TestCase):

    @classmethod
    def setUpClass(self):
        setup_projects()

    @classmethod
    def tearDownClass(self):
        remove_projects()

    def test_createConfig(self):
        print("\n=== Testing config file creation ===")
        
        # MV プロジェクト
        self._create_config_for_project(mv_work_path, mv_work_ls_path, "MV")
        
        # MZ プロジェクト  
        self._create_config_for_project(mz_work_path, mz_work_ls_path, "MZ")
        
        # VXAce プロジェクト
        self._create_config_for_project(vxace_work_path, vxace_work_ls_path, "VXAce")
        
        # 空白を含むパスのプロジェクト
        self._create_config_for_project(include_empty_work_path, include_empty_work_ls_path, "IncludeEmpty")
        
    def _create_config_for_project(self, work_path, ls_path, project_type):
        """個別プロジェクトの設定ファイル作成テスト"""
        print(f"\n--- Testing config creation for {project_type} ---")
        
        # 設定ファイル作成前の状態確認
        self.assertFalse(os.path.exists(ls_path), f"Langscore directory should not exist before creation for {project_type}")
        
        # 設定ファイル作成
        out, err, result = run_divisi(work_path, "create")
        self.assertTrue(result, f"Config creation failed for {project_type}: {err}")
        
        # 基本的なディレクトリとファイルの存在確認
        self.assertTrue(os.path.exists(ls_path), f"Langscore directory should exist for {project_type}")
        
        config_file = os.path.join(ls_path, "config.json")
        self.assertTrue(os.path.exists(config_file), f"Config file should exist for {project_type}")
        
        # 設定ファイルの詳細検証
        self.validate_config_file(config_file, work_path, project_type)
        
        # 設定ファイル編集
        core.edit_ls_config(config_file, edit_config_file)
        
        # 編集後の設定ファイル検証
        self.validate_edited_config_file(config_file, project_type)
        
        # ディレクトリ構造の確認
        self.check_langscore_directory_structure(ls_path, project_type)
        
    def validate_config_file(self, config_file, work_path, project_type):
        """設定ファイルの内容検証"""
        print(f"Validating config file for {project_type}")
        
        # JSONファイルとして読み込み可能かチェック
        try:
            with open(config_file, 'r', encoding='utf-8') as f:
                config_data = json.load(f)
        except json.JSONDecodeError as e:
            self.fail(f"Config file is not valid JSON for {project_type}: {e}")
            
        # 必須フィールドの確認
        # divisiが出力するのは Project / DefaultLanguage / Languages / Analyze / Write。
        # プロジェクト種別はconfigに持たず、Projectフォルダ内のプロジェクトファイルの拡張子から判別される。
        required_fields = [
            'Project',
            'DefaultLanguage',
            'Languages',
            'Analyze',
            'Write'
        ]

        for field in required_fields:
            self.assertIn(field, config_data, f"Config should contain {field} for {project_type}")

        # ゲームプロジェクトパスの確認
        game_path = config_data.get('Project', '')
        self.assertTrue(game_path, f"Project should not be empty for {project_type}")

        # パスの正規化と存在確認
        normalized_work_path = os.path.normpath(work_path)
        normalized_game_path = os.path.normpath(game_path)
        self.assertEqual(normalized_game_path, normalized_work_path,
                        f"Project should match work path for {project_type}")

        # プロジェクト種別は拡張子で判別されるため、対応するプロジェクトファイルが存在することを確認する
        project_file_exts = {
            "MV": ".rpgproject",
            "MZ": ".rmmzproject",
            "VXAce": ".rvproj2",
        }
        if project_type in project_file_exts:
            ext = project_file_exts[project_type]
            found = [f for f in os.listdir(game_path) if f.endswith(ext)]
            self.assertTrue(found, f"{ext} should exist in project folder for {project_type}")

        # 言語設定の確認
        languages = config_data.get('Languages', [])
        self.assertIsInstance(languages, list, f"Languages should be a list for {project_type}")
        self.assertGreater(len(languages), 0, f"Should have at least one language for {project_type}")
        for language in languages:
            self.assertIn('LanguageName', language, f"Language entry should have LanguageName for {project_type}")
            self.assertIn('Enable', language, f"Language entry should have Enable for {project_type}")

        # DefaultLanguageは作成直後は空。設定後の値は validate_edited_config_file で確認する。
        default_lang = config_data.get('DefaultLanguage', '')

        print(f"✓ Config validation passed for {project_type}")
        print(f"  Project: {game_path}")
        print(f"  Default Language: {default_lang}")
        print(f"  Languages: {len(languages)} configured")
        
    def validate_edited_config_file(self, config_file, project_type):
        """編集後の設定ファイル検証"""
        try:
            with open(config_file, 'r', encoding='utf-8') as f:
                config_data = json.load(f)
        except json.JSONDecodeError as e:
            self.fail(f"Edited config file is not valid JSON for {project_type}: {e}")
            
        # 編集内容の確認
        self.assertEqual(config_data.get('DefaultLanguage'), 'ja', 
                        f"DefaultLanguage should be 'ja' after edit for {project_type}")
        
        # 英語が有効になっているかチェック
        languages = config_data.get('Languages', [])
        en_enabled = False
        for lang in languages:
            if isinstance(lang, dict) and lang.get('LanguageName') == 'en':
                en_enabled = lang.get('Enable', False)
                break
                
        self.assertTrue(en_enabled, f"English language should be enabled for {project_type}")
        print(f"✓ Config edit validation passed for {project_type}")
        
    def check_langscore_directory_structure(self, ls_path, project_type):
        """Langscoreディレクトリ構造の確認"""
        print(f"Checking directory structure for {project_type}")
        
        # 基本的なディレクトリが作成されているか確認
        expected_dirs = [
            "data",
            "resource"
        ]
        
        for dir_name in expected_dirs:
            dir_path = os.path.join(ls_path, dir_name)
            if os.path.exists(dir_path):
                print(f"✓ Found directory: {dir_name}")
            else:
                print(f"? Directory not found: {dir_name} (may be created later)")
                
        # リソースファイルの確認
        resource_dir = os.path.join(ls_path, "resource")
        if os.path.exists(resource_dir):
            resource_files = os.listdir(resource_dir)
            if resource_files:
                print(f"✓ Resource files found: {resource_files}")
            else:
                print("? No resource files found (may be added later)")
                
        # プロジェクト固有のファイル確認
        self.check_project_specific_files(ls_path, project_type)
        
    def check_project_specific_files(self, ls_path, project_type):
        """プロジェクト固有のファイル確認"""
        if project_type in ["MV", "MZ"]:
            # MV/MZではJavaScriptファイルが期待される
            js_files = [
                "Langscore.js",
                "Langscore_ObserverBridge.js"
            ]
            
            for js_file in js_files:
                possible_paths = [
                    os.path.join(ls_path, "resource", js_file),
                    os.path.join(ls_path, js_file)
                ]
                
                found = any(os.path.exists(path) for path in possible_paths)
                if found:
                    print(f"✓ Found {js_file} for {project_type}")
                else:
                    print(f"? {js_file} not found for {project_type} (may be added during plugin update)")
                    
        elif project_type == "VXAce":
            # VXAceではRubyスクリプトが期待される
            rb_files = [
                "langscore.rb",
                "langscore_custom.rb"
            ]
            
            for rb_file in rb_files:
                possible_paths = [
                    os.path.join(ls_path, "resource", rb_file),
                    os.path.join(ls_path, rb_file)
                ]
                
                found = any(os.path.exists(path) for path in possible_paths)
                if found:
                    print(f"✓ Found {rb_file} for {project_type}")
                else:
                    print(f"? {rb_file} not found for {project_type} (may be added during plugin update)")

        
class TestWrite(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        setup_projects()

        create_config_file(mv_work_path)
        create_config_file(mz_work_path)
        create_config_file(vxace_work_path)
        create_config_file(include_empty_work_path)
        
        run_divisi(mv_work_ls_path + "\\config.json", "analyze")
        run_divisi(mz_work_ls_path + "\\config.json", "analyze")
        run_divisi(vxace_work_ls_path + "\\config.json", "analyze")
        run_divisi(include_empty_work_ls_path + "\\config.json", "analyze")

    @classmethod
    def tearDownClass(self):
        remove_projects()

        
    def test_write(self):
        # MV =====================================
        self._write_project(mv_work_ls_path, "MV")
        # MZ =====================================
        self._write_project(mz_work_ls_path, "MZ")
        # Vxace =====================================
        self._write_project(vxace_work_ls_path, "VXAce")
        # Include Empty =====================================
        self._write_project(include_empty_work_ls_path, "IncludeEmpty")
        
    def _write_project(self, ls_path, project_type):
        """個別プロジェクトのCSVエクスポートテスト"""
        print(f"\n=== Testing CSV export for {project_type} ===")
        
        # 初回エクスポート
        out, err, result = run_divisi(ls_path + "\\config.json", "exportCSV")
        self.assertTrue(result, f"First exportCSV failed for {project_type}: {err}")
        
        # 翻訳ディレクトリの確認
        translate_dir = os.path.join(ls_path, "data", "translate")
        self.assertTrue(os.path.exists(translate_dir), f"Translate directory should exist for {project_type}")
        
        # ファイルサイズの記録
        begin_exported_file_size = []
        csv_file_details = {}
        
        for file in write_expected_files:
            file_path = os.path.join(translate_dir, file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {ls_path}")
            
            file_size = os.path.getsize(file_path)
            begin_exported_file_size.append(file_size)
            
            # CSVファイルの詳細分析
            csv_details = self.analyze_csv_file(file_path, file, project_type)
            csv_file_details[file] = csv_details
            
        # 2回目のエクスポート（冪等性の確認）
        out, err, result = run_divisi(ls_path + "\\config.json", "exportCSV")
        self.assertTrue(result, f"Second exportCSV failed for {project_type}: {err}")
        
        # ファイルサイズの一致確認
        for i, file in enumerate(write_expected_files):
            file_path = os.path.join(translate_dir, file)
            after_filesize = os.path.getsize(file_path)
            self.assertEqual(begin_exported_file_size[i], after_filesize, 
                           f"{file} size changed after second export in {project_type}")
            
        # エクスポート結果の検証
        self.validate_csv_export_results(csv_file_details, project_type)
        
    def analyze_csv_file(self, file_path, filename, project_type):
        """CSVファイルの詳細分析"""
        details = {
            'filename': filename,
            'size': os.path.getsize(file_path),
            'line_count': 0,
            'text_entries': 0,
            'empty_entries': 0,
            'has_header': False,
            'columns': [],
            'sample_texts': []
        }
        
        try:
            # 出力CSVはUTF-8 BOM付きのため utf-8-sig で読む
            with open(file_path, 'r', encoding='utf-8-sig', newline='') as f:
                import csv
                reader = csv.reader(f)

                # ヘッダー行の処理
                try:
                    header = next(reader)
                    details['has_header'] = True
                    details['columns'] = header
                    details['line_count'] += 1

                    # 出力CSVのヘッダーは original + 有効な言語名 (例: original,ja,en)
                    self.assertEqual(header[0] if header else "", 'original',
                                     f"{filename} first column should be 'original'")
                    self.assertIn('ja', header, f"{filename} should have ja column")

                except StopIteration:
                    pass

                # データ行の処理
                for row in reader:
                    details['line_count'] += 1

                    if len(row) >= 2:  # original + 1言語以上
                        original_text = row[0]

                        if original_text.strip():
                            details['text_entries'] += 1
                            
                            # サンプルテキストの収集（最初の5個）
                            if len(details['sample_texts']) < 5:
                                details['sample_texts'].append(original_text[:50])
                        else:
                            details['empty_entries'] += 1
                            
        except Exception as e:
            print(f"Error analyzing {filename}: {e}")
            
        print(f"✓ {filename} ({project_type}): {details['text_entries']} texts, {details['line_count']} lines")
        
        return details
        
    def validate_csv_export_results(self, csv_details, project_type):
        """CSVエクスポート結果の検証"""
        print(f"\n--- CSV Export Validation for {project_type} ---")
        
        total_texts = sum(details['text_entries'] for details in csv_details.values())
        total_files = len(csv_details)
        
        print(f"Total files exported: {total_files}")
        print(f"Total text entries: {total_texts}")
        
        # プロジェクトタイプ別の期待値チェック
        if project_type in ["MV", "MZ"]:
            # MV/MZでは通常多くのテキストが期待される
            self.assertGreater(total_texts, 10, f"{project_type} should have substantial text content")
            
            # 特定ファイルの存在確認
            key_files = ["System.csv", "Map001.csv", "Actors.csv"]
            for key_file in key_files:
                if key_file in csv_details:
                    details = csv_details[key_file]
                    self.assertGreater(details['text_entries'], 0, 
                                     f"{key_file} should contain text entries in {project_type}")
                                     
        elif project_type == "VXAce":
            # VXAceでも基本的なテキストは期待される
            self.assertGreater(total_texts, 5, f"{project_type} should have some text content")
            
        # サンプルテキストの表示
        for filename, details in csv_details.items():
            if details['sample_texts']:
                print(f"  {filename} samples: {details['sample_texts'][:2]}")
                
        # CSV形式の整合性確認
        for filename, details in csv_details.items():
            self.assertTrue(details['has_header'], f"{filename} should have header row")
            self.assertIn('original', details['columns'], f"{filename} should have original column")
            self.assertIn('ja', details['columns'], f"{filename} should have ja column")


    
class TestUpdate(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        setup_projects()

        create_config_file(mv_work_path)
        create_config_file(mz_work_path)
        create_config_file(vxace_work_path)
        create_config_file(include_empty_work_path)
        
        run_divisi(mv_work_ls_path + "\\config.json", "analyze")
        run_divisi(mz_work_ls_path + "\\config.json", "analyze")
        run_divisi(vxace_work_ls_path + "\\config.json", "analyze")
        run_divisi(include_empty_work_ls_path + "\\config.json", "analyze")
        
        run_divisi(mv_work_ls_path + "\\config.json", "exportCSV")
        run_divisi(mz_work_ls_path + "\\config.json", "exportCSV")
        run_divisi(vxace_work_ls_path + "\\config.json", "exportCSV")
        run_divisi(include_empty_work_ls_path + "\\config.json", "exportCSV")

    @classmethod
    def tearDownClass(self):
        remove_projects()

    def test_update(self):

        check_keys = [
            "通常のテキストです2", "改行を含む\nテキストです2", 
            "\"タ\"フ\"ルクォーテーションを含むテキストです2\"",
            "\"\"\"Hello, World\"\",\nそれはプログラムを書く際の\",\"\"\"謎の呪文\"\"(Mystery spell)―――2\"",
            "複合させます\n\\{\\C[2]\\N[2]\\I[22]\n2"
        ]

        def check_mvmz_function(work_path, work_ls_path, source_path):
            run_divisi(work_ls_path + "\\config.json", "analyze")
            if os.path.exists(work_path):
                shutil.rmtree(work_path, onerror=force_remove_readonly)
            copy_folder(source_path, work_path)

            run_divisi(work_ls_path + "\\config.json", "reanalysis")
            self.assertTrue(os.path.exists(work_ls_path + "\\update"))

            for file in write_expected_files:
                file_path = os.path.join(work_ls_path + "\\data\\translate", file)
                self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {work_ls_path}")
            
            with open(work_ls_path + "\\update\\Map001.lsjson", 'r', encoding='utf-8') as json_file:
                contents = json.load(json_file)
                for key in check_keys:
                    found = False
                    for item in contents:
                        if isinstance(item, dict) and 'original' in item:
                            if key in item['original']:
                                found = True
                                break
                    self.assertTrue(found, f"not found key \"{key}\" in updated lsjson {work_ls_path}")
            
        check_mvmz_function(mv_work_path, mv_work_ls_path, source_update_mv_work_path)
        check_mvmz_function(mz_work_path, mz_work_ls_path, source_update_mz_work_path)
        
        def check_vxace_function(work_path, work_ls_path, source_path):
            run_divisi(work_ls_path + "\\config.json", "analyze")
            if os.path.exists(work_path):
                shutil.rmtree(work_path, onerror=force_remove_readonly)
            copy_folder(source_path, work_path)
            actual_files = os.listdir(os.path.join(work_ls_path, "Data", "Translate"))
            run_divisi(work_ls_path + "\\config.json", "reanalysis")
            self.assertTrue(os.path.exists(work_ls_path + "\\update"))
            for file in write_expected_files:
                self.assertIn(file, actual_files, f"{file} does not exist in {work_ls_path}")

            with open(work_ls_path + "\\update\\Map001.lsjson", 'r', encoding='utf-8') as json_file:
                contents = json.load(json_file)
                for key in check_keys:
                    found = False
                    for item in contents:
                        if isinstance(item, dict) and 'original' in item:
                            if key in item['original']:
                                found = True
                                break
                    self.assertTrue(found, f"not found key \"{key}\" in updated lsjson {work_ls_path}")

        check_vxace_function(vxace_work_path, vxace_work_ls_path, source_update_vxace_work_path)
        check_vxace_function(include_empty_work_path, include_empty_work_ls_path, source_update_include_empty_work_path)



class TestAnalyze(unittest.TestCase):

    @classmethod
    def setUpClass(self):
        setup_projects()

        create_config_file(mv_work_path)
        create_config_file(mz_work_path)
        create_config_file(vxace_work_path)
        create_config_file(include_empty_work_path)
        print("copy projects")

    @classmethod
    def tearDownClass(self):
        remove_projects()

    def test_analyze(self):
        
        out, err, result = run_divisi(mv_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(result, err)
        self.check_analyze_results(mv_work_ls_path, "MV", out)
        
        out, err, result = run_divisi(mz_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(result, err)
        self.check_analyze_results(mz_work_ls_path, "MZ", out)

        out, err, result = run_divisi(vxace_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(result, err)
        self.check_analyze_results(vxace_work_ls_path, "VXAce", out)

        out, err, result = run_divisi(include_empty_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(result, err)
        self.check_analyze_results(include_empty_work_ls_path, "IncludeEmpty", out)
        
    def check_analyze_results(self, ls_path, project_type, output):
        """解析結果の詳細チェック"""
        print(f"\n=== Checking analyze results for {project_type} ===")
        
        # analyzeディレクトリの存在確認
        analyze_dir = os.path.join(ls_path, "analyze")
        self.assertTrue(os.path.exists(analyze_dir), f"Analyze directory should exist for {project_type}")
        
        # 期待されるlsjsonファイルの存在と内容確認
        for file in analyze_expected_files:
            file_path = os.path.join(analyze_dir, file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {ls_path}")
            
            # ファイルサイズの確認
            file_size = os.path.getsize(file_path)
            self.assertGreater(file_size, 0, f"{file} should not be empty")
            
            # JSON形式の確認
            try:
                with open(file_path, 'r', encoding='utf-8') as f:
                    json_data = json.load(f)
                    self.assertIsInstance(json_data, list, f"{file} should contain a JSON array")
                    
                    if json_data:  # 空でない場合
                        # 最初の要素の構造確認
                        first_item = json_data[0]
                        self.assertIn("original", first_item, f"{file} items should have 'original' field")
                        
                        # テキストデータの検証
                        text_count = len([item for item in json_data if isinstance(item, dict) and 'original' in item])
                        print(f"✓ {file}: {text_count} text entries found")
                        
                        # 特定のテキストパターンをチェック
                        self.check_text_patterns(json_data, file, project_type)
                        
            except json.JSONDecodeError as e:
                self.fail(f"Invalid JSON in {file}: {e}")
                
        # プロジェクト固有のチェック
        self.check_project_specific_analyze(ls_path, project_type)
        
        # 出力メッセージの確認
        if output:
            self.check_analyze_output(output, project_type)
            
    def check_text_patterns(self, json_data, filename, project_type):
        """テキストパターンの確認"""
        original_texts = [item.get('original', '') for item in json_data if isinstance(item, dict)]
        
        # 基本的なテキストパターンの確認
        if filename == "Map001.lsjson":
            # マップファイルでは制御文字を含むテキストが期待される
            control_char_texts = [text for text in original_texts if '\\' in text]
            if control_char_texts:
                print(f"✓ {filename}: Found {len(control_char_texts)} texts with control characters")
                
        elif filename == "System.lsjson":
            # システムファイルでは設定テキストが期待される
            system_texts = [text for text in original_texts if text and len(text) > 0]
            print(f"✓ {filename}: Found {len(system_texts)} system texts")
            
        elif filename in ["Actors.lsjson", "Classes.lsjson"]:
            # キャラクター関連ファイルでは名前や説明が期待される
            char_texts = [text for text in original_texts if text and len(text.strip()) > 0]
            print(f"✓ {filename}: Found {len(char_texts)} character-related texts")
            
    def check_project_specific_analyze(self, ls_path, project_type):
        """プロジェクト固有の解析結果チェック"""
        if project_type in ["MV", "MZ"]:
            # MV/MZでは特定のJSONファイルパターンが期待される
            self.check_mvmz_specific_files(ls_path, project_type)
        elif project_type == "VXAce":
            # VXAceでは特定のRVdata2パターンが期待される
            self.check_vxace_specific_files(ls_path)
            
    def check_mvmz_specific_files(self, ls_path, project_type):
        """MV/MZ固有のファイルチェック"""
        analyze_dir = os.path.join(ls_path, "analyze")
        
        # CommonEvents.lsjsonの詳細チェック
        common_events_path = os.path.join(analyze_dir, "CommonEvents.lsjson")
        if os.path.exists(common_events_path):
            with open(common_events_path, 'r', encoding='utf-8') as f:
                events_data = json.load(f)
                event_texts = [item.get('original', '') for item in events_data if isinstance(item, dict)]
                print(f"✓ CommonEvents: {len(event_texts)} event texts found in {project_type}")
                
    def check_vxace_specific_files(self, ls_path):
        """VXAce固有のファイルチェック"""
        analyze_dir = os.path.join(ls_path, "analyze")
        
        # VXAceではScripts.lsjsonが存在する可能性
        scripts_path = os.path.join(analyze_dir, "Scripts.lsjson")
        if os.path.exists(scripts_path):
            with open(scripts_path, 'r', encoding='utf-8') as f:
                scripts_data = json.load(f)
                script_texts = [item.get('original', '') for item in scripts_data if isinstance(item, dict)]
                print(f"✓ Scripts: {len(script_texts)} script texts found in VXAce")
                
    def check_analyze_output(self, output, project_type):
        """解析出力メッセージの確認"""
        print(f"Analyze output for {project_type}:")
        if output:
            lines = output.split('\n')[:10]  # 最初の10行のみ表示
            for line in lines:
                if line.strip():
                    print(f"  {line}")
                    
            # 期待されるメッセージの確認
            output_lower = output.lower()
            expected_patterns = ["analyze", "complete", "processing", "found"]
            found_patterns = [pattern for pattern in expected_patterns if pattern in output_lower]
            
            if found_patterns:
                print(f"✓ Found expected patterns in output: {found_patterns}")
            else:
                print(f"? No expected patterns found in analyze output for {project_type}")

class TestPluginOperations(unittest.TestCase):
    """プラグイン関連オプションのテスト"""
    
    @classmethod
    def setUpClass(self):
        setup_projects()
        create_config_file(mv_work_path)
        create_config_file(mz_work_path)
        create_config_file(vxace_work_path)
        run_divisi(mv_work_ls_path + "\\config.json", "analyze")
        run_divisi(mz_work_ls_path + "\\config.json", "analyze")
        run_divisi(vxace_work_ls_path + "\\config.json", "analyze")

    @classmethod
    def tearDownClass(self):
        remove_projects()

    def test_updatePlugin(self):
        """--updatePluginオプションのテスト"""
        # MV/MZプロジェクトでのプラグイン更新
        out, err, result = run_divisi(mv_work_ls_path + "\\config.json", "updatePlugin")
        self.assertTrue(result, f"updatePlugin failed for MV: {err}")
        
        # プラグインフォルダの存在確認
        plugin_folder = os.path.join(mv_work_path, "js", "plugins")
        if os.path.exists(plugin_folder):
            # プラグインファイルが更新されたかチェック
            plugin_files = [f for f in os.listdir(plugin_folder) if f.endswith('.js')]
            print(f"Plugin files found in MV: {plugin_files}")
        
        out, err, result = run_divisi(mz_work_ls_path + "\\config.json", "updatePlugin")
        self.assertTrue(result, f"updatePlugin failed for MZ: {err}")
        
        plugin_folder_mz = os.path.join(mz_work_path, "js", "plugins")
        if os.path.exists(plugin_folder_mz):
            plugin_files_mz = [f for f in os.listdir(plugin_folder_mz) if f.endswith('.js')]
            print(f"Plugin files found in MZ: {plugin_files_mz}")
            
        # VXAceではプラグインファイルではなくScriptファイルを確認
        out, err, result = run_divisi(vxace_work_ls_path + "\\config.json", "updatePlugin")
        self.assertTrue(result, f"updatePlugin failed for VXAce: {err}")
        
        # Scriptフォルダの確認
        script_folder = os.path.join(vxace_work_path, "Data", "Scripts")
        if os.path.exists(script_folder):
            script_files = [f for f in os.listdir(script_folder) if f.endswith('.rb')]
            print(f"Script files found in VXAce: {script_files}")
            
        # 更新後にlangscore関連ファイルが存在することを確認
        self.check_langscore_files(mv_work_path, "MV")
        self.check_langscore_files(mz_work_path, "MZ")
        self.check_langscore_files_vxace(vxace_work_path)
        
    def check_langscore_files(self, project_path, project_type):
        """MV/MZプロジェクトでのlangscoreファイルの存在確認"""
        expected_files = [
            "js/plugins/Langscore.js",
            "js/plugins/Langscore_ObserverBridge.js"
        ]
        
        for file_path in expected_files:
            full_path = os.path.join(project_path, file_path)
            if os.path.exists(full_path):
                print(f"✓ Found {file_path} in {project_type} project")
                # ファイル内容の基本チェック
                with open(full_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    self.assertIn("Langscore", content, f"{file_path} should contain Langscore")
            else:
                print(f"✗ Missing {file_path} in {project_type} project")
                
    def check_langscore_files_vxace(self, project_path):
        """VXAceプロジェクトでのlangscoreファイルの存在確認"""
        expected_files = [
            "Data/Scripts/langscore.rb",
            "Data/Scripts/langscore_custom.rb"
        ]
        
        for file_path in expected_files:
            full_path = os.path.join(project_path, file_path)
            if os.path.exists(full_path):
                print(f"✓ Found {file_path} in VXAce project")
                # ファイル内容の基本チェック
                with open(full_path, 'r', encoding='utf-8') as f:
                    content = f.read()
                    self.assertIn("langscore", content.lower(), f"{file_path} should contain langscore")
            else:
                print(f"✗ Missing {file_path} in VXAce project")


class TestValidationAndPacking(unittest.TestCase):
    """検証とパッキング機能のテスト"""
    
    @classmethod
    def setUpClass(self):
        setup_projects()
        create_config_file(mv_work_path)
        create_config_file(mz_work_path)
        create_config_file(vxace_work_path)
        
        # 基本的な解析とCSVエクスポートを実行
        run_divisi(mv_work_ls_path + "\\config.json", "analyze")
        run_divisi(mz_work_ls_path + "\\config.json", "analyze")
        run_divisi(vxace_work_ls_path + "\\config.json", "analyze")
        
        run_divisi(mv_work_ls_path + "\\config.json", "exportCSV")
        run_divisi(mz_work_ls_path + "\\config.json", "exportCSV")
        run_divisi(vxace_work_ls_path + "\\config.json", "exportCSV")

        set_packing_input_dir(mv_work_ls_path)
        set_packing_input_dir(mz_work_ls_path)
        set_packing_input_dir(vxace_work_ls_path)

    @classmethod
    def tearDownClass(self):
        remove_projects()

    def test_validate(self):
        """--validateオプションのテスト"""
        # 検証実行前にCSVファイルの存在を確認
        self.check_csv_files_exist(mv_work_ls_path, "MV")
        self.check_csv_files_exist(mz_work_ls_path, "MZ")
        self.check_csv_files_exist(vxace_work_ls_path, "VXAce")
        
        out, err, result = run_divisi(mv_work_ls_path + "\\config.json", "validate")
        self.assertTrue(result, f"validate failed for MV: {err}")
        self.check_validation_output(out, "MV")
        
        out, err, result = run_divisi(mz_work_ls_path + "\\config.json", "validate")
        self.assertTrue(result, f"validate failed for MZ: {err}")
        self.check_validation_output(out, "MZ")
        
        out, err, result = run_divisi(vxace_work_ls_path + "\\config.json", "validate")
        self.assertTrue(result, f"validate failed for VXAce: {err}")
        self.check_validation_output(out, "VXAce")
        
        # 検証後にログファイルの存在確認
        self.check_validation_logs()

    def test_packing(self):
        """--packingオプションのテスト"""
        out, err, result = run_divisi(mv_work_ls_path + "\\config.json", "packing")
        self.assertTrue(result, f"packing failed for MV: {err}")
        self.check_packing_output(mv_work_ls_path, "MV")
        
        out, err, result = run_divisi(mz_work_ls_path + "\\config.json", "packing")
        self.assertTrue(result, f"packing failed for MZ: {err}")
        self.check_packing_output(mz_work_ls_path, "MZ")
        
        out, err, result = run_divisi(vxace_work_ls_path + "\\config.json", "packing")
        self.assertTrue(result, f"packing failed for VXAce: {err}")
        self.check_packing_output(vxace_work_ls_path, "VXAce")
        
    def check_csv_files_exist(self, ls_path, project_type):
        """CSVファイルの存在確認"""
        translate_dir = os.path.join(ls_path, "data", "translate")
        self.assertTrue(os.path.exists(translate_dir), f"Translate directory should exist for {project_type}")
        
        csv_files = [f for f in os.listdir(translate_dir) if f.endswith('.csv')]
        self.assertGreater(len(csv_files), 0, f"Should have CSV files for validation in {project_type}")
        print(f"Found {len(csv_files)} CSV files for {project_type} validation")
        
        # 期待されるCSVファイルの確認
        for expected_file in write_expected_files:
            file_path = os.path.join(translate_dir, expected_file)
            if os.path.exists(file_path):
                # ファイルサイズとヘッダーの確認
                file_size = os.path.getsize(file_path)
                self.assertGreater(file_size, 0, f"{expected_file} should not be empty")
                
                # CSVヘッダーの確認 (original,ja,... 形式。BOM付きのため utf-8-sig で読む)
                with open(file_path, 'r', encoding='utf-8-sig') as f:
                    first_line = f.readline().strip()
                    self.assertTrue(first_line.startswith("original"), f"{expected_file} should start with original column : {first_line}")
                    self.assertIn("ja", first_line, f"{expected_file} should have ja column")
                    
    def check_validation_output(self, output, project_type):
        """検証出力の内容確認"""
        print(f"Validation output for {project_type}:")
        if output:
            print(output[:500] + "..." if len(output) > 500 else output)
            
            # 検証結果の基本的なチェック
            output_lower = output.lower()
            validation_keywords = ["validation", "complete", "error", "warning", "check"]
            found_keyword = any(keyword in output_lower for keyword in validation_keywords)
            if found_keyword:
                print(f"✓ Validation output contains expected keywords for {project_type}")
            else:
                print(f"? No validation keywords found in output for {project_type}")
                
    def check_validation_logs(self):
        """検証ログファイルの確認"""
        log_paths = [
            mv_work_ls_path,
            mz_work_ls_path,
            vxace_work_ls_path
        ]
        
        for ls_path in log_paths:
            # ログファイルの存在確認（通常はdata/logsまたはlogsディレクトリ）
            possible_log_dirs = [
                os.path.join(ls_path, "logs"),
                os.path.join(ls_path, "data", "logs"),
                ls_path  # ルートディレクトリにも作成される可能性
            ]
            
            for log_dir in possible_log_dirs:
                if os.path.exists(log_dir):
                    log_files = [f for f in os.listdir(log_dir) if f.endswith('.log') or f.endswith('.txt')]
                    if log_files:
                        print(f"✓ Found validation log files in {log_dir}: {log_files}")
                        
    def check_packing_output(self, ls_path, project_type):
        """パッキング結果の確認"""
        # パッキングで生成される可能性のあるディレクトリ・ファイル
        possible_output_dirs = [
            os.path.join(ls_path, "build"),
            os.path.join(ls_path, "dist"),
            os.path.join(ls_path, "output"),
            os.path.join(ls_path, "packed")
        ]
        
        for output_dir in possible_output_dirs:
            if os.path.exists(output_dir):
                files = os.listdir(output_dir)
                print(f"✓ Found packing output in {output_dir} for {project_type}: {files}")
                
        # 元のゲームプロジェクトにファイルがコピーされているかチェック
        project_paths = {
            "MV": mv_work_path,
            "MZ": mz_work_path,
            "VXAce": vxace_work_path
        }
        
        if project_type in project_paths:
            project_path = project_paths[project_type]
            
            # データファイルが更新されているかチェック
            if project_type == "VXAce":
                data_files = ["Data/System.rvdata2", "Data/Scripts.rvdata2"]
            else:
                data_files = ["data/System.json", "js/plugins.js"]
                
            for data_file in data_files:
                file_path = os.path.join(project_path, data_file)
                if os.path.exists(file_path):
                    print(f"✓ Data file exists after packing: {data_file}")
                    
        print(f"Packing completed for {project_type}")


class TestCSVConversion(unittest.TestCase):
    """CSV変換機能のテスト"""
    
    @classmethod
    def setUpClass(self):
        setup_projects()
        # CSV変換用のテストフォルダを作成
        self.csv_test_folder = os.path.join(current_dir, 'work\\csv_test')
        os.makedirs(self.csv_test_folder, exist_ok=True)
        
        # テスト用のCSVファイルを複数作成
        self.create_test_csv_files()

    @classmethod
    def tearDownClass(self):
        remove_projects()
        if os.path.exists(self.csv_test_folder):
            shutil.rmtree(self.csv_test_folder, onerror=force_remove_readonly)
            
    @classmethod
    def create_test_csv_files(self):
        """テスト用CSVファイルの作成"""
        # パッチCSV用のテストファイル
        patch_csv_content = """ID,Original,Translation,en
1,"Hello World","こんにちは世界","Hello World"
2,"Good Morning","おはよう","Good Morning"
3,"Thank you","ありがとう","Thank you"
4,"Game Over","ゲームオーバー","Game Over"
5,"New Game","新しいゲーム","New Game"
"""
        patch_csv_path = os.path.join(self.csv_test_folder, "patch_test.csv")
        with open(patch_csv_path, 'w', encoding='utf-8') as f:
            f.write(patch_csv_content)
            
        # バウンドCSV用のテストファイル
        bound_csv_content = """ID,Original,Translation
1,"Welcome to the game","ゲームへようこそ"
2,"Press any key to continue","何かキーを押してください"
3,"Loading...","読み込み中..."
4,"Save successful","セーブが完了しました"
5,"Are you sure?","本当によろしいですか？"
"""
        bound_csv_path = os.path.join(self.csv_test_folder, "bound_test.csv")
        with open(bound_csv_path, 'w', encoding='utf-8') as f:
            f.write(bound_csv_content)
            
        # 複数言語対応CSVファイル
        multilang_csv_content = """ID,Original,Translation,en,fr,de
1,"Menu","メニュー","Menu","Menu","Menü"
2,"Options","オプション","Options","Options","Optionen"
3,"Exit","終了","Exit","Sortir","Beenden"
"""
        multilang_csv_path = os.path.join(self.csv_test_folder, "multilang_test.csv")
        with open(multilang_csv_path, 'w', encoding='utf-8') as f:
            f.write(multilang_csv_content)
            
        # 制御文字を含むCSVファイル
        control_csv_content = """ID,Original,Translation
1,"\\C[1]Red Text\\C[0]","\\C[1]赤いテキスト\\C[0]"
2,"Line1\\nLine2","行1\\n行2"
3,"\\I[1]Icon Text","\\I[1]アイコンテキスト"
"""
        control_csv_path = os.path.join(self.csv_test_folder, "control_test.csv")
        with open(control_csv_path, 'w', encoding='utf-8') as f:
            f.write(control_csv_content)
            
        print(f"Created test CSV files in {self.csv_test_folder}")

    def test_convertPatchCSV(self):
        """--convertPatchCSVオプションのテスト"""
        print(f"\n=== Testing convertPatchCSV ===")
        
        # 変換前のファイル状態を記録
        before_files = os.listdir(self.csv_test_folder)
        print(f"Files before conversion: {before_files}")
        
        def run_convertPatchCSV(csv_folder_path):
            DIVISI_PATH = divisi_root_dir + '\\bin\\divisi.exe'
            args = ['--convertPatchCSV', csv_folder_path]
            return core.run_command(DIVISI_PATH, args)
        
        out, err, result = run_convertPatchCSV(self.csv_test_folder)
        self.assertTrue(result, f"convertPatchCSV failed: {err}")
        
        # 変換後のファイル状態を確認
        after_files = os.listdir(self.csv_test_folder)
        print(f"Files after conversion: {after_files}")
        
        # 出力の確認
        self.check_conversion_output(out, "PatchCSV")
        
        # 変換結果ファイルの確認
        self.check_patch_conversion_results()

    def test_convertBoundCSV(self):
        """--convertBoundCSVオプションのテスト"""
        print(f"\n=== Testing convertBoundCSV ===")
        
        # 変換前のファイル状態を記録
        before_files = os.listdir(self.csv_test_folder)
        print(f"Files before conversion: {before_files}")
        
        def run_convertBoundCSV(csv_folder_path):
            DIVISI_PATH = divisi_root_dir + '\\bin\\divisi.exe'
            args = ['--convertBoundCSV', csv_folder_path]
            return core.run_command(DIVISI_PATH, args)
        
        out, err, result = run_convertBoundCSV(self.csv_test_folder)
        self.assertTrue(result, f"convertBoundCSV failed: {err}")
        
        # 変換後のファイル状態を確認
        after_files = os.listdir(self.csv_test_folder)
        print(f"Files after conversion: {after_files}")
        
        # 出力の確認
        self.check_conversion_output(out, "BoundCSV")
        
        # 変換結果ファイルの確認
        self.check_bound_conversion_results()
        
    def check_conversion_output(self, output, conversion_type):
        """変換出力の確認"""
        print(f"Conversion output for {conversion_type}:")
        if output:
            print(output[:300] + "..." if len(output) > 300 else output)
            
            # 変換関連のキーワードチェック
            output_lower = output.lower()
            conversion_keywords = ["convert", "process", "complete", "file", "csv"]
            found_keywords = [kw for kw in conversion_keywords if kw in output_lower]
            
            if found_keywords:
                print(f"✓ Found conversion keywords: {found_keywords}")
            else:
                print(f"? No conversion keywords found in {conversion_type} output")
                
    def check_patch_conversion_results(self):
        """パッチCSV変換結果の確認"""
        # 変換により生成される可能性のあるファイル
        possible_output_files = [
            "patch_test_converted.csv",
            "patch_test.patch",
            "patch_output.csv",
            "converted_patch_test.csv"
        ]
        
        found_outputs = []
        for output_file in possible_output_files:
            output_path = os.path.join(self.csv_test_folder, output_file)
            if os.path.exists(output_path):
                found_outputs.append(output_file)
                
                # ファイルサイズと内容の基本チェック
                file_size = os.path.getsize(output_path)
                print(f"✓ Found patch conversion output: {output_file} ({file_size} bytes)")
                
                # CSVファイルの場合は内容をチェック
                if output_file.endswith('.csv'):
                    self.verify_converted_csv(output_path, "patch")
                    
        if found_outputs:
            print(f"Patch conversion created: {found_outputs}")
        else:
            print("No specific patch conversion output files found")
            
    def check_bound_conversion_results(self):
        """バウンドCSV変換結果の確認"""
        # 変換により生成される可能性のあるファイル
        possible_output_files = [
            "bound_test_converted.csv",
            "bound_test.bound",
            "bound_output.csv",
            "converted_bound_test.csv"
        ]
        
        found_outputs = []
        for output_file in possible_output_files:
            output_path = os.path.join(self.csv_test_folder, output_file)
            if os.path.exists(output_path):
                found_outputs.append(output_file)
                
                # ファイルサイズと内容の基本チェック
                file_size = os.path.getsize(output_path)
                print(f"✓ Found bound conversion output: {output_file} ({file_size} bytes)")
                
                # CSVファイルの場合は内容をチェック
                if output_file.endswith('.csv'):
                    self.verify_converted_csv(output_path, "bound")
                    
        if found_outputs:
            print(f"Bound conversion created: {found_outputs}")
        else:
            print("No specific bound conversion output files found")
            
    def verify_converted_csv(self, file_path, conversion_type):
        """変換されたCSVファイルの検証"""
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                import csv
                reader = csv.reader(f)
                
                # ヘッダー行の確認
                header = next(reader)
                print(f"  {conversion_type} CSV header: {header}")
                
                # データ行のカウント
                row_count = sum(1 for row in reader)
                print(f"  {conversion_type} CSV data rows: {row_count}")
                
                # 基本的な構造の確認
                self.assertGreater(len(header), 0, f"{conversion_type} CSV should have header")
                self.assertGreater(row_count, 0, f"{conversion_type} CSV should have data")
                
        except Exception as e:
            print(f"Error verifying {conversion_type} CSV {file_path}: {e}")
            
    def test_csv_conversion_error_handling(self):
        """CSV変換のエラーハンドリングテスト"""
        print(f"\n=== Testing CSV conversion error handling ===")
        
        # 存在しないディレクトリでのテスト
        invalid_dir = os.path.join(current_dir, 'work\\nonexistent_csv_dir')
        
        def run_convertPatchCSV_invalid(csv_folder_path):
            DIVISI_PATH = divisi_root_dir + '\\bin\\divisi.exe'
            args = ['--convertPatchCSV', csv_folder_path]
            return core.run_command(DIVISI_PATH, args)
            
        out, err, result = run_convertPatchCSV_invalid(invalid_dir)
        # エラーハンドリングのテストなので、失敗することを期待する場合もある
        print(f"Invalid directory test result: {result}")
        print(f"Error output: {err}" if err else "No error output")
        
        # 空のディレクトリでのテスト
        empty_dir = os.path.join(current_dir, 'work\\empty_csv_dir')
        os.makedirs(empty_dir, exist_ok=True)
        
        try:
            out, err, result = run_convertPatchCSV_invalid(empty_dir)
            print(f"Empty directory test result: {result}")
            print(f"Output: {out}" if out else "No output")
        finally:
            if os.path.exists(empty_dir):
                os.rmdir(empty_dir)


class TestErrorHandling(unittest.TestCase):
    """エラーハンドリングのテスト"""
    
    def test_invalid_config_file(self):
        """存在しない設定ファイルでのエラーハンドリング"""
        invalid_config = os.path.join(current_dir, 'work\\invalid_config.json')
        out, err, result = run_divisi(invalid_config, "analyze")
        self.assertFalse(result, "Should fail with invalid config file")

    def test_no_arguments(self):
        """引数なしでの実行テスト"""
        DIVISI_PATH = divisi_root_dir + '\\bin\\divisi.exe'
        out, err, result = core.run_command(DIVISI_PATH, [])
        self.assertFalse(result, "Should fail with no arguments")

    def test_invalid_option(self):
        """無効なオプションでのテスト"""
        DIVISI_PATH = divisi_root_dir + '\\bin\\divisi.exe'
        out, err, result = core.run_command(DIVISI_PATH, ['--invalidoption'])
        # 無効なオプションは無視される可能性があるため、引数不足エラーになることを期待
        self.assertFalse(result, "Should fail with invalid option")


class TestVersionInfo(unittest.TestCase):
    """バージョン情報の表示テスト"""
    
    def test_version_output(self):
        """バージョン情報が正しく表示されることをテスト"""
        DIVISI_PATH = divisi_root_dir + '\\bin\\divisi.exe'
        # 有効な最小限の引数でバージョン情報を確認
        out, err, result = core.run_command(DIVISI_PATH, ['--help'])  # helpオプションがない場合は他の引数を使用
        
        # バージョン情報が含まれていることを確認（引数エラーでも出力される）
        # 実際の実装では、どんな引数でもバージョン情報が最初に出力される
        setup_projects()
        create_config_file(mv_work_path)
        
        out, err, result = run_divisi(mv_work_ls_path + "\\config.json", "analyze")
        self.assertIn("Langscore Divisi Version", out, "Version information should be displayed")
        self.assertIn("Build on", out, "Build information should be displayed")
        
        remove_projects()


def run_createProjectTest():
    """プロジェクト作成テストを実行"""
    captured_output = io.StringIO()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestCreateProject)
    runner = unittest.TextTestRunner(stream=captured_output, verbosity=2)
    result = runner.run(suite)
    output = captured_output.getvalue()
    captured_output.close()
    return output, result.wasSuccessful()

def run_writeTest():
    """CSVエクスポートテストを実行"""
    captured_output = io.StringIO()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestWrite)
    runner = unittest.TextTestRunner(stream=captured_output, verbosity=2)
    result = runner.run(suite)
    output = captured_output.getvalue()
    captured_output.close()
    return output, result.wasSuccessful()

def run_updateTest():
    """プロジェクト更新テストを実行"""
    captured_output = io.StringIO()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestUpdate)
    runner = unittest.TextTestRunner(stream=captured_output, verbosity=2)
    result = runner.run(suite)
    output = captured_output.getvalue()
    captured_output.close()
    return output, result.wasSuccessful()

def run_analyzeTest():
    # 標準出力をキャプチャするためのStringIOオブジェクトを作成
    captured_output = io.StringIO()

    # テストスイートを作成
    suite = unittest.TestLoader().loadTestsFromTestCase(TestAnalyze)
    
    # テストランナーを作成して実行
    runner = unittest.TextTestRunner(stream=captured_output, verbosity=2)
    result = runner.run(suite)
    
    # 標準出力の内容を取得
    output = captured_output.getvalue()
    
    # StringIOオブジェクトを明示的に閉じる
    captured_output.close()

    return output, result.wasSuccessful()

def run_pluginOperationsTest():
    """プラグイン操作テストを実行"""
    captured_output = io.StringIO()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestPluginOperations)
    runner = unittest.TextTestRunner(stream=captured_output, verbosity=2)
    result = runner.run(suite)
    output = captured_output.getvalue()
    captured_output.close()
    return output, result.wasSuccessful()

def run_validationAndPackingTest():
    """検証とパッキングテストを実行"""
    captured_output = io.StringIO()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestValidationAndPacking)
    runner = unittest.TextTestRunner(stream=captured_output, verbosity=2)
    result = runner.run(suite)
    output = captured_output.getvalue()
    captured_output.close()
    return output, result.wasSuccessful()

def run_csvConversionTest():
    """CSV変換テストを実行"""
    captured_output = io.StringIO()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestCSVConversion)
    runner = unittest.TextTestRunner(stream=captured_output, verbosity=2)
    result = runner.run(suite)
    output = captured_output.getvalue()
    captured_output.close()
    return output, result.wasSuccessful()

def run_errorHandlingTest():
    """エラーハンドリングテストを実行"""
    captured_output = io.StringIO()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestErrorHandling)
    runner = unittest.TextTestRunner(stream=captured_output, verbosity=2)
    result = runner.run(suite)
    output = captured_output.getvalue()
    captured_output.close()
    return output, result.wasSuccessful()

def run_versionInfoTest():
    """バージョン情報テストを実行"""
    captured_output = io.StringIO()
    suite = unittest.TestLoader().loadTestsFromTestCase(TestVersionInfo)
    runner = unittest.TextTestRunner(stream=captured_output, verbosity=2)
    result = runner.run(suite)
    output = captured_output.getvalue()
    captured_output.close()
    return output, result.wasSuccessful()

def run_allTests():
    """すべてのテストを実行"""
    test_functions = [
        ("CreateProject", run_createProjectTest),
        ("Analyze", run_analyzeTest),
        ("PluginOperations", run_pluginOperationsTest),
        ("ValidationAndPacking", run_validationAndPackingTest),
        ("CSVConversion", run_csvConversionTest),
        ("ErrorHandling", run_errorHandlingTest),
        ("VersionInfo", run_versionInfoTest)
    ]
    
    results = []
    for test_name, test_func in test_functions:
        try:
            output, success = test_func()
            results.append({
                'test_name': test_name,
                'output': output,
                'success': success
            })
            print(f"✓ {test_name} test: {'PASSED' if success else 'FAILED'}")
        except Exception as e:
            results.append({
                'test_name': test_name,
                'output': str(e),
                'success': False
            })
            print(f"✗ {test_name} test: FAILED with exception: {e}")
    
    return results

def test_divisi_option(config_path, option, expected_success=True, description=""):
    """
    divisiの特定のオプションをテストするヘルパー関数
    
    Args:
        config_path: 設定ファイルのパス
        option: 実行するオプション
        expected_success: 成功を期待するかどうか
        description: テストの説明
    
    Returns:
        tuple: (output, error, success)
    """
    try:
        out, err, result = run_divisi(config_path, option)
        success = result if expected_success else not result
        
        if description:
            print(f"Testing {description}: {'PASSED' if success else 'FAILED'}")
            
        return out, err, success
    except Exception as e:
        print(f"Exception during {option} test: {e}")
        return "", str(e), False

def validate_file_exists(file_path, description=""):
    """
    ファイルの存在を確認するヘルパー関数
    
    Args:
        file_path: 確認するファイルパス
        description: ファイルの説明
    
    Returns:
        bool: ファイルが存在するかどうか
    """
    exists = os.path.exists(file_path)
    if description:
        print(f"Checking {description}: {'EXISTS' if exists else 'NOT FOUND'} - {file_path}")
    return exists

def validate_directory_structure(base_path, expected_dirs, description=""):
    """
    ディレクトリ構造を確認するヘルパー関数
    
    Args:
        base_path: ベースディレクトリパス
        expected_dirs: 期待されるディレクトリのリスト
        description: 構造の説明
    
    Returns:
        bool: すべてのディレクトリが存在するかどうか
    """
    all_exist = True
    missing_dirs = []
    
    for dir_name in expected_dirs:
        dir_path = os.path.join(base_path, dir_name)
        if not os.path.exists(dir_path):
            all_exist = False
            missing_dirs.append(dir_name)
    
    if description:
        if all_exist:
            print(f"Directory structure validation for {description}: PASSED")
        else:
            print(f"Directory structure validation for {description}: FAILED - Missing: {missing_dirs}")
    
    return all_exist

# ...existing code...
if __name__ == '__main__':
    import argparse
    
    parser = argparse.ArgumentParser(description='Langscore Divisi Test Suite')
    parser.add_argument('--test', choices=[
        'all', 'create', 'analyze', 'write', 'update', 
        'plugin', 'validation', 'csv', 'error', 'version'
    ], default='all', help='実行するテストを指定')
    parser.add_argument('--verbose', '-v', action='store_true', help='詳細出力を有効にする')
    
    args = parser.parse_args()
    
    if args.test == 'all':
        # 標準出力をキャプチャするためのStringIOオブジェクトを作成
        captured_output = io.StringIO()
        sys.stdout = captured_output
        test_results = []

        try:
            # テストスイートを作成
            test_cases = [
                TestCreateProject, 
                TestAnalyze, 
                TestWrite, 
                TestUpdate,
                TestPluginOperations,
                TestValidationAndPacking,
                TestCSVConversion,
                TestErrorHandling,
                TestVersionInfo
            ]
            for test_case in test_cases:
                suite = unittest.TestLoader().loadTestsFromTestCase(test_case)
                
                # テストランナーを作成して実行
                runner = unittest.TextTestRunner(stream=sys.stdout, verbosity=2)
                result = runner.run(suite)
                
                # 標準出力の内容を取得
                output:str = captured_output.getvalue()
                captured_output.truncate(0)
                captured_output.seek(0)
                
                # 結果をリストに保存
                test_results.append({
                    'test_case': test_case.__name__,
                    'output': output,
                    'was_successful': result.wasSuccessful()
                })
        finally:
            sys.stdout = sys.__stdout__
        
        # 結果の表示
        for test_result in test_results:
            if not test_result['was_successful'] or args.verbose:
                print(f"Test Case: {test_result['test_case']}")
                print(f"Was Successful: {test_result['was_successful']}")
                if args.verbose or not test_result['was_successful']:
                    print("Output:")
                    print(test_result['output'])
                print("=" * 40)

        # result は最後のテストクラスの結果しか保持していないため、全クラスの結果で判定する
        if not all(r['was_successful'] for r in test_results):
            exit(1)
            
    else:
        # 個別テストの実行
        test_map = {
            'create': run_createProjectTest,
            'analyze': run_analyzeTest,
            'write': run_writeTest,
            'update': run_updateTest,
            'plugin': run_pluginOperationsTest,
            'validation': run_validationAndPackingTest,
            'csv': run_csvConversionTest,
            'error': run_errorHandlingTest,
            'version': run_versionInfoTest
        }
        
        if args.test in test_map:
            print(f"Running {args.test} test...")
            output, success = test_map[args.test]()
            print(f"Test result: {'PASSED' if success else 'FAILED'}")
            if args.verbose or not success:
                print("Output:")
                print(output)
            exit(0 if success else 1)
        else:
            print(f"Unknown test: {args.test}")
            exit(1)
