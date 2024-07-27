# PowerShell スクリプト
$PSDefaultParameterValues['*:Encoding'] = 'utf8'

# Rubyのインストールディレクトリを設定
$RUBY_DIR = Join-Path -Path $PSScriptRoot -ChildPath "ruby"

# Rubyの実行ファイルパスを設定
$RUBY_EXE = Join-Path -Path $RUBY_DIR -ChildPath "bin\ruby.exe"
$GEM_CMD = Join-Path -Path $RUBY_DIR -ChildPath "bin\gem.cmd"
$RUBY_INSTALLER_URL = "https://github.com/oneclick/rubyinstaller2/releases/download/RubyInstaller-3.3.4-1/rubyinstaller-3.3.4-1-x64.7z"
$RUBY_INSTALLER_EXE = Join-Path -Path $PSScriptRoot -ChildPath "rubyinstaller.7z"
$EXPECTED_HASH = "c3e55de3a1924848a871b4406eff6e78e8d6de06fc59186dcfec6ceb44d635a9"

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

        Rename-Item .\rubyinstaller-3.3.4-1-x64 .\ruby

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
# ruby 3.3.4-1, ocra最新では libgmp-10.dll, zlib1.dll の追加が必要。
& $RUBY_DIR\bin\ocra.bat rvcnv.rb --console --gem-all --add-all-core --icon "./icon.ico" --dll ruby_builtin_dlls\libgmp-10.dll --dll ruby_builtin_dlls\zlib1.dll 
Write-Output "done."
& $RUBY_EXE generate_hash.rb
