import os
import sys
import unittest
from unittest.mock import patch
from collections import defaultdict

current_dir = os.path.dirname(os.path.abspath(__file__))
# rootディレクトリのパスを取得してsys.pathに追加
root_dir = os.path.abspath(os.path.join(current_dir, '../../resource'))
sys.path.append(root_dir)
from lscsv import LSCSV, Langscore


class TestLSCSV(unittest.TestCase):

    @classmethod
    def setUpClass(cls):
        cls.lscsv = LSCSV()
        Langscore.Support_Language = ["ja", "en"]

    def test_fetch_header(self):
        csv_text = 'origin,ja,en\nvalue1,こんにちは,Hello'
        header = self.lscsv.fetch_header(csv_text)
        self.assertEqual(header, ['origin', 'ja', 'en'])

    def test_to_map_basic(self):
        csv_text = 'origin,ja,en\nvalue1,こんにちは,Hello\nvalue2,さようなら,Goodbye'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['value1']['ja'], 'こんにちは')
        self.assertEqual(result['value1']['en'], 'Hello')
        self.assertEqual(result['value2']['ja'], 'さようなら')
        self.assertEqual(result['value2']['en'], 'Goodbye')

    def test_special_format(self):
        csv_text = 'origin,ja,en\nvalue1,こんにちは,Hello\nvalue2,さようなら,Goodbye'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['value1']['ja'], 'こんにちは')
        self.assertEqual(result['value1']['en'], 'Hello')
        self.assertEqual(result['value2']['ja'], 'さようなら')
        self.assertEqual(result['value2']['en'], 'Goodbye')

    def test_empty_result_for_header_only(self):
        csv_text = 'origin,ja,en\n'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result, {})

    def test_first_row(self):
        csv_text = 'origin,ja,en\n通常のテキストです,通常のテキストです,Normal text.'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['通常のテキストです']['ja'], '通常のテキストです')
        self.assertEqual(result['通常のテキストです']['en'], 'Normal text.')

    def test_line_break_in_row(self):
        csv_text = 'origin,ja,en\n"改行を含む\nテキストです","改行を含む\nテキストです","Includes line breaks\nText."'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['改行を含む\nテキストです']['ja'], '改行を含む\nテキストです')
        self.assertEqual(result['改行を含む\nテキストです']['en'], 'Includes line breaks\nText.')

    def test_comma_in_row(self):
        csv_text = 'origin,ja,en\n"カンマを含む,テキストです","カンマを含む,テキストです","Including comma, text."'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['カンマを含む,テキストです']['ja'], 'カンマを含む,テキストです')
        self.assertEqual(result['カンマを含む,テキストです']['en'], 'Including comma, text.')

    def test_double_quotes_in_row(self):
        csv_text = 'origin,ja,en\n"""タ""フ""ルクォーテーションを含むテキストです""","""タ""フ""ルクォーテーションを含むテキストです""","""The text contains a t""ouh""le quotation."""'
        result = self.lscsv.to_map(csv_text)
        key = '"タ"フ"ルクォーテーションを含むテキストです"'
        self.assertEqual(result[key]['ja'], '"タ"フ"ルクォーテーションを含むテキストです"')
        self.assertEqual(result[key]['en'], '"The text contains a t"ouh"le quotation."')

    def test_complex_format(self):
        csv_text = 'origin,ja,en\n"""""""Hello, World"""",\nそれはプログラムを書く際の"",""""""謎の呪文""""(Mystery spell)―――""","""""""Hello, World"""",\nそれはプログラムを書く際の"",""""""謎の呪文""""(Mystery spell)―――""","""""""Hello, World"""",""\nIt is a "",""""Mystery spell""""(Mystery spell)---"""""" when writing a program."'
        result = self.lscsv.to_map(csv_text)
        key = '"""Hello, World"",\nそれはプログラムを書く際の","""謎の呪文""(Mystery spell)―――"'
        self.assertEqual(result[key]['ja'], '"""Hello, World"",\nそれはプログラムを書く際の","""謎の呪文""(Mystery spell)―――"')
        self.assertEqual(result[key]['en'], '"""Hello, World"","\nIt is a ",""Mystery spell""(Mystery spell)---""" when writing a program.')

    def test_name_change(self):
        csv_text = "origin,ja,en\n名前変えるよ,名前変えるよ,I'll change the name."
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['名前変えるよ']['ja'], '名前変えるよ')
        self.assertEqual(result['名前変えるよ']['en'], "I'll change the name.")

    def test_variable_value_in_row(self):
        csv_text = 'origin,ja,en\n1番の変数の値は \\V[1] です。,1番の変数の値は \\V[1] です。,The value of variable 1 is \\V[1].'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['1番の変数の値は \\V[1] です。']['ja'], '1番の変数の値は \\V[1] です。')
        self.assertEqual(result['1番の変数の値は \\V[1] です。']['en'], 'The value of variable 1 is \\V[1].')

    def test_actor_name_in_row(self):
        csv_text = 'origin,ja,en\n1番のアクターの名前は \\N[1] です。,1番のアクターの名前は \\N[1] です。,The name of the 1 actor is \\N[1].'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['1番のアクターの名前は \\N[1] です。']['ja'], '1番のアクターの名前は \\N[1] です。')
        self.assertEqual(result['1番のアクターの名前は \\N[1] です。']['en'], 'The name of the 1 actor is \\N[1].')

    def test_party_member_name_in_row(self):
        csv_text = 'origin,ja,en\n1番のパーティーメンバーの名前は \\P[1] です。,1番のパーティーメンバーの名前は \\P[1] です。,The name of party member 1 is \\P[1].'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['1番のパーティーメンバーの名前は \\P[1] です。']['ja'], '1番のパーティーメンバーの名前は \\P[1] です。')
        self.assertEqual(result['1番のパーティーメンバーの名前は \\P[1] です。']['en'], 'The name of party member 1 is \\P[1].')

    def test_money_in_row(self):
        csv_text = 'origin,ja,en\n現在の所持金は \\G です。,現在の所持金は \\G です。,The current amount of money you have is \\G'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['現在の所持金は \\G です。']['ja'], '現在の所持金は \\G です。')
        self.assertEqual(result['現在の所持金は \\G です。']['en'], 'The current amount of money you have is \\G')

    def test_color_in_row(self):
        csv_text = 'origin,ja,en\nこの文字は \\C[2] 赤色 \\C[0] 通常色 です。,この文字は \\C[2] 赤色 \\C[0] 通常色 です。,This letter is \\C[2] red \\C[0] normal color.'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['この文字は \\C[2] 赤色 \\C[0] 通常色 です。']['ja'], 'この文字は \\C[2] 赤色 \\C[0] 通常色 です。')
        self.assertEqual(result['この文字は \\C[2] 赤色 \\C[0] 通常色 です。']['en'], 'This letter is \\C[2] red \\C[0] normal color.')

    def test_icon_in_row(self):
        csv_text = 'origin,ja,en\nこれはアイコン \\I[64] の表示です。,これはアイコン \\I[64] の表示です。,This is the display of the icon \\I[64].'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['これはアイコン \\I[64] の表示です。']['ja'], 'これはアイコン \\I[64] の表示です。')
        self.assertEqual(result['これはアイコン \\I[64] の表示です。']['en'], 'This is the display of the icon \\I[64].')

    def test_font_size_in_row(self):
        csv_text = 'origin,ja,en\n文字サイズを \\{ 大きく \\} 小さくします。,文字サイズを \\{ 大きく \\} 小さくします。,The font size will be reduced to \\{large \\}smaller.'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['文字サイズを \\{ 大きく \\} 小さくします。']['ja'], '文字サイズを \\{ 大きく \\} 小さくします。')
        self.assertEqual(result['文字サイズを \\{ 大きく \\} 小さくします。']['en'], 'The font size will be reduced to \\{large \\}smaller.')

    def test_money_window_in_row(self):
        csv_text = 'origin,ja,en\n\\$ 所持金ウィンドウを表示します。,\\$ 所持金ウィンドウを表示します。,\\$ Display the money window.'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['\\$ 所持金ウィンドウを表示します。']['ja'], '\\$ 所持金ウィンドウを表示します。')
        self.assertEqual(result['\\$ 所持金ウィンドウを表示します。']['en'], '\\$ Display the money window.')

    def test_wait_in_middle_of_sentence_row(self):
        csv_text = 'origin,ja,en\n文章の途中で 1/4 秒間のウェイトを \. します。,文章の途中で 1/4 秒間のウェイトを \. します。,It will display a 1/4 second wait \. in the middle of a sentence.'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['文章の途中で 1/4 秒間のウェイトを \. します。']['ja'], '文章の途中で 1/4 秒間のウェイトを \. します。')
        self.assertEqual(result['文章の途中で 1/4 秒間のウェイトを \. します。']['en'], 'It will display a 1/4 second wait \. in the middle of a sentence.')

    def test_one_second_wait_in_sentence(self):
        csv_text = 'origin,ja,en\n文章の途中で 1 秒間のウェイトを \| します。,文章の途中で 1 秒間のウェイトを \| します。,The text is weighted for one second \| in the middle of a sentence.'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['文章の途中で 1 秒間のウェイトを \| します。']['ja'], '文章の途中で 1 秒間のウェイトを \| します。')
        self.assertEqual(result['文章の途中で 1 秒間のウェイトを \| します。']['en'], 'The text is weighted for one second \| in the middle of a sentence.')

    def test_button_input_wait_in_sentence(self):
        csv_text = 'origin,ja,en\n文章の途中でボタンの入力待ちを \! します。,文章の途中でボタンの入力待ちを \! します。,Waiting for button input in the middle of a sentence \! The button is pressed.'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['文章の途中でボタンの入力待ちを \! します。']['ja'], '文章の途中でボタンの入力待ちを \! します。')
        self.assertEqual(result['文章の途中でボタンの入力待ちを \! します。']['en'], 'Waiting for button input in the middle of a sentence \! The button is pressed.')

    def test_instant_display_row(self):
        csv_text = 'origin,ja,en\n\\>この行の文字を一瞬で表示します。\\<,\\>この行の文字を一瞬で表示します。\\<,\\>Display the characters of this line in a moment.\\<'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['\\>この行の文字を一瞬で表示します。\\<']['ja'], '\\>この行の文字を一瞬で表示します。\\<')
        self.assertEqual(result['\\>この行の文字を一瞬で表示します。\\<']['en'], '\\>Display the characters of this line in a moment.\\<')

    def test_no_input_wait_row(self):
        csv_text = 'origin,ja,en\n\\^文章表示後の入力待ちを行いません。,\\^文章表示後の入力待ちを行いません。,\\^Do not wait for input after displaying a sentence.'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['\\^文章表示後の入力待ちを行いません。']['ja'], '\\^文章表示後の入力待ちを行いません。')
        self.assertEqual(result['\\^文章表示後の入力待ちを行いません。']['en'], '\\^Do not wait for input after displaying a sentence.')

    def test_backslash_in_row(self):
        csv_text = 'origin,ja,en\nバックスラッシュの表示は \\\\ です。,バックスラッシュの表示は \\\\ です。,The backslash is displayed at \\\\'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['バックスラッシュの表示は \\\\ です。']['ja'], 'バックスラッシュの表示は \\\\ です。')
        self.assertEqual(result['バックスラッシュの表示は \\\\ です。']['en'], 'The backslash is displayed at \\\\')

    def test_combined_format_in_row(self):
        csv_text = 'origin,ja,en\n"複合させます\n\\{\\C[2]\\N[2]\\I[22]","複合させます\n\\{\\C[2]\\N[2]\\I[22]","Compounding\n\\{\\C[2]\\N[2]\\I[22]"'
        result = self.lscsv.to_map(csv_text)
        self.assertEqual(result['複合させます\n\\{\\C[2]\\N[2]\\I[22]']['ja'], '複合させます\n\\{\\C[2]\\N[2]\\I[22]')
        self.assertEqual(result['複合させます\n\\{\\C[2]\\N[2]\\I[22]']['en'], 'Compounding\n\\{\\C[2]\\N[2]\\I[22]')

if __name__ == '__main__':
    unittest.main()
