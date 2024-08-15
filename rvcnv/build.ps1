# PowerShell スクリプト
$PSDefaultParameterValues['*:Encoding'] = 'utf8'

# Rubyのインストールディレクトリを設定
$RUBY_DIR = Join-Path -Path $PSScriptRoot -ChildPath "ruby"

# Rubyの実行ファイルパスを設定
$RUBY_EXE = Join-Path -Path $RUBY_DIR -ChildPath "bin\ruby.exe"
$GEM_CMD = Join-Path -Path $RUBY_DIR -ChildPath "bin\gem.cmd"
$RUBY_INSTALL_VERSION = "2.7.2-1"
$RUBY_INSTALLER_URL = "https://github.com/oneclick/rubyinstaller2/releases/download/RubyInstaller-$RUBY_INSTALL_VERSION/rubyinstaller-$RUBY_INSTALL_VERSION-x64.7z"
$RUBY_INSTALLER_EXE = Join-Path -Path $PSScriptRoot -ChildPath "rubyinstaller.7z"
$EXPECTED_HASH = "925cc01d453951d1d0c077c44cec90849afc8d23e45946e19ecd3aaabc0c0ab3"

Write-Output "Check Path $RUBY_EXE"

# Rubyが存在するか確認
if (Test-Path -Path $RUBY_EXE) {
    Write-Output "Ruby is already installed."
} else {
    Write-Output "Ruby is not installed. Installing Ruby..."
    
    Write-Output "$RUBY_INSTALLER_URL"
    
    # RubyInstallerをダウンロード
    Write-Output "Downloading RubyInstaller..."
    Invoke-WebRequest -Uri $RUBY_INSTALLER_URL -OutFile $RUBY_INSTALLER_EXE

    # ダウンロードしたファイルのSHA256ハッシュを計算
    $DOWNLOADED_HASH = Get-FileHash -Path $RUBY_INSTALLER_EXE -Algorithm SHA256 | Select-Object -ExpandProperty Hash

    # ハッシュ値を比較
    Write-Output "Expected Hash: $EXPECTED_HASH"
    Write-Output "Downloaded Hash: $DOWNLOADED_HASH"

    if ($DOWNLOADED_HASH -ieq $EXPECTED_HASH) {
        Write-Output "Hash check passed. Installing Ruby..."
        
        # RubyInstallerをサイレントインストール
        # Start-Process -FilePath $RUBY_INSTALLER_EXE -ArgumentList "/silent", "/currentuser", "/dir=$RUBY_DIR", "/tasks=nomodpath" -NoNewWindow -Wait

        tar -xzf "$RUBY_INSTALLER_EXE"

        Rename-Item ".\rubyinstaller-$RUBY_INSTALL_VERSION-x64" .\ruby

        # インストーラを削除
        Remove-Item -Path $RUBY_INSTALLER_EXE

        Write-Output "Ruby has been installed."
    } else {
        Write-Output "Hash check failed. Aborting installation."
        Remove-Item -Path $RUBY_INSTALLER_EXE
        exit 1
    }
}

# ocraがインストールされているか確認
$ocra_installed = & $GEM_CMD list ocra -i

if ($LASTEXITCODE -ne 0) {
    Write-Output "ocra is not installed. Installing ocra..."
    Write-Output $GEM_CMD install ocra
    & $GEM_CMD install ocra
} else {
    Write-Output "ocra is already installed."
}

Write-Output "ocra rvcnv.rb --windows"
Write-Output "make rvcnv..."
# --windowは不可。--consoleにすること。
# デバッグする場合は--debug-extractを追加。
Remove-Item rvcnv.exe
Remove-Item rvcnv_hash.cpp
& $RUBY_DIR\bin\ocra.bat rvcnv.rb --console --gem-all --add-all-core --icon "./icon.ico" --dll ruby_builtin_dlls\libssp-0.dll --dll ruby_builtin_dlls\libwinpthread-1.dll --dll ruby_builtin_dlls\libgmp-10.dll --dll ruby_builtin_dlls\zlib1.dll 

if ($LASTEXITCODE -eq 0) {
    & $RUBY_EXE generate_hash.rb
    Write-Output "done."
}
else{
    return $LASTEXITCODE
}
