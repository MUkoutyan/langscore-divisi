# encoding: utf-8
#-------------------------------------------------------------------
# Scripts/_list.csv から Data/Scripts.rvdata2 を組み立てる。
# カレントディレクトリはツクールプロジェクトのルートであること。
#
#   ruby compress.rb                通常のビルド
#   ruby compress.rb <script.rb>    <script.rb> を Main の直前に挿入する (実機テスト用)
#
# スクリプト本体はバイナリとして扱う。読み込み側の Ruby は RGSS3 (1.9.2) で、
# 実行側の Ruby はバージョンが異なるため、エンコーディング変換を挟まない。
#-------------------------------------------------------------------
require 'zlib'
require 'csv'

EMPTY_SCRIPT_NAME = '_NONAME_'
INJECT_ID         = '999001'
INJECT_NAME       = 'LsInjectedTest'

inject_path = ARGV[0]
if inject_path && File.exist?(inject_path) == false
  abort "inject script not found: #{inject_path}"
end

def deflate(text)
  Zlib::Deflate.deflate(text, Zlib::DEFAULT_COMPRESSION)
end

def read_script(path)
  File.exist?(path) ? File.open(path, 'rb') { |f| f.read } : ''
end

scripts = []
CSV.foreach('Scripts/_list.csv') do |row|
  id   = row[0]
  name = row[1].to_s.strip
  name = '' if name.include?(EMPTY_SCRIPT_NAME)

  # テストスクリプトは Main より前に置く。Main の rgss_main が制御を返さないため。
  if inject_path && name == 'Main'
    scripts.push([INJECT_ID, INJECT_NAME, deflate(read_script(inject_path))])
  end

  scripts.push([id, name, deflate(read_script('Scripts/' + name + '.rb'))])
end

File.open('Data/Scripts.rvdata2', 'wb') { |f| f.write(Marshal.dump(scripts)) }
puts "compress: #{scripts.size} scripts" + (inject_path ? " (+#{File.basename(inject_path)})" : "")
