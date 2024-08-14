
require "test/unit"

def ls_output_log(message, level = 0)
  return if Langscore::FILTER_OUTPUT_LOG_LEVEL == 5
  p message if Langscore::FILTER_OUTPUT_LOG_LEVEL == 0

  if Langscore::FILTER_OUTPUT_LOG_LEVEL < level
    p message
  end
end


require_relative "../../resource/lscsv.rb"

module Langscore
  SUPPORT_LANGUAGE = ["ja", "en"]
  Langscore::FILTER_OUTPUT_LOG_LEVEL = 0
end

class TestLSCSV < Test::Unit::TestCase

  def test_fetch_header
    csv_text = "origin,ja,en\nvalue1,こんにちは,Hello"
    header = LSCSV.fetch_header(csv_text)
    assert_equal(["origin", "ja", "en"], header)
  end

  def test_to_hash_basic_parsing
    csv_text = "origin,ja,en\nvalue1,こんにちは,Hello\nvalue2,さようなら,Goodbye"
    result = LSCSV.from_content(csv_text)
    assert_equal("こんにちは", result["value1"]["ja"])
    assert_equal("Hello", result["value1"]["en"])
    assert_equal("さようなら", result["value2"]["ja"])
    assert_equal("Goodbye", result["value2"]["en"])
  end

  def test_to_hash_special_format
    csv_text = "origin,ja,en\nvalue1,こんにちは,Hello\nvalue2,さようなら,Goodbye"
    result = LSCSV.from_content(csv_text)
    assert_equal("こんにちは", result["value1"]["ja"])
    assert_equal("Hello", result["value1"]["en"])
    assert_equal("さようなら", result["value2"]["ja"])
    assert_equal("Goodbye", result["value2"]["en"])
  end

  def test_to_hash_empty_header
    csv_text = "origin,ja,en\n"
    result = LSCSV.from_content(csv_text)
    assert_equal({}, result)
  end

  def test_to_hash_single_line
    csv_text = "origin,ja,en\n通常のテキストです,通常のテキストです,Normal text."
    result = LSCSV.from_content(csv_text)
    assert_equal("通常のテキストです", result["通常のテキストです"]["ja"])
    assert_equal("Normal text.", result["通常のテキストです"]["en"])
  end

  def test_to_hash_with_line_breaks
    csv_text = "origin,ja,en\n\"改行を含む\nテキストです\",\"改行を含む\nテキストです\",\"Includes line breaks\nText.\""
    result = LSCSV.from_content(csv_text)
    assert_equal("改行を含む\nテキストです", result["改行を含む\nテキストです"]["ja"])
    assert_equal("Includes line breaks\nText.", result["改行を含む\nテキストです"]["en"])
  end

  def test_to_hash_with_line_rn_breaks
    csv_text = "origin,ja,en\r\n\"RN改行を含む\r\nテキストです\",\"RN改行を含む\r\nテキストです\",\"Includes line breaks\r\nRN Text.\""
    result = LSCSV.from_content(csv_text)
    assert_equal("RN改行を含む\r\nテキストです", result["RN改行を含む\r\nテキストです"]["ja"])
    assert_equal("Includes line breaks\r\nRN Text.", result["RN改行を含む\r\nテキストです"]["en"])
  end

  def test_to_hash_with_commas
    csv_text = "origin,ja,en\n\"カンマを含む,テキストです\",\"カンマを含む,テキストです\",\"Including comma, text.\""
    result = LSCSV.from_content(csv_text)
    assert_equal("カンマを含む,テキストです", result["カンマを含む,テキストです"]["ja"])
    assert_equal("Including comma, text.", result["カンマを含む,テキストです"]["en"])
  end

  def test_to_hash_with_double_quotes
    csv_text = "origin,ja,en\n\"\"\"タ\"\"フ\"\"ルクォーテーションを含むテキストです\"\"\",\"\"\"タ\"\"フ\"\"ルクォーテーションを含むテキストです\"\"\",\"\"\"The text contains a t\"\"ouh\"\"le quotation.\"\"\""
    result = LSCSV.from_content(csv_text)
    key = "\"タ\"フ\"ルクォーテーションを含むテキストです\""
    expect_ja = "\"タ\"フ\"ルクォーテーションを含むテキストです\""
    expect_en = "\"The text contains a t\"ouh\"le quotation.\""
    lang_hash = result[key]
    assert_equal(expect_ja, lang_hash["ja"])
    assert_equal(expect_en, lang_hash["en"])
  end

  def test_to_hash_with_mixed_brackets_and_line_breaks
    csv_text = "origin,ja,en\n\"\"\"\"\"\"\"Hello, World\"\"\"\",\nそれはプログラムを書く際の\"\",\"\"\"\"\"\"謎の呪文\"\"\"\"(Mystery spell)―――\"\"\",\"\"\"\"\"\"\"Hello, World\"\"\"\",\nそれはプログラムを書く際の\"\",\"\"\"\"\"\"謎の呪文\"\"\"\"(Mystery spell)―――\"\"\",\"\"\"\"\"\"\"Hello, World\"\"\"\",\"\"\nIt is a \"\",\"\"\"\"Mystery spell\"\"\"\"(Mystery spell)---\"\"\"\"\"\" when writing a program.\""
    result = LSCSV.from_content(csv_text)
    key = "\"\"\"Hello, World\"\",\nそれはプログラムを書く際の\",\"\"\"謎の呪文\"\"(Mystery spell)―――\""
    expect_ja = "\"\"\"Hello, World\"\",\nそれはプログラムを書く際の\",\"\"\"謎の呪文\"\"(Mystery spell)―――\""
    expect_en = "\"\"\"Hello, World\"\",\"\nIt is a \",\"\"Mystery spell\"\"(Mystery spell)---\"\"\" when writing a program."
    lang_hash = result[key]
    assert_equal(expect_ja, lang_hash["ja"])
    assert_equal(expect_en, lang_hash["en"])
  end

  def test_to_hash_with_renamed_value
    csv_text = "origin,ja,en\n名前変えるよ,名前変えるよ,I'll change the name."
    result = LSCSV.from_content(csv_text)
    assert_equal("名前変えるよ", result["名前変えるよ"]["ja"])
    assert_equal("I'll change the name.", result["名前変えるよ"]["en"])
  end

  def test_to_hash_with_variable_values
    csv_text = "origin,ja,en\n1番の変数の値は \\V[1] です。,1番の変数の値は \\V[1] です。,The value of variable 1 is \\V[1]."
    result = LSCSV.from_content(csv_text)
    assert_equal("1番の変数の値は \\V[1] です。", result["1番の変数の値は \\V[1] です。"]["ja"])
    assert_equal("The value of variable 1 is \\V[1].", result["1番の変数の値は \\V[1] です。"]["en"])
  end

  def test_to_hash_with_actor_names
    csv_text = "origin,ja,en\n1番のアクターの名前は \\N[1] です。,1番のアクターの名前は \\N[1] です。,The name of the 1 actor is \\N[1]."
    result = LSCSV.from_content(csv_text)
    assert_equal("1番のアクターの名前は \\N[1] です。", result["1番のアクターの名前は \\N[1] です。"]["ja"])
    assert_equal("The name of the 1 actor is \\N[1].", result["1番のアクターの名前は \\N[1] です。"]["en"])
  end

  def test_to_hash_with_party_member_names
    csv_text = "origin,ja,en\n1番のパーティーメンバーの名前は \\P[1] です。,1番のパーティーメンバーの名前は \\P[1] です。,The name of party member 1 is \\P[1]."
    result = LSCSV.from_content(csv_text)
    assert_equal("1番のパーティーメンバーの名前は \\P[1] です。", result["1番のパーティーメンバーの名前は \\P[1] です。"]["ja"])
    assert_equal("The name of party member 1 is \\P[1].", result["1番のパーティーメンバーの名前は \\P[1] です。"]["en"])
  end

  def test_to_hash_with_money_value
    csv_text = "origin,ja,en\n現在の所持金は \\G です。,現在の所持金は \\G です。,The current amount of money you have is \\G"
    result = LSCSV.from_content(csv_text)
    assert_equal("現在の所持金は \\G です。", result["現在の所持金は \\G です。"]["ja"])
    assert_equal("The current amount of money you have is \\G", result["現在の所持金は \\G です。"]["en"])
  end

  def test_to_hash_with_color_value
    csv_text = "origin,ja,en\nこの文字は \\C[2] 赤色 \\C[0] 通常色 です。,この文字は \\C[2] 赤色 \\C[0] 通常色 です。,This letter is \\C[2] red \\C[0] normal color."
    result = LSCSV.from_content(csv_text)
    assert_equal("この文字は \\C[2] 赤色 \\C[0] 通常色 です。", result["この文字は \\C[2] 赤色 \\C[0] 通常色 です。"]["ja"])
    assert_equal("This letter is \\C[2] red \\C[0] normal color.", result["この文字は \\C[2] 赤色 \\C[0] 通常色 です。"]["en"])
  end

  def test_to_hash_with_icon_display
    csv_text = "origin,ja,en\nこれはアイコン \\I[64] の表示です。,これはアイコン \\I[64] の表示です。,This is the display of the icon \\I[64]."
    result = LSCSV.from_content(csv_text)
    assert_equal("これはアイコン \\I[64] の表示です。", result["これはアイコン \\I[64] の表示です。"]["ja"])
    assert_equal("This is the display of the icon \\I[64].", result["これはアイコン \\I[64] の表示です。"]["en"])
  end

  def test_to_hash_with_font_size
    csv_text = "origin,ja,en\n文字サイズを \\{ 大きく \\} 小さくします。,文字サイズを \\{ 大きく \\} 小さくします。,The font size will be reduced to \\{large \\}smaller."
    result = LSCSV.from_content(csv_text)
    assert_equal("文字サイズを \\{ 大きく \\} 小さくします。", result["文字サイズを \\{ 大きく \\} 小さくします。"]["ja"])
    assert_equal("The font size will be reduced to \\{large \\}smaller.", result["文字サイズを \\{ 大きく \\} 小さくします。"]["en"])
  end

  def test_to_hash_with_money_window
    csv_text = "origin,ja,en\n\\$ 所持金ウィンドウを表示します。,\\$ 所持金ウィンドウを表示します。,\\$ Display the money window."
    result = LSCSV.from_content(csv_text)
    assert_equal("\\$ 所持金ウィンドウを表示します。", result["\\$ 所持金ウィンドウを表示します。"]["ja"])
    assert_equal("\\$ Display the money window.", result["\\$ 所持金ウィンドウを表示します。"]["en"])
  end

  def test_to_hash_with_mid_sentence_wait
    csv_text = "origin,ja,en\n文章の途中で 1/4 秒間のウェイトを \. します。,文章の途中で 1/4 秒間のウェイトを \. します。,It will display a 1/4 second wait \. in the middle of a sentence."
    result = LSCSV.from_content(csv_text)
    assert_equal("文章の途中で 1/4 秒間のウェイトを \. します。", result["文章の途中で 1/4 秒間のウェイトを \. します。"]["ja"])
    assert_equal("It will display a 1/4 second wait \. in the middle of a sentence.", result["文章の途中で 1/4 秒間のウェイトを \. します。"]["en"])
  end

  def test_to_hash_with_one_second_wait
    csv_text = "origin,ja,en\n文章の途中で 1 秒間のウェイトを \| します。,文章の途中で 1 秒間のウェイトを \| します。,The text is weighted for one second \| in the middle of a sentence."
    result = LSCSV.from_content(csv_text)
    assert_equal("文章の途中で 1 秒間のウェイトを \| します。", result["文章の途中で 1 秒間のウェイトを \| します。"]["ja"])
    assert_equal("The text is weighted for one second \| in the middle of a sentence.", result["文章の途中で 1 秒間のウェイトを \| します。"]["en"])
  end

  def test_to_hash_with_button_input_wait
    csv_text = "origin,ja,en\n文章の途中でボタンの入力待ちを \! します。,文章の途中でボタンの入力待ちを \! します。,Waiting for button input in the middle of a sentence \! The button is pressed."
    result = LSCSV.from_content(csv_text)
    assert_equal("文章の途中でボタンの入力待ちを \! します。", result["文章の途中でボタンの入力待ちを \! します。"]["ja"])
    assert_equal("Waiting for button input in the middle of a sentence \! The button is pressed.", result["文章の途中でボタンの入力待ちを \! します。"]["en"])
  end

  def test_to_hash_with_instant_display
    csv_text = "origin,ja,en\n\\>この行の文字を一瞬で表示します。\\<,\\>この行の文字を一瞬で表示します。\\<,\\>Display the characters of this line in a moment.\\<"
    result = LSCSV.from_content(csv_text)
    assert_equal("\\>この行の文字を一瞬で表示します。\\<", result["\\>この行の文字を一瞬で表示します。\\<"]["ja"])
    assert_equal("\\>Display the characters of this line in a moment.\\<", result["\\>この行の文字を一瞬で表示します。\\<"]["en"])
  end

  def test_to_hash_with_no_input_wait
    csv_text = "origin,ja,en\n\\^文章表示後の入力待ちを行いません。,\\^文章表示後の入力待ちを行いません。,\\^Do not wait for input after displaying a sentence."
    result = LSCSV.from_content(csv_text)
    assert_equal("\\^文章表示後の入力待ちを行いません。", result["\\^文章表示後の入力待ちを行いません。"]["ja"])
    assert_equal("\\^Do not wait for input after displaying a sentence.", result["\\^文章表示後の入力待ちを行いません。"]["en"])
  end

  def test_to_hash_with_backslash_display
    csv_text = "origin,ja,en\nバックスラッシュの表示は \\\\ です。,バックスラッシュの表示は \\\\ です。,The backslash is displayed at \\\\"
    result = LSCSV.from_content(csv_text)
    assert_equal("バックスラッシュの表示は \\\\ です。", result["バックスラッシュの表示は \\\\ です。"]["ja"])
    assert_equal("The backslash is displayed at \\\\", result["バックスラッシュの表示は \\\\ です。"]["en"])
  end

  def test_to_hash_with_combined_lines
    csv_text = "origin,ja,en\n\"複合させます\n\\{\\C[2]\\N[2]\\I[22]\",\"複合させます\n\\{\\C[2]\\N[2]\\I[22]\",\"Compounding\n\\{\\C[2]\\N[2]\\I[22]\""
    result = LSCSV.from_content(csv_text)
    assert_equal("複合させます\n\\{\\C[2]\\N[2]\\I[22]", result["複合させます\n\\{\\C[2]\\N[2]\\I[22]"]["ja"])
    assert_equal("Compounding\n\\{\\C[2]\\N[2]\\I[22]", result["複合させます\n\\{\\C[2]\\N[2]\\I[22]"]["en"])
  end
end
