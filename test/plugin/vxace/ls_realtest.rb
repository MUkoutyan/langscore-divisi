# encoding: utf-8
#-------------------------------------------------------------------
# Langscore VXAce 実機テスト
#
# Game.exe (RGSS3) の中で Main の直前に実行され、結果を ls_test_result.txt に書く。
# 標準ライブラリが使えないため、アサーションと結果出力は自前で持つ。
# 実行は test/plugin/run_vxace_test.py 経由。
#
# テスト内容は test/plugin/Langscore_mvmz.test.js と対応させている。
# MV/MZ にしか存在しない機能は skip で理由を残す。
#-------------------------------------------------------------------

module LsTest

  RESULT_FILE = "ls_test_result.txt"

  class Failure < StandardError; end

  @results = []
  @suite   = ""
  @before  = nil

  class << self

    def suite(name)
      @suite  = name
      @before = nil
      yield
    end

    # 各テストの前に実行する処理。suite ブロックの先頭で指定する。
    def before(&block)
      @before = block
    end

    def test(name)
      begin
        @before.call if @before
        yield
        @results.push(["OK", @suite, name, ""])
      rescue Exception => e
        @results.push(["NG", @suite, name, describe_error(e)])
      end
    end

    # 実装が無いなどの理由で意図的に実行しないテスト
    def skip(name, reason)
      @results.push(["SKIP", @suite, name, reason])
    end

    #--- アサーション -------------------------------------------------
    def eq(expected, actual, msg = "")
      return if expected == actual
      fail_with("expected #{ins(expected)} but was #{ins(actual)}", msg)
    end

    def ne(unexpected, actual, msg = "")
      return if unexpected != actual
      fail_with("expected not #{ins(unexpected)}", msg)
    end

    def ok(cond, msg = "")
      return if cond
      fail_with("expected truthy but was #{ins(cond)}", msg)
    end

    def ng(cond, msg = "")
      return unless cond
      fail_with("expected falsy but was #{ins(cond)}", msg)
    end

    def includes(collection, value, msg = "")
      return if collection.include?(value)
      fail_with("#{ins(collection)} does not include #{ins(value)}", msg)
    end

    #--- 結果出力 -----------------------------------------------------
    def report
      lines = @results.map { |r| r.map { |c| c.to_s.gsub(/[\t\r\n]/, " ") }.join("\t") }
      lines.push("DONE\t#{@results.size}")
      File.open(RESULT_FILE, "wb") { |f| f.write(lines.join("\n") + "\n") }
    end

    def fatal(e)
      @results.push(["NG", @suite, "(fatal)", describe_error(e)])
    end

    private

    def fail_with(text, msg)
      raise Failure, (msg.to_s.empty? ? text : "#{text} : #{msg}")
    end

    def ins(value)
      s = value.inspect
      s.length > 160 ? s[0, 160] + "..." : s
    end

    def describe_error(e)
      where = (e.backtrace || []).find { |b| b.include?("LsInjectedTest") }
      "#{e.class}: #{e.message}" + (where ? " @ #{where}" : "")
    end
  end
end

#-------------------------------------------------------------------
# テストデータ (Langscore_mvmz.test.js と同じ対訳表)
#-------------------------------------------------------------------
module LsData
  # 制御文字や記号を含む代表的なマップテキスト。全件は Map001 の対訳表から検証する。
  # 対訳の中身はプロジェクトごとに異なるため、ここ以外の期待値は対訳表から取る。
  MAP_SAMPLES = [
    ["通常のテキストです",                     "Normal text."],
    ["カンマを含む,テキストです",              "Including comma, text."],
    ["1番の変数の値は \\V[1] です。",          "The value of variable 1 is \\V[1]."],
    ["この文字は \\C[2] 赤色 \\C[0] 通常色 です。",
     "This letter is \\C[2] red \\C[0] normal color."],
    ["バックスラッシュの表示は \\\\ です。",   "The backslash is displayed at \\\\"],
  ]

  # アクター名は MV/MZ のテストと共通のデータを使っている
  ACTOR_NAMES = [["エルーシェ", "eluche"], ["ラフィーナ", "Rafina."]]
end

#-------------------------------------------------------------------
# ゲーム状態のセットアップ
#-------------------------------------------------------------------
def ls_reset_database
  Langscore.translate_list_reset
  DataManager.load_normal_database
  DataManager.create_game_objects
end

def ls_new_game
  DataManager.init
  DataManager.setup_new_game
  SceneManager.goto(Scene_Map)
  SceneManager.scene.start
end

def ls_map_event(map_event_id)
  $game_map.events.each_value { |ev| return ev if ev.id == map_event_id }
  nil
end

# イベントリストに指定のコマンドコードを含むマップイベントを探す。
# イベント番号はプロジェクト側の都合で変わるため、番号ではなく内容で特定する。
def ls_find_event_with_code(code)
  $game_map.events.each_value do |ev|
    ev.list.each { |cmd| return ev if cmd.code == code } if ev.list
  end
  nil
end

# イベントを最後まで実行する。会話ウィンドウは即座に閉じる。
def ls_run_event(event)
  interpreter = Game_Interpreter.new
  interpreter.setup(event.list, event.id)
  300.times do
    interpreter.update
    $game_message.clear
    break unless interpreter.running?
  end
  interpreter
end

# 対訳表から原文に対する訳文を引く。langscore.rb と同じく末尾の改行を落として再試行する。
def ls_lookup(tr, text, lang)
  entry = tr[text] || tr[text.to_s.chomp("\r\n")]
  return nil if entry.nil?
  value = entry[lang]
  (value.nil? || value.empty?) ? nil : value
end

# data_list の各フィールドが ja→en→ja で置き換わり、かつ元に戻ることを確認する。
# Langscore_mvmz.test.js の expectDatabaseTranslated と同じ検証。
# 期待値はプロジェクトの対訳表そのものから取る (MV/MZ とは訳文が異なるため)。
# fetch は $data_items のようなグローバル変数を返す lambda。
# changeLanguage がデータベースを読み直して配列自体を作り替えるため、都度取り直す。
def ls_assert_database_translated(label, fetch, csv_name, fields)
  tr = LSCSV.to_hash(csv_name)
  LsTest.ok(tr.size > 0, "#{label}: 対訳表 #{csv_name} が空です")

  Langscore.changeLanguage("ja", true)
  ja_values = fetch.call.map { |obj| obj.nil? ? nil : fields.map { |f| obj.send(f) } }

  Langscore.changeLanguage("en", true)
  checked = 0
  fetch.call.each_with_index do |obj, i|
    next if obj.nil?
    fields.each_with_index do |f, j|
      en = ls_lookup(tr, ja_values[i][j], "en")
      next if en.nil?
      LsTest.eq(en, obj.send(f), "#{label}[#{i}].#{f}")
      checked += 1
    end
  end
  LsTest.ok(checked > 0, "#{label}: 翻訳された項目がありません")

  Langscore.changeLanguage("ja", true)
  fetch.call.each_with_index do |obj, i|
    next if obj.nil?
    fields.each_with_index { |f, j| LsTest.eq(ja_values[i][j], obj.send(f), "#{label}[#{i}].#{f} (復帰)") }
  end
end

#===================================================================
# 通常設定でのテスト
#===================================================================
def ls_run_default_tests

  LsTest.suite("Langscore") do
    LsTest.before { ls_reset_database }

    LsTest.test("言語の変更が行えている") do
      Langscore.changeLanguage("en", true)
      LsTest.eq("en", $langscore_current_language)
      Langscore.changeLanguage("ja", true)
      LsTest.eq("ja", $langscore_current_language)
    end

    LsTest.test("テキストを正しく翻訳すること") do
      hash = {
        "通常のテキストです"      => {"ja" => "通常のテキストです", "en" => "Normal text."},
        "カンマを含む,テキストです" => {"ja" => "カンマを含む,テキストです", "en" => "Including comma, text."},
      }
      hash.each do |key, trans|
        LsTest.eq(trans["en"], Langscore.translate(key, hash, "en"), key)
        LsTest.eq(trans["ja"], Langscore.translate(key, hash, "ja"), key)
      end
    end

    LsTest.test("翻訳が利用できない場合、元のテキストを返すこと") do
      hash = {"Hello" => {"ja" => "こんにちは", "en" => "Hello"}}
      LsTest.eq("Goodbye", Langscore.translate("Goodbye", hash, "ja"))
      # 対応する言語列が無い場合も原文を返す
      LsTest.eq("Hello", Langscore.translate("Hello", hash, "zh-cn"))
      # ハッシュそのものが無い場合も落ちない
      LsTest.eq("Hello", Langscore.translate("Hello", nil, "ja"))
    end

    LsTest.test("翻訳文から原文テキストを正しく取得すること") do
      hash = {
        "Hello"   => {"ja" => "こんにちは", "en" => "Hello"},
        "Goodbye" => {"ja" => "さようなら", "en" => "Goodbye"},
      }
      LsTest.eq("Hello",   Langscore.fetch_original_text("こんにちは", hash))
      LsTest.eq("Goodbye", Langscore.fetch_original_text("さようなら", hash))
      LsTest.eq("未知のテキスト", Langscore.fetch_original_text("未知のテキスト", hash))
    end

    LsTest.test("画像ファイルを正しくロードすること") do
      # デフォルト言語かつ ENABLE_TRANSLATION_FOR_DEFLANG が false なら言語別画像を探さない
      Langscore.changeLanguage("ja", true)
      $ls_graphic_cache.clear
      Cache.load_bitmap("Graphics/Pictures/", "nantoka8")
      LsTest.eq(nil, $ls_graphic_cache["nantoka8"], "デフォルト言語で言語別画像を探索しています")

      # 既定言語以外では "<ファイル名>_<言語>" の有無を調べ、結果をキャッシュする。
      # vxace_test には nantoka8_en が無いため、探索した結果として false が入る。
      Langscore.changeLanguage("en", true)
      bitmap = Cache.load_bitmap("Graphics/Pictures/", "nantoka8")
      LsTest.eq(false, $ls_graphic_cache["nantoka8"], "言語別画像の探索が行われていません")
      LsTest.ok(bitmap.is_a?(Bitmap) && bitmap.disposed? == false, "Bitmapが生成されていません")

      # 翻訳表で明示的に別ファイルが指定されている場合はそちらを読む。
      # vxace_test の Graphics 対訳表は空のため、対訳をその場で与えて経路だけを確認する。
      $ls_graphics_tr["Graphics/Pictures/nantoka10"] = {"en" => "Graphics/Pictures/nantoka8"}
      swapped = Cache.load_bitmap("Graphics/Pictures/", "nantoka10")
      LsTest.ok(swapped.equal?(bitmap), "対訳表で指定した画像に差し替わっていません")
    end

    LsTest.test("アクター名が正しく更新されること") do
      Langscore.changeLanguage("ja", true)
      LsTest.eq("エルーシェ", $game_actors[1].name)
      LsTest.eq("雑用係",     $game_actors[1].nickname)
      LsTest.eq("エルーシェ", $data_actors[1].name)

      Langscore.changeLanguage("en", true)
      LsTest.eq("eluche",        $game_actors[1].name)
      LsTest.eq("Compassionate", $game_actors[1].nickname)
      LsTest.eq("eluche",        $data_actors[1].name)
      LsTest.eq("Rafina.",       $game_actors[2].name)
      LsTest.eq("arrogant",      $game_actors[2].nickname)

      Langscore.changeLanguage("ja", true)
      LsTest.eq("エルーシェ", $game_actors[1].name)
      LsTest.eq("雑用係",     $game_actors[1].nickname)
    end

    LsTest.test("スキルが正しく更新されること") do
      ls_assert_database_translated("skills", lambda { $data_skills }, "Skills",
                                    [:name, :description, :message1, :message2])
    end

    LsTest.test("アイテム・武器・防具・職業・敵・ステートが正しく更新されること") do
      ls_assert_database_translated("items",   lambda { $data_items },   "Items",   [:name, :description])
      ls_assert_database_translated("weapons", lambda { $data_weapons }, "Weapons", [:name, :description])
      ls_assert_database_translated("armors",  lambda { $data_armors },  "Armors",  [:name, :description])
      ls_assert_database_translated("classes", lambda { $data_classes }, "Classes", [:name])
      ls_assert_database_translated("enemies", lambda { $data_enemies }, "Enemies", [:name])
      ls_assert_database_translated("states",  lambda { $data_states },  "States",
                                    [:name, :message1, :message2, :message3, :message4])
    end

    LsTest.test("システム用語と通貨単位が正しく更新されること") do
      terms = lambda do
        $data_system.terms.basic + $data_system.terms.commands + $data_system.terms.params +
          $data_system.terms.etypes + $data_system.elements + $data_system.skill_types +
          [$data_system.currency_unit]
      end

      Langscore.changeLanguage("ja", true)
      ja = terms.call
      Langscore.changeLanguage("en", true)
      en = terms.call

      LsTest.eq(ja.size, en.size, "用語の数が変わっています")
      changed = 0
      ja.zip(en) { |a, b| changed += 1 if a != b }
      LsTest.ok(changed > 0, "翻訳された用語がありません")
      LsTest.ne($data_system.currency_unit, "", "通貨単位が空です")

      Langscore.changeLanguage("ja", true)
      LsTest.eq(ja, terms.call, "日本語へ戻せていません")
    end

    LsTest.test("セーブデータが正しく保存されること") do
      Langscore.changeLanguage("ja", true)
      DataManager.save_game(2)
      DataManager.load_game(2)
      LsTest.eq("エルーシェ", $game_actors[1].name)
      LsTest.eq("雑用係",     $game_actors[1].nickname)
      LsTest.eq("ラフィーナ", $game_actors[2].name)

      Langscore.changeLanguage("en", true)
      DataManager.save_game(2)
      DataManager.load_game(2)
      LsTest.eq("eluche",        $game_actors[1].name)
      LsTest.eq("warrior",       $game_actors[1].class.name)
      LsTest.eq("Compassionate", $game_actors[1].nickname)
      LsTest.eq("Rafina.",       $game_actors[2].name)
      LsTest.eq("magician",      $game_actors[2].class.name)
    end

    LsTest.test("セーブデータの生成が元データを壊さないこと") do
      Langscore.changeLanguage("ja", true)
      before_dump = Marshal.dump($game_actors)
      contents = DataManager.make_save_contents
      LsTest.eq("エルーシェ", contents[:actors][1].name)
      LsTest.eq(before_dump, Marshal.dump($game_actors),
                "make_save_contents の呼び出しで $game_actors が変化しています")
    end

    LsTest.test("Scripts.csv によりスクリプト内テキストが翻訳されること") do
      # VXAce の Scripts 対訳表はキーが行情報 ("<スクリプトID>:<行>:<桁>") で、
      # 各言語列に訳文が入る。langscore_custom.rb が String#lstrans 経由で引く。
      key = nil
      $ls_scripts_tr.each do |line_info, trans|
        next if trans["ja"].to_s.empty? || trans["en"].to_s.empty? || trans["ja"] == trans["en"]
        key = line_info
        break
      end
      LsTest.ok(key, "Scripts の対訳が空です")
      trans = $ls_scripts_tr[key]

      Langscore.changeLanguage("en", true)
      LsTest.eq(trans["en"], Langscore.translate_for_script(key))
      LsTest.eq(trans["en"], key.lstrans(key))

      Langscore.changeLanguage("ja", true)
      LsTest.eq(trans["ja"], Langscore.translate_for_script(key))
    end

    LsTest.skip("言語変更が Langscore_ObserverBridge 経由で通知されること",
                "ObserverBridge は MV/MZ のプラグイン間連携用で VXAce には無い")

    LsTest.test("言語選択メニューの表示と言語変更ができること") do
      Langscore.changeLanguage("ja", true)
      ls_new_game
      SceneManager.snapshot_for_background

      Langscore.display_language_menu
      scene = SceneManager.scene
      LsTest.eq(Langscore::UI::Scene_Language, scene.class)
      scene.start

      window = scene.instance_variable_get(:@command_window)
      langs  = Langscore.get_available_languages
      LsTest.eq(["ja", "en"], langs)
      # 表示名は SYSTEM2 (divisi が生成する言語名テーブル) から取る
      names = (0...window.item_max).map { |i| window.command_name(i) }
      LsTest.eq(langs.map { |l| Langscore::SYSTEM2[l] }, names)
      LsTest.eq(langs.index("ja"), window.index, "現在の言語が選択されていません")

      # OK/Cancel ウィンドウは OK / Reselect / Cancel の3項目
      okcancel = scene.instance_variable_get(:@okcancel_window)
      LsTest.eq(3, okcancel.item_max)

      # English を選んで確定すると言語が変わる
      window.select(langs.index("en"))
      scene.terminate
      LsTest.eq("en", $langscore_current_language)
    end
  end

  #-----------------------------------------------------------------
  LsTest.suite("Langscore for Map") do
    LsTest.before do
      ls_new_game
      $game_map.setup(1)
    end

    LsTest.test("翻訳文を正しく取得できるか") do
      Langscore.changeLanguage("en", true)

      LsData::MAP_SAMPLES.each do |ja, en|
        LsTest.eq(en, Langscore.translate_for_map(ja), ja)
      end

      # 代表例だけでなく Map001 の対訳表を全件確認する
      checked = 0
      $ls_current_map[1].each do |origin, trans|
        en = trans["en"]
        next if en.nil? || en.empty?
        LsTest.eq(en, Langscore.translate_for_map(origin), origin)
        checked += 1
      end
      LsTest.ok(checked > 0, "Map001 の対訳表が空です")
    end

    LsTest.test("テキストの変換に成功するか") do
      Langscore.changeLanguage("en", true)
      window = Window_Base.new(0, 0, 100, 100)
      begin
        $ls_current_map[1].each do |origin, trans|
          en = trans["en"]
          next if en.nil? || en.empty?
          # 翻訳後に制御文字を展開した結果は、英文をそのまま展開した結果と一致する
          # 翻訳が見つかった場合、langscore.rb は Game_Message.all_text と同じく改行を付与する
          LsTest.eq(window.ls_base_convert_escape_characters(en + "
"),
                    window.convert_escape_characters(origin), origin)
        end
      ensure
        window.dispose
      end
    end

    LsTest.test("アクター名の変更イベントが正しく反映されているか") do
      event = ls_find_event_with_code(320)
      LsTest.ok(event, "名前変更 (320) を含むイベントがありません")

      Langscore.changeLanguage("en", true)
      ls_run_event(event)
      LsTest.ne("エルーシェ", $game_actors[1].name, "名前変更後に日本語へ戻っています")

      Langscore.changeLanguage("ja", true)
      LsTest.ok($game_actors[1].name.length > 0)
    end

    LsTest.test("二つ名の変更イベントが正しく反映されているか") do
      event = ls_find_event_with_code(324)
      LsTest.ok(event, "二つ名変更 (324) を含むイベントがありません")

      Langscore.changeLanguage("en", true)
      ls_run_event(event)
      LsTest.ne("雑用係", $game_actors[1].nickname, "二つ名変更後に日本語へ戻っています")
    end

    LsTest.test("ショップの商品名が翻訳されているか") do
      event = ls_find_event_with_code(302)
      LsTest.ok(event, "ショップ (302) を含むイベントがありません")

      goods_names = lambda do
        ls_run_event(event)
        scene = SceneManager.scene
        LsTest.eq(Scene_Shop, scene.class)
        goods = scene.instance_variable_get(:@goods)
        LsTest.ok(goods && goods.size > 0, "商品が空です")
        tables = [$data_items, $data_weapons, $data_armors]
        names = goods.map { |g| tables[g[0]][g[1]].name }
        SceneManager.return
        names
      end

      Langscore.changeLanguage("ja", true)
      ja_names = goods_names.call
      Langscore.changeLanguage("en", true)
      en_names = goods_names.call

      LsTest.eq(ja_names.size, en_names.size)
      LsTest.ne(ja_names, en_names, "ショップの商品名が翻訳されていません")
      # 商品名は翻訳後のデータベースと一致していること
      LsTest.includes(en_names, "portion")
      LsTest.includes(ja_names, "ポーション")
    end

    LsTest.test("セーブファイルの書き込みとロード") do
      window = Window_Base.new(0, 0, 100, 100)
      begin
        Langscore.changeLanguage("ja", true)
        LsTest.eq("エルーシェ", $game_actors[1].name)
        DataManager.save_game(3)

        Langscore.changeLanguage("en", true)
        LsTest.eq("eluche", $game_actors[1].name)

        # 言語設定はセーブデータの影響を受けないため、ロード後も英語のままが正しい
        DataManager.load_game(3)
        LsTest.eq("en", $langscore_current_language)
        LsTest.eq("eluche", $game_actors[1].name)

        $game_map.setup(1)
        $ls_current_map[1].each do |origin, trans|
          en = trans["en"]
          next if en.nil? || en.empty?
          # 翻訳が見つかった場合、langscore.rb は Game_Message.all_text と同じく改行を付与する
          LsTest.eq(window.ls_base_convert_escape_characters(en + "
"),
                    window.convert_escape_characters(origin), origin)
        end
      ensure
        window.dispose
      end
    end

    LsTest.test("2024/06報告不具合 セーブ後に一部の制御文字が日本語のまま表示される") do
      window = Window_Base.new(0, 0, 100, 100)
      begin
        Langscore.changeLanguage("ja", true)
        LsTest.eq("エルーシェ", $game_actors[1].name)

        Langscore.changeLanguage("en", true)
        DataManager.save_game(3)

        LsTest.eq("eluche", $game_actors[1].name)
        $ls_current_map[1].each do |origin, trans|
          en = trans["en"]
          next if en.nil? || en.empty?
          # 翻訳が見つかった場合、langscore.rb は Game_Message.all_text と同じく改行を付与する
          LsTest.eq(window.ls_base_convert_escape_characters(en + "
"),
                    window.convert_escape_characters(origin), origin)
        end
      ensure
        window.dispose
      end
    end

    LsTest.skip("プロフィールの変更イベントが正しく反映されているか",
                "langscore.rb はプロフィール変更 (325) を alias していない (MV/MZ のみ対応)")
    LsTest.skip("2024/10報告不具合 名前入力が常に英語になる",
                "VXAce の Game_System#japanese? は $data_system.japanese を返すだけで、選択言語に追従しない")
  end

  #-----------------------------------------------------------------
  LsTest.suite("入力値の検証") do
    LsTest.before { ls_reset_database }

    LsTest.test("言語コードは許可リストにあるものだけを受け付けること") do
      LsTest.ok(Langscore::STSTEM_ALLOWED_LANGUAGES.size > 0)
      Langscore::STSTEM_ALLOWED_LANGUAGES.each do |lang|
        LsTest.ok(Langscore.is_valid_language_code(lang), lang)
      end
      ["xx", "", "JA", "../ja"].each do |lang|
        LsTest.eq(false, Langscore.is_valid_language_code(lang), lang)
      end
    end

    LsTest.test("パッチモードが無効なら SUPPORT_LANGUAGE がそのまま利用可能言語になること") do
      LsTest.eq(false, Langscore::ENABLE_PATCH_MODE)
      LsTest.eq(Langscore::SUPPORT_LANGUAGE, Langscore.get_available_languages(true))
    end

    LsTest.skip("言語フォルダ名は英数字・ハイフン・アンダースコアのみ許可すること",
                "langscore.rb に isValidLanguageFolder 相当が無い (Langscore.js のみ)")
    LsTest.skip("翻訳ファイル名はパストラバーサルを弾くこと",
                "langscore.rb に isValidFileName 相当が無い (Langscore.js のみ)")
    LsTest.skip("ファイル名のサニタイズで危険な文字が除去されること",
                "langscore.rb に sanitizeFileName 相当が無い (Langscore.js のみ)")
    LsTest.skip("サポート外の言語への変更は無視されること",
                "langscore.rb の changeLanguage は言語コードを検証していない (Langscore.js のみ)")
  end

  #-----------------------------------------------------------------
  LsTest.suite("言語判定とフォント") do
    LsTest.before { ls_reset_database }

    LsTest.test("言語変更時に該当言語のフォント定義へ切り替わること") do
      Langscore::SUPPORT_LANGUAGE.each do |lang|
        Langscore.changeLanguage(lang, true)
        font = Langscore::LS_FONT[lang]
        LsTest.ok(font, "#{lang} のフォント定義がありません")
        LsTest.eq(font[:name], Font.default_name, lang)
        LsTest.eq(font[:size], Font.default_size, lang)
      end
    end

    LsTest.test("ウィンドウのフォント設定にLangscoreのフォントが反映されること") do
      ls_new_game
      Langscore.changeLanguage("en", true)
      font = Langscore::LS_FONT["en"]

      window = Window_Base.new(0, 0, 100, 100)
      begin
        LsTest.eq(font[:name], window.contents.font.name)
        LsTest.eq(font[:size], window.contents.font.size)
        # 実フォントで幅が取れること (スタブでは検証できない部分)
        LsTest.ok(window.text_size("Langscore").width > 0, "文字幅が取得できません")
      ensure
        window.dispose
      end
    end

    LsTest.test("シーンが保持するウィンドウのフォントも更新されること") do
      ls_new_game
      Langscore.changeLanguage("ja", true)
      scene_window = Window_Base.new(0, 0, 100, 100)
      begin
        SceneManager.scene.instance_variable_set(:@ls_test_window, scene_window)
        Langscore.changeLanguage("en", true)
        LsTest.eq(Langscore::LS_FONT["en"][:name], scene_window.contents.font.name)
        LsTest.eq(Langscore::LS_FONT["en"][:size], scene_window.contents.font.size)
      ensure
        scene_window.dispose
      end
    end

    LsTest.skip("Game_System の言語判定が現在の言語に追従すること",
                "VXAce には isCJK / isJapanese 相当が無い (Game_System#japanese? はプロジェクト設定)")
  end

  #-----------------------------------------------------------------
  LsTest.suite("Language State Variable") do
    LsTest.before { ls_reset_database }

    LsTest.test("対応言語の利用可否が指定変数から順に格納されること") do
      # LANG_STATE_STARTVARIABLE は langscore.rb では -1 固定のため、テスト中だけ差し替える。
      before = Langscore::LANG_STATE_STARTVARIABLE
      warn_level = $VERBOSE
      begin
        $VERBOSE = nil
        Langscore.const_set(:LANG_STATE_STARTVARIABLE, 1)
        Langscore.update_language_state_variables

        Langscore::STSTEM_ALLOWED_LANGUAGES.each_with_index do |lang, i|
          expected = Langscore::SUPPORT_LANGUAGE.include?(lang) ? 1 : 0
          LsTest.eq(expected, $game_variables[1 + i], lang)
        end
      ensure
        Langscore.const_set(:LANG_STATE_STARTVARIABLE, before)
        $VERBOSE = warn_level
      end
    end

    LsTest.test("開始変数が -1 の場合は何もしないこと") do
      LsTest.eq(-1, Langscore::LANG_STATE_STARTVARIABLE)
      $game_variables[1] = 99
      Langscore.update_language_state_variables
      LsTest.eq(99, $game_variables[1])
    end
  end

  #-----------------------------------------------------------------
  LsTest.suite("Enable Translation For Default Language") do
    LsTest.before { ls_reset_database }

    LsTest.test("false (既定): デフォルト言語では翻訳処理を通さないこと") do
      LsTest.eq(false, Langscore::ENABLE_TRANSLATION_FOR_DEFLANG)
      Langscore.changeLanguage("ja", true)
      $ls_graphic_cache.clear
      Cache.load_bitmap("Graphics/Pictures/", "nantoka8")
      LsTest.eq(0, $ls_graphic_cache.size, "デフォルト言語で言語別画像を探索しています")

      window = Window_Base.new(0, 0, 100, 100)
      begin
        origin = LsData::MAP_SAMPLES[0][0]
        LsTest.eq(origin, window.convert_escape_characters(origin))
      ensure
        window.dispose
      end
    end
  end
end

#===================================================================
# ENABLE_TRANSLATION_FOR_DEFLANG = true でのテスト
#===================================================================
def ls_run_deflang_tests
  LsTest.suite("Enable Translation For Default Language") do
    LsTest.before { ls_reset_database }

    LsTest.test("true: デフォルト言語でも翻訳処理を通すこと") do
      LsTest.eq(true, Langscore::ENABLE_TRANSLATION_FOR_DEFLANG)
      Langscore.changeLanguage("ja", true)

      $ls_graphic_cache.clear
      Cache.load_bitmap("Graphics/Pictures/", "nantoka8")
      LsTest.eq(false, $ls_graphic_cache.empty?, "デフォルト言語でも言語別画像を探索するはずです")
    end

    LsTest.test("true: デフォルト言語でもマップテキストの変換処理を通ること") do
      ls_new_game
      $game_map.setup(1)
      Langscore.changeLanguage("ja", true)
      window = Window_Base.new(0, 0, 100, 100)
      begin
        origin = LsData::MAP_SAMPLES[0][0]
        LsTest.eq(origin, window.convert_escape_characters(origin))
      ensure
        window.dispose
      end
    end
  end
end

#===================================================================
# ENABLE_PATCH_MODE = true でのテスト
#===================================================================
def ls_run_patch_tests
  LsTest.suite("Language Patch Mode") do

    LsTest.test("Data/Translate 配下のフォルダから選択可能言語が決まること") do
      LsTest.eq(true, Langscore::ENABLE_PATCH_MODE)
      langs = Langscore.get_available_languages(true)
      # 実行前に runner が ja / en / xx フォルダを用意している。
      # xx は許可リストに無いため除外される。
      LsTest.eq(["en", "ja"], langs.sort)
      LsTest.ng(langs.include?("xx"), "許可されない言語フォルダが混入しています")
    end

    LsTest.test("翻訳フォルダのパスに現在の言語が含まれること") do
      Langscore.changeLanguage("en", true)
      LsTest.eq(Langscore::TRANSLATE_FOLDER + "/en", Langscore.get_translate_folder)
      Langscore.changeLanguage("ja", true)
      LsTest.eq(Langscore::TRANSLATE_FOLDER + "/ja", Langscore.get_translate_folder)
    end

    LsTest.skip("言語フォルダの翻訳が適用されること",
                "vxace_test に言語別の翻訳データ (Data/Translate/<lang>/*.rvdata2) が無い")
    LsTest.skip("パッチモードでの言語変更はエラーになること",
                "Webブラウザ実行の判定は MV/MZ のみ")
  end
end

#===================================================================
begin
  if Langscore::ENABLE_PATCH_MODE
    ls_run_patch_tests
  elsif Langscore::ENABLE_TRANSLATION_FOR_DEFLANG
    ls_run_deflang_tests
  else
    ls_run_default_tests
  end
rescue Exception => e
  LsTest.fatal(e)
end

LsTest.report
exit
