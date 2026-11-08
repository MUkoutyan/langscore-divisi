# encoding: utf-8
"""VXAce 実機テストの実行。

resource/langscore.rb をテスト用プロジェクトへ展開し、テストスクリプトを
Scripts.rvdata2 へ埋め込んで Game.exe (RGSS3) を起動する。
テストは実際の RGSS3 上で動くため、Bitmap / Font / Cache も本物を使う。

langscore.rb の設定値はスクリプト内の定数のため、設定ごとに Game.exe を起動し直す。

    python run_vxace_test.py            全設定
    python run_vxace_test.py default    既定設定のみ (default / deflang / patch)
"""
import os
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..")))
from internal import test_core

# コンソールが cp932 でも日本語のテスト名を落とさない
for stream in (sys.stdout, sys.stderr):
    try:
        stream.reconfigure(encoding="utf-8", errors="replace")
    except Exception:
        pass

PLUGIN_DIR   = os.path.dirname(os.path.abspath(__file__))
TEST_SCRIPT  = os.path.join(PLUGIN_DIR, "vxace", "ls_realtest.rb")
COMPRESS_RB  = os.path.join(PLUGIN_DIR, "vxace", "compress.rb")
RESULT_NAME  = "ls_test_result.txt"
GAME_TIMEOUT = 120

# (名前, sync_vxace.js へ渡す引数, パッチ用フォルダを作るか)
VARIANTS = [
    ("default", [],              False),
    ("deflang", ["--deflang"],   False),
    ("patch",   ["--patch-mode"], True),
]


def run(command, cwd, timeout=180):
    result = subprocess.run(command, cwd=cwd, capture_output=True, text=True,
                            encoding="utf-8", errors="replace", timeout=timeout)
    if result.stdout:
        for line in result.stdout.splitlines():
            print(f"  {line}")
    return result


def build_scripts(project, inject):
    """Scripts/_list.csv から Data/Scripts.rvdata2 を作る。inject を渡すと埋め込む。"""
    command = ["ruby", COMPRESS_RB]
    if inject:
        command.append(inject)
    result = run(command, cwd=project)
    if result.returncode != 0:
        raise RuntimeError(f"compress.rb failed: {result.stderr.strip()}")


def read_results(project):
    """Game.exe が書いた結果ファイルを読む。DONE 行が無ければ途中で落ちている。"""
    path = os.path.join(project, RESULT_NAME)
    if not os.path.exists(path):
        raise RuntimeError("結果ファイルが生成されませんでした (Game.exe が起動していない可能性があります)")

    with open(path, encoding="utf-8") as f:
        lines = [line.rstrip("\n") for line in f if line.strip()]

    if not lines or not lines[-1].startswith("DONE"):
        raise RuntimeError("結果ファイルが途中で終わっています (テスト中に異常終了しました)")

    results = []
    for line in lines[:-1]:
        parts = line.split("\t")
        while len(parts) < 4:
            parts.append("")
        results.append(tuple(parts[:4]))
    return results


def prepare_patch_folders(project):
    """パッチモード用の言語フォルダ。xx は許可されない言語コードの確認用。"""
    created = []
    for name in ("ja", "en", "xx"):
        path = os.path.join(project, "Data", "Translate", name)
        if not os.path.exists(path):
            os.makedirs(path)
            created.append(path)
    return created


def run_variant(project, name, sync_args, needs_patch_folders):
    print(f"[{name}]")
    temp_dirs = []

    sync = run(["node", os.path.join(PLUGIN_DIR, "sync_vxace.js")] + sync_args, cwd=PLUGIN_DIR)
    if sync.returncode != 0:
        raise RuntimeError(f"sync_vxace.js failed: {sync.stderr.strip()}")

    if needs_patch_folders:
        temp_dirs = prepare_patch_folders(project)

    build_scripts(project, TEST_SCRIPT)

    result_path = os.path.join(project, RESULT_NAME)
    if os.path.exists(result_path):
        os.remove(result_path)

    try:
        # "test" を渡すと RGSS3 が $TEST を true にする。translate_list_reset が必要とする。
        run([os.path.join(project, "Game.exe"), "test"], cwd=project, timeout=GAME_TIMEOUT)
        return read_results(project)
    finally:
        for path in temp_dirs:
            shutil.rmtree(path, ignore_errors=True)
        if os.path.exists(result_path):
            os.remove(result_path)


def restore_project(project, game_ini_backup):
    """Game.ini (言語設定が書き戻される) とテスト用セーブを元に戻す。"""
    if game_ini_backup is not None:
        with open(os.path.join(project, "Game.ini"), "wb") as f:
            f.write(game_ini_backup)
    for index in (3, 4):
        save = os.path.join(project, f"Save{index:02d}.rvdata2")
        if os.path.exists(save):
            os.remove(save)
    try:
        build_scripts(project, None)     # 埋め込み無しのスクリプトへ戻す
    except Exception as e:
        print(f"  警告: Scripts.rvdata2 を戻せませんでした: {e}")


def print_summary(results):
    """run_test.py の解析器 (analyze_vxace_test_result) が読める形式で出力する。"""
    failures = [r for r in results if r[0] == "NG"]
    skips    = [r for r in results if r[0] == "SKIP"]

    for r in results:
        mark = {"OK": "  .", "NG": "  F", "SKIP": "  S"}.get(r[0], "  ?")
        print(f"{mark} {r[1]} : {r[2]}" + (f"  -- {r[3]}" if r[3] else ""))

    print("")
    for i, r in enumerate(failures, 1):
        print(f"{i}) Failure:\n{r[2]} [{r[1]}]:\n{r[3]}\n")

    print(f"{len(results)} tests, {len(results)} assertions, "
          f"{len(failures)} failures, 0 errors, {len(skips)} skips")
    return len(failures) == 0


def main():
    selection = [a for a in sys.argv[1:] if not a.startswith("-")]
    variants = [v for v in VARIANTS if not selection or v[0] in selection]
    if not variants:
        print(f"unknown variant: {selection}")
        return False

    project = test_core.require_test_data("plugin", "vxace_test")
    game_ini = os.path.join(project, "Game.ini")
    backup = open(game_ini, "rb").read() if os.path.exists(game_ini) else None

    results = []
    try:
        for name, args, patch in variants:
            results += run_variant(project, name, args, patch)
    except Exception as e:
        print(f"Error: {e}")
        results.append(("NG", "runner", "(setup)", str(e)))
    finally:
        restore_project(project, backup)

    return print_summary(results)


if __name__ == "__main__":
    sys.exit(0 if main() else 1)
