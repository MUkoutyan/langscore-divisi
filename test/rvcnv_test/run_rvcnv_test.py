import subprocess
import shutil
import os
import csv
import sys
import io
import unittest

sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

current_file = os.path.abspath(__file__)
current_dir = os.path.dirname(current_file)
test_root_dir = os.path.abspath(os.path.dirname(os.path.join(current_dir, "../")))
# rootディレクトリのパスを取得してsys.pathに追加
sys.path.append(test_root_dir)
from internal import test_core as core

RVCNV_PATH = os.path.abspath(f"{test_root_dir}/../rvcnv/rvcnv.rb")

def copy_folder(src, dest):
    ret = shutil.copytree(src, dest)
    core.remove_read_only(dest)
    return ret


is_delete_files = True
def run_command(args):
    if ".exe" in RVCNV_PATH:
        out, err, code = core.run_command(RVCNV_PATH, args)
    elif ".rb" in RVCNV_PATH:
        out, err, code = core.run_ruby_script(RVCNV_PATH, args)
        
    if code == False:
        print(f"Error executing command: {RVCNV_PATH}, {args}")
        print(f"Standard Output:\n{out}")
        print(f"Standard Error:\n{err}")
        is_delete_files = False
        sys.exit(1)
    return out, err, code


def convert_rvdata2_to_csv(rvdata2_path, csv_path):
    ruby_script = f"{current_dir}\\extract_rvdata2.rb"
    args = [rvdata2_path, csv_path]
    out, err, code = core.run_ruby_script(ruby_script, args=args)
    if code == False:
        print(f"Error executing command: ruby {args}")
        print(f"Standard Output:\n{out}")
        print(f"Standard Error:\n{err}")
        is_delete_files = False
        sys.exit(1)
    return out, err, code

    
def extract_script_data(project_path, output_path):
    ruby_script = f"{current_dir}\\extract_script.rb"
    args = [ruby_script, project_path, output_path]
    out, err, code = core.run_ruby_script(ruby_script, args=args)
    if code == False:
        print(f"Error executing command: ruby {args}")
        print(f"Standard Output:\n{out}")
        print(f"Standard Error:\n{err}")
        is_delete_files = False
        sys.exit(1)
    return out, err, code

def normalize_newlines(text, newline_type):
    return text.replace("\n", "").replace("\r", "")

def validate_newlines_in_csv(file_path):
    is_map = "Map" in file_path
    with open(file_path, newline='', encoding='utf-8') as f:
        reader = csv.reader(f)
        for row in reader:
            for cell in row:
                if is_map and "\r\n" in cell:
                    return False
                if not is_map and "\n" in cell and "\r\n" not in cell:
                    return False
    return True

def compare_csv_files(file1, file2):
    with open(file1, newline='', encoding='utf-8') as f1, open(file2, newline='', encoding='utf-8') as f2:
        reader1 = csv.reader(f1)
        reader2 = csv.reader(f2)
        
        for row1, row2 in zip(reader1, reader2):
            if "Map" in file1 or "Map" in file2:
                row1 = [normalize_newlines(cell, "Map") for cell in row1]
                row2 = [normalize_newlines(cell, "Map") for cell in row2]
            else:
                row1 = [normalize_newlines(cell, "Normal") for cell in row1]
                row2 = [normalize_newlines(cell, "Normal") for cell in row2]
            
            if row1 != row2:
                print(f"!=\n{row1}\n {row2}")
                return False

        # Check if both files have the same number of rows
        for _ in reader1:
            print(f"reader1 {reader1}")
            return False
        for _ in reader2:
            print(f"reader2 {reader2}")
            return False

    return True

def check_files_in_directory(directory, filename_contains):
    """
    [使用例]
    directory = "./path/to/directory"  # チェックするディレクトリのパスを指定
    filename_contains = "example"  # ファイル名に含まれるべき文字列を指定
    check_files_in_directory(directory, filename_contains)
    """
    try:
        files = os.listdir(directory)
        for file in files:
            if filename_contains in file:
                file_path = os.path.join(directory, file)
                file_size = os.path.getsize(file_path)
                if file_size <= 0:
                    print(f"{file} found but empty.")
                    return False
                return True
        print(f"No files containing '{filename_contains}' were found in {directory}")
        return False
    except Exception as e:
        print(f"An error occurred: {e}")
        return False

data_project_path = f'{test_root_dir}\\data\\vxace\\ソポァゼゾタダＡボマミ'
class TestRVCNV(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # テスト用のディレクトリを作成して必要なファイルを配置するなどの準備を行う
        # ここにテストに必要なファイルの作成コードを追加
        if os.path.exists(f"{current_dir}\\test_analyze"):
            shutil.rmtree(f"{current_dir}\\test_analyze")
        if os.path.exists(f"{current_dir}\\test_compress"):
            shutil.rmtree(f"{current_dir}\\test_compress")
        if os.path.exists(f"{current_dir}\\test_packing"):
            shutil.rmtree(f"{current_dir}\\test_packing")

    @classmethod
    def tearDownClass(cls):
        # テスト用に作成したファイルやディレクトリを削除する
        if is_delete_files:
            if os.path.exists(f"{current_dir}\\test_analyze"):
                shutil.rmtree(f"{current_dir}\\test_analyze")
            if os.path.exists(f"{current_dir}\\test_compress"):
                shutil.rmtree(f"{current_dir}\\test_compress")
            if os.path.exists(f"{current_dir}\\test_packing"):
                shutil.rmtree(f"{current_dir}\\test_packing")
        pass

    def setUp(self):
        # 各テストの前に実行されるセットアップ
        pass

    def tearDown(self):
        pass

    def test_analyze(self):
        analyze_args = ['-i', data_project_path, '-o', f'{current_dir}\\test_analyze']
        run_command(analyze_args)
        
        self.assertTrue(os.path.exists(f"{current_dir}\\test_analyze"), "test_analyze folder does not exist.")
        self.assertTrue(os.path.exists(f"{current_dir}\\test_analyze\\Scripts"), "Scripts folder does not exist.")
        self.assertTrue(os.path.exists(f"{current_dir}\\test_analyze\\Scripts\\_list.csv"), "_list.csv does not exist.")
                
        # test_analyzeフォルダ内の特定ファイルの存在チェック
        expected_files = [
            r"Actors.json", r"Armors.json", r"Classes.json",
            r"CommonEvents.json", r"Enemies.json",r"Items.json", r"Map001.json",
            r"Map002.json",r"Map003.json",r"MapInfos.csv",r"Skills.json",
            r"States.json",r"System.json",r"Troops.json",r"Weapons.json",
        ]
        
        for file_path in expected_files:
            self.assertTrue(os.path.exists(f"{current_dir}\\test_analyze\\" + file_path), f"{file_path} does not exist.")

        # _list.csvの内容とScriptsフォルダ内のファイルの確認
        with open(f"{current_dir}\\test_analyze\\Scripts\\_list.csv", newline='', encoding='utf-8') as csvfile:
            csvreader = csv.reader(csvfile)
            for row in csvreader:
                file_id, file_name = row
                file_path = os.path.join(f"{current_dir}\\test_analyze\\Scripts", f"{file_id}.rb")
                self.assertTrue(os.path.exists(file_path), f"{file_path} does not exist.")
    
    def test_compress(self):
        copy_folder(data_project_path, f'{current_dir}\\test_compress\\ソポァゼゾタダＡボマミ')
        copy_folder(f'{data_project_path}_langscore', f'{current_dir}\\test_compress\\ソポァゼゾタダＡボマミ_langscore')

        compress_args = ['-c', '-i', f'{current_dir}\\test_compress\\ソポァゼゾタダＡボマミ']
        run_command(compress_args)

        decompress_script = os.path.abspath(os.path.join(f"{current_dir}", "../internal/decompress.rb"))
        core.run_ruby_script(decompress_script, args=['-i', f'{current_dir}\\test_compress\\ソポァゼゾタダＡボマミ'])

        self.assertTrue(extract_script_data(f"{current_dir}\\test_compress\\ソポァゼゾタダＡボマミ\\Data", f'{current_dir}\\test_compress\\ソポァゼゾタダＡボマミ\\Data'), "test_packing folder does not exist.")

        self.assertTrue(check_files_in_directory(f'{current_dir}\\test_compress\\ソポァゼゾタダＡボマミ\\Scripts', "langscore"), "langscore script is not written.")
        self.assertTrue(check_files_in_directory(f'{current_dir}\\test_compress\\ソポァゼゾタダＡボマミ\\Scripts', "langscore_custom"), "langscore_custom script is not written.")

        before_script_data = os.path.getsize(f"{data_project_path}\\Data\\Scripts.rvdata2")
        after_script_data = os.path.getsize(f"{current_dir}\\test_compress\\ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2")
        self.assertLess(before_script_data, after_script_data, "書き出し後のファイルサイズが書き出し前を下回っています。")


    def test_packing(self):
        base_csv_folder = f"{test_root_dir}\\data\\vxace\\packing_test_translates"
        packing_args = ['-p', '-i', base_csv_folder, '-o', f'{current_dir}\\test_packing']
        out, err, code = run_command(packing_args)
        
        self.assertTrue(os.path.exists(f"{current_dir}\\test_packing"), "test_packing folder does not exist.")
        
        expected_files = [
            r"Actors", r"Armors", r"Classes",
            r"CommonEvents", r"Enemies",r"Items", r"Map001",
            r"Map002",r"Map003", r"Skills",
            r"States",r"System",r"Troops",r"Weapons",
        ]
        output_folder = f"{current_dir}\\test_packing\\"
        
        for file_path in expected_files:
            original_csv_path = base_csv_folder + "\\" + file_path + ".csv"
            rvdata_path = output_folder + file_path + ".rvdata2"
            extracted_csv_path = output_folder + file_path + ".csv"
            #パッキングで生成したrvdata2からcsvを作成。rvdata2と同階層に置く。
            convert_rvdata2_to_csv(rvdata_path, extracted_csv_path)

            # 生成されたCSVファイルの改行コードの検証
            self.assertTrue(validate_newlines_in_csv(extracted_csv_path), "Newline characters in the packed CSV are not correctly set.")

            # 変換されたCSVファイルの内容を元のCSVファイルと比較
            self.assertTrue(compare_csv_files(original_csv_path, extracted_csv_path), f"The content of the packed CSV does not match the original CSV. | {file_path}")
    

# 実行例
if __name__ == '__main__':
    if 2 <= len(sys.argv) and len(sys.argv[1]) != 0:
        RVCNV_PATH = os.path.abspath(sys.argv[1])

    try:
        suite = unittest.TestLoader().loadTestsFromTestCase(TestRVCNV)
        runner = unittest.TextTestRunner(verbosity=2)
        result = runner.run(suite)
    except Exception as e:
        print(f"Failed to run command: {e}")

