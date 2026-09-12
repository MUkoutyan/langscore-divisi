import os
import sys
import io
import shutil
import json
from datetime import datetime, timedelta
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

current_dir = os.path.dirname(os.path.abspath(__file__))
test_root_dir = current_dir
divisi_root = os.path.abspath(os.path.join(test_root_dir, "../"))
# rootディレクトリのパスを取得してsys.pathに追加
sys.path.append(test_root_dir)
from internal import test_core as core

test_plugin_path = os.path.join(test_root_dir, "plugin")

def elapsed_time(start, end):
    time_difference  = end - start
    hours, remainder = divmod(time_difference.total_seconds(), 3600)
    minutes, seconds = divmod(remainder, 60)
    formatted_time_difference = f"{int(hours):02}:{int(minutes):02}:{int(seconds):02}"
    return formatted_time_difference

def extract_failed_tests(test_results):
    failed_tests = []
    for suite in test_results['testsuites']:
        for case in suite['testsuite']:
            # 失敗したテストも result は COMPLETED のままなので、failures の有無で判定する。
            failures = case.get('failures', [])
            if not failures and case.get('result') == 'COMPLETED':
                continue
            failed_tests.append({
                'test_suite': suite['name'],
                'test_case': case['name'],
                # gtest の JSON はメッセージを 'failure' キーに入れる。
                'failure_message': failures[0].get('failure', 'No message') if failures else case.get('result')
            })
    return failed_tests

def run_rvcnv_test(rvcnv_script_path, details_md_text, results_md_texts):
    TEST_NAME = "rvcnv"

    start_date = datetime.now()

    output, error_base, result = core.run_python_script(os.path.join(rvcnv_script_path, "run_rvcnv_test.py"), args=[os.path.join(test_root_dir, "../rvcnv/rvcnv.exe")])
    success, failures = core.analyze_python_test_result(error_base)
    test_result = result and (len(failures) == 0)
    end_date = datetime.now()

    if test_result:
        results_md_texts.append(f"| {TEST_NAME} | Passed | None | {elapsed_time(start_date, end_date)} |")
    elif 0 < len(failures):
        for failure in failures:
            results_md_texts.append(f"| {TEST_NAME} | Failed | {failure} | {elapsed_time(start_date, end_date)} |")
    else:
        results_md_texts.append(f"| {TEST_NAME} | Failed | | {elapsed_time(start_date, end_date)} |")

    if not test_result:
        details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
        for failure in failures:
            details_md_text.append(f"- {failure}\n")
        if error_base and 0 < len(error_base):
            details_md_text.append(f"```\n")
            details_md_text.append(f"- {error_base}\n")
            details_md_text.append(f"```\n")

        if 0 < len(output):
            details_md_text.append(f"## {TEST_NAME} Test Output\n\n")
            details_md_text.append("```\n" + output.rstrip() + "\n```\n\n")
        if 0 < len(failures):
            details_md_text.append(f"## {TEST_NAME} Test Error\n\n")
            details_md_text.append("```\n" + error_base.rstrip() + "\n```\n\n")

    return test_result

def run_lscsv_test(lscsv_script_path, details_md_text, results_md_texts):
    TEST_NAME = "lscsv"
    lscsv_path = os.path.join(test_root_dir, "lscsv")
    is_show_failures_title = False
    test_result = True
    output_total = ""

    start_date = datetime.now()

    #Ruby
    output, error, result = core.run_ruby_script(os.path.join(lscsv_path, "lscsv_test.rb"), cwd=lscsv_path)
    success, failures, error_txt = core.analyze_ruby_test_result(output)
    test_result &= result and (len(failures) == 0) and (len(error_txt) == 0)
    
    if not test_result:
        output_total += output
        details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
        is_show_failures_title = True
        for failure in failures:
            details_md_text.append(f"- {failure}\n")
        if 0 < len(error):
            details_md_text.append(f"```\n")
            details_md_text.append(f"- {error}\n")
            details_md_text.append(f"```\n")

    #Python
    output, error, result = core.run_python_script(os.path.join(lscsv_path, "lscsv_test.py"), cwd=lscsv_path)
    success, failures = core.analyze_python_test_result(error)
    test_result &= result and (len(failures) == 0)
    
    if not test_result:
        output_total += output
        if is_show_failures_title == False:
            details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
            is_show_failures_title = True
        for failure in failures:
            details_md_text.append(f"- {failure}\n")
        if 0 < len(error):
            details_md_text.append(f"```\n")
            details_md_text.append(f"- {error}\n")
            details_md_text.append(f"```\n")


    #Jest
    output, error, result = core.run_command("npm.cmd", ["test"], cwd=lscsv_path, encoding="utf-8")
    success, failures, error_txt = core.analyze_jest_test_result(output, False)
    test_result &= result and (len(failures) == 0) and (len(error_txt) == 0)
    
    if not test_result:
        output_total += output
        if is_show_failures_title == False:
            details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
            is_show_failures_title = True
        for failure in failures:
            details_md_text.append(f"- {failure}\n")
        if 0 < len(error):
            details_md_text.append(f"```\n")
            details_md_text.append(f"- {error}\n")
            details_md_text.append(f"```\n")

    end_date = datetime.now()

    if test_result:
        results_md_texts.append(f"| {TEST_NAME} | Passed | None | {elapsed_time(start_date, end_date)} |")
    elif 0 < len(failures):
        for failure in failures:
            results_md_texts.append(f"| {TEST_NAME} | Failed | {failure} | {elapsed_time(start_date, end_date)} |")
    else:
        results_md_texts.append(f"| {TEST_NAME} | Failed | | {elapsed_time(start_date, end_date)} |")


    if not test_result:
        if 0 < len(output_total):
            details_md_text.append(f"## {TEST_NAME} Test Output\n\n")
            details_md_text.append("```\n" + output_total.rstrip() + "\n```\n\n")
        if 0 < len(error):
            details_md_text.append(f"## {TEST_NAME} Test Error\n\n")
            details_md_text.append("```\n" + error.rstrip() + "\n```\n\n")

    return test_result



def run_divisi_test(divisi_script_path, details_md_text, results_md_texts):
    TEST_NAME = "divisi"

    start_date = datetime.now()
    output, error, result = core.run_python_script(divisi_script_path, cwd=os.path.join(test_root_dir, "divisi_ct"), _timeout=3000)
    success, failures = core.analyze_python_test_result(output)
    test_result = result and (len(failures) == 0)
    end_date = datetime.now()

    if test_result:
        results_md_texts.append(f"| {TEST_NAME} | Passed | None | {elapsed_time(start_date, end_date)}(total) |")
    elif 0 < len(failures):
        for failure in failures:
            results_md_texts.append(f"| {TEST_NAME} | Failed | {failure} | {elapsed_time(start_date, end_date)}(total) |")
    else:
        results_md_texts.append(f"| {TEST_NAME} | Failed | | {elapsed_time(start_date, end_date)}(total) |")

    if not test_result:
        details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
        for failure in failures:
            details_md_text.append(f"- {failure}\n")

        if 0 < len(output):
            details_md_text.append(f"## {TEST_NAME} Test Output\n\n")
            details_md_text.append("```\n" + output.rstrip() + "\n```\n\n")
        if 0 < len(failures):
            details_md_text.append(f"## {TEST_NAME} Test Error\n\n")
            details_md_text.append("```\n" + error.rstrip() + "\n```\n\n")

    return test_result

def run_vxace_test(vxace_script_path, details_md_text, results_md_texts):
    TEST_NAME = "vxace"
    
    start_date = datetime.now()

    # VXAce は Game.exe (RGSS3) を起動して実機上でテストする
    output, error, result = core.run_python_script(vxace_script_path, cwd=test_plugin_path)
    failures = core.analyze_vxace_test_result(output)
    test_result = result and len(failures) == 0

    end_date = datetime.now()

    if test_result:
        results_md_texts.append(f"| {TEST_NAME} | Passed | None | {elapsed_time(start_date, end_date)} |")
    elif 0 < len(failures):
        for failure in failures:
            results_md_texts.append(f"| {TEST_NAME} | Failed | {failure} | {elapsed_time(start_date, end_date)} |")
    else:
        results_md_texts.append(f"| {TEST_NAME} | Failed | | {elapsed_time(start_date, end_date)} |")


    if not test_result:
        details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
        for failure in failures:
            details_md_text.append(f"- {failure}\n")
        if 0 < len(error):
            details_md_text.append(f"```\n")
            details_md_text.append(f"- {error}\n")
            details_md_text.append(f"```\n")

        if 0 < len(output):
            details_md_text.append(f"## {TEST_NAME} Test Output\n\n")
            details_md_text.append("```\n" + output.rstrip() + "\n```\n\n")
        if 0 < len(error):
            details_md_text.append(f"## {TEST_NAME} Test Error\n\n")
            details_md_text.append("```\n" + error.rstrip() + "\n```\n\n")

    return test_result


def run_mvmz_test(TEST_NAME, details_md_text, results_md_texts):
    """test/plugin の mocha テスト (NW.js / ブラウザ の2モード) を npm scripts 経由で実行する。"""
    start_date = datetime.now()
    failures = []
    outputs = []
    errors = []
    test_result = True

    _, err, ok = core.run_command("npm.cmd", ["run", "pretest"], cwd=test_plugin_path, encoding="utf-8")
    if not ok:
        errors.append(err)
        test_result = False

    for is_nwjs in (True, False):
        script = f"test-{TEST_NAME.lower()}-{'nwjs' if is_nwjs else 'browser'}"
        output, error, result = core.run_command("npm.cmd", ["run", script], cwd=test_plugin_path, encoding="utf-8")
        _, script_failures, _ = core.analyze_jest_test_result(output, is_nwjs)
        failures.extend(script_failures)
        outputs.append(output)
        if error: errors.append(error)
        test_result &= result and len(script_failures) == 0

    end_date = datetime.now()

    if test_result:
        results_md_texts.append(f"| {TEST_NAME} | Passed | None | {elapsed_time(start_date, end_date)}(total) |")
    else:
        for test_case, is_nwjs in failures:
            prefix = f"{TEST_NAME} (NWJs)" if is_nwjs else f"{TEST_NAME} (Browser)"
            results_md_texts.append(f"| {prefix} | Failed | {test_case} | {elapsed_time(start_date, end_date)}(total) |")
        if not failures:
            results_md_texts.append(f"| {TEST_NAME} | Failed | | {elapsed_time(start_date, end_date)}(total) |")

    if not test_result:
        details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
        for test_case, is_nwjs in failures:
            details_md_text.append(f"- **{ 'NWjs' if is_nwjs else 'Browser' }** {test_case}\n")
        details_md_text.append(f"## {TEST_NAME} Test Output\n\n")
        details_md_text.append("```\n" + "\n".join(outputs).rstrip() + "\n```\n\n")
        if errors:
            details_md_text.append(f"## {TEST_NAME} Test Error\n\n")
            details_md_text.append("```\n" + "\n".join(errors).rstrip() + "\n```\n\n")

    return test_result

def run_mv_test(details_md_text, results_md_texts):
    return run_mvmz_test("MV", details_md_text, results_md_texts)

def run_mz_test(details_md_text, results_md_texts):
    return run_mvmz_test("MZ", details_md_text, results_md_texts)


def run_cpp_test(details_md_text, results_md_texts):
    TEST_NAME = "CPP"

    start_date = datetime.now()

    # ルートのCMakeLists.txtは add_subdirectory(test) をしていないため、
    # test/CMakeLists.txt (divisi_testプロジェクト) を直接構成する。
    test_build_directory = os.path.join(divisi_root, "build\\Test_Debug")
    cmake_args = [
        "-S", os.path.join(divisi_root, "test"),
        "-B", test_build_directory,
        "-G", "Ninja",
        "-DCMAKE_BUILD_TYPE:STRING=Test_Debug",
        f"-DTEST_DATA_SRC:STRING={core.require_test_data('data')}",
    ]
    # ビルドディレクトリは削除しない。divisi_test は初回実行時に test/data (3.4GB) を
    # ここへコピーするため、消すと毎回コピーとgoogletestの取得が発生する。
    os.makedirs(test_build_directory, exist_ok=True)

    # cl.exe / ninja へパスを通す。開発者コマンドプロンプト以外からも実行できるようにする。
    build_env = core.vs_environment()

    output, error, result = core.run_command("cmake.exe", cmake_args, cwd=test_build_directory,
                                             env=build_env, timeout=900)
    if result == False:
        results_md_texts.append(f"| {TEST_NAME} | Failed | CMake | --- |")
        details_md_text.append(f"```\n")
        details_md_text.append(f"- {error}\n")
        details_md_text.append(f"```\n")
        return False

    if result == True:
        output, error, result = core.run_command("ninja", cwd=test_build_directory,
                                                 env=build_env, timeout=1800)
        if result == False:
            results_md_texts.append(f"| {TEST_NAME} | Failed | Ninja | --- |")
            details_md_text.append(f"```\n")
            details_md_text.append(f"- {error}\n")
            details_md_text.append(f"```\n")
            return False

    # 実行に必要なDLLはtest/CMakeLists.txtのPOST_BUILDでコピーされる。
    # カレントディレクトリからは解決されない場合があるため絶対パスで実行する。
    # 初回はtest/dataのコピーが走るため、タイムアウトを長めに取る。
    divisi_test_exe = os.path.join(test_build_directory, "divisi_test.exe")
    output, error, result = core.run_command(divisi_test_exe, ["--gtest_output=json:cpp_test_results.json"],
                                             cwd=test_build_directory, timeout=1800)
    failures = []
    result_json = os.path.join(test_build_directory, 'cpp_test_results.json')
    if os.path.exists(result_json):
        with open(result_json, 'r', encoding='utf-8') as f:
            failures = extract_failed_tests(json.load(f))
    test_result = result and len(failures) == 0
    
    end_date = datetime.now()
    
    if test_result:
        results_md_texts.append(f"| {TEST_NAME} | Passed | None | {elapsed_time(start_date, end_date)}(total) |")
    else:
        for failure in failures:
            test_case = f"{failure['test_suite']}.{failure['test_case']}"
            results_md_texts.append(f"| {TEST_NAME} | Failed | {test_case} | {elapsed_time(start_date, end_date)}(total) |")

    if not test_result:
        details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
        for failure in failures:
            details_md_text.append(f"{failure}\n")
        if not error is None and 0 < len(error):
            details_md_text.append(f"```\n")
            details_md_text.append(f"- {error}\n")
            details_md_text.append(f"```\n")

        if output and 0 < len(output):
            details_md_text.append(f"## {TEST_NAME} Test Output\n\n")
            details_md_text.append("```\n" + output.rstrip() + "\n```\n\n")
        if not error is None and 0 < len(error):
            details_md_text.append(f"## {TEST_NAME} Test Error\n\n")
            details_md_text.append("```\n" + error.rstrip() + "\n```\n\n")

    return test_result

def main():
    if len(sys.argv) < 2 or 0 == len(sys.argv[1]):
        test_selection = ['all']
    else:
        test_selection = sys.argv[1:]  


    print(f"run tests {test_selection}")
    
    # 各スクリプトのパスを生成
    lscsv_script_path   = f'{test_root_dir}\\lscsv\\run_lscsv_test.ps1'
    rvcnv_test_path     = f'{test_root_dir}\\rvcnv_test'
    divisi_script_path  = f'{test_root_dir}\\divisi_ct\\run_divisi_test.py'
    vxace_script_path   = f'{test_plugin_path}\\run_vxace_test.py'
    # MV/MZ は test/plugin の npm scripts、C++ は build/Test_Debug を直接使う (引数不要)

    # ログファイルのパス
    log_file_path = f'{test_root_dir}\\test_log.md'
    results_md_texts   = []
    details_md_text    = []

    start_date = datetime.now()
    
    all_tests_passed = True
    if 'all' in test_selection or 'plugin' in test_selection or 'lscsv' in test_selection:
        all_tests_passed &= run_lscsv_test(lscsv_script_path, details_md_text, results_md_texts)

    if 'all' in test_selection or 'plugin' in test_selection or 'rvcnv' in test_selection:
        all_tests_passed &= run_rvcnv_test(rvcnv_test_path, details_md_text, results_md_texts)

    if 'all' in test_selection or 'plugin' in test_selection or 'vxace' in test_selection:
        all_tests_passed &= run_vxace_test(vxace_script_path, details_md_text, results_md_texts)

    if 'all' in test_selection or 'plugin' in test_selection or 'mv' in test_selection:
        all_tests_passed &= run_mv_test(details_md_text, results_md_texts)

    if 'all' in test_selection or 'plugin' in test_selection or 'mz' in test_selection:
        all_tests_passed &= run_mz_test(details_md_text, results_md_texts)
        
    if 'all' in test_selection or 'cpp' in test_selection:
        all_tests_passed &= run_cpp_test(details_md_text, results_md_texts)

    if 'all' in test_selection or 'divisi' in test_selection or 'divisi_ct' in test_selection:
        all_tests_passed &= run_divisi_test(divisi_script_path, details_md_text, results_md_texts)

    end_date = datetime.now()

    
    with open(log_file_path, 'w', encoding='utf-8') as log_file:
        # Overall result
        print(f"All tests {'passed' if all_tests_passed else 'failed'}")
        log_file.write(f"\n## Overall Result: {'Passed' if all_tests_passed else 'Failed'}\n")

        # テーブルのヘッダー
        log_file.write("| Test Suite | Result | Failures | Time |\n")
        log_file.write("|------------|--------|----------|------|\n")
        
        # テーブルの内容を書き出す
        for result in results_md_texts:
            log_file.write(result + "\n")

        log_file.write('\n\n')
        log_file.write(f'* Start Time : {start_date.strftime("%Y-%m-%d %H:%M:%S")}\n')
        log_file.write(f'* End Time : {end_date.strftime("%Y-%m-%d %H:%M:%S")}\n')

        log_file.write(f'* Elapsed : {elapsed_time(start_date, end_date)}\n')
        
        log_file.write("\n\n")
        # 詳細を書き出す
        log_file.write("\n".join(details_md_text))

    return all_tests_passed

if __name__ == '__main__':
    # CI やスクリプトから結果を判定できるよう、失敗時は終了コードを返す。
    sys.exit(0 if main() else 1)
