"""
Langscore Divisi Test Suite

bin/divisi.exe を起動して、divisi.exe でしか確認できない動作をテストする。
- 引数の解釈（引数なし・不明なオプション・ANSI コードページ外の文字を含むパス）
- 終了コード（失敗時に abort せず -1 で終わること）
- バージョン情報の表示

各コマンド（--createConfigFile / --analyze / --reanalysis / --updatePlugin / --exportCSV /
--validate / --packing / --convertPatchCSV / --convertBoundCSV）の処理は、
gtest（test/test_commands.cpp・test/test_core.cpp）で langscore::divisi を直接呼んで確認する。

使用方法:
python run_divisi_test.py
"""

import os
import sys
import shutil
import stat
import unittest

current_dir = os.path.dirname(os.path.abspath(__file__))
test_root_dir = os.path.abspath(os.path.join(current_dir, '../'))
sys.path.append(test_root_dir)
divisi_root_dir = os.path.abspath(os.path.join(current_dir, '../../'))
from internal import test_core as core

sys.stdout.reconfigure(encoding='utf-8')

DIVISI_PATH = os.path.join(divisi_root_dir, 'bin', 'divisi.exe')
work_dir = os.path.join(current_dir, 'work')


def force_remove_readonly(func, path, _):
    os.chmod(path, stat.S_IWRITE)
    func(path)


def run_divisi(args):
    return core._run([DIVISI_PATH] + args, default_encoding='utf-8')


class TestErrorHandling(unittest.TestCase):
    """エラーハンドリングのテスト"""

    def test_invalid_config_file(self):
        """存在しない設定ファイルではエラー終了すること"""
        result = run_divisi(['-c', os.path.join(work_dir, 'invalid_config.json'), '--analyze'])
        self.assertNotEqual(result.returncode, 0, "Should fail with invalid config file")

    def test_no_arguments(self):
        """引数なしではエラー終了すること"""
        result = run_divisi([])
        self.assertNotEqual(result.returncode, 0, "Should fail with no arguments")

    def test_invalid_option(self):
        """不明なオプションだけではエラー終了すること（-c が無いため）"""
        result = run_divisi(['--invalidoption'])
        self.assertNotEqual(result.returncode, 0, "Should fail with invalid option")

    def test_config_path_outside_ansi_codepage(self):
        """ANSI コードページで表せない文字を含むパスの config.json を読めること"""
        config_dir = os.path.join(work_dir, '한글_é')
        os.makedirs(config_dir, exist_ok=True)
        try:
            config_file = os.path.join(config_dir, 'config.json')
            with open(config_file, 'w', encoding='utf-8') as f:
                f.write('{}')
            #langscore-app (invoker.cpp) と同じく、パスを " で括って渡す。処理のオプションは付けない
            result = run_divisi(['-c', f'"{config_file}"'])
            self.assertEqual(result.returncode, 0, f"config.json を開けませんでした: {result.stderr}")
        finally:
            shutil.rmtree(config_dir, onerror=force_remove_readonly)

    def test_csv_conversion_invalid_folder(self):
        """存在しないフォルダを変換しようとしたら、abort せずにエラー終了（-1）すること"""
        invalid_dir = os.path.join(work_dir, 'nonexistent_csv_dir')
        for option in ['--convertPatchCSV', '--convertBoundCSV']:
            with self.subTest(option=option):
                result = run_divisi([option, invalid_dir])
                #abort でも失敗になるため、標準エラー出力で abort していないことを確かめる
                self.assertNotIn("terminate", result.stderr, f"{option} が abort しました: {result.stderr}")
                #Windows の終了コードは符号なしで返るため、-1 は 0xFFFFFFFF になる
                self.assertEqual(result.returncode & 0xFFFFFFFF, 0xFFFFFFFF,
                    f"{option} がエラー終了（-1）しませんでした: {result.stderr}")


class TestVersionInfo(unittest.TestCase):
    """バージョン情報の表示テスト"""

    def test_version_output(self):
        """引数に関係なく、最初にバージョン情報を表示すること"""
        result = run_divisi([])
        self.assertIn("Langscore Divisi Version", result.stdout)
        self.assertIn("Build on", result.stdout)


if __name__ == '__main__':
    os.makedirs(work_dir, exist_ok=True)
    unittest.main(verbosity=2)
