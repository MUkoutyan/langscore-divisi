"""テスト用のゲームプロジェクト (テストデータ側) を divisi で作り直す。

通常のテスト実行では不要。resource/ のテンプレートや翻訳CSVの出力内容を変えたときに、
フィクスチャ側を追従させるために使う。

ビルドは CMake プリセットで行うため、このスクリプトはビルドを行わない。
事前に divisi をビルドして <repo>/bin/divisi.exe を用意しておくこと。

    cmake --preset x64-release
    cmake --build --preset x64-release
    python test/prepare_test.py

書き換え対象は langscore-divisi-test-data/plugin 以下のゲームプロジェクトで、
リポジトリの外にある。実行すると元に戻せないため注意。
"""
import os
import sys

test_root_dir = os.path.dirname(os.path.abspath(__file__))
divisi_root = os.path.abspath(os.path.join(test_root_dir, ".."))
sys.path.append(test_root_dir)
from internal import test_core as core

DIVISI_EXE = os.path.join(divisi_root, "bin", "divisi.exe")

# (プロジェクト名, divisi に渡すオプション)
# --analyze で解析結果を作り、--exportCSV で翻訳CSVを書き出す。
# VXAce は --packing で CSV を Data/Translate の rvdata2 へ変換する。
PROJECTS = [
    ("mv_test",    ["--analyze", "--exportCSV"]),
    ("mz_test",    ["--analyze", "--exportCSV"]),
    ("vxace_test", ["--analyze", "--exportCSV", "--packing"]),
]


def run_divisi(config_path, options):
    for option in options:
        print(f"  divisi {option}")
        output, error, ok = core.run_command(DIVISI_EXE, ["-c", config_path, option], timeout=900)
        if not ok:
            print(output)
            print(f"divisi {option} が失敗しました: {config_path}\n{error}")
            return False
    return True


def update_test_projects():
    if not os.path.exists(DIVISI_EXE):
        print(f"divisi.exe がありません: {DIVISI_EXE}")
        print("  cmake --build --preset x64-release でビルドしてください。")
        return False

    plugin_dir = core.require_test_data("plugin")

    for name, options in PROJECTS:
        config_path = os.path.join(plugin_dir, f"{name}_langscore", "config.json")
        if not os.path.exists(config_path):
            print(f"設定ファイルがありません: {config_path}")
            return False
        print(f"[{name}]")
        if not run_divisi(config_path, options):
            return False

    # VXAce の Scripts フォルダは Data/Scripts.rvdata2 から展開する。
    # decompress.rb はテストデータ側ではなくリポジトリで管理している。
    print("[vxace_test] decompress")
    decompress = os.path.join(test_root_dir, "plugin", "vxace", "decompress.rb")
    vxace_project = os.path.join(plugin_dir, "vxace_test")
    output, error, ok = core.run_ruby_script(decompress, cwd=vxace_project, timeout=300)
    if not ok:
        print(output)
        print(f"decompress.rb が失敗しました:\n{error}")
        return False

    print("Update projects successfully.")
    return True


if __name__ == '__main__':
    sys.exit(0 if update_test_projects() else 1)
