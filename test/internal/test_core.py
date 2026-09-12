import subprocess
import re
import os
import stat
import sys
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
    
TEST_DATA_DIR_NAME = "langscore-divisi-test-data"

def test_data_root():
    """テストデータ (ツクールのゲームプロジェクト) の置き場所を返す。

    サイズと再配布の都合でリポジトリには含めないため、既定ではリポジトリと同階層の
    langscore-divisi-test-data を参照する。環境変数 LANGSCORE_TEST_DATA で上書きできる。
    """
    env_path = os.environ.get("LANGSCORE_TEST_DATA")
    if env_path:
        return os.path.abspath(env_path)

    repo_root = os.path.abspath(os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", ".."))
    return os.path.abspath(os.path.join(repo_root, "..", TEST_DATA_DIR_NAME))


def require_test_data(*sub_paths):
    """テストデータ内のパスを返す。見つからない場合は理由を示して終了する。"""
    root = test_data_root()
    path = os.path.join(root, *sub_paths) if sub_paths else root
    if not os.path.exists(root):
        print(f"テストデータが見つかりません: {root}")
        print(f"  リポジトリと同階層に {TEST_DATA_DIR_NAME} を配置するか、")
        print(f"  環境変数 LANGSCORE_TEST_DATA で場所を指定してください。")
        sys.exit(1)
    return path


def find_vcvars():
    """Visual Studio の vcvars64.bat を探す。バージョンは決め打ちにしない。"""
    program_files_x86 = os.environ.get("ProgramFiles(x86)", "C:\\Program Files (x86)")
    vswhere = os.path.join(program_files_x86, "Microsoft Visual Studio", "Installer", "vswhere.exe")
    if os.path.exists(vswhere):
        result = subprocess.run(
            [vswhere, "-latest", "-products", "*",
             "-requires", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
             "-property", "installationPath"],
            capture_output=True, text=True, errors='replace'
        )
        for install_path in result.stdout.splitlines():
            vcvars = os.path.join(install_path.strip(), "VC", "Auxiliary", "Build", "vcvars64.bat")
            if os.path.exists(vcvars):
                return vcvars

    # vswhere が無い場合は既定の配置を新しいものから探す
    program_files = os.environ.get("ProgramFiles", "C:\\Program Files")
    for root in [program_files, program_files_x86]:
        base = os.path.join(root, "Microsoft Visual Studio")
        if not os.path.isdir(base):
            continue
        for version in sorted(os.listdir(base), reverse=True):
            for edition in ("Community", "Professional", "Enterprise", "BuildTools"):
                vcvars = os.path.join(base, version, edition, "VC", "Auxiliary", "Build", "vcvars64.bat")
                if os.path.exists(vcvars):
                    return vcvars
    return None


_vs_environment_cache = None

def vs_environment():
    """cl.exe / ninja を実行できる環境変数を返す。
    開発者コマンドプロンプトから実行されている場合や、VSが見つからない場合はNone
    (= 現在の環境をそのまま使う) を返す。"""
    global _vs_environment_cache

    if os.environ.get("VSCMD_ARG_TGT_ARCH"):
        return None
    if _vs_environment_cache is not None:
        return _vs_environment_cache

    vcvars = find_vcvars()
    if vcvars is None:
        print("Warning: vcvars64.bat not found. Run from a Developer Command Prompt if the build fails.")
        return None

    result = subprocess.run(f'"{vcvars}" >nul && set', shell=True,
                            capture_output=True, text=True,
                            encoding=locale.getpreferredencoding(), errors='replace')
    if result.returncode != 0:
        print(f"Warning: failed to run {vcvars}")
        return None

    env = dict(os.environ)
    for line in result.stdout.splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            env[key] = value
    _vs_environment_cache = env
    return env


def _run(command, default_encoding=None, **option):
    """subprocess.run の共通ラッパー。
    encoding と timeout は呼び出し側で上書きできる。(二重指定でTypeErrorにならないようpopする)
    復号できない文字があってもテストを落とさないよう errors='replace' を使う。"""
    encoding = option.pop('encoding', default_encoding or locale.getpreferredencoding())
    # ビルドやテストデータのコピーは既定の180秒では収まらないため、呼び出し側で上書きできる。
    timeout = option.pop('timeout', 180)
    return subprocess.run(command,
        capture_output=True, text=True,
        encoding=encoding, errors='replace', timeout=timeout, **option
    )

def run_command(command_path, args=None, **option):
    try:
        command = []
        command.append(command_path)
        if args: command += args
        result = _run(command, shell=True, **option)
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run command: {e}")
        return str(e), str(e), False

def run_powershell_script(script_path, args=None, **option):
    try:
        command = [
            'powershell.exe',
            '-ExecutionPolicy', 'Bypass',  # 一時的に実行ポリシーをバイパス
            '-File', script_path
        ]

        if args: 
            command += [
                '-Args', args
            ]
        result = _run(command, **option)
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
        result = _run(command, default_encoding='utf-8', **option)
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
        result = _run(command, default_encoding='utf-8', timeout=option.pop('timeout', _timeout), **option)
        return result.stdout, result.stderr, result.returncode == 0
    except Exception as e:
        print(f"Failed to run `Python` script: {e}")
        return str(), str(e), False


def run_ruby_script(script_path, args=None, **option):
    try:
        command = ['ruby']
        command.append(script_path)
        if args: command += args
        result = _run(command, default_encoding='utf-8', **option)
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

def analyze_vxace_test_result(output):
    # run_vxace_test.py の出力から失敗したテストを抽出する
    pattern = r'(\d+)\) Failure:\n(.+?) \[(.+?)\]:\n(.+)'
    matches = re.findall(pattern, output)
    return [f"{i}) {name} [{suite}] {message}" for i, name, suite, message in matches]

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
    # mocha の出力から失敗したテストケースを抽出する。
    # 末尾の "N failing" 以降はスイート名付きの再掲なので、それより前 (テスト一覧) だけを見る。
    failures = []
    listing = re.split(r'^\s*\d+ failing', output or '', maxsplit=1, flags=re.MULTILINE)[0]
    failing_tests = re.findall(r'^\s*\d+\) (.+)$', listing, re.MULTILINE)
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