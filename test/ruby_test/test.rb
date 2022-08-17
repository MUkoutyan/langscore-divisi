require 'test/unit'
require_relative '../../resource/lscsv.rb'

module Langscore
	TRANSLATE_FOLDER = "./"
	SUPPORT_LANGUAGE = ["ja", "en", "zh"]
end

class TestSample < Test::Unit::TestCase

  def write_csv(lines, filename = "test_csv.csv")
	File.open(filename, "w") do | file |
	  lines.each do |l|
        file.puts(l)
	  end
	end
  end

  def test_openfile

	write_csv(["original,ja,en,zh", "ほに,ほげ,Honi,Honnni"])

	result = LSCSV.open("test_csv.csv")
	assert_not_nil result
  end

  def test_csvrow1
  
	write_csv(["original,ja,en,zh", "ほに,ほげ,Honi,Honnni"])
	lines = LSCSV.parse_row(LSCSV.open("test_csv.csv"))

	assert_equal "original,ja,en,zh\n", lines[0]
	assert_equal "ほに,ほげ,Honi,Honnni\n", lines[1]
  end
  
  def test_csvrow2
  
	write_csv(["original,ja,en,zh", "ほに,ほげ,Honi,Honnni", "\n"])
	lines = LSCSV.parse_row(LSCSV.open("test_csv.csv"))

	assert_equal "original,ja,en,zh\n", lines[0]
	assert_equal "ほに,ほげ,Honi,Honnni\n", lines[1]
  end
  
  def test_csvrow_dq1

	write_csv(["original,ja,en,\"zh", "ほに\",ほげ,\"Honi\",Honnni"])
	rows = LSCSV.parse_row(LSCSV.open("test_csv.csv"))
	lines = LSCSV.parse_col(rows)

	assert_equal ["original","ja","en","zh\nほに","ほげ","Honi","Honnni"], lines[0]
  end

  def test_csvrow_dq2

	write_csv(["original,ja,en,zh", "\"ほに\"ほに,ほげ,H\"on\"i,Honnni"])
	rows = LSCSV.parse_row(LSCSV.open("test_csv.csv"))
	lines = LSCSV.parse_col(rows)

	assert_equal ["original","ja","en","zh"], lines[0]
	assert_equal ["\"ほに\"ほに","ほげ","H\"on\"i","Honnni"], lines[1]
  end

  def test_csvrow_dq3

  text = ["\"ゲオルイース
「魔族には本来【魔王】という絶対的な王が
　存在するはずなのだが、この七年間一度も姿を見せていない。
　おそらく・・・本当に\"\"いない\"\"んだろう。」
\",\"en-ゲオルイース
「魔族には本来【魔王】という絶対的な王が
　存在するはずなのだが、この七年間一度も姿を見せていない。
　おそらく・・・本当に\"\"いない\"\"んだろう。」
\",\"ゲオルイース
「魔族には本来【魔王】という絶対的な王が
　存在するはずなのだが、この七年間一度も姿を見せていない。
　おそらく・・・本当に\"\"いない\"\"んだろう。」
\",\"zh-sc-ゲオルイース
「魔族には本来【魔王】という絶対的な王が
　存在するはずなのだが、この七年間一度も姿を見せていない。
　おそらく・・・本当に\"\"いない\"\"んだろう。」
\""]

	write_csv(text, "test1.csv")
	rows = LSCSV.parse_row(LSCSV.open("test1.csv"))
	lines = LSCSV.parse_col(rows)

	result_text = ["ゲオルイース
「魔族には本来【魔王】という絶対的な王が
　存在するはずなのだが、この七年間一度も姿を見せていない。
　おそらく・・・本当に\"いない\"んだろう。」
","en-ゲオルイース
「魔族には本来【魔王】という絶対的な王が
　存在するはずなのだが、この七年間一度も姿を見せていない。
　おそらく・・・本当に\"いない\"んだろう。」
","ゲオルイース
「魔族には本来【魔王】という絶対的な王が
　存在するはずなのだが、この七年間一度も姿を見せていない。
　おそらく・・・本当に\"いない\"んだろう。」
","zh-sc-ゲオルイース
「魔族には本来【魔王】という絶対的な王が
　存在するはずなのだが、この七年間一度も姿を見せていない。
　おそらく・・・本当に\"いない\"んだろう。」
"]
	assert_equal result_text, lines[0]
  end

  
  def test_varidate

	write_csv(["original,ja,en,zh", "\"ほに\"ほに,ほげ,H\"on\"i,"])
	rows = LSCSV.parse_row(LSCSV.open("test_csv.csv"))
	lines = LSCSV.parse_col(rows)

	LSCSV.varidate("test_csv.csv", lines)
  end

end