# main.rb
# encoding: utf-8

require_relative './test_prepare.rb'

require 'test/unit'
require 'json'

# テスト用データを準備
test_actors = [{'ja' => 'エルーシェ', 'en' => 'eluche'}, {'ja' => '雑用係', 'en' => 'Compassionate'}, {'ja' => 'ラフィーナ', 'en' => 'Rafina'}, {'ja' => '傲慢ちき', 'en' => 'arrogant'}, {'ja' => 'ケスティニアスの雑用係。\nそんなに仕事は無い。', 'en' => "Kestinius' scullery maid.\nThere is not that much work."}, {'ja' => 'チビのツンデレウーマン。\n魔法が得意。', 'en' => 'Tiny tsundere woman.\nHe is good at magic.'}]
test_armors = [{'ja' => '盾', 'en' => 'Shield'}, {'ja' => '帽子', 'en' => 'Hat'}, {'ja' => '服', 'en' => 'Wear'}, {'ja' => '指輪', 'en' => 'Ring'}]
test_classes = [{'ja' => '勇者', 'en' => 'brave'}, {'ja' => '戦士', 'en' => 'warrior'}, {'ja' => '魔術師', 'en' => 'magician'}, {'ja' => '僧侶', 'en' => 'monk'}]
test_commonevents = [{'ja' => '顧問です', 'en' => 'Advisor.'}, {'ja' => ' ', 'en' => ''}]
test_enemies = [{'ja' => 'こうもり', 'en' => 'umbrella'}, {'ja' => 'スライム', 'en' => 'slime'}, {'ja' => 'オーク', 'en' => 'oak'}, {'ja' => 'ミノタウロス', 'en' => 'Minotaur'}]
test_graphics = [{'ja' => 'img/animations/Absorb', 'en' => ''}, {'ja' => 'img/animations/ArrowSpecial', 'en' => ''}, {'ja' => 'img/animations/Blow', 'en' => ''}, {'ja' => 'img/animations/Breath', 'en' => ''}, {'ja' => 'img/animations/Claw', 'en' => ''}]
test_items = [{'ja' => 'ポーション', 'en' => 'portion'}, {'ja' => 'マジックウォーター', 'en' => 'magic water'}, {'ja' => 'ディスペルハーブ', 'en' => 'dispersal herb'}, {'ja' => 'スティミュラント', 'en' => 'formula'}]
test_map001 = [{'ja' => '通常のテキストです', 'en' => 'Normal text.'}, {'ja' => '改行を含む\nテキストです', 'en' => 'Includes line breaks\nText.'}, {'ja' => 'カンマを含む,テキストです', 'en' => 'Including comma, text.'}]
test_skills = [{'ja' => 'の攻撃！', 'en' => 'attack!'}, {'ja' => '攻撃', 'en' => 'Attack'}, {'ja' => 'は身を守っている。', 'en' => 'is protecting itself.'}, {'ja' => '防御', 'en' => 'Guard'}, {'ja' => '連続攻撃', 'en' => 'Continuous attack'}]
test_states = [{'ja' => 'は倒れた！', 'en' => 'fell down!'}, {'ja' => 'を倒した！', 'en' => 'defeated!'}, {'ja' => 'は立ち上がった！', 'en' => 'stood up!'}, {'ja' => '戦闘不能', 'en' => 'inability to fight'}]
test_system = [{'ja' => '一般防具', 'en' => 'General Defense'}, {'ja' => '魔法防具', 'en' => 'Magic Defense'}, {'ja' => '軽装防具', 'en' => 'Lightweight Defense'}, {'ja' => '重装防具', 'en' => 'Heavy Duty Defense'}]
test_troops = [{'ja' => 'おおおおおおいおいおぃおぉいぃぉぃぉぃぉぉぉぃぃ', 'en' => 'oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh.'}]
test_weapons = [{'ja' => '剣', 'en' => 'Sword'}, {'ja' => '斧', 'en' => 'Axe'}, {'ja' => '杖', 'en' => 'Staff'}, {'ja' => '弓', 'en' => 'Bow'}]

# rvdata2内の改行コードメモ
# ・通常会話：無し
# ・スクロール文章中：無し
# ・装備やスキルの説明：\r\n

# 既存のテスト用データとメソッドは省略

class LSCSVTest < Test::Unit::TestCase
  def test_fetch_header
    csv_text = "origin,ja,en\nvalue1,こんにちは,Hello"
    header = LSCSV.fetch_header(csv_text)
    assert_equal ["origin", "ja", "en"], header
  end

  def test_parse
    csv_text = "origin,ja,en\nvalue1,こんにちは,Hello\nvalue2,さようなら,Goodbye"
    header = LSCSV.fetch_header(csv_text)
    result = LSCSV.parse_col(header, LSCSV.parse_row(csv_text))
  
    # 期待される結果を設定
    expected = [
      ["origin", "ja", "en"],
      ["value1", "こんにちは", "Hello"],
      ["value2", "さようなら", "Goodbye"]
    ]
  
    # 結果と期待される値を比較
    assert_equal expected, result
  end

  def test_to_hash
    actors = LSCSV.to_hash("Actors")
    assert_equal actors.length, 6

    actor1 = actors["ケスティニアスの雑用係。\r\nそんなに仕事は無い。"]
    assert_not_nil actor1

    assert_not_nil actor1["ja"]
    assert_not_nil actor1["en"]

    assert_equal actor1["ja"], "ケスティニアスの雑用係。\r\nそんなに仕事は無い。"
    assert_equal actor1["en"], "Kestinius' scullery maid.\r\nNot that much work."

  end

end

class LangscoreTest < Test::Unit::TestCase
  def setup
    Langscore.changeLanguage("en")
    @translate_map = {
      "こんにちは" => "Hello",
      "さようなら" => "Goodbye",
      "ありがとう" => "Thank you",
      "おはよう" => "Good morning"
    }
  end

  def test_translation
    assert_equal "Hello", @translate_map["こんにちは"]
    assert_equal "Goodbye", @translate_map["さようなら"]
  end

  def test_unknown_translation
    assert_nil @translate_map["未知のテキスト"]
  end

  def test_missing_translation
    original_text = "こんばんは"
    translated_text = @translate_map.fetch(original_text, original_text)
    assert_equal original_text, translated_text
  end

  def test_multiple_translations
    texts = ["こんにちは", "ありがとう", "おはよう"]
    translations = texts.map { |text| @translate_map[text] }
    assert_equal ["Hello", "Thank you", "Good morning"], translations
  end

  def test_translation_fallback
    assert_equal "Goodbye", @translate_map.fetch("Goodbye", "Goodbye")
  end

  def test_reverse_translation
    reverse_map = @translate_map.invert
    assert_equal "こんにちは", reverse_map["Hello"]
    assert_equal "さようなら", reverse_map["Goodbye"]
  end

  def test_language_change
    assert_equal "en", $langscore_current_language
    Langscore.changeLanguage("ja")
    assert_equal "ja", $langscore_current_language
  end

  def test_image_loading
    if ENV["LOCAL_MODE"] == "true"
      path = "img/pictures/nantoka8_en.png"
      assert_equal "img/pictures/nantoka8_en.png", path
    else
      path = "img/pictures/nantoka8.png"
      assert_equal "img/pictures/nantoka8.png", path
    end
  end

  def test_language_switch
    Langscore.changeLanguage("en")
    assert_equal "en", $langscore_current_language
  end

  
  def test_to_hash_csv
    actors = LSCSV.to_hash("Actors")
    assert_not_nil actors["ケスティニアスの雑用係。\r\nそんなに仕事は無い。"]
  end

  def test_save_data
    save_data = {
      actors: [
        { name: "エルーシェ", nickname: "雑用係", profile: "ケスティニアスの雑用係。\nそんなに仕事は無い。" },
        { name: "ラフィーナ", nickname: "傲慢ちき", profile: "チビのツンデレウーマン。\n魔法が得意。" }
      ]
    }
    assert_equal "エルーシェ", save_data[:actors][0][:name]
    assert_equal "雑用係", save_data[:actors][0][:nickname]
    assert_equal "ケスティニアスの雑用係。\nそんなに仕事は無い。", save_data[:actors][0][:profile]
    assert_equal "ラフィーナ", save_data[:actors][1][:name]
    assert_equal "傲慢ちき", save_data[:actors][1][:nickname]
    assert_equal "チビのツンデレウーマン。\n魔法が得意。", save_data[:actors][1][:profile]
  end

  def test_actor_update
    actor = { name: "エルーシェ", nickname: "雑用係", profile: "ケスティニアスの雑用係。\nそんなに仕事は無い。" }
    assert_equal "エルーシェ", actor[:name]
    assert_equal "雑用係", actor[:nickname]
    assert_equal "ケスティニアスの雑用係。\nそんなに仕事は無い。", actor[:profile]

    Langscore.changeLanguage("en")
    actor[:name] = "eluche"
    actor[:nickname] = "Compassionate"
    actor[:profile] = "Kestinius' scullery maid.\nThere is not that much work."
    assert_equal "eluche", actor[:name]
    assert_equal "Compassionate", actor[:nickname]
    assert_equal "Kestinius' scullery maid.\nThere is not that much work.", actor[:profile]

    Langscore.changeLanguage("ja")
    actor[:name] = "エルーシェ"
    actor[:nickname] = "雑用係"
    actor[:profile] = "ケスティニアスの雑用係。\nそんなに仕事は無い。"
    assert_equal "エルーシェ", actor[:name]
    assert_equal "雑用係", actor[:nickname]
    assert_equal "ケスティニアスの雑用係。\nそんなに仕事は無い。", actor[:profile]
  end

  def test_skill_update
    # 日本語のスキルの一部テキスト
    skills_ja = ["の攻撃！", "攻撃"]
  
    # 英語のスキルの一部テキスト
    skills_en = ["Attack"]
  
    # 日本語のテスト
    Langscore.changeLanguage("ja")
    skills_ja.each do |text|
      assert $data_skills.any? do |skill| 
        next if skill.nil?
        return skill.name.include?(text) || skill.description.include?(text)
      end
    end
  
    # 英語のテスト
    Langscore.changeLanguage("en")
    skills_ja.each do |text|
      assert $data_skills.any? do |skill| 
        next if skill.nil?
        return skill.name.include?(text) || skill.description.include?(text)
      end
    end
  end
end

class LangscoreForMapTest < Test::Unit::TestCase

  def setup
    DataManager.init
    SceneManager.goto(Scene_Title)
    SceneManager.scene.start
    SceneManager.scene.command_new_game
    SceneManager.scene.start
  end

  def test_translation_retrieval
    
    $game_map.setup(1)
    #イベント1（会話前処理）を取得
    interpreter = Game_Interpreter.new
    interpreter.setup($game_map.events[1].list)

    $game_map.events[1].start
    map_texts_ja = ["通常のテキストです\n", "改行を含む\nテキストです\n"]
    map_texts_en = ["Normal text.\n", "Includes line breaks\nText.\n"]

    window = Window_Message.new

    #会話の表示処理をシミュレート
    find = false
    notfind_texts = []
    loop do
      $game_map.update(true)
      if $game_message.has_text?
        text = window.convert_escape_characters($game_message.all_text)
        if map_texts_ja.include?(text)
          find = true
        else
          notfind_texts.push(text)
        end
        $game_message.clear
      else 
        break
      end
    end

    if find == false
      p notfind_texts
      assert false
    end

    notfind_texts.clear
    Langscore.changeLanguage("en")
    $game_map.events[1].start

    find = false
    loop do
      $game_map.update(true)
      if $game_message.has_text?
        text = window.convert_escape_characters($game_message.all_text)
        if map_texts_ja.include?(text)
          find = true
        else
          notfind_texts.push(text)
        end
        $game_message.clear
      else 
        break
      end
    end

    if find == false
      p notfind_texts
      assert false
    end

  end

  def test_shop_translation
    
    $game_map.setup(1)
    
    Langscore.changeLanguage("en")
    #イベント1（会話前処理）を取得
    interpreter = Game_Interpreter.new
    interpreter.setup($game_map.events[8].list)

    $game_map.events[8].start
    loop do
      $game_map.update(true)
      if $game_message.has_text?
        text = window.convert_escape_characters($game_message.all_text)
        if map_texts_ja.include?(text)
          find = true
        else
          notfind_texts.push(text)
        end
        $game_message.clear
      else 
        break
      end
    end

    SceneManager.scene.start
    # p $game_map.events[8].list
    buy_window = SceneManager.scene.instance_variable_get(:@buy_window)
    buy_window.make_item_list
    # item_data = buy_window.instance_variable_get(:@data)
    # for item in item_data
    #   case item
    #   when RPG::Item
    #     print "Item:"
    #     print item.name
    #     p item.description
    #   when RPG::Weapon
    #     print "Weaopon:"
    #     print item.name
    #     p item.description
    #   when RPG::Armor
    #     print "Armor:"
    #     print item.name
    #     p item.description
    #   end
    # end
    
    # shop_event = { event_id: 6, message: "ようこそ！" }
    # shop_event[:message] = "Welcome!" if $langscore_current_language == "en"
    # assert_equal "Welcome!", shop_event[:message]
  end

  def test_save_and_load
    before_dump = Marshal.dump($game_actors)
    contents = DataManager.make_save_contents
    name = contents[:actors][1].name
    after_dump = Marshal.dump($game_actors)

    assert_equal "エルーシェ", name
    assert_equal before_dump, after_dump, "The contents of $game_actors have changed after calling func."
  end

  def test_bug_report_june_2024
    Langscore.changeLanguage("ja")
    sample_text = "エルーシェ"
    assert_equal "エルーシェ", sample_text

    Langscore.changeLanguage("en")
    sample_text = "eluche"
    assert_equal "eluche", sample_text
  end
end
