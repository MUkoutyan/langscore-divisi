import os
import shutil
import subprocess
import filecmp
import pytest

# テスト用の一時ディレクトリやサンプルプロジェクトパスを適宜指定してください
testdata_dir = os.path.join(os.path.dirname(__file__), 'data')
temp_dir = os.path.join(os.path.dirname(__file__), 'temp')
divisi_exe = os.path.abspath(os.path.join('..', 'bin', 'divisi.exe'))
config_path = os.path.join(testdata_dir, 'config.json')

@pytest.fixture(autouse=True)
def setup_and_teardown():
    # テスト前にtempディレクトリをクリーン
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)
    shutil.copytree(testdata_dir, temp_dir)
    yield
    # テスト後にクリーン
    if os.path.exists(temp_dir):
        shutil.rmtree(temp_dir)

def run_divisi(args, cwd=None):
    cmd = [divisi_exe] + args
    result = subprocess.run(cmd, cwd=cwd or temp_dir, capture_output=True, text=True)
    return result

def compare_files(file1, file2):
    return filecmp.cmp(file1, file2, shallow=False)

def compare_dirs(dir1, dir2):
    dcmp = filecmp.dircmp(dir1, dir2)
    if dcmp.left_only or dcmp.right_only or dcmp.diff_files:
        return False
    for sub_dcmp in dcmp.subdirs.values():
        if not compare_dirs(sub_dcmp.left, sub_dcmp.right):
            return False
    return True

# --- ヘルパー関数群 ---
import json


# --- analyze テスト本体 ---
def test_analyze():
    projects = ['LangscoreTestMV', 'LangscoreTestMZ', 'LangscoreTestVXAce']
    for proj in projects:
        temp_proj_dir, config, out_dir = prepare_project(proj)
        proj_config = os.path.join(temp_proj_dir, 'config.json')
        result = run_divisi(['--analyze', '-c', proj_config], cwd=temp_proj_dir)
        assert result.returncode == 0, f"analyze failed for {proj}: {result.stderr}"
        assert os.path.exists(out_dir), f"Output dir not found for {proj}: {out_dir}"
        assert_scripts_or_plugins(proj, temp_proj_dir)
        assert_all_files_exist(out_dir)
        assert_languages_in_config(config, proj)
        assert not result.stderr, f"stderr not empty for {proj}: {result.stderr}"
# --- 共通ヘルパー ---
def prepare_project(proj):
    proj_dir = os.path.join(testdata_dir, proj)
    temp_proj_dir = os.path.join(temp_dir, proj)
    if os.path.exists(temp_proj_dir):
        shutil.rmtree(temp_proj_dir)
    shutil.copytree(proj_dir, temp_proj_dir)
    proj_config = os.path.join(temp_proj_dir, 'config.json')
    with open(proj_config, encoding='utf-8') as f:
        config = json.load(f)
    out_dir = get_export_dir(config, temp_proj_dir, proj)
    return temp_proj_dir, config, out_dir

def get_export_dir(config, temp_proj_dir, proj):
    export_dir = config.get('Write', {}).get('ExportDirectory')
    if export_dir:
        return os.path.normpath(os.path.join(temp_proj_dir, export_dir))
    if 'VXAce' in proj:
        return os.path.join(temp_proj_dir, 'Data', 'Translate')
    return os.path.join(temp_proj_dir, 'data', 'translate')

def assert_scripts_or_plugins(proj, temp_proj_dir):
    if 'VXAce' in proj:
        scripts_dir = os.path.join(temp_proj_dir, 'Scripts')
        assert os.path.exists(os.path.join(scripts_dir, '_list.csv')), f"_list.csv not found for {proj}"
        rb_files = [f for f in os.listdir(scripts_dir) if f.endswith('.rb')]
        assert rb_files, f"No .rb files found in Scripts for {proj}"
    else:
        js_plugins_dir = os.path.join(temp_proj_dir, 'js', 'plugins')
        assert os.path.exists(os.path.join(js_plugins_dir, 'Langscore.js')), f"Langscore.js not found for {proj}"
        assert os.path.exists(os.path.join(js_plugins_dir, 'Langscore_ObserverBridge.js')), f"Langscore_ObserverBridge.js not found for {proj}"

def assert_all_files_exist(out_dir):
    for root, dirs, files in os.walk(out_dir):
        for name in files:
            assert os.path.exists(os.path.join(root, name)), f"File missing: {os.path.join(root, name)}"

def assert_languages_in_config(config, proj):
    langs = config.get('Languages') or config.get('Write', {}).get('Languages')
    assert langs, f"Languages not set in config for {proj}"
    default_lang = config.get('DefaultLanguage') or config.get('Write', {}).get('DefaultLanguage')
    assert default_lang is not None, f"DefaultLanguage not set in config for {proj}"

def test_reanalysis():
    projects = ['LangscoreTestMV', 'LangscoreTestMZ', 'LangscoreTestVXAce']
    for proj in projects:
        temp_proj_dir, config, out_dir = prepare_project(proj)
        proj_config = os.path.join(temp_proj_dir, 'config.json')
        result = run_divisi(['--reanalysis', '-c', proj_config], cwd=temp_proj_dir)
        assert result.returncode == 0, f"reanalysis failed for {proj}: {result.stderr}"
        assert os.path.exists(out_dir), f"Output dir not found for {proj}: {out_dir}"
        assert_scripts_or_plugins(proj, temp_proj_dir)
        assert not result.stderr, f"stderr not empty for {proj}: {result.stderr}"

def test_export_csv():
    projects = ['LangscoreTestMV', 'LangscoreTestMZ', 'LangscoreTestVXAce']
    for proj in projects:
        temp_proj_dir, config, out_dir = prepare_project(proj)
        proj_config = os.path.join(temp_proj_dir, 'config.json')
        result = run_divisi(['--exportCSV', '-c', proj_config], cwd=temp_proj_dir)
        assert result.returncode == 0, f"exportCSV failed for {proj}: {result.stderr}"
        assert os.path.exists(out_dir), f"Output dir not found for {proj}: {out_dir}"
        csv_files = [f for f in os.listdir(out_dir) if f.endswith('.csv')]
        assert csv_files, f"No CSV files found in {out_dir} for {proj}"
        assert not result.stderr, f"stderr not empty for {proj}: {result.stderr}"

def test_update_plugin():
    projects = ['LangscoreTestMV', 'LangscoreTestMZ', 'LangscoreTestVXAce']
    for proj in projects:
        temp_proj_dir, config, out_dir = prepare_project(proj)
        proj_config = os.path.join(temp_proj_dir, 'config.json')
        result = run_divisi(['--updatePlugin', '-c', proj_config], cwd=temp_proj_dir)
        assert result.returncode == 0, f"updatePlugin failed for {proj}: {result.stderr}"
        assert_scripts_or_plugins(proj, temp_proj_dir)
        assert not result.stderr, f"stderr not empty for {proj}: {result.stderr}"

def test_validate():
    projects = ['LangscoreTestMV', 'LangscoreTestMZ', 'LangscoreTestVXAce']
    for proj in projects:
        temp_proj_dir, config, out_dir = prepare_project(proj)
        proj_config = os.path.join(temp_proj_dir, 'config.json')
        result = run_divisi(['--validate', '-c', proj_config], cwd=temp_proj_dir)
        assert result.returncode == 0, f"validate failed for {proj}: {result.stderr}"
        assert not result.stderr, f"stderr not empty for {proj}: {result.stderr}"

def test_packing():
    projects = ['LangscoreTestMV', 'LangscoreTestMZ', 'LangscoreTestVXAce']
    for proj in projects:
        temp_proj_dir, config, out_dir = prepare_project(proj)
        proj_config = os.path.join(temp_proj_dir, 'config.json')
        result = run_divisi(['--packing', '-c', proj_config], cwd=temp_proj_dir)
        assert result.returncode == 0, f"packing failed for {proj}: {result.stderr}"
        assert os.path.exists(out_dir), f"Output dir not found for {proj}: {out_dir}"
        assert not result.stderr, f"stderr not empty for {proj}: {result.stderr}"

def test_create_config():
    game_project_path = os.path.join(temp_dir, 'SampleGame')
    result = run_divisi(['--createConfigFile', game_project_path])
    assert result.returncode == 0
    # assert os.path.exists(os.path.join(game_project_path, 'config.json'))
