"""CMake/CTest でテスト一式を実行し、結果を Markdown レポート (test_log.md) にまとめる。

ビルド手順・DLLのコピー・テストデータのパス解決は全て CMake 側 (test/CMakeLists.txt) にある。
このスクリプトがやるのは以下の3つだけ。
  1. プリセットで configure / build
  2. ctest --output-junit で実行
  3. JUnit XML (と gtest の JSON) を test_log.md へ変換

使い方:
    python run_test.py [all|cpp|lscsv|rvcnv|vxace|mv|mz|plugin|divisi] ...
    python run_test.py --preset x64-release-test
"""
import os
import sys
import json
import xml.etree.ElementTree as ET
from datetime import datetime

# コンソールが cp932 でも日本語を落とさない。
# sys.stdout を差し替えると行バッファリングが効かなくなるため reconfigure を使う。
try:
    sys.stdout.reconfigure(encoding='utf-8', errors='replace')
except AttributeError:
    pass

divisi_root = os.path.dirname(os.path.abspath(__file__))
test_root_dir = os.path.join(divisi_root, "test")
sys.path.append(test_root_dir)
from internal import test_core as core

DEFAULT_PRESET = "x64-debug-test"

# コマンドライン引数 -> ctest -R に渡す正規表現。
# 'all' は指定なし (全テスト実行)。
TEST_FILTERS = {
    'cpp':      r'^cpp$',
    'lscsv':    r'^lscsv_',
    'rvcnv':    r'^rvcnv$',
    'vxace':    r'^vxace_',
    'mv':       r'^plugin_mv_',
    'mz':       r'^plugin_mz_',
    'plugin':   r'^(plugin_|vxace_|lscsv_|rvcnv$)',
    'divisi':   r'^divisi_ct$',
    'divisi_ct': r'^divisi_ct$',
}


def elapsed_time(seconds):
    hours, remainder = divmod(int(seconds), 3600)
    minutes, seconds = divmod(remainder, 60)
    return f"{hours:02}:{minutes:02}:{seconds:02}"


def gtest_failures(build_dir):
    """gtest の JSON から失敗したテストケース名を拾う。ctest からは cpp 1件にしか
    見えないため、C++ だけは内訳をこちらで補う。"""
    path = os.path.join(build_dir, 'cpp_test_results.json')
    if not os.path.exists(path):
        return []
    # gtest は失敗メッセージをプロセスの narrow encoding のまま書くため、
    # 日本語環境では CP932 のバイトが混ざる。レポート生成を落とさないよう置換して読む。
    with open(path, 'r', encoding='utf-8', errors='replace') as f:
        results = json.load(f)

    failed = []
    for suite in results.get('testsuites', []):
        for case in suite.get('testsuite', []):
            # 失敗したテストも result は COMPLETED のままなので、failures の有無で判定する。
            failures = case.get('failures', [])
            if not failures and case.get('result') == 'COMPLETED':
                continue
            message = failures[0].get('failure', 'No message') if failures else case.get('result')
            failed.append((f"{suite['name']}.{case['name']}", message))
    return failed


def parse_junit(junit_path):
    """ctest の JUnit XML を (name, passed, time, output) のリストにする。"""
    tree = ET.parse(junit_path)
    cases = []
    for case in tree.iter('testcase'):
        skipped = case.find('skipped') is not None
        failure = case.find('failure')
        output = case.findtext('system-out') or ''
        if failure is not None and failure.text:
            output = failure.text + '\n' + output
        cases.append({
            'name': case.get('name'),
            'passed': failure is None and not skipped,
            'skipped': skipped,
            'time': float(case.get('time') or 0),
            'output': output,
        })
    return cases


def write_report(log_path, cases, cpp_failures, start_date, end_date):
    all_passed = all(c['passed'] or c['skipped'] for c in cases)

    with open(log_path, 'w', encoding='utf-8') as log:
        log.write(f"\n## Overall Result: {'Passed' if all_passed else 'Failed'}\n")
        log.write("| Test Suite | Result | Failures | Time |\n")
        log.write("|------------|--------|----------|------|\n")

        for case in cases:
            name, took = case['name'], elapsed_time(case['time'])
            if case['skipped']:
                log.write(f"| {name} | Skipped | | {took} |\n")
            elif case['passed']:
                log.write(f"| {name} | Passed | None | {took} |\n")
            elif name == 'cpp' and cpp_failures:
                # C++ は失敗したテストケースを1行ずつ出す。
                for test_case, _ in cpp_failures:
                    log.write(f"| {name} | Failed | {test_case} | {took} |\n")
            else:
                log.write(f"| {name} | Failed | | {took} |\n")

        log.write('\n\n')
        log.write(f'* Start Time : {start_date.strftime("%Y-%m-%d %H:%M:%S")}\n')
        log.write(f'* End Time : {end_date.strftime("%Y-%m-%d %H:%M:%S")}\n')
        log.write(f'* Elapsed : {elapsed_time((end_date - start_date).total_seconds())}\n\n\n')

        for case in cases:
            if case['passed'] or case['skipped']:
                continue
            log.write(f"### Failures in {case['name']} Test\n\n")
            if case['name'] == 'cpp':
                for test_case, message in cpp_failures:
                    log.write(f"- **{test_case}**\n\n```\n{message.rstrip()}\n```\n\n")
            if case['output'].strip():
                log.write(f"## {case['name']} Test Output\n\n")
                log.write("```\n" + case['output'].rstrip() + "\n```\n\n")

    return all_passed


def main():
    args = [a for a in sys.argv[1:] if a]
    preset = DEFAULT_PRESET
    if '--preset' in args:
        i = args.index('--preset')
        if i + 1 >= len(args):
            print("--preset にプリセット名が指定されていません")
            return False
        preset = args[i + 1]
        del args[i:i + 2]

    selection = args or ['all']
    print(f"run tests {selection} (preset: {preset})")

    build_dir = os.path.join(divisi_root, "build", preset)
    junit_path = os.path.join(build_dir, "test_results.xml")
    gtest_json_path = os.path.join(build_dir, "cpp_test_results.json")
    log_file_path = os.path.join(test_root_dir, "test_log.md")

    # cl.exe / ninja へパスを通す。開発者コマンドプロンプト以外からも実行できるようにする。
    build_env = core.vs_environment()

    _, error, ok = core.run_command("cmake.exe", ["--preset", preset], cwd=divisi_root,
                                    env=build_env, timeout=900)
    if not ok:
        print(f"CMake configure failed:\n{error}")
        return False

    _, error, ok = core.run_command("cmake.exe", ["--build", "--preset", preset], cwd=divisi_root,
                                    env=build_env, timeout=3600)
    if not ok:
        print(f"Build failed:\n{error}")
        return False

    ctest_args = ["--test-dir", build_dir, "--output-on-failure",
                  "--output-junit", junit_path]
    if 'all' not in selection:
        unknown = [s for s in selection if s not in TEST_FILTERS]
        if unknown:
            print(f"unknown test selection: {unknown} (available: {sorted(TEST_FILTERS)})")
            return False
        ctest_args += ["-R", "|".join(TEST_FILTERS[s] for s in selection)]

    # 前回の結果が残っていると、ctest が起動できなかった場合に
    # 古い結果を読んで「成功」と判定してしまうため先に消す。
    for stale in (junit_path, gtest_json_path):
        if os.path.exists(stale):
            os.remove(stale)

    start_date = datetime.now()
    output, error, ctest_ok = core.run_command("ctest.exe", ctest_args, cwd=divisi_root,
                                               env=build_env, timeout=7200)
    end_date = datetime.now()
    print(output)

    if not os.path.exists(junit_path):
        print(f"ctest の結果が出力されませんでした:\n{error}")
        return False

    cases = parse_junit(junit_path)
    if not cases:
        print(f"テストが1件も実行されませんでした。選択: {selection}\n"
              f"  プリセット {preset} でテストが CTest に登録されているか "
              f"(DIVISI_BUILD_TESTS=ON か) 確認してください。")
        return False

    all_passed = write_report(log_file_path, cases, gtest_failures(build_dir),
                              start_date, end_date)

    # ctest の終了コードと結果が食い違う場合 (JUnitに残らない形で落ちた等) は失敗扱いにする。
    if all_passed and not ctest_ok:
        print("ctest が異常終了しましたが、結果には失敗がありません。出力を確認してください。")
        all_passed = False

    print(f"All tests {'passed' if all_passed else 'failed'} -> {log_file_path}")
    return all_passed


if __name__ == '__main__':
    # CI やスクリプトから結果を判定できるよう、失敗時は終了コードを返す。
    sys.exit(0 if main() else 1)
