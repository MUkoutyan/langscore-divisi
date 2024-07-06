
require 'zlib'
require 'csv'

SCRIPTLIST_ID       = 0
SCRIPTLIST_PATH     = 1
SCRIPTLIST_NAME     = 2
SCRIPTLIST_DATA     = 3
EMPTY_SCRIPT_NAME   = "_NONAME_"
script_folder = "./Scripts"
#スクリプトの展開
script_list = []
script_filename_list = []
File.open("./Data/Scripts.rvdata2", 'rb') do |file|
  Marshal.load(file.read).each do |id, name, script|
    d = Zlib::Inflate.inflate(script)
    
    name = EMPTY_SCRIPT_NAME if name.empty?

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