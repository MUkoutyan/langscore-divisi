require 'csv'
require 'zlib'
require_relative '../../resource/lscsv.rb'

class LsDumpData
  attr_accessor :data
end

def load_rvdata2(file_path)
  result = ""
  File.open(file_path, 'rb') do |file|
    dump = Marshal.load(file.read())
    result = dump.data
  end
  result
end

def write_to_csv(data, output_path)
  File.open(output_path, 'wb', encoding: 'UTF-8') do |file|
    # p "write : #{data}"
    file.write(data)
  end
end

if ARGV.length != 2
  puts "Usage: ruby extract_rvdata2.rb <input.rvdata2> <output.csv>"
  exit 1
end

input_file = ARGV[0]
output_file = ARGV[1]

data = load_rvdata2(input_file)
# p "data #{data}"
write_to_csv(data, output_file)
