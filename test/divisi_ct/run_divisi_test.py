import subprocess
import io
import os
import re
import sys
import shutil
import stat
import unittest
import pprint

current_dir = os.path.dirname(os.path.abspath(__file__))
# rootディレクトリのパスを取得してsys.pathに追加
test_root_dir = os.path.abspath(os.path.join(current_dir, '../'))
sys.path.append(test_root_dir)
divisi_root_dir = os.path.abspath(os.path.join(current_dir, '../../'))
sys.path.append(divisi_root_dir)
from internal import test_core as core

from resource import lscsv

def copy_folder(src, dest):
    ret = shutil.copytree(src, dest)
    core.remove_read_only(dest)
    return ret

def run_divisi(project_path, option):
    DIVISI_PATH = '..\\..\\bin\\divisi.exe'
    if option == "create":
        args = [
            '--createConfigFile', project_path,
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


source_mv_work_path            = os.path.join(test_root_dir, 'data\\LangscoreTestMV')
source_mz_work_path            = os.path.join(test_root_dir, 'data\\LangscoreTestMZ')
source_vxace_work_path         = os.path.join(test_root_dir, 'data\\LangscoreTestVXAce')
source_include_empty_work_path = os.path.join(test_root_dir, 'data\\vxace\\Include WhiteSpacePath Project')
source_update_mv_work_path            = os.path.join(test_root_dir, 'data\\LangscoreTestMV_updated')
source_update_mz_work_path            = os.path.join(test_root_dir, 'data\\LangscoreTestMZ_updated')
source_update_vxace_work_path         = os.path.join(test_root_dir, 'data\\LangscoreTestVXAce_updated')
source_update_include_empty_work_path = os.path.join(test_root_dir, 'data\\vxace\\Include WhiteSpacePath Project_updated')

analyze_expected_files = [
    "System.csv","Troops.csv","Weapons.csv","Actors.csv",
    "Armors.csv","Classes.csv","CommonEvents.csv",
    "Enemies.csv","Items.csv","Map001.csv","Map002.csv","Map003.csv",
    "Skills.csv","States.csv"
]
write_expected_files = [
    "System.csv","Troops.csv","Weapons.csv","Actors.csv",
    "Armors.csv","Classes.csv","CommonEvents.csv",
    "Enemies.csv","Items.csv","Map001.csv","Map002.csv","Map003.csv",
    "Skills.csv","States.csv"
]

def setup_projects():
    if os.path.exists(mv_work_path):
        shutil.rmtree(mv_work_path)
    if os.path.exists(mz_work_path):
        shutil.rmtree(mz_work_path)
    if os.path.exists(vxace_work_path):
        shutil.rmtree(vxace_work_path)
    if os.path.exists(include_empty_work_path):
        shutil.rmtree(include_empty_work_path)

    copy_folder(source_mv_work_path, mv_work_path)
    copy_folder(source_mz_work_path, mz_work_path)
    copy_folder(source_vxace_work_path, vxace_work_path)
    copy_folder(source_include_empty_work_path, include_empty_work_path)
    print("copy projects")

def remove_projects():
    if os.path.exists(mv_work_path):
        shutil.rmtree(mv_work_path)
    if os.path.exists(mz_work_path):
        shutil.rmtree(mz_work_path)
    if os.path.exists(vxace_work_path):
        shutil.rmtree(vxace_work_path)
    if os.path.exists(include_empty_work_path):
        shutil.rmtree(include_empty_work_path)
    if os.path.exists(mv_work_ls_path):
        shutil.rmtree(mv_work_ls_path)
    if os.path.exists(mz_work_ls_path):
        shutil.rmtree(mz_work_ls_path)
    if os.path.exists(vxace_work_ls_path):
        shutil.rmtree(vxace_work_ls_path)
    if os.path.exists(include_empty_work_ls_path):
        shutil.rmtree(include_empty_work_ls_path)
    print("remove projects")

def edit_config_file(data):
    data['DefaultLanguage'] = "ja"

    for language in data['Languages']:
        if language['LanguageName'] == 'en':
            language['Enable'] = True

def create_config_file(work_path):
    run_divisi(work_path, "create")
    core.edit_ls_config(work_path+"_langscore\config.json", edit_config_file)


class TestCreateProject(unittest.TestCase):

    @classmethod
    def setUpClass(self):
        setup_projects()

    @classmethod
    def tearDownClass(self):
        remove_projects()

    def test_createConfig(self):
        create_config_file(mv_work_path)
        self.assertTrue(os.path.exists(mv_work_ls_path))
        self.assertTrue(os.path.exists(mv_work_ls_path + "\\config.json"))
        create_config_file(mz_work_path)
        self.assertTrue(os.path.exists(mz_work_ls_path))
        self.assertTrue(os.path.exists(mz_work_ls_path + "\\config.json"))
        create_config_file(vxace_work_path)
        self.assertTrue(os.path.exists(vxace_work_ls_path))
        self.assertTrue(os.path.exists(vxace_work_ls_path + "\\config.json"))
        create_config_file(include_empty_work_path)
        self.assertTrue(os.path.exists(include_empty_work_ls_path))
        self.assertTrue(os.path.exists(include_empty_work_ls_path + "\\config.json"))

        
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
        run_divisi(mv_work_ls_path + "\\config.json", "write")
        self.assertTrue(os.path.exists(mv_work_ls_path + "\\data\\translate"))
        for file in write_expected_files:
            file_path = os.path.join(mv_work_ls_path + "\\data\\translate", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {mv_work_ls_path}")

        run_divisi(mz_work_ls_path + "\\config.json", "write")
        self.assertTrue(os.path.exists(mz_work_ls_path + "\\data\\translate"))
        for file in write_expected_files:
            file_path = os.path.join(mz_work_ls_path + "\\data\\translate", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {mz_work_ls_path}")

        run_divisi(vxace_work_ls_path + "\\config.json", "write")
        actual_files = os.listdir(os.path.join(vxace_work_ls_path, "Data", "Translate"))
        self.assertTrue(os.path.exists(vxace_work_ls_path + "\\Data\\Translate"))
        for file in write_expected_files:
            self.assertIn(file, actual_files, f"{file} does not exist in {vxace_work_ls_path}")

        run_divisi(include_empty_work_ls_path + "\\config.json", "write")
        self.assertTrue(os.path.exists(include_empty_work_ls_path + "\\data\\translate"))
        for file in write_expected_files:
            file_path = os.path.join(include_empty_work_ls_path + "\\data\\translate", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {include_empty_work_ls_path}")


    
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
        
        run_divisi(mv_work_ls_path + "\\config.json", "write")
        run_divisi(mz_work_ls_path + "\\config.json", "write")
        run_divisi(vxace_work_ls_path + "\\config.json", "write")
        run_divisi(include_empty_work_ls_path + "\\config.json", "write")

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
            if os.path.exists(work_path):
                shutil.rmtree(work_path)
            copy_folder(source_path, work_path)
            run_divisi(work_ls_path + "\\config.json", "update")
            self.assertTrue(os.path.exists(work_ls_path + "\\update"))
            for file in analyze_expected_files:
                file_path = os.path.join(work_ls_path + "\\data\\translate", file)
                self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {work_ls_path}")
            
            with open(work_ls_path + "\\update\\Map001.csv", 'r', encoding='utf-8') as csv_file:
                contents = lscsv.LSCSV.to_map(csv_file.read())
                for key in check_keys:
                    self.assertIn(key, contents, f"not valid \"updated\" csv {work_ls_path}")
            
        check_mvmz_function(mv_work_path, mv_work_ls_path, source_update_mv_work_path)
        check_mvmz_function(mz_work_path, mz_work_ls_path, source_update_mz_work_path)
        
        def check_vxace_function(work_path, work_ls_path, source_path):
            if os.path.exists(work_path):
                shutil.rmtree(work_path)
            copy_folder(source_path, work_path)
            actual_files = os.listdir(os.path.join(work_ls_path, "Data", "Translate"))
            run_divisi(work_ls_path + "\\config.json", "update")
            self.assertTrue(os.path.exists(work_ls_path + "\\update"))
            for file in analyze_expected_files:
                self.assertIn(file, actual_files, f"{file} does not exist in {work_ls_path}")

            with open(work_ls_path + "\\update\\Map001.csv", 'r', encoding='utf-8') as csv_file:
                contents = lscsv.LSCSV.to_map(csv_file.read())
                for key in check_keys:
                    self.assertIn(key, contents, f"not valid \"updated\" csv {work_ls_path}")

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
        self.assertTrue(os.path.exists(mv_work_ls_path + "\\analyze"))
        for file in analyze_expected_files:
            file_path = os.path.join(mv_work_ls_path + "\\analyze", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {mv_work_ls_path}")

        out, err, result = run_divisi(mz_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(result, err)
        self.assertTrue(os.path.exists(mz_work_ls_path + "\\analyze"))
        for file in analyze_expected_files:
            file_path = os.path.join(mz_work_ls_path + "\\analyze", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {mz_work_ls_path}")

        out, err, result = run_divisi(vxace_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(result, err)
        self.assertTrue(os.path.exists(vxace_work_ls_path + "\\analyze"))
        for file in analyze_expected_files:
            file_path = os.path.join(vxace_work_ls_path + "\\analyze", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {vxace_work_ls_path}")

        out, err, result = run_divisi(include_empty_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(result, err)
        self.assertTrue(os.path.exists(include_empty_work_ls_path + "\\analyze"))
        for file in analyze_expected_files:
            file_path = os.path.join(include_empty_work_ls_path + "\\analyze", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {include_empty_work_ls_path}")

def run_createProjectTest():


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

# 実行例
if __name__ == '__main__':
    # 標準出力をキャプチャするためのStringIOオブジェクトを作成
    captured_output = io.StringIO()
    sys.stdout = captured_output
    test_results = []

    try:
        # テストスイートを作成
        test_cases = [TestCreateProject, TestAnalyze, TestWrite, TestUpdate]
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
    
    for test_result in test_results:
        if not test_result['was_successful']:
            print(f"Test Case: {test_result['test_case']}")
            print(f"Was Successful: {test_result['was_successful']}")
            print("Output:")
            print(test_result['output'])
            print("=" * 40)

    if not result.wasSuccessful():
        exit(1)
