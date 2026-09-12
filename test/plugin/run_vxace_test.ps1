[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

# RGSS3 相当の Ruby 1.9.2 のパス。環境変数 LANGSCORE_RUBY19 で上書きできます。
$rubyPath = if ($env:LANGSCORE_RUBY19) { $env:LANGSCORE_RUBY19 } else { "C:\ruby-1.9.2\bin\ruby.exe" }
if (-not (Test-Path $rubyPath)) {
    Write-Host "Ruby 1.9.2 not found: $rubyPath (set LANGSCORE_RUBY19)"
    exit 1
}

# resource/langscore.rb をテンプレート展開して vxace_test/Scripts へ反映する。
# (MV/MZ の sync_plugin.js と同じ目的。テストが古いスクリプトを見ないようにする)
node sync_vxace.js
if ($LASTEXITCODE -ne 0) {
    Write-Host "sync_vxace failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

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