#---------------------------------------------------------------
#
# Langscore DebugScript "RemoteHub"
# Version 1.0.0
# Written by BreezeSinfonia 來奈津
#
# テストプレイ中のみ動作します。
# ・ゲーム実行中にLangscoreアプリと連携し、編集した翻訳文を反映させるためのプラグインです。
# ・製品リリース時にはこのプラグインは削除して下さい。
#
# 表示中のメッセージは次のメッセージから新しい翻訳になります。
# メニューなどのウィンドウは読み直した後に再描画します。
#---------------------------------------------------------------
if $TEST

module LangscoreRemoteHub
  #Langscoreのプロジェクトはゲームのフォルダーの隣にある。見つからなければゲームのフォルダーに作る
  LANGSCORE_DIR = "../" + File.basename(Dir.pwd) + "_langscore"
  HUB_DIR = (File.directory?(LANGSCORE_DIR) ? LANGSCORE_DIR + "/" : "") + "remote"
  #翻訳ファイルの変更を確かめる間隔（フレーム）
  CHECK_INTERVAL = 30

  #データベースの翻訳は nil にすると、changeLanguage の中で読み直される
  DATABASE_FILES = {
    "Actors"  => lambda { $ls_actor_tr = nil },
    "System"  => lambda { $ls_system_tr = nil },
    "Classes" => lambda { $ls_classes_tr = nil },
    "Skills"  => lambda { $ls_skill_tr = nil },
    "States"  => lambda { $ls_states_tr = nil },
    "Weapons" => lambda { $ls_weapons_tr = nil },
    "Armors"  => lambda { $ls_armors_tr = nil },
    "Items"   => lambda { $ls_item_tr = nil },
    "Enemies" => lambda { $ls_enemies_tr = nil },
    "Graphics" => lambda { $ls_graphics_tr = LSCSV.to_hash("Graphics") },
    "Scripts"  => lambda { $ls_scripts_tr = LSCSV.to_hash("Scripts") },
  }
  TEXT_FILES = {
    "Troops"       => lambda { $ls_troop_tr = LSCSV.to_hash("Troops") },
    "CommonEvents" => lambda { $ls_common_event = LSCSV.to_hash("CommonEvents") },
  }

  @frame = 0
  @file_times = nil
  @pending_files = []
  @capture = nil
  @shown_texts = []

  def self.translate_files
    Dir.glob(Langscore::TRANSLATE_FOLDER + "/**/*.{rvdata2,csv}")
  end

  def self.read_file_times
    times = {}
    translate_files.each { |f| times[f] = File.mtime(f) rescue nil }
    times
  end

  def self.update
    @frame += 1
    return if @frame % CHECK_INTERVAL != 0

    times = read_file_times
    if @file_times.nil?
      @file_times = times
      return
    end
    changed = times.keys.select { |f| times[f] != @file_times[f] }
    @file_times = times
    #書き込み途中のファイルを読まないよう、変わってから次に確かめるまで変わらなかったものを読み直す
    ready = @pending_files.select { |f| !changed.include?(f) }
    @pending_files = (@pending_files - ready) | changed
    reload(ready.map { |f| File.basename(f, ".*") }.uniq) unless ready.empty?
  end

  def self.reload(names)
    need_change_language = false
    names.each do |name|
      if name =~ /^Map(\d+)$/
        map_id = $1.to_i
        #読み込んでいないマップは、移動したときに新しい翻訳が読まれる
        $ls_current_map[map_id] = LSCSV.to_hash(name) if $ls_current_map.include?(map_id)
      elsif TEXT_FILES.include?(name)
        TEXT_FILES[name].call
      elsif DATABASE_FILES.include?(name)
        restore_actor_names if name == "Actors"
        DATABASE_FILES[name].call
        need_change_language = true
      else
        next
      end
      p "Langscore_RemoteHub: reloaded #{name}"
    end
    Langscore.changeLanguage($langscore_current_language, true) if need_change_language
    #load_data は壊れたファイルを読み損ねると、GC までファイルを開いたままにする。
    #開いたままだと Langscore がファイルを書き換えられないため、すぐに閉じさせる
    GC.start
    refresh_windows
  end

  #アクター名は画面上の訳文から原文を引いて訳し直すため、古い翻訳があるうちに原文へ戻す
  def self.restore_actor_names
    return unless $game_actors && $ls_actor_tr
    actors = $game_actors.instance_variable_get(:@data)
    return unless actors
    actors.each do |actor|
      next unless actor
      actor.name     = Langscore.fetch_original_text(actor.name, $ls_actor_tr)
      actor.nickname = Langscore.fetch_original_text(actor.nickname, $ls_actor_tr)
    end
  end

  def self.refresh_windows
    scene = SceneManager.scene
    return unless scene
    scene.instance_variables.each do |var|
      w = scene.instance_variable_get(var)
      next unless w.is_a?(Window_Base) && !w.disposed? && w.visible && w.open?
      next unless w.respond_to?(:refresh) && w.method(:refresh).arity == 0
      begin
        w.refresh
      rescue => e
        p "Langscore_RemoteHub: refresh failed #{w.class} #{e}"
      end
    end
  end

  #---------------------------------------------------------------
  # 表示中の文章の書き出し
  #---------------------------------------------------------------

  def self.translate_file_name(hash)
    return "Troops.csv" if hash.equal?($ls_troop_tr)
    return "CommonEvents.csv" if hash.equal?($ls_common_event)
    $ls_current_map.each { |map_id, h| 
      return sprintf("Map%03d.csv", map_id) if hash.equal?(h)
    }
    ""
  end

  #convert_escape_characters の中で、最初に原文が見つかった翻訳ファイルを控える
  def self.on_translate(text, hash, result)
    return unless @capture && @capture[:file].empty? && hash
    return unless hash.has_key?(text) || hash.has_key?(text.chomp("\r\n"))
    @capture[:file] = translate_file_name(hash)
    @capture[:translated] = result
  end

  def self.capture(window, text)
    @capture = { 
      :window => window.class.name, 
      :file => "", 
      :original => text.chomp("\n"), 
      :translated => "",
      :choice => window.is_a?(Window_ChoiceList), 
      :selected => false 
    }
    shown = @capture
    begin
      result = yield
    ensure
      @capture = nil
    end
    #選択肢は幅の計算でも呼ばれるため、同じ文章は1回だけ記録する
    unless @shown_texts.any? { |t| t[:window] == shown[:window] && t[:original] == shown[:original] }
      @shown_texts.push(shown)
      write_current
    end
    result
  end

  #カーソルの当たっている選択肢に印を付ける。original が nil なら外す
  def self.select_choice(original)
    changed = false
    @shown_texts.each do |t|
      selected = t[:choice] && t[:original] == original
      changed = true if t[:selected] != selected
      t[:selected] = selected
    end
    write_current if changed
  end

  def self.clear_shown_texts
    @shown_texts = []
    @focus_key = nil
  end

  #---------------------------------------------------------------
  # カーソルの当たっているデータベースの文章
  # データベースは言語を変えたときに訳文で書き換えているため、表示している訳文から原文を引く
  #---------------------------------------------------------------
  DATABASE_HASHES = [
    ["Actors.csv",  lambda { $ls_actor_tr }],
    ["Classes.csv", lambda { $ls_classes_tr }],
    ["Skills.csv",  lambda { $ls_skill_tr }],
    ["Items.csv",   lambda { $ls_item_tr }],
    ["Weapons.csv", lambda { $ls_weapons_tr }],
    ["Armors.csv",  lambda { $ls_armors_tr }],
    ["Enemies.csv", lambda { $ls_enemies_tr }],
    ["States.csv",  lambda { $ls_states_tr }],
    ["System.csv",  lambda { $ls_system_tr }],
    ["Scripts.csv", lambda { $ls_scripts_tr }],
  ]

  #訳文の原文と翻訳ファイルを探す。preferred の翻訳ファイルから先に探す
  def self.find_database_text(text, preferred)
    return nil if text.nil? || text.empty?

    first, rest = DATABASE_HASHES.partition { |file, _| file == preferred }
    (first + rest).each do |file, get_hash|
      hash = get_hash.call
      next unless hash.is_a?(Hash)
      return [file, text] if hash.has_key?(text)
      hash.each { |original, translates| 
        return [file, original] if translates.is_a?(Hash) && translates.values.include?(text) 
      }
    end

    nil
  end

  #ウィンドウのカーソルの当たっている項目の訳文と、探す翻訳ファイル
  def self.focused_text(w)
    index = w.index
    return nil if index < 0
    #アイテム・スキル・装備の一覧
    if w.respond_to?(:item) && (item = w.item) && item.respond_to?(:name)
      file = case item
             when RPG::Skill  then "Skills.csv"
             when RPG::Weapon then "Weapons.csv"
             when RPG::Armor  then "Armors.csv"
             else "Items.csv"
             end
      return [item.name, file]
    end
    if w.respond_to?(:enemy) && (enemy = w.enemy)
      return [enemy.original_name, "Enemies.csv"]
    end
    actor = case w
            when Window_MenuStatus then $game_party.members[index]
            when Window_BattleStatus then $game_party.battle_members[index]
            end
    return [actor.name, "Actors.csv"] if actor
    #メニューや戦闘のコマンド
    return [w.command_name(index), "System.csv"] if w.respond_to?(:command_name)
    nil
  end

  #カーソルの当たっている項目が変わったら書き出す。メッセージや選択肢より後に動いたものを優先する
  @focus_key = nil
  def self.update_focus(scene)
    found = nil
    scene.instance_variables.each do |var|
      w = scene.instance_variable_get(var)
      next unless w.is_a?(Window_Selectable) && !w.is_a?(Window_ChoiceList) && !w.disposed?
      next unless w.active && w.visible && w.open?
      found = (focused_text(w) rescue nil)
      if found
        found.push(w.class.name)
        break
      end
    end
    key = found ? found.join("\n") : nil
    return if key == @focus_key
    @focus_key = key

    @shown_texts.reject! { |t| t[:focus] }
    db = found ? find_database_text(found[0], found[1]) : nil
    if db
      @shown_texts.each { |t| t[:selected] = false }
      @shown_texts.push({ :window => found[2], :file => db[0], :original => db[1], :translated => found[0],
                          :choice => false, :selected => true, :focus => true })
    end
    write_current
  end

  JSON_ESCAPE = { "\\" => "\\\\", "\"" => "\\\"", "\n" => "\\n", "\r" => "\\r", "\t" => "\\t" }
  def self.json_string(s)
    s = s.to_s.dup.force_encoding("UTF-8")
    "\"" + s.gsub(/[\\"\x00-\x1f]/) { |c| JSON_ESCAPE[c] || sprintf("\\u%04x", c.ord) } + "\""
  end

  #アプリがゲームの起動中を確かめられるよう、プロセスIDを書いておく
  def self.write_pid
    Dir.mkdir(HUB_DIR) unless File.directory?(HUB_DIR)
    pid = Win32API.new('kernel32', 'GetCurrentProcessId', '', 'L').call
    File.open(HUB_DIR + "/pid", "wb") { |f| f.write(pid.to_s) }
  rescue => e
    p "Langscore_RemoteHub: failed to write pid #{e}"
  end

  def self.write_current
    texts = @shown_texts.map do |t|
      "{" + 
      [:window, :file, :original, :translated].map { |k| 
        json_string(k.to_s) + ":" + json_string(t[k]) 
      }.join(",") +
      ",\"choice\":#{t[:choice]},\"selected\":#{t[:selected]}}"
    end
    map_id = $game_map ? $game_map.map_id : 0
    json = "{\"language\":#{json_string($langscore_current_language)},\"mapId\":#{map_id},\"texts\":[#{texts.join(",")}]}"
    begin
      Dir.mkdir(HUB_DIR) unless File.directory?(HUB_DIR)
      File.open(HUB_DIR + "/current.json", "wb") { |f| f.write(json) }
    rescue => e
      p "Langscore_RemoteHub: failed to write current.json #{e}"
    end
  end
end

module Langscore
  class << self
    alias ls_remotehub_translate translate
  end
  def self.translate(text, langscore_hash, lang = $langscore_current_language)
    result = ls_remotehub_translate(text, langscore_hash, lang)
    LangscoreRemoteHub.on_translate(text, langscore_hash, result)
    result
  end
end

class Window_Base < Window
  alias ls_remotehub_convert_escape_characters convert_escape_characters
  def convert_escape_characters(text)
    is_message = is_a?(Window_Message) || is_a?(Window_ChoiceList) || is_a?(Window_ScrollText)
    return ls_remotehub_convert_escape_characters(text) if !is_message || text.nil? || text.empty?
    LangscoreRemoteHub.capture(self, text) { ls_remotehub_convert_escape_characters(text) }
  end
end

class Window_ChoiceList < Window_Command
  alias ls_remotehub_select select
  def select(index)
    ls_remotehub_select(index)
    LangscoreRemoteHub.select_choice(index && 0 <= index ? $game_message.choices[index] : nil)
  end
end

class Window_Message < Window_Base
  #新しいメッセージが始まったら、前の画面の文章は消す
  alias ls_remotehub_process_all_text process_all_text
  def process_all_text
    LangscoreRemoteHub.clear_shown_texts
    ls_remotehub_process_all_text
  end
end

class Scene_Base
  alias ls_remotehub_update update
  def update
    ls_remotehub_update
    LangscoreRemoteHub.update
    LangscoreRemoteHub.update_focus(self)
  end
end

LangscoreRemoteHub.write_pid

end # if $TEST
