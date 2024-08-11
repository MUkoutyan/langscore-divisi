const chai = require('chai');
const expect = chai.expect;
const { before } = require('mocha');

class Langscore {
};
global.Langscore = Langscore;
global.Langscore.Support_Language = ["ja", "en"];

const LSCSV = require('../../resource/lscsv');


let _lscsv = new LSCSV;

describe('LSCSV', function() {

  it('CSVテキストからヘッダーを正しく取得すること', function() {
    const csvText = 'origin,ja,en\nvalue1,こんにちは,Hello';
    const header = _lscsv.fetch_header(csvText);
    expect(header).to.deep.equal(['origin', 'ja', 'en']);
  });

  it('CSVテキストを正しく解析すること', function() {
    const csvText = 'origin,ja,en\nvalue1,こんにちは,Hello\nvalue2,さようなら,Goodbye';
    const result = _lscsv.to_map(csvText);
    expect(result['value1']['ja']).to.deep.equal('こんにちは');
    expect(result['value1']['en']).to.deep.equal('Hello');
    expect(result['value2']['ja']).to.deep.equal('さようなら');
    expect(result['value2']['en']).to.deep.equal('Goodbye');
  });

  it('特殊な形式を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\nvalue1,こんにちは,Hello\nvalue2,さようなら,Goodbye';
    const result = _lscsv.to_map(csvText);
    expect(result['value1']['ja']).to.deep.equal('こんにちは');
    expect(result['value1']['en']).to.deep.equal('Hello');
    expect(result['value2']['ja']).to.deep.equal('さようなら');
    expect(result['value2']['en']).to.deep.equal('Goodbye');
  });

  it('ヘッダーを正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n';
    const result = _lscsv.to_map(csvText);
    expect(result).to.deep.equal(new Map());
  });

  it('最初の行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n通常のテキストです,通常のテキストです,Normal text.';
    const result = _lscsv.to_map(csvText);
    expect(result['通常のテキストです']['ja']).to.equal('通常のテキストです');
    expect(result['通常のテキストです']['en']).to.equal('Normal text.');
  });

  it('改行を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n"改行を含む\nテキストです","改行を含む\nテキストです","Includes line breaks\nText."';
    const result = _lscsv.to_map(csvText);
    expect(result['改行を含む\nテキストです']['ja']).to.equal('改行を含む\nテキストです');
    expect(result['改行を含む\nテキストです']['en']).to.equal('Includes line breaks\nText.');
  });

  it('カンマを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n"カンマを含む,テキストです","カンマを含む,テキストです","Including comma, text."';
    const result = _lscsv.to_map(csvText);
    expect(result['カンマを含む,テキストです']['ja']).to.equal('カンマを含む,テキストです');
    expect(result['カンマを含む,テキストです']['en']).to.equal('Including comma, text.');
  });

  it('ダブルクォーテーションを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n"""タ""フ""ルクォーテーションを含むテキストです""","""タ""フ""ルクォーテーションを含むテキストです""","""The text contains a t""ouh""le quotation."""';
    const result = _lscsv.to_map(csvText);
    var key = '"タ"フ"ルクォーテーションを含むテキストです"';
    var expect_ja = '"タ"フ"ルクォーテーションを含むテキストです"'
    var expect_en = '"The text contains a t"ouh"le quotation."'
    var lang_hash = result[key];
    expect(lang_hash['ja']).to.equal(expect_ja);
    expect(lang_hash['en']).to.equal(expect_en);
  });

  it('"括りや改行が混在する文章を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n"""""""Hello, World"""",\nそれはプログラムを書く際の"",""""""謎の呪文""""(Mystery spell)―――""","""""""Hello, World"""",\nそれはプログラムを書く際の"",""""""謎の呪文""""(Mystery spell)―――""","""""""Hello, World"""",""\nIt is a "",""""Mystery spell""""(Mystery spell)---"""""" when writing a program."'
    const result = _lscsv.to_map(csvText);
    var key = '"""Hello, World"",\nそれはプログラムを書く際の","""謎の呪文""(Mystery spell)―――"';
    var expect_ja = '"""Hello, World"",\nそれはプログラムを書く際の","""謎の呪文""(Mystery spell)―――"'
    var expect_en = '"""Hello, World"","\nIt is a ",""Mystery spell""(Mystery spell)---""" when writing a program.'
    var lang_hash = result[key];
    expect(lang_hash['ja']).to.equal(expect_ja);
    expect(lang_hash['en']).to.equal(expect_en);
  });

  it('名前を変える行を正しく解析できるか', function() {
    const csvText = "origin,ja,en\n名前変えるよ,名前変えるよ,I\'ll change the name.";
    const result = _lscsv.to_map(csvText);
    expect(result['名前変えるよ']['ja']).to.equal('名前変えるよ');
    expect(result['名前変えるよ']['en']).to.equal("I'll change the name.");
  });

  it('変数の値を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n1番の変数の値は \\V[1] です。,1番の変数の値は \\V[1] です。,The value of variable 1 is \\V[1].';
    const result = _lscsv.to_map(csvText);
    expect(result['1番の変数の値は \\V[1] です。']['ja']).to.equal('1番の変数の値は \\V[1] です。');
    expect(result['1番の変数の値は \\V[1] です。']['en']).to.equal('The value of variable 1 is \\V[1].');
  });

  it('アクターの名前を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n1番のアクターの名前は \\N[1] です。,1番のアクターの名前は \\N[1] です。,The name of the 1 actor is \\N[1].';
    const result = _lscsv.to_map(csvText);
    expect(result['1番のアクターの名前は \\N[1] です。']['ja']).to.equal('1番のアクターの名前は \\N[1] です。');
    expect(result['1番のアクターの名前は \\N[1] です。']['en']).to.equal('The name of the 1 actor is \\N[1].');
  });

  it('パーティーメンバーの名前を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n1番のパーティーメンバーの名前は \\P[1] です。,1番のパーティーメンバーの名前は \\P[1] です。,The name of party member 1 is \\P[1].';
    const result = _lscsv.to_map(csvText);
    expect(result['1番のパーティーメンバーの名前は \\P[1] です。']['ja']).to.equal('1番のパーティーメンバーの名前は \\P[1] です。');
    expect(result['1番のパーティーメンバーの名前は \\P[1] です。']['en']).to.equal('The name of party member 1 is \\P[1].');
  });

  it('所持金を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n現在の所持金は \\G です。,現在の所持金は \\G です。,The current amount of money you have is \\G';
    const result = _lscsv.to_map(csvText);
    expect(result['現在の所持金は \\G です。']['ja']).to.equal('現在の所持金は \\G です。');
    expect(result['現在の所持金は \\G です。']['en']).to.equal('The current amount of money you have is \\G');
  });

  it('色を含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\nこの文字は \\C[2] 赤色 \\C[0] 通常色 です。,この文字は \\C[2] 赤色 \\C[0] 通常色 です。,This letter is \\C[2] red \\C[0] normal color.';
    const result = _lscsv.to_map(csvText);
    expect(result['この文字は \\C[2] 赤色 \\C[0] 通常色 です。']['ja']).to.equal('この文字は \\C[2] 赤色 \\C[0] 通常色 です。');
    expect(result['この文字は \\C[2] 赤色 \\C[0] 通常色 です。']['en']).to.equal('This letter is \\C[2] red \\C[0] normal color.');
  });

  it('アイコンを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\nこれはアイコン \\I[64] の表示です。,これはアイコン \\I[64] の表示です。,This is the display of the icon \\I[64].';
    const result = _lscsv.to_map(csvText);
    expect(result['これはアイコン \\I[64] の表示です。']['ja']).to.equal('これはアイコン \\I[64] の表示です。');
    expect(result['これはアイコン \\I[64] の表示です。']['en']).to.equal('This is the display of the icon \\I[64].');
  });

  it('文字サイズを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n文字サイズを \\{ 大きく \\} 小さくします。,文字サイズを \\{ 大きく \\} 小さくします。,The font size will be reduced to \\{large \\}smaller.';
    const result = _lscsv.to_map(csvText);
    expect(result['文字サイズを \\{ 大きく \\} 小さくします。']['ja']).to.equal('文字サイズを \\{ 大きく \\} 小さくします。');
    expect(result['文字サイズを \\{ 大きく \\} 小さくします。']['en']).to.equal('The font size will be reduced to \\{large \\}smaller.');
  });

  it('所持金ウィンドウを表示する行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n\\$ 所持金ウィンドウを表示します。,\\$ 所持金ウィンドウを表示します。,\\$ Display the money window.';
    const result = _lscsv.to_map(csvText);
    expect(result['\\$ 所持金ウィンドウを表示します。']['ja']).to.equal('\\$ 所持金ウィンドウを表示します。');
    expect(result['\\$ 所持金ウィンドウを表示します。']['en']).to.equal('\\$ Display the money window.');
  });

  it('文章の途中でウェイトを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n文章の途中で 1/4 秒間のウェイトを \. します。,文章の途中で 1/4 秒間のウェイトを \. します。,It will display a 1/4 second wait \. in the middle of a sentence.';
    const result = _lscsv.to_map(csvText);
    expect(result['文章の途中で 1/4 秒間のウェイトを \. します。']['ja']).to.equal('文章の途中で 1/4 秒間のウェイトを \. します。');
    expect(result['文章の途中で 1/4 秒間のウェイトを \. します。']['en']).to.equal('It will display a 1/4 second wait \. in the middle of a sentence.');
  });

  it('文章の途中で1秒間のウェイトを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n文章の途中で 1 秒間のウェイトを \| します。,文章の途中で 1 秒間のウェイトを \| します。,The text is weighted for one second \| in the middle of a sentence.';
    const result = _lscsv.to_map(csvText);
    expect(result['文章の途中で 1 秒間のウェイトを \| します。']['ja']).to.equal('文章の途中で 1 秒間のウェイトを \| します。');
    expect(result['文章の途中で 1 秒間のウェイトを \| します。']['en']).to.equal('The text is weighted for one second \| in the middle of a sentence.');
  });

  it('文章の途中でボタン入力待ちを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n文章の途中でボタンの入力待ちを \! します。,文章の途中でボタンの入力待ちを \! します。,Waiting for button input in the middle of a sentence \! The button is pressed.';
    const result = _lscsv.to_map(csvText);
    expect(result['文章の途中でボタンの入力待ちを \! します。']['ja']).to.equal('文章の途中でボタンの入力待ちを \! します。');
    expect(result['文章の途中でボタンの入力待ちを \! します。']['en']).to.equal('Waiting for button input in the middle of a sentence \! The button is pressed.');
  });

  it('一瞬で表示する行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n\\>この行の文字を一瞬で表示します。\\<,\\>この行の文字を一瞬で表示します。\\<,\\>Display the characters of this line in a moment.\\<';
    const result = _lscsv.to_map(csvText);
    expect(result['\\>この行の文字を一瞬で表示します。\\<']['ja']).to.equal('\\>この行の文字を一瞬で表示します。\\<');
    expect(result['\\>この行の文字を一瞬で表示します。\\<']['en']).to.equal('\\>Display the characters of this line in a moment.\\<');
  });

  it('入力待ちを行わない行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n\\^文章表示後の入力待ちを行いません。,\\^文章表示後の入力待ちを行いません。,\\^Do not wait for input after displaying a sentence.';
    const result = _lscsv.to_map(csvText);
    expect(result['\\^文章表示後の入力待ちを行いません。']['ja']).to.equal('\\^文章表示後の入力待ちを行いません。');
    expect(result['\\^文章表示後の入力待ちを行いません。']['en']).to.equal('\\^Do not wait for input after displaying a sentence.');
  });

  it('バックスラッシュを含む行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\nバックスラッシュの表示は \\\\ です。,バックスラッシュの表示は \\\\ です。,The backslash is displayed at \\\\';
    const result = _lscsv.to_map(csvText);
    expect(result['バックスラッシュの表示は \\\\ です。']['ja']).to.equal('バックスラッシュの表示は \\\\ です。');
    expect(result['バックスラッシュの表示は \\\\ です。']['en']).to.equal('The backslash is displayed at \\\\');
  });

  it('複合させた行を正しく解析できるか', function() {
    const csvText = 'origin,ja,en\n"複合させます\n\\{\\C[2]\\N[2]\\I[22]","複合させます\n\\{\\C[2]\\N[2]\\I[22]","Compounding\n\\{\\C[2]\\N[2]\\I[22]"';
    const result = _lscsv.to_map(csvText);
    expect(result['複合させます\n\\{\\C[2]\\N[2]\\I[22]']['ja']).to.equal('複合させます\n\\{\\C[2]\\N[2]\\I[22]');
    expect(result['複合させます\n\\{\\C[2]\\N[2]\\I[22]']['en']).to.equal('Compounding\n\\{\\C[2]\\N[2]\\I[22]');
  });
});
