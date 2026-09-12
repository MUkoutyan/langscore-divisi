#requires -Version 5.1
# テストデータ (langscore-divisi-test-data) を1つのzipにまとめる。
# 別のマシンへ持っていく場合や、バックアップを取る場合に使う。
#
# テストデータはリポジトリに含めない。既定ではリポジトリと同階層の
# langscore-divisi-test-data を対象にし、環境変数 LANGSCORE_TEST_DATA で上書きできる。

[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

$testDataRoot = if ($env:LANGSCORE_TEST_DATA) {
    $env:LANGSCORE_TEST_DATA
} else {
    Join-Path $PSScriptRoot "..\..\langscore-divisi-test-data"
}

if (-not (Test-Path $testDataRoot)) {
    Write-Host "テストデータが見つかりません: $testDataRoot"
    Write-Host "  リポジトリと同階層に langscore-divisi-test-data を配置するか、環境変数 LANGSCORE_TEST_DATA で場所を指定してください。"
    exit 1
}

$testDataRoot = (Resolve-Path $testDataRoot).Path
$zipFilePath = Join-Path (Split-Path $testDataRoot -Parent) "langscore-divisi-test-data.zip"

if (Test-Path $zipFilePath) {
    Remove-Item $zipFilePath -Force
}

Write-Host "圧縮中: $testDataRoot"
Compress-Archive -Path (Join-Path $testDataRoot "*") -DestinationPath $zipFilePath

# 展開したデータが最新かを判定できるよう、ハッシュも出力する。
$hash = (Get-FileHash $zipFilePath -Algorithm SHA256).Hash
Set-Content -Path "$zipFilePath.sha256" -Value $hash -NoNewline -Encoding ascii

$sizeMB = [math]::Round((Get-Item $zipFilePath).Length / 1MB, 1)
Write-Host "作成しました: $zipFilePath ($sizeMB MB)"
Write-Host "SHA256: $hash"
