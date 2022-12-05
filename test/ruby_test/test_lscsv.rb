require 'test/unit'

module Langscore
	TRANSLATE_FOLDER = "."
	SUPPORT_LANGUAGE = ["ja", "en", "zh-cn", "zh-tw"]
end

def load_data(path)
	raise "Not Found #{path}" unless File.exist?(path)
	File.open(path, 'rb') do | data |
		return Marshal.load(data)
	end
	return nil
end

require_relative '../../resource/lscsv.rb'

class TestLSCSV < Test::Unit::TestCase

  def write_csv(lines, filename = "test_csv")

		File.open(filename+".csv", "wb:utf-8:utf-8") do | file |
			lines.each do |l|
				file.puts(l)
			end
		end
  end

  # def test_openfile

	# 	write_csv(["original,ja,en,zh-cn,zh-tw", "ほに,ほげ,Honi,Honnni"])

	# 	result = LSCSV.open("test_csv")
	# 	assert_not_nil result
  # end

  # def test_csvrow1
		
	# 	write_csv(["original,ja,en,zh-cn,zh-tw", "ほに,ほげ,Honi,Honnni"])
	# 	lines = LSCSV.parse_row(LSCSV.open("test_csv"))

	# 	assert_equal "original,ja,en,zh-cn,zh-tw\n", lines[0]
	# 	assert_equal "ほに,ほげ,Honi,Honnni\n", lines[1]
  # end
  
  # def test_csvrow2
		
	# 	write_csv(["original,ja,en,zh-cn,zh-tw", "ほに,ほげ,Honi,Honnni", "\n"])
	# 	lines = LSCSV.parse_row(LSCSV.open("test_csv"))

	# 	assert_equal "original,ja,en,zh-cn,zh-tw\n", lines[0]
	# 	assert_equal "ほに,ほげ,Honi,Honnni\n", lines[1]
  # end
  
  # def test_csvrow_dq1

	# 	write_csv(["original,ja,en,zh-cn,\"zh-tw", "ほに\",ほげ,\"Honi\",Honnni"])
	# 	rows = LSCSV.parse_row(LSCSV.open("test_csv"))
	# 	lines = LSCSV.parse_col(rows)

	# 	assert_equal ["original","ja","en","zh-cn","zh-tw\nほに","ほげ","Honi","Honnni"], lines[0]
  # end

  # def test_csvrow_dq2
	# 	#"を濁点として使用した場合の対応
	# 	write_csv(["original,ja,en,zh-cn,zh-tw", "\"ほに\"ほに,ほげ,H\"on\"i,Honnni"])
	# 	rows = LSCSV.parse_row(LSCSV.open("test_csv"))
	# 	lines = LSCSV.parse_col(rows)

	# 	assert_equal ["original","ja","en","zh-cn","zh-tw"], lines[0]
	# 	assert_equal ["\"ほに\"ほに","ほげ","H\"on\"i","Honnni"], lines[1]
  # end
  
  def test_csvrow_double_nl
		#VXAce「文章のスクロール」対応
		write_csv(["original,ja", "ほにほに,\"ほに\n\nほに\n\nほに\""])
		rows = LSCSV.parse_row(LSCSV.open("test_csv"))
		lines = LSCSV.parse_col(rows)

		assert_equal ["original","ja"], lines[0]
		assert_equal ["ほにほに","ほに\n\nほに\n\nほに"], lines[1]
  end

  # def test_csvrow_dq3

  # text = []

	# write_csv(text, "test1")
	# rows = LSCSV.parse_row(LSCSV.open("test1"))
	# lines = LSCSV.parse_col(rows)

	# result_text = []
	# assert_equal result_text, lines[0]
  # end

  
  # def test_varidate

	# 	write_csv(["original,ja,en,zh-cn,zh-tw", "\"ほに\"ほに,ほげ,H\"on\"i,H\'o\"nn\"n\'ii,Niiin"])
	# 	rows = LSCSV.parse_row(LSCSV.open("test_csv"))
	# 	lines = LSCSV.parse_col(rows)

	# 	LSCSV.varidate("test_csv.csv", lines)
  # end

	# def test_readbomcsv
	# 	result = LSCSV.to_hash("bom_csv")
	# 	assert_equal result.length, 2
	# end

	# def test_readcsv
	# 	result = LSCSV.to_hash("Actors_csv")
	# 	assert_not_nil result
	# 	assert(!result.empty?)

	# 	text = "数々の戦場を渡り歩き、生き延びてきた歴戦の傭兵。普段は\r\n温厚だが、ひとたび戦いが始まれば狂戦士と化す。"
	# 	assert(result.has_key?(text))
	# 	assert(result[text]["en"] != "")
	# 	assert(result[text]["ja"] != "")
	# 	assert(result[text]["zh-cn"] != "")
	# 	assert(result[text]["zh-tw"] != "")
		

	# end

	# def test_readrvdata2
	# 	result = LSCSV.to_hash("Actors")
	# 	assert_not_nil result
	# 	assert(!result.empty?)
		
	# 	text = "数々の戦場を渡り歩き、生き延びてきた歴戦の傭兵。普段は\r\n温厚だが、ひとたび戦いが始まれば狂戦士と化す。"
	# 	assert(result.has_key?(text))
	# 	assert_equal(result[text]["en"], "en-"+text)
	# 	assert_equal(result[text]["ja"], "ja-"+text)
	# 	assert_equal(result[text]["zh-cn"], "zh-cn-"+text)
	# 	assert_equal(result[text]["zh-tw"], "zh-tw-"+text)
		
	# 	text = ""
	# 	assert(result.has_key?(text))
	# 	assert_equal(result[text]["en"], "en-"+text)
	# 	assert_equal(result[text]["ja"], "ja-"+text)
	# 	assert_equal(result[text]["zh-cn"], "zh-cn-"+text)
	# 	assert_equal(result[text]["zh-tw"], "zh-tw-"+text)

	# end

end