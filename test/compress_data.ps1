# 基準となるディレクトリを指定（相対パスの場合に基準となるディレクトリ）
$baseDirectory = $PWD.Path

$itemsToZip = @(
    ".\data",
    ".\lscsv\Skills.csv",
    ".\lscsv\Skills.rvdata2",
    ".\rvcnv_test\test_data",
    ".\plugin\mv_test",
    ".\plugin\mz_test",
    ".\plugin\vxace_test"
)
# Compress-Archive -Path $itemsToZip -DestinationPath .\data_backup.zip -Update

# ZIPファイルの保存先
$zipFilePath = ".\test_data.zip"

# 圧縮フォルダの一時作業ディレクトリを作成
$tempDir = New-Item -Path (Join-Path $env:TEMP "ZipTemp") -ItemType Directory -Force

# 圧縮対象のファイルとフォルダを一時ディレクトリにコピー
foreach ($item in $itemsToZip) {
    $fullPath = Join-Path $baseDirectory $item
    $destination = Join-Path $tempDir $item
    $parentDir = Split-Path $destination -Parent

    # コピー先ディレクトリを作成
    New-Item -Path $parentDir -ItemType Directory -Force

    # ファイルやフォルダをコピー
    Copy-Item -Path $fullPath -Destination $destination -Recurse -Force
}

# 一時ディレクトリをZIPファイルに圧縮
Compress-Archive -Path $tempDir\* -DestinationPath $zipFilePath -Force

# 一時ディレクトリを削除
Remove-Item -Path $tempDir -Recurse -Force
