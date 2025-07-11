import subprocess
import re
import os
import stat
import locale
import json

def remove_read_only(folder_path):
    try:
        for root, dirs, files in os.walk(folder_path):
            for file in files:
                file_path = os.path.join(root, file)
                os.chmod(file_path, stat.S_IWRITE)  # Remove read-only flag    
    except Exception as e:
        print(f"Failed to run command: {e}")

def convert_path_for_wsl(windows_path):
    try:
        drive, path = windows_path.split(':', 1)
        path = path.replace('\\', '/')
        return f'/mnt/{drive.lower()}{path}'
    except Exception as e:
        return windows_path

def edit_ls_config(config_path, edit_function):

    with open(config_path, 'r') as file:
        data = json.load(file)

    edit_function(data)

    with open(config_path, 'w') as file:
        json.dump(data, file, indent=4)  
    
def run_command(command_path, args=None, **option):
    try:
        system_encoding = locale.getpreferredencoding()
        command = []
        command.append(command_path)
        if args: command += args
        result = subprocess.run(command, 
            capture_output=True, text=True, shell=True, 
            encoding=system_encoding, timeout=180, **option
        )
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run command: {e}")
        return str(e), str(e.stdout), False

def run_powershell_script(script_path, args=None, **option):
    try:
        system_encoding = locale.getpreferredencoding()
        command = [
            'powershell.exe',
            '-ExecutionPolicy', 'Bypass',  # 一時的に実行ポリシーをバイパス
            '-File', script_path
        ]

        if args: 
            command += [
                '-Args', args
            ]
        result = subprocess.run(command, capture_output=True, text=True, 
            encoding=system_encoding, timeout=180, **option
        )
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run PowerShell script: {e}")
        return str(), str(e), False

def run_wsl_script(script_path, args=None, **option):
    try:
        wsl_path = convert_path_for_wsl(script_path)
        command = ['wsl']
        command.append(wsl_path)
        if args: command += args
        result = subprocess.run(
            command,  # WSL上でスクリプトを直接実行
            capture_output=True, text=True, 
            encoding='utf-8', timeout=180, **option
        )
        # print(f"WSL Bash Output: {result.stdout}")
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run WSL script: {e}")
        return str(), str(e), False

def run_python_script(script_path, args=None, _timeout=180, **option):
    try:
        command = ['python']
        command.append(script_path)
        if args: command += args
        command.append('-v')
        result = subprocess.run(
            command,
            capture_output=True, text=True,
            encoding='utf-8', timeout=_timeout, **option
        )
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run `Python` script: {e}")
        return str(), str(e), False


def run_ruby_script(script_path, args=None, **option):
    try:
        command = ['ruby']
        command.append(script_path)
        if args: command += args
        result = subprocess.run(
            command,
            capture_output=True, text=True,
            encoding='utf-8', timeout=180, **option
        )
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run `Ruby` script: {e}")
        return str(), str(e), False

def analyze_python_test_result(output):
    success_tests = set()
    failed_tests = {}

    if output == None:
        return list(success_tests), failed_tests
    
    lines = output.splitlines()
    failed_test_name = None

    for line in lines:
        # テストの結果判定
        success_match = re.match(r'(\w+) \(__main__\.(\w+\.?\w+)\) \.\.\. ok', line)
        failed_match = re.match(r'(\w+) \(__main__\.(\w+\.?\w+)\) \.\.\. FAIL', line)

        if success_match:
            success_tests.add(success_match.group(2))
        elif failed_match:
            failed_test_name = failed_match.group(2)
            failed_tests[failed_test_name] = ""  # 失敗テストとして登録

        # 失敗の詳細を取得
        if failed_test_name and "Traceback" in line:
            trace_index = lines.index(line) + 1  # Traceback の次の行からメッセージを探す
            error_message = []
            while trace_index < len(lines) and lines[trace_index].strip():
                error_message.append(lines[trace_index].strip())
                trace_index += 1
            failed_tests[failed_test_name] = " ".join(error_message)  # 失敗理由を格納
            failed_test_name = None  # 解析が終わったのでリセット

    return list(success_tests), failed_tests

def analyze_ruby19_test_result(output):
    # Ruby1.9.2でのテスト結果を解析
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

def analyze_ruby_test_result(output):
    # Rubyのテスト結果を解析
    lines = output.splitlines()
    tests, assertions, failures, errors, skips = 0, 0, 0, 0, 0
    pendings, omissions, notifications = 0, 0, 0
    failure_cases = []

    for line in lines:
        # 各行でテスト結果を検出
        if re.search(r'\d+ tests', line):
            tests = int(re.search(r'(\d+) tests', line).group(1))
        if re.search(r'\d+ assertions', line):
            assertions = int(re.search(r'(\d+) assertions', line).group(1))
        if re.search(r'\d+ failures', line):
            failures = int(re.search(r'(\d+) failures', line).group(1))
        if re.search(r'\d+ errors', line):
            errors = int(re.search(r'(\d+) errors', line).group(1))
        if re.search(r'\d+ skips', line):
            skips = int(re.search(r'(\d+) skips', line).group(1))
        if re.search(r'\d+ pendings', line):
            pendings = int(re.search(r'(\d+) pendings', line).group(1))
        if re.search(r'\d+ omissions', line):
            omissions = int(re.search(r'(\d+) omissions', line).group(1))
        if re.search(r'\d+ notifications', line):
            notifications = int(re.search(r'(\d+) notifications', line).group(1))

    if failures > 0 or errors > 0:
        # 失敗したテストケースを抽出
        failure_matches = re.findall(r'Failure: (.*)\'', output, re.DOTALL)
        for lines in failure_matches:
            failure_cases += lines.splitlines()

    return (tests, assertions, failures, errors, skips, pendings, omissions, notifications), failure_cases, []


def analyze_jest_test_result(output, is_nwjs):
    # Jestのテスト結果を解析
    failures = []
    # 失敗したテストケースを抽出する正規表現
    failing_tests = re.findall(r'^\s*\d+\) (.+)\n', output, re.MULTILINE)
    for test_case in failing_tests:
        failures.append((test_case.strip(), is_nwjs))
    return [], failures, []

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