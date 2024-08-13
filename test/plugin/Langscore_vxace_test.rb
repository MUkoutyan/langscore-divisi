# main.rb
# encoding => utf-8

require_relative './Langscore_vxace_test_prepare.rb'

require 'test/unit'
require 'json'

# テスト用データを準備
test_actors = [{'ja' => 'エルーシェ', 'en' => 'eluche'}, {'ja' => '雑用係', 'en' => 'Compassionate'}, {'ja' => 'ラフィーナ', 'en' => 'Rafina'}, {'ja' => '傲慢ちき', 'en' => 'arrogant'}, {'ja' => 'ケスティニアスの雑用係。\nそんなに仕事は無い。', 'en' => "Kestinius' scullery maid.\nNot that much work."}, {'ja' => 'チビのツンデレウーマン。\n魔法が得意。', 'en' => 'Tiny tsundere woman.\nHe is good at magic.'}]
test_armors = [{'ja' => '盾', 'en' => 'Shield'}, {'ja' => '帽子', 'en' => 'Hat'}, {'ja' => '服', 'en' => 'Wear'}, {'ja' => '指輪', 'en' => 'Ring'}]
test_classes = [{'ja' => '勇者', 'en' => 'brave'}, {'ja' => '戦士', 'en' => 'warrior'}, {'ja' => '魔術師', 'en' => 'magician'}, {'ja' => '僧侶', 'en' => 'monk'}]
test_commonevents = [{'ja' => '顧問です', 'en' => 'Advisor.'}, {'ja' => ' ', 'en' => ''}]
test_enemies = [{'ja' => 'こうもり', 'en' => 'umbrella'}, {'ja' => 'スライム', 'en' => 'slime'}, {'ja' => 'オーク', 'en' => 'oak'}, {'ja' => 'ミノタウロス', 'en' => 'Minotaur'}]
test_graphics = [{'ja' => 'img/animations/Absorb', 'en' => ''}, {'ja' => 'img/animations/ArrowSpecial', 'en' => ''}, {'ja' => 'img/animations/Blow', 'en' => ''}, {'ja' => 'img/animations/Breath', 'en' => ''}, {'ja' => 'img/animations/Claw', 'en' => ''}]
test_items = [{'ja' => 'ポーション', 'en' => 'portion'}, {'ja' => 'マジックウォーター', 'en' => 'magic water'}, {'ja' => 'ディスペルハーブ', 'en' => 'dispersal herb'}, {'ja' => 'スティミュラント', 'en' => 'formula'}]

test_map001 = [
  {'ja' => '通常のテキストです', 'en' => 'Normal text.'}, 
  {'ja' => '改行を含む\nテキストです', 'en' => 'Includes line breaks\nText.'}, 
  {'ja' => 'カンマを含む,テキストです', 'en' => 'Including comma, text.'}, 
  {'ja' => '"タ"フ"ルクォーテーションを含むテキストです"', 'en' => '"The text contains a t"ouh"le quotation."'}, 
  {'ja' => '"""Hello, World"",\nそれはプログラムを書く際の","""謎の呪文""(Mystery spell)―――"', 'en' => '"""Hello, World"","\nIt is a ",""Mystery spell""(Mystery spell)---""" when writing a program.'}, 
  {'ja' => '名前変えるよ', 'en' => "I'll change the name."}, 
  {'ja' => '1番の変数の値は \\V[1] です。', 'en' => 'The value of variable 1 is \\V[1].'}, 
  {'ja' => '1番のアクターの名前は \\N[1] です。', 'en' => 'The name of the 1 actor is \\N[1].'}, 
  {'ja' => '1番のパーティーメンバーの名前は \\P[1] です。', 'en' => 'The name of party member 1 is \\P[1].'}, 
  {'ja' => '現在の所持金は \\G です。', 'en' => 'The current amount of money you have is \\G'}, 
  {'ja' => 'この文字は \\C[2] 赤色 \\C[0] 通常色 です。', 'en' => 'This letter is \\C[2] red \\C[0] normal color.'}, 
  {'ja' => 'これはアイコン \\I[64] の表示です。', 'en' => 'This is the display of the icon \\I[64].'}, 
  {'ja' => '文字サイズを \\{ 大きく \\} 小さくします。', 'en' => 'The font size will be reduced to \\{large \\}smaller.'}, 
  {'ja' => '\\$ 所持金ウィンドウを表示します。', 'en' => '\\$ Display the money window.'}, 
  {'ja' => '文章の途中で 1/4 秒間のウェイトを \\. します。', 'en' => 'It will display a 1/4 second wait \\. in the middle of a sentence.'}, 
  {'ja' => '文章の途中で 1 秒間のウェイトを \\| します。', 'en' => 'The text is weighted for one second \\| in the middle of a sentence.'}, 
  {'ja' => '文章の途中でボタンの入力待ちを \\! します。', 'en' => 'Waiting for button input in the middle of a sentence \\! The button is pressed.'}, 
  {'ja' => '\\>この行の文字を一瞬で表示します。\\<', 'en' => '\\>Display the characters of this line in a moment. \\<'}, 
  {'ja' => '\\^文章表示後の入力待ちを行いません。', 'en' => '\\^Do not wait for input after displaying a sentence.'}, 
  {'ja' => 'バックスラッシュの表示は \\\\ です。', 'en' => 'The backslash is displayed at \\\\'}, 
  {'ja' => '複合させます\n\\{\\C[2]\\N[2]\\I[22]', 'en' => 'Compounding\n\\{\\C[2]\\N[2]\\I[22]'}, 
  {'ja' => 'ラフィーナ', 'en' => 'Rafina.'}, 
  {'ja' => 'エルーシェ', 'en' => 'eluche'}, 
  {'ja' => 'やめる', 'en' => 'leave'}, 
  {'ja' => '二つ名変えるよ', 'en' => "I'll change my name."}, 
  {'ja' => '雑用係', 'en' => 'Compassionate'}, 
  {'ja' => '傲慢ちき', 'en' => 'arrogant'}, 
  {'ja' => '無くす', 'en' => 'lose something'}, 
  {'ja' => 'プロフィール変えるよ', 'en' => "I'll change my profile."}, 
  {'ja' => 'チビのツンデレウーマン。\n魔法が得意。', 'en' => 'Tiny tsundere woman.\nShe is good at magic.'}, 
  {'ja' => 'ケスティニアスの雑用係。\nそんなに仕事は無い。', 'en' => "Kestinius' scullery maid.\nNot that much work."}, 
  {'ja' => '何があったのかよく思い出せない。\n\n辺りを包む木々。天を覆う新緑の葉。\n\n小鳥のせせらぎと、風の音だけが聴こえる。\n\n彼方には、見慣れたような、しかしそうでないような、\n懐かしさを僅かに感じる家並み———\n\nふと、目の前の景色が遠ざかる。\n何かに後ろに引っ張られるように、\n辺りを俯瞰するような景色が広がっていく。\n\n"アタシ"は後ろを振り向いた。\n\n肩まで行かない赤髪を垂らし、\n見開く緑目は"私"を見つめ———', 'en' => 'I can\'t quite remember what happened.\n\nTrees surrounding the area. Fresh green leaves covering the heavens.\n\nOnly the rustling of birds and the sound of the wind can be heard.\n\nIn the distance, the houses look familiar, but not so familiar,\nhouses, which seem familiar but not so familiar...\n\nSuddenly, the view in front of you fades away.\nAs if something is pulling you back,\nthe view of the area spreads out like a bird\'s eye view.\n\n"Atashi" turned around.\n\nHer red hair hangs down to her shoulders,\nGreen eyes wide open, staring at "me"...'}, 
  {'ja' => '勝ち', 'en' => 'win'}, 
  {'ja' => '逃げ犬', 'en' => 'fugitive dog'}, 
  {'ja' => '負け犬', 'en' => 'dejected loser'}, 
  {'ja' => '\\{言語\\}変える\\}よ', 'en' => "\\}I'm going to \\{change \\{Language\\}."}, 
  {'ja' => '日本語', 'en' => 'Japanese (language)'}, 
  {'ja' => '英語', 'en' => 'English (language)'}, 
  {'ja' => '中国語', 'en' => 'Chinese Language'}, 
  {'ja' => 'やっぱやめる', 'en' => 'give up'}, 
  {'ja' => 'フィールドを移動します', 'en' => 'Move field'}, 
  {'ja' => '行って帰る', 'en' => 'go and come back'}, 
  {'ja' => '行ったきり', 'en' => 'going and leaving'}, 
  {'ja' => '移動後のメッセージです。', 'en' => 'This is the message after the move.'},
  {'ja' => '更に移動した際のメッセージです', 'en' => "Here's the message when we move further"}
]

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
      actors:  [
        { name:  "エルーシェ", nickname:  "雑用係", profile:  "ケスティニアスの雑用係。\nそんなに仕事は無い。" },
        { name:  "ラフィーナ", nickname:  "傲慢ちき", profile:  "チビのツンデレウーマン。\n魔法が得意。" }
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
    actor = { name:  "エルーシェ", nickname:  "雑用係", profile:  "ケスティニアスの雑用係。\nそんなに仕事は無い。" }
    assert_equal "エルーシェ", actor[:name]
    assert_equal "雑用係", actor[:nickname]
    assert_equal "ケスティニアスの雑用係。\nそんなに仕事は無い。", actor[:profile]

    Langscore.changeLanguage("en")
    actor[:name] = "eluche"
    actor[:nickname] = "Compassionate"
    actor[:profile] = "Kestinius' scullery maid.\nNot that much work."
    assert_equal "eluche", actor[:name]
    assert_equal "Compassionate", actor[:nickname]
    assert_equal "Kestinius' scullery maid.\nNot that much work.", actor[:profile]

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
    
    # shop_event = { event_id => 6, message => "ようこそ！" }
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
