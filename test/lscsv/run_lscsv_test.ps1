# 出力をキャプチャするための変数
$output = ""

# 出力を標準出力に加え、キャプチャする関数
function Write-And-Capture {
    param (
        [string]$message
    )
    Write-Host $message
    $global:output += $message + "`n"
}

# Rubyスクリプトの実行
Write-And-Capture "Running Ruby script..."
$rubyScript = "./lscsv_test.rb"
ruby $rubyScript
if ($LASTEXITCODE -ne 0) {
    Write-And-Capture "Ruby script failed with exit code $LASTEXITCODE"
    $output | Write-Error
    exit $LASTEXITCODE
}

# Pythonスクリプトの実行
Write-And-Capture "Running Python script..."
$pythonScript = "./lscsv_test.py"
python $pythonScript
if ($LASTEXITCODE -ne 0) {
    Write-And-Capture "Python script failed with exit code $LASTEXITCODE"
    $output | Write-Error
    exit $LASTEXITCODE
}

# WSL経由でnpmテストの実行
Write-And-Capture "Running npm test in WSL..."
$wslPath = "/mnt/d/Programming/Github/langscore-divisi/test/lscsv"
wsl -e bash -c "cd $wslPath && npm test"
if ($LASTEXITCODE -ne 0) {
    Write-And-Capture "npm test failed with exit code $LASTEXITCODE"
    $output | Write-Error
    exit $LASTEXITCODE
}

Write-And-Capture "All tests completed successfully."
exit 0
