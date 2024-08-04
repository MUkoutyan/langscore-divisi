import subprocess
import os
import re
import sys

def convert_path_for_wsl(windows_path):
    drive, path = windows_path.split(':', 1)
    path = path.replace('\\', '/')
    return f'/mnt/{drive.lower()}{path}'

def run_powershell_script(script_path):
    try:
        result = subprocess.run([
            'powershell.exe',
            '-ExecutionPolicy', 'Bypass',  # 一時的に実行ポリシーをバイパス
            '-File', script_path
        ], capture_output=True, text=True, encoding='utf-8')
        # print(f"PowerShell Output: {result.stdout}")
        return result.stdout, result.returncode == 0
    except Exception as e:
        print(f"Failed to run PowerShell script: {e}")
        return str(e), False

def run_wsl_script(script_path):
    try:
        wsl_path = convert_path_for_wsl(script_path)
        result = subprocess.run(
            ['wsl', wsl_path],  # WSL上でスクリプトを直接実行
            capture_output=True, 
            text=True, 
            encoding='utf-8'  # ここでエンコーディングを指定
        )
        # print(f"WSL Bash Output: {result.stdout}")
        return result.stdout, result.returncode == 0
    except Exception as e:
        print(f"Failed to run WSL script: {e}")
        return str(e), False

def run_python_script(script_path):
    try:
        result = subprocess.run(
            ['python', script_path, "-v"],
            capture_output=True, text=True,
            encoding='utf-8'
        )
        return result.stdout, result.returncode == 0
    except Exception as e:
        print(f"Failed to run `Python` script: {e}")
        return str(e), False

def analyze_python_test_result(output):
    success_tests = []
    failed_tests = []
    error_tests = []

    lines = output.splitlines()
    for line in lines:
        # テストケースの成功を示す行
        success_match = re.match(r'test_\w+ \(__main__\.(\w+\.\w+)\) \.\.\. ok', line)
        if success_match:
            success_tests.append(success_match.group(1))
        
        # テストケースの失敗を示す行
        failed_match = re.match(r'test_\w+ \(__main__\.(\w+\.\w+)\) \.\.\. FAIL', line)
        if failed_match:
            failed_tests.append(failed_match.group(1))
        
        # テストケースのエラーを示す行
        error_match = re.match(r'test_\w+ \(__main__\.(\w+\.\w+)\) \.\.\. ERROR', line)
        if error_match:
            error_tests.append(error_match.group(1))

    return success_tests, failed_tests, error_tests

def analyze_ruby_test_result(output):
    # Rubyのテスト結果を解析
    match = re.search(r'(\d+) tests, (\d+) assertions, (\d+) failures, (\d+) errors, (\d+) skips', output)
    failures = []
    if match:
        failure_count = int(match.group(3))
        error_count = int(match.group(4))
        if failure_count > 0 or error_count > 0:
            # 失敗したテストケースを抽出
            failure_matches = re.findall(r'(\d+)\) Failure:\n(.+?) \[(.+?)\]:\n<.+?> expected but was\n<.+?>\.', output, re.DOTALL)
            for index, test_case, location in failure_matches:
                failures.append(f"{index}) {test_case.strip()} [{location.strip()}]")
    return failures

def analyze_jest_test_result(output, is_nwjs):
    # Jestのテスト結果を解析
    failures = []
    # 失敗したテストケースを抽出する正規表現
    failing_tests = re.findall(r'^\s*\d+\) (.+)\n', output, re.MULTILINE)
    for test_case in failing_tests:
        failures.append((test_case.strip(), is_nwjs))
    return failures

def process_logs(logs):
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
        mz_failures.extend(analyze_jest_test_result('\n'.join(nwjs_logs), True))
        
    if 0 < len(browser_logs):
        mz_failures.extend(analyze_jest_test_result('\n'.join(browser_logs), False))

    return mz_failures

def escape_ansi(line):
    ansi_escape = re.compile(r'(?:\x1B[@-_]|[\x80-\x9F][ -/]*[@-~])')
    return ansi_escape.sub('', line)

def convert_ansi_to_html(text):
    ansi_color_code_to_html = {
        '31': 'red', '32': 'green', '33': 'yellow', '34': 'blue',
        '35': 'magenta', '36': 'cyan', '37': 'white', '90': 'grey'
    }
    ansi_escape = re.compile(r'\x1B\[(?P<code>\d+);1m')
    html_text = text
    for match in ansi_escape.finditer(text):
        color = ansi_color_code_to_html.get(match.group('code'), 'black')
        html_text = html_text.replace(match.group(0), f'<span style="color:{color}">').replace('\x1B[0m', '</span>')
    return html_text

def main():
    if len(sys.argv) < 2:
        test_selection = ['all']
    else:
        test_selection = sys.argv[1:]  

    print(f"run tests {test_selection}")

    current_path = os.getcwd()
    
    # 各スクリプトのパスを生成
    rvcnv_script_path = f'{current_path}/run_rvcnv_test.py'
    vxace_script_path = f'{current_path}/run_vxace_test.ps1'
    mv_test_script_path = f'{current_path}/run_mv_test.sh'
    mz_test_script_path = f'{current_path}/run_mz_test.sh'
    
    # ログファイルのパス
    log_file_path = f'{current_path}/test_log.md'
    results = []
    details = []
    
    with open(log_file_path, 'w', encoding='utf-8') as log_file:
        rvcnv_test_result = []
        vxace_test_result = []
        mv_test_result = []
        mz_test_result = []
        
        all_tests_passed = True
        if 'all' in test_selection or 'rvcnv' in test_selection:
            rvcnv_output, rvcnv_result = run_python_script(rvcnv_script_path)
            rvcnv_success, rvcnv_failures, rvcnv_error = analyze_python_test_result(rvcnv_output)
            rvcnv_test_result = (len(rvcnv_failures) == 0)
            all_tests_passed &= rvcnv_test_result
            
            if not rvcnv_failures:
                results.append("| rvcnv | Passed | None |")
            else:
                for failure in rvcnv_failures:
                    results.append(f"| rvcnv | Failed | {failure} |")

            print(f"rvcnv test {'succeeded' if rvcnv_test_result else 'failed'}")
            if rvcnv_failures:
                details.append("### Failures in rvcnv Test\n\n")
                for failure in rvcnv_failures:
                    details.append(f"- {failure}\n")
            details.append("## rvcnv Test Output\n\n")
            details.append("```\n" + rvcnv_output.rstrip() + "\n```\n\n")

        if 'all' in test_selection or 'vxace' in test_selection:
            # VXAce test using PowerShell on Windows
            vxace_output, vxace_result = run_powershell_script(vxace_script_path)
            vxace_failures = analyze_ruby_test_result(vxace_output)
            vxace_test_result = len(vxace_failures) == 0
            all_tests_passed &= vxace_test_result
            
            if not vxace_failures:
                results.append("| VXAce | Passed | None |")
            else:
                for failure in vxace_failures:
                    results.append(f"| VXAce | Failed | {failure} |")

            print(f"VXAce test {'succeeded' if vxace_test_result else 'failed'}")
            if vxace_failures:
                details.append("### Failures in VXAce Test\n\n")
                for failure in vxace_failures:
                    details.append(f"- {failure}\n")
            details.append("## VXAce Test Output\n\n")
            details.append("```\n" + vxace_output.rstrip() + "\n```\n\n")

        if 'all' in test_selection or 'mv' in test_selection:
            # MV test using WSL script
            mv_output, mv_result = run_wsl_script(mv_test_script_path)
            mv_failures = process_logs(mv_output)
            mv_test_result = len(mv_failures) == 0
            all_tests_passed &= mv_test_result
            
            if not mv_failures:
                results.append("| MV | Passed | None |")
            else:
                for failure in mv_failures:
                    is_nwjs = failure[1]
                    test_case = failure[0]
                    prefix = "MV (NWJs)" if is_nwjs else "MV (Browser)"
                    results.append(f"| {prefix} | Failed | {test_case} |")

            print(f"MV test {'succeeded' if mv_test_result else 'failed'}")
            if mv_failures:
                details.append("### Failures in MV Test\n\n")
                for failure in mv_failures:
                    # print(f"- {failure}")
                    details.append(f"- **{ 'NWjs' if failure[1] else 'Browser' }** {failure}\n")
            details.append("## MV Test Output\n\n")
            details.append("```\n" + mv_output.rstrip() + "\n```\n\n")

        if 'all' in test_selection or 'mz' in test_selection:
            # MZ test using WSL script
            mz_output, mz_result = run_wsl_script(mz_test_script_path)
            mz_failures = process_logs(mz_output)
            mz_test_result = len(mz_failures) == 0
            all_tests_passed &= mz_test_result
            
            if not mz_failures:
                results.append("| MZ | Passed | None |")
            else:
                for failure in mz_failures:
                    is_nwjs = failure[1]
                    test_case = failure[0]
                    prefix = "MZ (NWJs)" if is_nwjs else "MZ (Browser)"
                    results.append(f"| {prefix} | Failed | {test_case} |")

            print(f"MZ test {'succeeded' if mz_test_result else 'failed'}")
            if mz_failures:
                details.append("### Failures in MZ Test\n\n")
                for failure in mz_failures:
                    details.append(f"- **{ 'NWjs' if failure[1] else 'Browser' }** {failure[0]}\n")
            details.append("## MZ Test Output\n\n")
            details.append("```\n" + mz_output.rstrip() + "\n```\n\n")

        # Overall result
        print(f"All tests {'passed' if all_tests_passed else 'failed'}")
        log_file.write(f"\n## Overall Result: {'Passed' if all_tests_passed else 'Failed'}\n")

        # テーブルのヘッダー
        log_file.write("| Test Suite | Result | Failures |\n")
        log_file.write("|------------|--------|----------|\n")
        
        # テーブルの内容を書き出す
        for result in results:
            log_file.write(result + "\n")

        # 詳細を書き出す
        log_file.write("\n".join(details))

if __name__ == '__main__':
    main()
