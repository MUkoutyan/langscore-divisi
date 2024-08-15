# encoding: utf-8
Encoding.default_external = 'UTF-8'
Encoding.default_external = 'UTF-8'

require 'zlib'
require 'csv'
require 'optparse'

SCRIPTLIST_ID       = 0
SCRIPTLIST_PATH     = 1
SCRIPTLIST_NAME     = 2
SCRIPTLIST_DATA     = 3
EMPTY_SCRIPT_NAME   = "_NONAME_"


opt = OptionParser.new
input_folder_path = "."
opt.on_head('-i PROJPATH', '--input PROJPATH'){ |v| 
  input_folder_path = File.absolute_path(v)
  input_folder_path.gsub!("\\", "/")
  input_folder_path.chop! if input_folder_path.end_with?('/')
}

opt.parse!(ARGV)

script_folder = "#{input_folder_path}/Scripts"
#スクリプトの展開
script_list = []
script_filename_list = []
File.open("#{input_folder_path}/Data/Scripts.rvdata2", 'rb') do |file|
  Marshal.load(file.read).each do |id, name, script|
    d = Zlib::Inflate.inflate(script)
    next if d.length == 0
    
    name = EMPTY_SCRIPT_NAME if name.empty?

    next if name == "( ここに追加 )"

    script_filename_list.push(name)
    script_path = script_folder+'/'+name+'.rb'
    script_list.push([id.to_s, script_path, name, d])
  end
end


#再圧縮のためにリストを用意しておく
Dir.mkdir(script_folder) unless File.exists?(script_folder)

script_list_path = script_folder+'/_list.csv'
File.delete(script_list_path) if File.exists?(script_list_path)

CSV.open(script_list_path, 'w') do |file|
  script_list.each do |r|
    #ID, NAME
    file.puts([r[SCRIPTLIST_ID], r[SCRIPTLIST_NAME]])
    
    File.open(r[SCRIPTLIST_PATH], "wb") do |out|
      out.write(r[SCRIPTLIST_DATA]) 
    end
  end
end