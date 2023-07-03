require 'test/unit'

require_relative '../../rvcnv/rvcnv.rb'
class LsDumpData
  attr_accessor :data
end

def load_data(path)
	raise "Not Found #{path}" unless File.exist?(path)
	File.open(path, 'rb') do | data |
		return Marshal.load(data)
	end
	return nil
end

data = load_data('./Actors.rvdata2')

p data

# class TestRVCNV < Test::Unit::TestCase

#   def test_packing

#   end
# end

