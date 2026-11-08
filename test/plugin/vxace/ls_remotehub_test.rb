# encoding: utf-8
#-------------------------------------------------------------------
# Langscore_RemoteHub (VXAce) 実機テスト
#
# Game.exe (RGSS3) の中で、Langscore_RemoteHub.rb の後・Main の直前に実行され、
# 結果を ls_test_result.txt に書く。実行は test/plugin/run_vxace_test.py remotehub 経由。
# ゲームのプロジェクトは run_vxace_test.py が一時フォルダへコピーしたものを使う。
#
# 確認する内容 (優先度順)
#  1. ゲームのプロジェクトを書き換えないこと (ファイル全体の比較は run_vxace_test.py が行う)
#  2. 壊れた翻訳ファイルや書き込みの失敗でゲームが止まらないこと
#  3. Langscore のプロジェクトでは Langscore_RemoteHub フォルダ以外を書き換えないこと
#  4. 表示中の文章・選択肢・カーソルの項目の書き出しと、翻訳の読み直し
#-------------------------------------------------------------------

module RhTest
  RESULT_FILE = "ls_test_result.txt"
  @results = []
  @suite = ""

  class Failure < StandardError; end

  class << self
    def suite(name)
      @suite = name
      yield
    end

    def test(name)
      begin
        yield
        @results.push(["OK", @suite, name, ""])
      rescue Exception => e
        where = (e.backtrace || []).first(4).join(" < ")
        @results.push(["NG", @suite, name, "#{e.class}: #{e.message} @ #{where}"])
      end
    end

    def eq(expected, actual, msg = "")
      return if expected == actual
      raise Failure, "expected #{expected.inspect} but was #{actual.inspect} #{msg}"
    end

    def ok(cond, msg = "")
      raise Failure, "expected truthy #{msg}" unless cond
    end

    def report
      lines = @results.map { |r| r.map { |c| c.to_s.gsub(/[\t\r\n]/, " ") }.join("\t") }
      lines.push("DONE\t#{@results.size}")
      File.open(RESULT_FILE, "wb") { |f| f.write(lines.join("\n") + "\n") }
    end

    def fatal(e)
      @results.push(["NG", @suite, "(fatal)", "#{e.class}: #{e.message}"])
    end
  end
end

HUB = LangscoreRemoteHub::HUB_DIR

# current.json の texts。Langscore_RemoteHub.rb の write_current の形式に合わせている
def rh_texts
  json = File.open(HUB + "/current.json", "rb") { |f| f.read }.force_encoding("UTF-8")
  str = '"((?:[^"\\\\]|\\\\.)*)"'
  pattern = /\{"window":#{str},"file":#{str},"original":#{str},"translated":#{str},"choice":(true|false),"selected":(true|false)\}/
  json.scan(pattern).map do |w, f, o, t, c, s|
    unescape = lambda { |v| v.gsub("\\n", "\n").gsub("\\r", "\r").gsub("\\\"", "\"").gsub("\\\\", "\\") }
    { :window => w, :file => f, :original => unescape.call(o), :translated => unescape.call(t),
      :choice => c == "true", :selected => s == "true" }
  end
end

def rh_selected
  rh_texts.select { |t| t[:selected] }
end

# カーソルの項目を書き出すときに Scene_Base#update が渡すシーンの代わり
def rh_scene(*windows)
  scene = Object.new
  windows.each_with_index { |w, i| scene.instance_variable_set("@window#{i}", w) }
  scene
end

def rh_open(w)
  w.openness = 255
  w.visible = true
  w.activate
  w
end

# Langscore_RemoteHub.update は CHECK_INTERVAL フレームごとに翻訳ファイルを確かめる
def rh_update_frames
  (LangscoreRemoteHub::CHECK_INTERVAL * 2).times { LangscoreRemoteHub.update }
end

def rh_translate_file(name)
  Langscore::TRANSLATE_FOLDER + "/" + name
end

# 翻訳ファイルを書き換え、ブロックの後で元の内容と時刻に戻す
def rh_with_translate_file(name, content)
  path = rh_translate_file(name)
  original = File.exist?(path) ? File.open(path, "rb") { |f| f.read } : nil
  mtime = File.exist?(path) ? File.mtime(path) : nil
  begin
    rh_update_frames
    if content.nil?
      File.delete(path) if File.exist?(path)
    else
      File.open(path, "wb") { |f| f.write(content) }
      File.utime(Time.now + 10, Time.now + 10, path)
    end
    yield
  ensure
    if original
      File.open(path, "wb") { |f| f.write(original) }
      File.utime(mtime, mtime, path)
    elsif File.exist?(path)
      File.delete(path)
    end
    rh_update_frames
  end
end

# 翻訳ファイル (rvdata2) の CSV の文字列を書き換えた内容
def rh_edited_rvdata(name, from, to)
  data = load_data(rh_translate_file(name))
  data.data = data.data.sub(from, to)
  Marshal.dump(data)
end

def rh_new_game
  DataManager.init
  DataManager.setup_new_game
  $game_map.setup(1)
  Langscore.changeLanguage("en", true)
end

#===================================================================
begin
  rh_new_game
  #メニューのコマンドは、タイトル画面でカーソルの位置を初期化してから作る
  Window_MenuCommand.init_command_position

  RhTest.suite("起動") do
    RhTest.test("Langscore のプロジェクトに Langscore_RemoteHub フォルダを作ること") do
      game_folder = File.basename(Dir.pwd)
      RhTest.eq("../#{game_folder}_langscore/Langscore_RemoteHub", HUB)
      RhTest.ok(File.directory?(HUB))
      RhTest.ok(!File.exist?("Langscore_RemoteHub"), "ゲームのフォルダに作っています")
    end

    RhTest.test("pid に自分のプロセスIDを書くこと") do
      pid = Win32API.new('kernel32', 'GetCurrentProcessId', '', 'L').call
      RhTest.eq(pid.to_s, File.open(HUB + "/pid", "rb") { |f| f.read })
    end
  end

  RhTest.suite("表示中の文章の書き出し") do
    RhTest.test("メッセージの原文・訳文・翻訳ファイルを書き出すこと") do
      LangscoreRemoteHub.clear_shown_texts
      window = Window_Base.new(0, 0, 100, 100)
      message = Window_Message.new
      begin
        message.convert_escape_characters("通常のテキストです")
        shown = rh_texts.find { |t| t[:original] == "通常のテキストです" }
        RhTest.ok(shown, "書き出されていません")
        RhTest.eq("Map001.csv", shown[:file])
        RhTest.eq("Window_Message", shown[:window])
        RhTest.eq(Langscore.translate_for_map("通常のテキストです"), shown[:translated])
        RhTest.eq(false, shown[:choice])
      ensure
        message.dispose
        window.dispose
      end
    end

    RhTest.test("翻訳ファイルに無い文章は、翻訳ファイルを空にして書き出すこと") do
      LangscoreRemoteHub.clear_shown_texts
      message = Window_Message.new
      begin
        message.convert_escape_characters("翻訳ファイルに無い文章")
        shown = rh_texts.find { |t| t[:original] == "翻訳ファイルに無い文章" }
        RhTest.ok(shown, "書き出されていません")
        RhTest.eq("", shown[:file])
      ensure
        message.dispose
      end
    end

    RhTest.test("選択肢はカーソルの当たっているものに印を付けること") do
      LangscoreRemoteHub.clear_shown_texts
      message = Window_Message.new
      choice = Window_ChoiceList.new(message)
      begin
        $game_message.choices.replace(["ラフィーナ", "エルーシェ", "やめる"])
        choice.start
        RhTest.eq(3, rh_texts.count { |t| t[:choice] })
        RhTest.eq(["ラフィーナ"], rh_selected.map { |t| t[:original] })
        RhTest.eq("Map001.csv", rh_texts.find { |t| t[:original] == "ラフィーナ" }[:file])
        choice.select(2)
        RhTest.eq(["やめる"], rh_selected.map { |t| t[:original] })
        choice.select(-1)
        RhTest.eq([], rh_selected)
      ensure
        $game_message.clear
        choice.dispose
        message.dispose
      end
    end
  end

  RhTest.suite("カーソルの当たっているデータベースの項目") do
    RhTest.test("アイテムの一覧は、アイテム名の原文と Items.csv を書き出すこと") do
      LangscoreRemoteHub.clear_shown_texts
      list = rh_open(Window_ItemList.new(0, 0, 300, 200))
      begin
        list.instance_variable_set(:@data, [$data_items[1]])
        list.select(0)
        LangscoreRemoteHub.update_focus(rh_scene(list))
        RhTest.eq(1, rh_selected.size)
        focused = rh_selected.first
        RhTest.eq("Items.csv", focused[:file])
        RhTest.eq($data_items[1].name, focused[:translated])
        RhTest.eq(Langscore.fetch_original_text($data_items[1].name, $ls_item_tr), focused[:original])
        RhTest.ok($ls_item_tr.has_key?(focused[:original]))

        # 操作中のウィンドウが無くなったら外す
        list.deactivate
        LangscoreRemoteHub.update_focus(rh_scene(list))
        RhTest.eq([], rh_selected)
      ensure
        list.dispose
      end
    end

    RhTest.test("コマンドは、コマンド名の原文と System.csv を書き出すこと") do
      LangscoreRemoteHub.clear_shown_texts
      command = rh_open(Window_MenuCommand.new)
      begin
        index = (0...command.item_max).find { |i| command.command_name(i) == Vocab::item }
        RhTest.ok(index, "アイテムのコマンドがありません")
        command.select(index)
        LangscoreRemoteHub.update_focus(rh_scene(command))
        focused = rh_selected.first
        RhTest.ok(focused, "書き出されていません")
        RhTest.eq("System.csv", focused[:file])
        RhTest.eq(Vocab::item, focused[:translated])
        RhTest.ok($ls_system_tr.has_key?(focused[:original]))
      ensure
        command.dispose
      end
    end

    RhTest.test("アクターの選択は、アクター名の原文と Actors.csv を書き出すこと") do
      LangscoreRemoteHub.clear_shown_texts
      status = rh_open(Window_MenuStatus.new(0, 0))
      begin
        status.select(0)
        LangscoreRemoteHub.update_focus(rh_scene(status))
        focused = rh_selected.first
        RhTest.ok(focused, "書き出されていません")
        RhTest.eq("Actors.csv", focused[:file])
        RhTest.eq($game_party.members[0].name, focused[:translated])
        RhTest.ok($ls_actor_tr.has_key?(focused[:original]))
      ensure
        status.dispose
      end
    end

    RhTest.test("データベースに無い項目では何も書き出さないこと") do
      LangscoreRemoteHub.clear_shown_texts
      list = rh_open(Window_ItemList.new(0, 0, 300, 200))
      begin
        item = RPG::Item.new
        item.name = "データベースに無いアイテム"
        list.instance_variable_set(:@data, [item])
        list.select(0)
        LangscoreRemoteHub.update_focus(rh_scene(list))
        RhTest.eq([], rh_selected)
      ensure
        list.dispose
      end
    end

    RhTest.test("項目の取得でエラーが出ても、ゲームを止めないこと") do
      list = rh_open(Window_ItemList.new(0, 0, 300, 200))
      begin
        list.instance_variable_set(:@data, [$data_items[1]])
        list.select(0)
        def list.item; raise "broken window"; end
        LangscoreRemoteHub.update_focus(rh_scene(list))
      ensure
        list.dispose
      end
    end
  end

  RhTest.suite("翻訳の読み直し") do
    RhTest.test("データベースの翻訳ファイルを書き換えると、アイテム名が変わること") do
      origin = Langscore.fetch_original_text($data_items[1].name, $ls_item_tr)
      before = $data_items[1].name
      edited = rh_edited_rvdata("Items.rvdata2", "," + before, "," + before + " edited")
      rh_with_translate_file("Items.rvdata2", edited) do
        rh_update_frames
        RhTest.eq(before + " edited", $data_items[1].name)
      end
      # 元に戻すと戻る
      RhTest.eq(before, $data_items[1].name)
      RhTest.eq(origin, Langscore.fetch_original_text($data_items[1].name, $ls_item_tr))
    end

    RhTest.test("マップの翻訳ファイルを書き換えると、文章の訳が変わること") do
      before = Langscore.translate_for_map("通常のテキストです")
      edited = rh_edited_rvdata("Map001.rvdata2", before, "Normal text edited.")
      rh_with_translate_file("Map001.rvdata2", edited) do
        rh_update_frames
        RhTest.eq("Normal text edited.", Langscore.translate_for_map("通常のテキストです"))
      end
      RhTest.eq(before, Langscore.translate_for_map("通常のテキストです"))
    end

    RhTest.test("書き込み途中で変わり続けている翻訳ファイルは、変わらなくなってから読み直すこと") do
      before = $data_items[1].name
      edited = rh_edited_rvdata("Items.rvdata2", "," + before, "," + before + " edited")
      rh_with_translate_file("Items.rvdata2", edited) do
        LangscoreRemoteHub::CHECK_INTERVAL.times { LangscoreRemoteHub.update }
        RhTest.eq(before, $data_items[1].name, "変わった直後に読み直しています")
        LangscoreRemoteHub::CHECK_INTERVAL.times { LangscoreRemoteHub.update }
        RhTest.eq(before + " edited", $data_items[1].name)
      end
    end

    RhTest.test("壊れた翻訳ファイルを読んだ後も、Langscore がファイルを書き換えられること") do
      rh_with_translate_file("Items.rvdata2", "\x04\x08broken") do
        rh_update_frames
        path = rh_translate_file("Items.rvdata2")
        File.utime(Time.now + 20, Time.now + 20, path)
        File.open(path, "wb") { |f| f.write("\x04\x08broken") }
      end
    end

    RhTest.test("壊れた翻訳ファイルでもゲームを止めないこと") do
      rh_with_translate_file("Items.rvdata2", "\x04\x08broken") do
        rh_update_frames
        RhTest.ok($data_items[1].name.is_a?(String))
      end
      rh_with_translate_file("Map001.rvdata2", "") do
        rh_update_frames
        Langscore.translate_for_map("通常のテキストです")
      end
    end

    RhTest.test("翻訳ファイルを消しても、ゲームを止めないこと") do
      rh_with_translate_file("Weapons.rvdata2", nil) do
        rh_update_frames
        RhTest.ok($data_weapons[1].name.is_a?(String))
      end
    end
  end

  RhTest.suite("書き出しの失敗") do
    RhTest.test("current.json に書けなくても、文章の表示を止めないこと") do
      path = HUB + "/current.json"
      File.delete(path) if File.exist?(path)
      Dir.mkdir(path)
      message = Window_Message.new
      begin
        RhTest.ok(message.convert_escape_characters("通常のテキストです").is_a?(String))
      ensure
        message.dispose
        Dir.rmdir(path)
      end
    end
  end
rescue Exception => e
  RhTest.fatal(e)
end

RhTest.report
exit
