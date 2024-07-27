require 'csv'
require 'zlib'

class LsDumpData
  attr_accessor :data
end

def load_rvdata2(file_path)
  File.open(file_path, 'rb') do |file|
    dump = Marshal.load(file.read)
    return dump.data
  end
end

def write_to_csv(data, csv_path)
  File.open(csv_path, 'w', encoding: 'UTF-8') do |csv|
    csv.write(data)
  end
end

if ARGV.length != 2
  puts "Usage: ruby extract_rvdata2.rb <input.rvdata2> <output.csv>"
  exit
end

input_file = ARGV[0]
output_file = ARGV[1]

data = load_rvdata2(input_file)
write_to_csv(data, output_file)
