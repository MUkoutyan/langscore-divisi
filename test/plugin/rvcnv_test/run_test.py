import subprocess
import shutil
import os
import csv
import sys
import unittest

def run_command(args):
    RVCNV_PATH = r"..\\..\\..\\rvcnv\\rvcnv.exe"
    command = [RVCNV_PATH] + args
    result = subprocess.run(command, capture_output=True, text=True, shell=True, encoding='utf-8')
    if result.returncode != 0:
        print(f"Error executing command: {command}")
        print(f"Standard Output:\n{result.stdout}")
        print(f"Standard Error:\n{result.stderr}")
        sys.exit(1)
    return result


def convert_rvdata2_to_csv(rvdata2_path, csv_path):
    ruby_script = r"extract_rvdata2.rb"
    command = ["ruby", ruby_script, rvdata2_path, csv_path]
    result = subprocess.run(command, capture_output=True, text=True, shell=True, encoding='utf-8')
    if result.returncode != 0:
        print(f"Error executing command: {command}")
        print(f"Standard Output:\n{result.stdout}")
        print(f"Standard Error:\n{result.stderr}")
        sys.exit(1)
    return result

def compare_csv_files(file1, file2):
    with open(file1, newline='', encoding='utf-8') as f1, open(file2, newline='', encoding='utf-8') as f2:
        reader1 = csv.reader(f1)
        reader2 = csv.reader(f2)
        
        for row1, row2 in zip(reader1, reader2):
            if row1 != row2:
                print(f"{row1} != {row2}")
                return False

        # Check if both files have the same number of rows
        for _ in reader1:
            print("reader1")
            return False
        for _ in reader2:
            print("reader2")
            return False

    return True

class TestAnalyzeAndPack(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        # テスト用のディレクトリを作成して必要なファイルを配置するなどの準備を行う
        # ここにテストに必要なファイルの作成コードを追加
        pass

    @classmethod
    def tearDownClass(cls):
        # テスト用に作成したファイルやディレクトリを削除する
        # if os.path.exists(r".\\test_analyze"):
        #     shutil.rmtree(r".\\test_analyze")
        # if os.path.exists(r".\\test_packing"):
        #     shutil.rmtree(r".\\test_packing")
        pass

    def setUp(self):
        # 各テストの前に実行されるセットアップ
        pass

    def tearDown(self):
        pass

    def test_analyze(self):
        analyze_args = ['-i', '..\\data\\vxace\\ソポァゼゾタダＡボマミ', '-o', '.\\test_analyze']
        run_command(analyze_args)
        
        self.assertTrue(os.path.exists(r".\\test_analyze"), "test_analyze folder does not exist.")
        self.assertTrue(os.path.exists(r".\\test_analyze\\Scripts"), "Scripts folder does not exist.")
        self.assertTrue(os.path.exists(r".\\test_analyze\\Scripts\\_list.csv"), "_list.csv does not exist.")
                
        # test_analyzeフォルダ内の特定ファイルの存在チェック
        expected_files = [
            r"Actors.json", r"Animations.json",r"Armors.json", r"Classes.json",
            r"CommonEvents.json", r"Enemies.json",r"Items.json", r"Map001.json",
            r"Map002.json",r"Map003.json",r"MapInfos.csv",r"Skills.json",
            r"States.json",r"System.json",r"Troops.json",r"Weapons.json",
        ]
        
        for file_path in expected_files:
            self.assertTrue(os.path.exists("./test_analyze/" + file_path), f"{file_path} does not exist.")

        # _list.csvの内容とScriptsフォルダ内のファイルの確認
        with open(r".\\test_analyze\\Scripts\\_list.csv", newline='', encoding='utf-8') as csvfile:
            csvreader = csv.reader(csvfile)
            for row in csvreader:
                file_id, file_name = row
                file_path = os.path.join(r".\\test_analyze\\Scripts", f"{file_id}.rb")
                self.assertTrue(os.path.exists(file_path), f"{file_path} does not exist.")
    
    def test_packing(self):
        base_csv_folder = "..\\data\\Translate"
        packing_args = ['-p', '-i', base_csv_folder, '-o', '.\\test_packing']
        run_command(packing_args)
        
        self.assertTrue(os.path.exists(r".\\test_packing"), "test_packing folder does not exist.")
        
        expected_files = [
            r"Actors", r"Animations",r"Armors", r"Classes",
            r"CommonEvents", r"Enemies",r"Items", r"Map001",
            r"Map002",r"Map003",r"MapInfos",r"Skills",
            r"States",r"System",r"Troops",r"Weapons",
        ]
        output_folder = r".\\test_packing\\"
        
        for file_path in expected_files:
            original_csv_path = base_csv_folder + "\\" + file_path + ".csv"
            rvdata_path = output_folder + file_path + ".rvdata2"
            extracted_csv_path = output_folder + file_path + ".csv"
            convert_rvdata2_to_csv(rvdata_path, extracted_csv_path)
            
            # 変換されたCSVファイルの内容を元のCSVファイルと比較
            print(original_csv_path)
            print(extracted_csv_path)
            self.assertTrue(compare_csv_files(original_csv_path, extracted_csv_path), f"{file_path}.csv does not match the original CSV.")

if __name__ == "__main__":
    unittest.main()
