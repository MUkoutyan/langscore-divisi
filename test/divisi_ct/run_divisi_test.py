import subprocess
import io
import os
import re
import sys
import shutil
import stat
import unittest
import pprint

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

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

mv_work_path            = os.path.abspath('.\\work\\LangscoreTestMV')
mz_work_path            = os.path.abspath('.\\work\\LangscoreTestMZ')
vxace_work_path         = os.path.abspath('.\\work\\LangscoreTestVXAce')
include_empty_work_path = os.path.abspath('.\\work\\Include WhiteSpacePath Project')
mv_work_ls_path            = os.path.abspath('.\\work\\LangscoreTestMV_langscore')
mz_work_ls_path            = os.path.abspath('.\\work\\LangscoreTestMZ_langscore')
vxace_work_ls_path         = os.path.abspath('.\\work\\LangscoreTestVXAce_langscore')
include_empty_work_ls_path = os.path.abspath('.\\work\\Include WhiteSpacePath Project_langscore')


source_mv_work_path            = os.path.abspath('..\\data\\LangscoreTestMV')
source_mz_work_path            = os.path.abspath('..\\data\\LangscoreTestMZ')
source_vxace_work_path         = os.path.abspath('..\\data\\LangscoreTestVXAce')
source_include_empty_work_path = os.path.abspath('..\\data\\vxace\\Include WhiteSpacePath Project')
source_update_mv_work_path            = os.path.abspath('..\\data\\LangscoreTestMV_updated')
source_update_mz_work_path            = os.path.abspath('..\\data\\LangscoreTestMZ_updated')
source_update_vxace_work_path         = os.path.abspath('..\\data\\LangscoreTestVXAce_updated')
source_update_include_empty_work_path = os.path.abspath('..\\data\\vxace\\Include WhiteSpacePath Project_updated')

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

class TestCreateProject(unittest.TestCase):
    @classmethod
    def setUpClass(self):
        
        if os.path.exists(mv_work_path):
            shutil.rmtree(mv_work_path)
        if os.path.exists(mz_work_path):
            shutil.rmtree(mz_work_path)
        if os.path.exists(vxace_work_path):
            shutil.rmtree(vxace_work_path)
        if os.path.exists(include_empty_work_path):
            shutil.rmtree(include_empty_work_path)


        copy_folder('..\\data\\LangscoreTestMV', mv_work_path)
        copy_folder('..\\data\\LangscoreTestMZ', mz_work_path)
        copy_folder('..\\data\\LangscoreTestVXAce', vxace_work_path)
        copy_folder('..\\data\\vxace\\Include WhiteSpacePath Project', include_empty_work_path)

        print("copy projects")

    @classmethod
    def tearDownClass(self):
        shutil.rmtree(mv_work_path)
        shutil.rmtree(mz_work_path)
        shutil.rmtree(vxace_work_path)
        shutil.rmtree(include_empty_work_path)
        shutil.rmtree(mv_work_ls_path)
        shutil.rmtree(mz_work_ls_path)
        shutil.rmtree(vxace_work_ls_path)
        shutil.rmtree(include_empty_work_ls_path)
        print("remove projects")

    def setUp(self):
        # 各テストの前に実行されるセットアップ
        pass

    def tearDown(self):
        pass

    def test_createConfig(self):
        run_divisi(mv_work_path, "create")
        self.assertTrue(os.path.exists(mv_work_ls_path))
        self.assertTrue(os.path.exists(mv_work_ls_path + "\\config.json"))
        run_divisi(mz_work_path, "create")
        self.assertTrue(os.path.exists(mz_work_ls_path))
        self.assertTrue(os.path.exists(mz_work_ls_path + "\\config.json"))
        run_divisi(vxace_work_path, "create")
        self.assertTrue(os.path.exists(vxace_work_ls_path))
        self.assertTrue(os.path.exists(vxace_work_ls_path + "\\config.json"))
        run_divisi(include_empty_work_path, "create")
        self.assertTrue(os.path.exists(include_empty_work_ls_path))
        self.assertTrue(os.path.exists(include_empty_work_ls_path + "\\config.json"))

class TestAnalyzeAndPack(unittest.TestCase):

    @classmethod
    def setUpClass(self):
        
        if os.path.exists(mv_work_path):
            shutil.rmtree(mv_work_path)
        if os.path.exists(mz_work_path):
            shutil.rmtree(mz_work_path)
        if os.path.exists(vxace_work_path):
            shutil.rmtree(vxace_work_path)
        if os.path.exists(include_empty_work_path):
            shutil.rmtree(include_empty_work_path)


        copy_folder('..\\data\\LangscoreTestMV', mv_work_path)
        copy_folder('..\\data\\LangscoreTestMZ', mz_work_path)
        copy_folder('..\\data\\LangscoreTestVXAce', vxace_work_path)
        copy_folder('..\\data\\vxace\\Include WhiteSpacePath Project', include_empty_work_path)

        run_divisi(mv_work_path, "create")
        run_divisi(mz_work_path, "create")
        run_divisi(vxace_work_path, "create")
        run_divisi(include_empty_work_path, "create")
        print("copy projects")

    @classmethod
    def tearDownClass(self):
        # shutil.rmtree(mv_work_path)
        # shutil.rmtree(mz_work_path)
        # shutil.rmtree(vxace_work_path)
        # shutil.rmtree(include_empty_work_path)
        # shutil.rmtree(mv_work_ls_path)
        # shutil.rmtree(mz_work_ls_path)
        # shutil.rmtree(vxace_work_ls_path)
        # shutil.rmtree(include_empty_work_ls_path)
        print("remove projects")

    def setUp(self):
        # 各テストの前に実行されるセットアップ
        pass

    def tearDown(self):
        pass

    def test_analyze(self):
        run_divisi(mv_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(os.path.exists(mv_work_ls_path + "\\analyze"))
        for file in analyze_expected_files:
            file_path = os.path.join(mv_work_ls_path + "\\analyze", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {mv_work_ls_path}")

        run_divisi(mz_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(os.path.exists(mz_work_ls_path + "\\analyze"))
        for file in analyze_expected_files:
            file_path = os.path.join(mz_work_ls_path + "\\analyze", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {mz_work_ls_path}")

        run_divisi(vxace_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(os.path.exists(vxace_work_ls_path + "\\analyze"))
        for file in analyze_expected_files:
            file_path = os.path.join(vxace_work_ls_path + "\\analyze", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {vxace_work_ls_path}")

        run_divisi(include_empty_work_ls_path + "\\config.json", "analyze")
        self.assertTrue(os.path.exists(include_empty_work_ls_path + "\\analyze"))
        for file in analyze_expected_files:
            file_path = os.path.join(include_empty_work_ls_path + "\\analyze", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {include_empty_work_ls_path}")

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


    def test_update(self):
        check_keys = [
            "通常のテキストです2", "改行を含む\nテキストです2", 
            "\"タ\"フ\"ルクォーテーションを含むテキストです2\"",
            "\"\"\"Hello, World\"\",\nそれはプログラムを書く際の\",\"\"\"謎の呪文\"\"(Mystery spell)―――2\"",
            "複合させます\n\\{\\C[2]\\N[2]\\I[22]\n2"
        ]
        if os.path.exists(mv_work_path):
            shutil.rmtree(mv_work_path)
        copy_folder(source_update_mv_work_path, mv_work_path)
        run_divisi(mv_work_ls_path + "\\config.json", "update")
        self.assertTrue(os.path.exists(mv_work_ls_path + "\\update"))
        for file in analyze_expected_files:
            file_path = os.path.join(mv_work_ls_path + "\\data\\translate", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {mv_work_ls_path}")
        
        with open(mv_work_ls_path + "\\update\\Map001.csv", 'r', encoding='utf-8') as csv_file:
            contents = lscsv.LSCSV.to_map(csv_file.read())
            for key in check_keys:
                self.assertIn(key, contents, f"not valid \"updated\" csv {mv_work_ls_path}")

        if os.path.exists(mz_work_path):
            shutil.rmtree(mz_work_path)
        copy_folder(source_update_mz_work_path, mz_work_path)
        run_divisi(mz_work_ls_path + "\\config.json", "update")
        self.assertTrue(os.path.exists(mz_work_ls_path + "\\update"))
        for file in analyze_expected_files:
            file_path = os.path.join(mz_work_ls_path + "\\data\\translate", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {mz_work_ls_path}")

        with open(mz_work_ls_path + "\\update\\Map001.csv", 'r', encoding='utf-8') as csv_file:
            contents = lscsv.LSCSV.to_map(csv_file.read())
            for key in check_keys:
                self.assertIn(key, contents, f"not valid \"updated\" csv {mz_work_ls_path}")

        if os.path.exists(vxace_work_path):
            shutil.rmtree(vxace_work_path)
        copy_folder(source_update_vxace_work_path, vxace_work_path)
        actual_files = os.listdir(os.path.join(vxace_work_ls_path, "Data", "Translate"))
        run_divisi(vxace_work_ls_path + "\\config.json", "update")
        self.assertTrue(os.path.exists(vxace_work_ls_path + "\\update"))
        for file in analyze_expected_files:
            self.assertIn(file, actual_files, f"{file} does not exist in {vxace_work_ls_path}")

        with open(vxace_work_ls_path + "\\update\\Map001.csv", 'r', encoding='utf-8') as csv_file:
            contents = lscsv.LSCSV.to_map(csv_file.read())
            for key in check_keys:
                self.assertIn(key, contents, f"not valid \"updated\" csv {vxace_work_ls_path}")

        if os.path.exists(include_empty_work_path):
            shutil.rmtree(include_empty_work_path)
        copy_folder(source_update_include_empty_work_path, include_empty_work_path)
        run_divisi(include_empty_work_ls_path + "\\config.json", "update")
        self.assertTrue(os.path.exists(include_empty_work_ls_path + "\\update"))
        for file in analyze_expected_files:
            file_path = os.path.join(include_empty_work_ls_path + "\\update", file)
            self.assertTrue(os.path.exists(file_path), f"{file} does not exist in {include_empty_work_ls_path}")

        with open(include_empty_work_ls_path + "\\update\\Map001.csv", 'r', encoding='utf-8') as csv_file:
            contents = lscsv.LSCSV.to_map(csv_file.read())
            for key in check_keys:
                self.assertIn(key, contents, f"not valid \"updated\" csv {include_empty_work_ls_path}")


        

def run_createProjectTest():
    # 標準出力をキャプチャするためのStringIOオブジェクトを作成
    captured_output = io.StringIO()
    sys.stdout = captured_output

    try:
        # テストスイートを作成
        suite = unittest.TestLoader().loadTestsFromTestCase(TestCreateProject)
        
        # テストランナーを作成して実行
        runner = unittest.TextTestRunner(stream=sys.stdout, verbosity=2)
        result = runner.run(suite)
        
        # 標準出力の内容を取得
        output = captured_output.getvalue()
    finally:
        # 標準出力を元に戻す
        sys.stdout = sys.__stdout__

    return output, result.wasSuccessful()

def run_analyzeTest():
    # 標準出力をキャプチャするためのStringIOオブジェクトを作成
    captured_output = io.StringIO()

    # テストスイートを作成
    suite = unittest.TestLoader().loadTestsFromTestCase(TestAnalyzeAndPack)
    
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
    output, success = run_analyzeTest()
    print(output)
