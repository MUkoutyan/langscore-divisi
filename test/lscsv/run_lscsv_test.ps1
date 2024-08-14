# Rubyスクリプトの実行
Write-Host "Running Ruby script..."
$rubyScript = "./lscsv_test.rb"
ruby $rubyScript

# Pythonスクリプトの実行
Write-Host "Running Python script..."
$pythonScript = "./lscsv_test.py"
python $pythonScript

# WSL経由でnpmテストの実行
Write-Host "Running npm test in WSL..."
$wslPath = "/mnt/d/Programming/Github/langscore-divisi/test/lscsv"
wsl -e bash -c "cd $wslPath && npm test"

Write-Host "All tests completed."