[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

# Ruby 実行ファイルのパスを変数に格納します
$rubyPath = "C:\ruby-1.9.2\bin\ruby.exe"

# 実行する Ruby スクリプトのパスを変数に格納します
$scriptPath = ".\Langscore_vxace_test.rb"

Set-Location .\vxace_test
& $rubyPath "compress.rb"

if ($LASTEXITCODE -ne 0) {
    $code = $LASTEXITCODE
    Write-Host "compress failed with exit code $LASTEXITCODE"
    Set-Location ..
    exit $code
}
Set-Location ..

# Ruby スクリプトを実行します
$env:RUBYOPT="-KU"
& $rubyPath $scriptPath
if ($LASTEXITCODE -ne 0) {
    $code = $LASTEXITCODE
    Write-Host "vxace_test failed with exit code $LASTEXITCODE"
    exit $code
}
exit 0  # 正常終了