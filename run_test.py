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
import subprocess
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


def fix_encoding(text):
    """gtest は失敗メッセージをプロセスの narrow encoding のまま書くため、
    日本語環境では UTF-8 の JSON に CP932 のバイトが混ざる。混ざっていたら CP932 として読み直す。"""
    try:
        return text.encode('utf-8', 'surrogateescape').decode('utf-8')
    except UnicodeDecodeError:
        return text.encode('utf-8', 'surrogateescape').decode('cp932', 'replace')


def gtest_failures(build_dir):
    """gtest の JSON から (テスト総数, 失敗したテストケース) を拾う。ctest からは cpp 1件にしか
    見えないため、C++ だけは内訳をこちらで補う。
    失敗したテストケースは (名前, [失敗メッセージ, ...]) のリスト。"""
    path = os.path.join(build_dir, 'cpp_test_results.json')
    if not os.path.exists(path):
        return 0, []
    with open(path, 'r', encoding='utf-8', errors='surrogateescape') as f:
        results = json.load(f)

    failed = []
    for suite in results.get('testsuites', []):
        for case in suite.get('testsuite', []):
            # 失敗したテストも result は COMPLETED のままなので、failures の有無で判定する。
            failures = case.get('failures', [])
            if not failures and case.get('result') == 'COMPLETED':
                continue
            messages = [fix_encoding(f.get('failure', 'No message')).strip() for f in failures]
            failed.append((f"{suite['name']}.{case['name']}", messages or [case.get('result')]))
    return results.get('tests', 0), failed


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


def output_before_failure(output, test_name, limit=8):
    """gtest の出力から、失敗したテストが失敗する直前に出していた行を拾う。
    クラッシュ (SEH例外) は失敗メッセージに手掛かりが無いため、これを添える。"""
    lines = output.splitlines()
    try:
        begin = lines.index(f"[ RUN      ] {test_name}")
    except ValueError:
        return []
    block = []
    for line in lines[begin + 1:]:
        if ': error: ' in line or line.startswith('[  FAILED  ]'):
            break
        if line.strip():
            block.append(line)
    return block[-limit:]


def write_report(log_path, cases, cpp_total, cpp_failures, start_date, end_date):
    all_passed = all(c['passed'] or c['skipped'] for c in cases)
    failed_cases = [c for c in cases if not c['passed'] and not c['skipped']]
    skipped = sum(c['skipped'] for c in cases)

    def short(text):
        return text.replace(divisi_root + os.sep, '')

    def cell(text):
        return short(text).replace('|', '\\|')

    with open(log_path, 'w', encoding='utf-8') as log:
        log.write(f"# Test Report: {'✅ Passed' if all_passed else '❌ Failed'}\n\n")
        log.write(f"- Suites : {len(cases)} total / {len(cases) - len(failed_cases) - skipped} passed"
                  f" / {len(failed_cases)} failed / {skipped} skipped\n")
        log.write(f'- Start : {start_date.strftime("%Y-%m-%d %H:%M:%S")}\n')
        log.write(f'- Elapsed : {elapsed_time((end_date - start_date).total_seconds())}\n\n')

        # スイートごとに1行。失敗の内訳は下の節に分ける。
        log.write("| Test Suite | Result | Failed | Time |\n")
        log.write("|------------|--------|--------|------|\n")
        for case in cases:
            name, took = case['name'], elapsed_time(case['time'])
            if case['skipped']:
                log.write(f"| {name} | ⏭ Skipped | | {took} |\n")
            elif case['passed']:
                log.write(f"| {name} | ✅ Passed | | {took} |\n")
            elif name == 'cpp' and cpp_failures:
                log.write(f"| {name} | ❌ Failed | {len(cpp_failures)} / {cpp_total} | {took} |\n")
            else:
                log.write(f"| {name} | ❌ Failed | | {took} |\n")
        log.write('\n')

        for case in failed_cases:
            if case['name'] == 'cpp' and cpp_failures:
                log.write(f"## cpp: {len(cpp_failures)} failed\n\n")
                # 失敗メッセージは「場所 \n 内容...」の形。一覧には内容の1行目だけ出す。
                log.write("| Test | Location | Message |\n")
                log.write("|------|----------|---------|\n")
                for test_case, messages in cpp_failures:
                    location, _, body = messages[0].partition('\n')
                    more = f" (+{len(messages) - 1})" if len(messages) > 1 else ""
                    log.write(f"| {test_case} | {cell(location)} "
                              f"| {cell(body.split(chr(10))[0][:100])}{more} |\n")
                log.write('\n')

                for test_case, messages in cpp_failures:
                    log.write(f"### {test_case}\n\n")
                    for message in messages:
                        log.write(f"```\n{short(message)}\n```\n\n")
                    before = output_before_failure(case['output'], test_case)
                    if before:
                        log.write("Output before failure:\n\n```\n" + '\n'.join(before) + "\n```\n\n")
            else:
                log.write(f"## {case['name']}: failed\n\n")

            if case['output'].strip():
                # 全出力は長いので畳んでおく。
                log.write(f"<details>\n<summary>{case['name']} full output</summary>\n\n")
                log.write("```\n" + case['output'].rstrip() + "\n```\n\n</details>\n\n")

    return all_passed


def run_live(command, timeout, **option):
    """出力をキャプチャせずコンソールへそのまま流す。(ビルドの進行や ctest の n/m がその場で見える)"""
    print(f"\n=== {' '.join(command)}", flush=True)
    try:
        # shell=True: env の PATH で実行ファイルを探させるため。(core.run_command と同じ)
        return subprocess.run(command, shell=True, timeout=timeout, **option).returncode == 0
    except subprocess.TimeoutExpired:
        print(f"timeout ({timeout}s): {command[0]}")
        return False


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

    if not run_live(["cmake.exe", "--preset", preset], 900, cwd=divisi_root, env=build_env):
        print("CMake configure failed")
        return False

    if not run_live(["cmake.exe", "--build", "--preset", preset], 3600, cwd=divisi_root, env=build_env):
        print("Build failed")
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
    ctest_ok = run_live(["ctest.exe"] + ctest_args, 7200, cwd=divisi_root, env=build_env)
    end_date = datetime.now()

    if not os.path.exists(junit_path):
        print("ctest の結果が出力されませんでした")
        return False

    cases = parse_junit(junit_path)
    if not cases:
        print(f"テストが1件も実行されませんでした。選択: {selection}\n"
              f"  プリセット {preset} でテストが CTest に登録されているか "
              f"(DIVISI_BUILD_TESTS=ON か) 確認してください。")
        return False

    cpp_total, cpp_failures = gtest_failures(build_dir)
    all_passed = write_report(log_file_path, cases, cpp_total, cpp_failures,
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
