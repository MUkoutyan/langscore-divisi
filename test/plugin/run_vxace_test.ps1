[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

# RGSS3 相当の Ruby 1.9.2 のパス。環境変数 LANGSCORE_RUBY19 で上書きできます。
$rubyPath = if ($env:LANGSCORE_RUBY19) { $env:LANGSCORE_RUBY19 } else { "C:\ruby-1.9.2\bin\ruby.exe" }
if (-not (Test-Path $rubyPath)) {
    Write-Host "Ruby 1.9.2 not found: $rubyPath (set LANGSCORE_RUBY19)"
    exit 1
}

# テストデータはリポジトリ外に置く。既定はリポジトリと同階層の langscore-divisi-test-data。
$testDataRoot = if ($env:LANGSCORE_TEST_DATA) { $env:LANGSCORE_TEST_DATA } else { Join-Path $PSScriptRoot "..\..\..\langscore-divisi-test-data" }
$pluginRoot = Join-Path $testDataRoot "plugin"
$vxaceProject = Join-Path $pluginRoot "vxace_test"
if (-not (Test-Path $vxaceProject)) {
    Write-Host "テストデータが見つかりません: $vxaceProject"
    Write-Host "  リポジトリと同階層に langscore-divisi-test-data を配置するか、環境変数 LANGSCORE_TEST_DATA で場所を指定してください。"
    exit 1
}

# resource/langscore.rb をテンプレート展開して vxace_test/Scripts へ反映する。
# (MV/MZ の sync_plugin.js と同じ目的。テストが古いスクリプトを見ないようにする)
node sync_vxace.js
if ($LASTEXITCODE -ne 0) {
    Write-Host "sync_vxace failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

# Langscore_vxace_test_prepare.rb が ./vxace_test/... を参照するため、
# プロジェクトの親フォルダを作業ディレクトリにして実行する。
$scriptPath = Join-Path $PSScriptRoot "Langscore_vxace_test.rb"

# compress.rb はテストデータ側ではなくリポジトリで管理する (Scripts/ を rvdata2 に固める処理)
Push-Location $vxaceProject
& $rubyPath (Join-Path $PSScriptRoot "vxace\compress.rb")
Pop-Location

if ($LASTEXITCODE -ne 0) {
    $code = $LASTEXITCODE
    Write-Host "compress failed with exit code $LASTEXITCODE"
    exit $code
}

# Ruby スクリプトを実行します
$env:RUBYOPT="-KU"
Push-Location $pluginRoot
& $rubyPath $scriptPath
Pop-Location
if ($LASTEXITCODE -ne 0) {
    $code = $LASTEXITCODE
    Write-Host "vxace_test failed with exit code $LASTEXITCODE"
    exit $code
}
exit 0  # 正常終了