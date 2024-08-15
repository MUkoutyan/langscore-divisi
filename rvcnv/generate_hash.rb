require 'digest/md5'

hash = Digest::MD5.file('rvcnv.exe').to_s
p "Hash : #{hash}"

File.open("rvcnv_hash.cpp", "w") do |f|
f.write(<<CPP)
/********************************************
 * "#{Time.now.strftime("%Y/%m/%d/ %H:%M")}"
*********************************************/
const char rvcnv_hash[] = "#{hash}";
CPP
end