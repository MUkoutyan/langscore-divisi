import subprocess
import re
import os
import stat
import locale

def remove_read_only(folder_path):
    try:
        for root, dirs, files in os.walk(folder_path):
            for file in files:
                file_path = os.path.join(root, file)
                os.chmod(file_path, stat.S_IWRITE)  # Remove read-only flag    
    except Exception as e:
        print(f"Failed to run command: {e}")

def convert_path_for_wsl(windows_path):
    drive, path = windows_path.split(':', 1)
    path = path.replace('\\', '/')
    return f'/mnt/{drive.lower()}{path}'

    
def run_command(command_path, args=None, **option):
    try:
        system_encoding = locale.getpreferredencoding()
        command = []
        command.append(command_path)
        if args: command += args
        result = subprocess.run(command, 
            capture_output=True, text=True, shell=True, 
            encoding='utf-8', timeout=180, **option
        )
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run command: {e}")
        return str(), str(e), False

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
            encoding='utf-8', timeout=180, **option
        )
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run PowerShell script: {e}")
        return str(), str(e), False

def run_wsl_script(script_path, args=None, **option):
    try:
        wsl_path = convert_path_for_wsl(script_path)
        result = subprocess.run(
            ['wsl', wsl_path],  # WSL上でスクリプトを直接実行
            capture_output=True, text=True, 
            encoding='utf-8', timeout=180, **option
        )
        # print(f"WSL Bash Output: {result.stdout}")
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run WSL script: {e}")
        return str(), str(e), False

def run_python_script(script_path):
    try:
        result = subprocess.run(
            ['python', script_path, "-v"],
            capture_output=True, text=True,
            encoding='utf-8', timeout=180
        )
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run `Python` script: {e}")
        return str(), str(e), False


def run_ruby_script(script_path, args=None, **option):
    try:
        result = subprocess.run(
            ['ruby', script_path],
            capture_output=True, text=True,
            encoding='utf-8', timeout=180
        )
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run `Ruby` script: {e}")
        return str(), str(e), False

def analyze_python_test_result(output):
    success_tests = []
    failed_tests = []
    error_tests = []

    lines = output.splitlines()
    for line in lines:
        # テストケースの成功を示す行
        success_match = re.match(r'\w* \(__main__\.(\w+\.?\w+)\) \.\.\. ok', line)
        if success_match:
            success_tests.append(success_match.group(1))
        
        # テストケースの失敗を示す行
        failed_match = re.match(r'\w* \(__main__\.(\w+\.?\w+)\) \.\.\. FAIL', line)
        if failed_match:
            failed_tests.append(failed_match.group(1))
        
        # テストケースのエラーを示す行
        error_match = re.match(r'\w* \(__main__\.(\w+\.?\w+)\) \.\.\. ERROR', line)
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