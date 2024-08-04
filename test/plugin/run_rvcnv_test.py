import subprocess
import shutil
import os
import csv
import sys
import io
import unittest

is_delete_files = True
def run_command(args):
    RVCNV_PATH = r"..\\..\\rvcnv\\rvcnv.exe"
    command = [RVCNV_PATH] + args
    result = subprocess.run(command, capture_output=True, text=True, shell=True, encoding='utf-8')
    if result.returncode != 0:
        print(f"Error executing command: {command}")
        print(f"Standard Output:\n{result.stdout}")
        print(f"Standard Error:\n{result.stderr}")
        is_delete_files = False
        sys.exit(1)
    return result


def convert_rvdata2_to_csv(rvdata2_path, csv_path):
    ruby_script = r".\\rvcnv_test\\extract_rvdata2.rb"
    command = ["ruby", ruby_script, rvdata2_path, csv_path]
    result = subprocess.run(command, capture_output=True, text=True, shell=True, encoding='utf-8')
    if result.returncode != 0:
        print(f"Error executing command: {command}")
        print(f"Standard Output:\n{result.stdout}")
        print(f"Standard Error:\n{result.stderr}")
        is_delete_files = False
        sys.exit(1)
    return result

    
def extract_script_data(project_path, output_path):
    ruby_script = r".\\rvcnv_test\\extract_script.rb"
    command = ["ruby", ruby_script, project_path, output_path]
    result = subprocess.run(command, capture_output=True, text=True, shell=True, encoding='utf-8')
    if result.returncode != 0:
        print(f"Error executing command: {command}")
        print(f"Standard Output:\n{result.stdout}")
        print(f"Standard Error:\n{result.stderr}")
        is_delete_files = False
        sys.exit(1)
    return result

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

class TestAnalyzeAndPack(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # テスト用のディレクトリを作成して必要なファイルを配置するなどの準備を行う
        # ここにテストに必要なファイルの作成コードを追加
        if os.path.exists(r".\\rvcnv_test\\test_analyze"):
            shutil.rmtree(r".\\rvcnv_test\\test_analyze")
        if os.path.exists(r".\\rvcnv_test\\test_compress"):
            shutil.rmtree(r".\\rvcnv_test\\test_compress")
        if os.path.exists(r".\\rvcnv_test\\test_packing"):
            shutil.rmtree(r".\\rvcnv_test\\test_packing")

    @classmethod
    def tearDownClass(cls):
        # テスト用に作成したファイルやディレクトリを削除する
        if is_delete_files:
            if os.path.exists(r".\\rvcnv_test\\test_analyze"):
                shutil.rmtree(r".\\rvcnv_test\\test_analyze")
            # if os.path.exists(r".\\rvcnv_test\\test_compress"):
            #     shutil.rmtree(r".\\rvcnv_test\\test_compress")
            if os.path.exists(r".\\rvcnv_test\\test_packing"):
                shutil.rmtree(r".\\rvcnv_test\\test_packing")
        pass

    def setUp(self):
        # 各テストの前に実行されるセットアップ
        pass

    def tearDown(self):
        pass

    def test_analyze(self):
        analyze_args = ['-i', '.\\data\\vxace\\ソポァゼゾタダＡボマミ', '-o', '.\\rvcnv_test\\test_analyze']
        run_command(analyze_args)
        
        self.assertTrue(os.path.exists(r".\\rvcnv_test\\test_analyze"), "test_analyze folder does not exist.")
        self.assertTrue(os.path.exists(r".\\rvcnv_test\\test_analyze\\Scripts"), "Scripts folder does not exist.")
        self.assertTrue(os.path.exists(r".\\rvcnv_test\\test_analyze\\Scripts\\_list.csv"), "_list.csv does not exist.")
                
        # test_analyzeフォルダ内の特定ファイルの存在チェック
        expected_files = [
            r"Actors.json", r"Animations.json",r"Armors.json", r"Classes.json",
            r"CommonEvents.json", r"Enemies.json",r"Items.json", r"Map001.json",
            r"Map002.json",r"Map003.json",r"MapInfos.csv",r"Skills.json",
            r"States.json",r"System.json",r"Troops.json",r"Weapons.json",
        ]
        
        for file_path in expected_files:
            self.assertTrue(os.path.exists("./rvcnv_test/test_analyze/" + file_path), f"{file_path} does not exist.")

        # _list.csvの内容とScriptsフォルダ内のファイルの確認
        with open(r".\\rvcnv_test\\test_analyze\\Scripts\\_list.csv", newline='', encoding='utf-8') as csvfile:
            csvreader = csv.reader(csvfile)
            for row in csvreader:
                file_id, file_name = row
                file_path = os.path.join(r".\\rvcnv_test\\test_analyze\\Scripts", f"{file_id}.rb")
                self.assertTrue(os.path.exists(file_path), f"{file_path} does not exist.")
    
    def test_compress(self):
        shutil.copytree('.\\data\\vxace\\ソポァゼゾタダＡボマミ', '.\\rvcnv_test\\test_compress\\ソポァゼゾタダＡボマミ')
        ret = shutil.copytree('.\\data\\vxace\\ソポァゼゾタダＡボマミ_langscore', '.\\rvcnv_test\\test_compress\\ソポァゼゾタダＡボマミ_langscore')

        compress_args = ['-c', '-i', '.\\rvcnv_test\\test_compress\\ソポァゼゾタダＡボマミ', '-o', '.\\rvcnv_test\\test_compress\\dest']
        run_command(compress_args)

        self.assertTrue(extract_script_data(".\\rvcnv_test\\test_compress\\ソポァゼゾタダＡボマミ\\Data", '.\\rvcnv_test\\test_compress\\dest'), "test_packing folder does not exist.")

        self.assertTrue(check_files_in_directory('.\\rvcnv_test\\test_compress\\dest', "langscore"), "langscore script is not written.")
        self.assertTrue(check_files_in_directory('.\\rvcnv_test\\test_compress\\dest', "langscore_custom"), "langscore_custom script is not written.")

        before_script_data = os.path.getsize(".\\data\\vxace\\ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2")
        after_script_data = os.path.getsize(".\\rvcnv_test\\test_compress\\ソポァゼゾタダＡボマミ\\Data\\Scripts.rvdata2")
        self.assertLess(before_script_data, after_script_data, "書き出し後のファイルサイズが書き出し前を下回っています。")


    def test_packing(self):
        base_csv_folder = ".\\data\\Translate"
        packing_args = ['-p', '-i', base_csv_folder, '-o', '.\\rvcnv_test\\test_packing']
        run_command(packing_args)
        
        self.assertTrue(os.path.exists(r".\\rvcnv_test\\test_packing"), "test_packing folder does not exist.")
        
        expected_files = [
            r"Actors", r"Armors", r"Classes",
            r"CommonEvents", r"Enemies",r"Items", r"Map001",
            r"Map002",r"Map003", r"Skills",
            r"States",r"System",r"Troops",r"Weapons",
        ]
        output_folder = r".\\rvcnv_test\\test_packing\\"
        
        for file_path in expected_files:
            original_csv_path = base_csv_folder + "\\" + file_path + ".csv"
            rvdata_path = output_folder + file_path + ".rvdata2"
            extracted_csv_path = output_folder + file_path + ".csv"
            convert_rvdata2_to_csv(rvdata_path, extracted_csv_path)

            # 生成されたCSVファイルの改行コードの検証
            self.assertTrue(validate_newlines_in_csv(extracted_csv_path), "Newline characters in the packed CSV are not correctly set.")

            # 変換されたCSVファイルの内容を元のCSVファイルと比較
            self.assertTrue(compare_csv_files(original_csv_path, extracted_csv_path), "The content of the packed CSV does not match the original CSV.")

def run_unittest():
    # 標準出力をキャプチャするためのStringIOオブジェクトを作成
    captured_output = io.StringIO()
    sys.stdout = captured_output

    try:
        # テストスイートを作成
        suite = unittest.TestLoader().loadTestsFromTestCase(TestAnalyzeAndPack)
        
        # テストランナーを作成して実行
        runner = unittest.TextTestRunner(stream=sys.stdout, verbosity=2)
        result = runner.run(suite)
        
        # 標準出力の内容を取得
        output = captured_output.getvalue()
    finally:
        # 標準出力を元に戻す
        sys.stdout = sys.__stdout__

    return output, result.wasSuccessful()

# 実行例
if __name__ == '__main__':
    output, success = run_unittest()
    print(output)
