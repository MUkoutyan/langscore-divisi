require 'zlib'
require 'csv'

data = ''
index = []

class ScriptData
  attr_accessor :id
  attr_accessor :name
  attr_accessor :data
end

CSV_SCRIPTLIST_ID   = 0
CSV_SCRIPTLIST_NAME = 1
EMPTY_SCRIPT_NAME   = "_NONAME_"

compressData = []
CSV.foreach('Scripts/_list.csv') do | row |
  id         = row[CSV_SCRIPTLIST_ID]
  scriptname = row[CSV_SCRIPTLIST_NAME]
  scriptname = "" if scriptname.include?(EMPTY_SCRIPT_NAME)
  filepath   = 'Scripts/'+scriptname+'.rb'
  if File.exists?(filepath) == false
    compressData.push([id, scriptname, Zlib::Deflate.deflate("", Zlib::DEFAULT_COMPRESSION )])
  else 
    File.open(filepath) do |file|
      contents = file.readlines().join()
      compressed = Zlib::Deflate.deflate(contents, Zlib::DEFAULT_COMPRESSION );
      compressData.push([id, scriptname, compressed])
    end
  end
end

File.open('Data/Scripts.rvdata2', 'wb') do |file|
  file.write(Marshal.dump(compressData))
end
