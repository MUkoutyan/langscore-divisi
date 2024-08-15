require 'csv'
require 'zlib'

class LsDumpData
  attr_accessor :data
end

def extract_rvdata2(data_folder, output_folder)
    script_list = []
    if File.exist?(data_folder+"/Scripts.rvdata2")
      p "Extending Scripts.rvdata2"
      File.open(data_folder+"/Scripts.rvdata2", 'rb') do |file|
        Marshal.load(file.read).each do |id, name, script|
          d = Zlib::Inflate.inflate(script)
          
          name = "_NONAME_" if name.empty?
    
          script_path = output_folder+'/'+id.to_s+'_'+name+'.rb'
          script_list.push([script_path, d])
        end
      end
    else
        puts "Not found Script.rvdata2"
        exit
    end
    script_list.each do |r|
        File.open(r[0], "wb") do |out|
            out.write(r[1]) 
        end
    end
end


if ARGV.length != 2
  puts "Usage: ruby extract_script.rb <input.rvdata2> <output.csv>"
  exit
end

input_file = ARGV[0]
output_folder = ARGV[1]

extract_rvdata2(input_file, output_folder)
