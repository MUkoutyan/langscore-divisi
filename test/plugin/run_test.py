import os
import sys
import io
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding='utf-8')

current_dir = os.path.dirname(os.path.abspath(__file__))
# rootディレクトリのパスを取得してsys.pathに追加
test_root_dir = os.path.abspath(os.path.join(current_dir, '../'))
sys.path.append(test_root_dir)
from internal import test_core as core


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
        mz_failures.extend(core.analyze_jest_test_result('\n'.join(nwjs_logs), True))
        
    if 0 < len(browser_logs):
        mz_failures.extend(core.analyze_jest_test_result('\n'.join(browser_logs), False))

    return mz_failures


def main():
    if len(sys.argv) < 2:
        test_selection = ['all']
    else:
        test_selection = sys.argv[1:]  

    print(f"run tests {test_selection}")

    current_path = os.getcwd()
    print(current_path)
    
    # 各スクリプトのパスを生成
    rvcnv_script_path = f'{current_path}\\run_rvcnv_test.py'
    vxace_script_path = f'{current_path}\\run_vxace_test.ps1'
    mv_test_script_path = f'{current_path}\\run_mv_test.sh'
    mz_test_script_path = f'{current_path}\\run_mz_test.sh'
    
    # ログファイルのパス
    log_file_path = f'{current_path}\\test_log.md'
    results = []
    details = []
    
    with open(log_file_path, 'w', encoding='utf-8') as log_file:
        rvcnv_test_result = []
        vxace_test_result = []
        mv_test_result = []
        mz_test_result = []
        
        all_tests_passed = True
        if 'all' in test_selection or 'rvcnv' in test_selection:
            rvcnv_output, rvcnv_err, rvcnv_result = core.run_python_script(rvcnv_script_path)
            rvcnv_success, rvcnv_failures, rvcnv_error = core.analyze_python_test_result(rvcnv_output)
            rvcnv_test_result = (len(rvcnv_failures) == 0) and (len(rvcnv_error) == 0)
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
            vxace_output, vxace_err, vxace_result = core.run_powershell_script(vxace_script_path)
            vxace_failures = core.analyze_ruby_test_result(vxace_output)
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
            mv_output, mv_err, mv_result = core.run_wsl_script(mv_test_script_path)
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
            mz_output, mz_err,  mz_result = core.run_wsl_script(mz_test_script_path)
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
        
        log_file.write("\n\n")
        # 詳細を書き出す
        log_file.write("\n".join(details))

if __name__ == '__main__':
    main()
