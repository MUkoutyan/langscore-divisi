[Console]::OutputEncoding = [System.Text.Encoding]::UTF8

# Ruby 実行ファイルのパスを変数に格納します
$rubyPath = "C:\ruby-1.9.2\bin\ruby.exe"

# 実行する Ruby スクリプトのパスを変数に格納します
$scriptPath = ".\Langscore_vxace_test.rb"

# Ruby スクリプトを実行します
& $rubyPath $scriptPath