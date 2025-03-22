import os
import sys
import io
import shutil
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

def process_mvmz_logs(logs):
    is_nwjs = None
    mz_failures = []
    current_output = []

    nwjs_logs = []
    browser_logs = []

    for line in logs.splitlines():
        if 'IS_NWJS=true' in line:
            is_nwjs = True
            next
        elif 'IS_NWJS=false' in line:
            is_nwjs = False
            next
        elif 'failing' in line:
            is_nwjs = None
            next

        if is_nwjs == None:
            next
        elif is_nwjs == True:
            nwjs_logs.append(line)
        else:
            browser_logs.append(line)

        current_output.append(line)

    # 最後の部分を解析
    if 0 < len(nwjs_logs):
        success, failures, errors = core.analyze_jest_test_result('\n'.join(nwjs_logs), True)
        mz_failures.extend(failures)
        
    if 0 < len(browser_logs):
        success, failures, errors = core.analyze_jest_test_result('\n'.join(browser_logs), False)
        mz_failures.extend(failures)

    return mz_failures

def extract_failed_tests(test_results):
    failed_tests = []
    for suite in test_results['testsuites']:
        for case in suite['testsuite']:
            if case['result'] != 'COMPLETED':
                failed_tests.append({
                    'test_suite': suite['name'],
                    'test_case': case['name'],
                    'failure_message': case.get('failures', [{}])[0].get('message', 'No message')
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
    output, error, result = core.run_wsl_script("npm", ["test"], cwd=lscsv_path)
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

    # VXAce test using PowerShell on Windows
    output, error, result = core.run_powershell_script(vxace_script_path, cwd=test_plugin_path)
    failures = core.analyze_ruby19_test_result(output)
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


def run_mv_test(mv_test_script_path, details_md_text, results_md_texts):
    TEST_NAME = "MV"
    
    start_date = datetime.now()

    # MV test using WSL script
    output, error, result = core.run_wsl_script(mv_test_script_path, cwd=test_plugin_path)
    failures = process_mvmz_logs(output)
    test_result = result and len(failures) == 0
    
    end_date = datetime.now()
    
    if test_result:
        results_md_texts.append(f"| {TEST_NAME} | Passed | None | {elapsed_time(start_date, end_date)}(total) |")
    else:
        for failure in failures:
            is_nwjs = failure[1]
            test_case = failure[0]
            prefix = f"{TEST_NAME} (NWJs)" if is_nwjs else f"{TEST_NAME} (Browser)"
            results_md_texts.append(f"| {prefix} | Failed | {test_case} | {elapsed_time(start_date, end_date)}(total) |")

    if not test_result:
        details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
        for failure in failures:
            details_md_text.append(f"- **{ 'NWjs' if failure[1] else 'Browser' }** {failure}\n")
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

    
def run_mz_test(mz_test_script_path, details_md_text, results_md_texts):
    TEST_NAME = "MZ"

    start_date = datetime.now()

    # MZ test using WSL script
    output, error,  result = core.run_wsl_script(mz_test_script_path, cwd=test_plugin_path)
    failures = process_mvmz_logs(output)
    test_result = result and len(failures) == 0
    
    end_date = datetime.now()
    
    if test_result:
        results_md_texts.append(f"| {TEST_NAME} | Passed | None | {elapsed_time(start_date, end_date)}(total) |")
    else:
        for failure in failures:
            is_nwjs = failure[1]
            test_case = failure[0]
            prefix = f"{TEST_NAME} (NWJs)" if is_nwjs else f"{TEST_NAME} (Browser)"
            results_md_texts.append(f"| {prefix} | Failed | {test_case} | {elapsed_time(start_date, end_date)}(total) |")

    if not test_result:
        details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
        for failure in failures:
            details_md_text.append(f"- **{ 'NWjs' if failure[1] else 'Browser' }** {failure}\n")
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

    
def run_cpp_test(mz_test_script_path, details_md_text, results_md_texts):
    TEST_NAME = "CPP"

    start_date = datetime.now()

    # MZ test using WSL script

    test_build_directory = os.path.join(divisi_root, "build\\Test_Debug")
    cmake_args = [
        f"{divisi_root}",
        "-G", 
        "Ninja",  
        "-DCMAKE_BUILD_TYPE:STRING=Test_Debug",
        f"-DCMAKE_BINARY_DIR:STRING={test_build_directory}"
        f"-DTEST_DATA_SRC:STRING={divisi_root}\\test\\data"
        f"{divisi_root}"
        f"{test_build_directory}"
    ]
    if os.path.exists(test_build_directory):
        core.remove_read_only(test_build_directory)
        shutil.rmtree(test_build_directory)
    
    os.makedirs(test_build_directory)
    output, error, result = core.run_command("cmake.exe", cmake_args, cwd=test_build_directory)
    if result == False:
        results_md_texts.append(f"| {TEST_NAME} | Failed | CMake | --- |")
        details_md_text.append(f"```\n")
        details_md_text.append(f"- {error}\n")
        details_md_text.append(f"```\n")
        return False

    if result == True:
        output, error, result = core.run_command("ninja", cwd=test_build_directory)
        if result == False:
            results_md_texts.append(f"| {TEST_NAME} | Failed | Ninja | --- |")
            details_md_text.append(f"```\n")
            details_md_text.append(f"- {error}\n")
            details_md_text.append(f"```\n")
            return False

    dll_files = os.listdir(f"{test_build_directory}/bin")
    # DLLファイルをフィルタリングし、コピーする
    for file in dll_files:
        if file.endswith('.dll'):
            source_path = os.path.join(f"{test_build_directory}/bin", file)
            destination_path = os.path.join(test_build_directory, file)
            shutil.copy(source_path, destination_path)

    output, error, result = core.run_command("divisi_test.exe", ["--gtest_output=json:cpp_test_results.json"], cwd=test_build_directory)
    failures = []
    if result == True:
        with open(os.path.join(test_build_directory, 'test_results.json'), 'r') as f:
            failures = extract_failed_tests(f)
        test_result = result and len(failures) == 0
    else:
        test_result = False
    
    end_date = datetime.now()
    
    if test_result:
        results_md_texts.append(f"| {TEST_NAME} | Passed | None | {elapsed_time(start_date, end_date)}(total) |")
    else:
        for failure in failures:
            test_case = failure['test_cast']
            prefix = f"{TEST_NAME}"
            results_md_texts.append(f"| {prefix} | Failed | {test_case} | {elapsed_time(start_date, end_date)}(total) |")

    if not test_result:
        details_md_text.append(f"### Failures in {TEST_NAME} Test\n\n")
        for failure in failures:
            details_md_text.append(f"{failure}\n")
        if not error is None and 0 < len(error):
            details_md_text.append(f"```\n")
            details_md_text.append(f"- {error}\n")
            details_md_text.append(f"```\n")

        if 0 < len(output):
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
    vxace_script_path   = f'{test_plugin_path}\\run_vxace_test.ps1'
    mv_test_script_path = f'{test_plugin_path}\\run_mv_test.sh'
    mz_test_script_path = f'{test_plugin_path}\\run_mz_test.sh'
    cpp_test_script_path = f'{test_plugin_path}\\run_mz_test.sh'
    
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
        all_tests_passed &= run_mv_test(mv_test_script_path, details_md_text, results_md_texts)

    if 'all' in test_selection or 'plugin' in test_selection or 'mz' in test_selection:
        all_tests_passed &= run_mz_test(mz_test_script_path, details_md_text, results_md_texts)
        
    if 'all' in test_selection or 'cpp' in test_selection:
        all_tests_passed &= run_cpp_test(cpp_test_script_path, details_md_text, results_md_texts)

    if 'all' in test_selection or 'divisi' in test_selection:
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

if __name__ == '__main__':
    main()
