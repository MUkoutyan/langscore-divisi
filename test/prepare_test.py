import subprocess
import os
import sys
import shutil
from internal import test_core

def setup_vs_environment(vs_path="C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Auxiliary\\Build\\vcvarsall.bat"):
    # vcvarsall.bat の実行
    command = f'"{vs_path}" x64 && set'
    
    # subprocessでコマンドを実行して環境変数を取得
    result = subprocess.run(command, shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    
    if result.returncode != 0:
        raise RuntimeError(f"Failed to set up Visual Studio environment: {result.stderr}")
    
    # 出力された環境変数を解析
    env = {}
    for line in result.stdout.splitlines():
        if "=" in line:
            key, value = line.split("=", 1)
            env[key] = value
    return env


current_dir = os.path.dirname(os.path.abspath(__file__))
divisi_root = os.path.abspath(f"{current_dir}/..")

def build_divisi_with_vs():
    print("build")
    win_build_dir = os.path.abspath(f"{divisi_root}/build/x64-release")
    linux_build_dir = os.path.abspath(f"{divisi_root}/build-linux")

    # ディレクトリが存在しない場合は作成
    if not os.path.exists(win_build_dir):
        os.makedirs(win_build_dir)
    else:
        shutil.rmtree(win_build_dir)
        os.makedirs(win_build_dir)

    if not os.path.exists(linux_build_dir):
        os.makedirs(linux_build_dir)
    else:
        shutil.rmtree(linux_build_dir)
        os.makedirs(linux_build_dir)

    win_cmake_args = [divisi_root, "-G", "Ninja", "-DCMAKE_BUILD_TYPE:STRING=Release"]
    win_build_command = "ninja"
    
    linux_cmake_args = ["..", "-DCMAKE_BUILD_TYPE:STRING=Release", " -DLLVM_USE_LINKER=mold"]
    linux_build_command = f"{divisi_root}/build_linux.sh"

    try:
        rv_stdout, rv_stderr, rv_result = test_core.run_powershell_script("build.ps1", cwd=f"{divisi_root}/rvcnv")
        if rv_result == False:
            print(f"Error RVCNV Build {rv_stderr}")
            exit(1)
        print("Complete build rvcnv")

        # VS環境を設定
        env = setup_vs_environment()

        win_stdout, win_stderr, win_result = test_core.run_command("cmake", win_cmake_args, cwd=win_build_dir, check=True, env=env)
        if win_result == False:
            print(f"Error Windows CMake {win_stderr}")
            exit(1)
        win_stdout, win_stderr, win_result = test_core.run_command(win_build_command, cwd=win_build_dir, check=True, env=env)
        if win_result == False:
            print(f"Error Windows Build {win_stderr}")
            exit(1)
        print("Complete build divisi(win)")
         
        lx_stdout, lx_stderr, lx_result = test_core.run_wsl_script(linux_build_command, cwd=divisi_root, check=True)
        if lx_result == False:
            print(f"Error Linux Build {lx_stderr}")
            exit(1)
        print("Complete build divisi(linux)")

        copy_files = [
		    f"{divisi_root}/resource/Langscore.js",
		    f"{divisi_root}/resource/Langscore.rb",
		    f"{divisi_root}/resource/lscsv.js",
		    f"{divisi_root}/resource/lscsv.rb",
		    f"{divisi_root}/resource/vocab.csv",
            f"{divisi_root}/rvcnv/rvcnv.exe",
            f"{win_build_dir}/divisi.exe",
            f"{linux_build_dir}/divisi"
        ]

        if not os.path.exists(f"{divisi_root}/bin"):
            os.mkdir(f"{divisi_root}/bin")
            
        if not os.path.exists(f"{divisi_root}/bin/resource"):
            os.mkdir(f"{divisi_root}/bin/resource")

        for f in copy_files:
            file_name, ext = os.path.splitext(f)
            if ext == ".exe" or ext == "":
                dest = os.path.abspath(os.path.join(f"{divisi_root}/bin", os.path.basename(f)))
            else:
                dest = os.path.abspath(os.path.join(f"{divisi_root}/bin/resource", os.path.basename(f)))
            
            shutil.copyfile(f, dest)

        print("Build process completed successfully.")

    except subprocess.CalledProcessError as e:
        print(f"Error occurred")
        if e.stderr:
            for line in e.stderr.splitlines():
                if line: print(line)
    except RuntimeError as e:
        print(f"Setup failed: {e}")

def update_test_projects():
    print("update")
    test_dir = os.path.dirname(os.path.abspath(__file__))
    plugin_dir = os.path.join(test_dir, "plugin")

    test_core.run_command(
        f"{divisi_root}/bin/divisi.exe", 
        ["-c", f"{plugin_dir}/vxace_test_langscore/config.json", "--write"]
    )   
    test_core.run_command(
        f"{divisi_root}/bin/divisi.exe", 
        ["-c", f"{plugin_dir}/vxace_test_langscore/config.json", "--packing"]
    )
    test_core.run_command(
        f"{divisi_root}/bin/divisi.exe", 
        ["-c", f"{plugin_dir}/mv_test_langscore/config.json", "--write"]
    )
    test_core.run_command(
        f"{divisi_root}/bin/divisi.exe", 
        ["-c", f"{plugin_dir}/mz_test_langscore/config.json", "--write"]
    )

    before_path = os.getcwd()
    os.chdir(os.path.join(plugin_dir, "vxace_test"))
    test_core.run_ruby_script(os.path.join(plugin_dir, "vxace_test\\decompress.rb"), cwd=os.path.join(plugin_dir, "vxace_test"), check=True)
    os.chdir(before_path)

    print("Update projects successfully.")


if __name__ == '__main__':
    test_selection = []
    if 1 < len(sys.argv):
        test_selection = sys.argv[1:]

    if len(test_selection) == 0:
        build_divisi_with_vs()
        update_test_projects()
    elif test_selection[0] == "build":
        build_divisi_with_vs()
    elif test_selection[0] == "update":
        update_test_projects()
    
