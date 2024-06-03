#---------------------------------------------------------------
# 
# Langscore CoreScript "Unison" 
# Version 1.0.1
# Written by BreezeSinfonia 來奈津
# 
# 注意：このスクリプトは自動生成されました。編集は非推奨です。
#---------------------------------------------------------------
module Langscore

  %{SUPPORT_LANGUAGE}%
  %{DEFAULT_LANGUAGE}%

  %{SUPPORT_FONTS}%

  %{TRANSLATE_FOLDER}%

  $langscore_current_language = Langscore::DEFAULT_LANGUAGE
  #ハッシュそのものからハッシュを格納するハッシュに変更。
  #イベント中にマップ移動が行われた場合、翻訳内容がクリアされる問題への対応のため。
  $ls_current_map = {}
  $ls_graphic_cache = {}
 
end

#-----------------------------------------------------
String.class_eval <<-eval
  def lstrans(line_info, *args)
    text = Langscore.translate_for_script(line_info)
    return sprintf(text, *args)
  end
eval

%{UNISON_LSCSV}%

#-----------------------------------------------------

module Langscore
  def self.translate(text, langscore_hash, lang = $langscore_current_language)

    return text if langscore_hash == nil
    
    key = text

    if langscore_hash.has_key?(key)
      translist = langscore_hash[key]
      if translist.has_key?(lang)
        t = translist[lang]
        text = t unless t.empty?
      end
    end
    text
  end
  
  def self.translate_for_map(text)
    $ls_current_map.each_value do |trans|
      result = Langscore.translate(text, trans)
      if result != text
        return result
      end
    end
    return text
  end

  def self.translate_for_script(text)
    result = self.translate(text, $ls_scripts_tr)
    result
  end

  def self.fetch_original_text(transed_text, langscore_hash)

    origin = transed_text
    result = langscore_hash.each do |k,hash|
      if hash.values.include?(transed_text)
        origin = k
        break
      end
    end

    origin
  end
  
  def self.translate_list_reset
    return if $TEST == false
    $ls_actor_tr.clear
    $ls_system_tr.clear
    $ls_classes_tr.clear
    $ls_skill_tr.clear
    $ls_states_tr.clear
    $ls_weapons_tr.clear
    $ls_armors_tr.clear
    $ls_item_tr.clear
    $ls_enemies_tr.clear
    $ls_graphics_tr = LSCSV.to_hash("Graphics")
    $ls_scripts_tr = LSCSV.to_hash("Scripts")
    $ls_troop_tr ||= LSCSV.to_hash("Troops")
    $ls_common_event ||= LSCSV.to_hash("CommonEvents")

    changeLanguage($langscore_current_language)
  end

  def self.changeLanguage(lang)

    $langscore_current_language = lang
    Langscore.Translate_Script_Text
    Langscore.updateFont(lang)

    #データベースを初期化
    DataManager.ls_base_load_normal_database

    functions = [
      lambda{Langscore.updateSkills},
      lambda{Langscore.updateClasses},
      lambda{Langscore.updateStates},
      lambda{Langscore.updateEnemies},
      lambda{Langscore.updateItems},
      lambda{Langscore.updateArmors},
      lambda{Langscore.updateWeapons},
      lambda{Langscore.updateSystem},
    ]
    updateThreads = []
    functions.each do |f|
      updateThreads << Thread.new do
        f.call
      end
    end
    updateThreads.each do |t| t.join end

    #Game_Actors.newをしているため並列処理から除外
    Langscore.updateActor
    
    $ls_graphic_cache ||= {}
    $ls_graphic_cache.clear
    GC.start
  end

  private
  def self.updateFont(lang)

    beforeFontName = Font.default_name
    if LS_FONT[lang] == nil
      return beforeFontName != Font.default_name
    end

    Font.default_name = LS_FONT[lang][:name]
    Font.default_size = LS_FONT[lang][:size]
    
    if SceneManager.scene != nil
      SceneManager.scene.instance_variables.each do |varname|
        #instance_variablesだけだとself.instance_variablesになるので、
        #ちゃんとシーンを指定する。
        ivar = SceneManager.scene.instance_variable_get(varname)
        #シーンが持っているウィンドウ全てのフォントを更新
        #disposedをチェックしないと、解放済みの場合に死ぬ
        if ivar.is_a?(Window_Base) && ivar.disposed? == false
          ivar.contents.font.name = Font.default_name
          ivar.contents.font.size = Font.default_size
        end
      end
    end
    return beforeFontName != Font.default_name
  end
  
  def self.updateForNameAndDesc(data_list, tr_list)
    elm_trans = lambda do |el|
      el = Langscore.translate(el, tr_list)
    end
    data_list.each.with_index do |obj,i|
      next if data_list[i].nil?
      data_list[i].name        = elm_trans.call(obj.name)
      data_list[i].description = elm_trans.call(obj.description)
    end
  end
  
  def self.updateForName(data_list, tr_list)
    elm_trans = lambda do |el|
      el = Langscore.translate(el, tr_list)
    end
    data_list.each.with_index do |obj,i|
      next if data_list[i].nil?
      data_list[i].name        = elm_trans.call(obj.name)
    end
  end

  def self.updateActor
    $ls_actor_tr ||= LSCSV.to_hash("Actors")

    #大元のデータベースを更新。Game_Actor作成時に使用されるため必要。
    updateForNameAndDesc($data_actors, $ls_actor_tr)
    
    elm_trans = lambda do |el|
        el = Langscore.translate(el, $ls_actor_tr)
    end

    #既にGame_Actorが作成されている場合、インスタンス側も更新。
    #他のデータベースと同様に初期化を行うと、パラメータ値等も全部初期化されるので、名前以外の内容は保持する。
    actors = $game_actors.instance_variable_get(:@data)
    return unless actors
    actors.each do |actor|
      next unless actor #空アクターの場合はnil
      actor_id = actor.instance_variable_get(:@actor_id)
      name = Langscore.fetch_original_text(actor.name, $ls_actor_tr)
      nickname = Langscore.fetch_original_text(actor.nickname, $ls_actor_tr)
      $game_actors[actor_id].name     = elm_trans.call(name)
      $game_actors[actor_id].nickname = elm_trans.call(nickname)
    end
  end

  def self.updateSystem
    $ls_system_tr ||= LSCSV.to_hash("System")

    elm_trans = lambda do |el| 
      el = Langscore.translate(el, $ls_system_tr)
    end
    $data_system.elements.map! &elm_trans
    $data_system.skill_types.map! &elm_trans
    $data_system.terms.params.map! &elm_trans
    $data_system.terms.etypes.map! &elm_trans
    $data_system.terms.commands.map! &elm_trans
    $data_system.terms.basic.map! &elm_trans

    $data_system.currency_unit = Langscore.translate($data_system.currency_unit, $ls_system_tr)
  end

  def self.updateClasses
    $ls_classes_tr ||= LSCSV.to_hash("Classes")
    updateForName($data_classes, $ls_classes_tr)
  end

  def self.updateSkills
    $ls_skill_tr ||= LSCSV.to_hash("Skills")

    elm_trans = lambda do |el|
      el = Langscore.translate(el, $ls_skill_tr)
    end
    $data_skills.each.with_index do |skill,i|
      next if $data_skills[i].nil?
      $data_skills[i].name        = elm_trans.call(skill.name)
      $data_skills[i].description = elm_trans.call(skill.description)
      $data_skills[i].message1    = elm_trans.call(skill.message1)
      $data_skills[i].message2    = elm_trans.call(skill.message2)
    end
  end
  
  def self.updateStates
    $ls_states_tr ||= LSCSV.to_hash("States")
    
    elm_trans = lambda do |el|
      el = Langscore.translate(el, $ls_states_tr)
    end
    $data_states.each.with_index do |skill,i|
      next if $data_states[i].nil?
      $data_states[i].name        = elm_trans.call(skill.name)
      $data_states[i].message1    = elm_trans.call(skill.message1)
      $data_states[i].message2    = elm_trans.call(skill.message2)
      $data_states[i].message3    = elm_trans.call(skill.message3)
      $data_states[i].message4    = elm_trans.call(skill.message4)
    end
  end

  def self.updateWeapons
    $ls_weapons_tr ||= LSCSV.to_hash("Weapons")
    updateForNameAndDesc($data_weapons, $ls_weapons_tr)
  end

  def self.updateArmors
    $ls_armors_tr ||= LSCSV.to_hash("Armors")
    updateForNameAndDesc($data_armors, $ls_armors_tr)
  end
  
  def self.updateItems
    $ls_item_tr ||= LSCSV.to_hash("Items")
    updateForNameAndDesc($data_items, $ls_item_tr)
  end

  def self.updateEnemies
    $ls_enemies_tr ||= LSCSV.to_hash("Enemies")
    updateForName($data_enemies, $ls_enemies_tr)
  end

end


#-----------------------------------------------------

class Game_Map

  def Game_Map.ls_finalize
    proc {
      $ls_current_map.delete(@map_id)
    }
  end
  
  alias ls_base_setup setup
  def setup(map_id)
    ls_base_setup(map_id)
    load_langscore_file(map_id)
  end

  def load_langscore_file(map_id)
    file_name  = sprintf("Map%03d", @map_id)

    return if $ls_current_map.include?(@map_id)
    $ls_current_map[@map_id] = LSCSV.to_hash(file_name)
    #メモリ節約のためファイナライズで翻訳内容を消す。GC頼りなのでおまじない程度の気持ち。
    #会話中に何故かクラッシュした場合は真っ先に外す。
    ObjectSpace.define_finalizer(self, Game_Map.ls_finalize)
  end
end

class Game_Interpreter
  #アクター名の変更
  alias ls_command_320 command_320
  def command_320
    ls_command_320

    Langscore.updateActor
  end

  #二つ名の変更
  alias ls_command_324 command_324
  def command_324
    ls_command_324
    Langscore.updateActor
  end
end

class Window_Base < Window
  
  alias ls_base_convert_escape_characters convert_escape_characters
  def convert_escape_characters(text)

    if text.empty?
      return ls_base_convert_escape_characters(text)
    end
    
    key = text.chomp("\n");
    
    #マップ・バトル・コモンイベントかを判別できないので苦肉の策
    result_text = [text, text, text]
    updateThreads = []
    if $ls_current_map
      updateThreads << Thread.new do
        result_text[0] = Langscore.translate_for_map(key)
      end
    end
    updateThreads << Thread.new do
      result_text[1] = Langscore.translate(key, $ls_troop_tr)
    end
    updateThreads << Thread.new do
      result_text[2] = Langscore.translate(key, $ls_common_event)
    end

    updateThreads.each { |t| t.join }

    result = result_text.select do |t| 
      t != text
    end

    if result.empty? 
      return ls_base_convert_escape_characters(text)
    end
    
    #Game_Message.all_textと同様に改行を付与する。
    ls_base_convert_escape_characters(result[0] + "\n")
  end

end


#モジュールの上書き
#シーン遷移に関わらない翻訳ファイルは初期化時に読み込み
DataManager::module_eval <<-eval
  class << DataManager
    alias ls_base_load_normal_database load_normal_database
    alias ls_make_save_contents make_save_contents
    alias ls_extract_save_contents extract_save_contents
    alias ls_reload_map_if_updated reload_map_if_updated
  end

  #戦闘テスト用は未対応
  def self.load_normal_database
    ls_base_load_normal_database
    
    updateThreads = []
    
    updateThreads << Thread.new do
      $ls_graphics_tr ||= LSCSV.to_hash("Graphics")
    end

    updateThreads << Thread.new do
      $ls_scripts_tr ||= LSCSV.to_hash("Scripts")
    end
    
    updateThreads << Thread.new do
      $ls_troop_tr ||= LSCSV.to_hash("Troops")
    end
    
    updateThreads << Thread.new do
      $ls_common_event ||= LSCSV.to_hash("CommonEvents")
    end

    updateThreads.each { |t| t.join }

    Langscore.changeLanguage($langscore_current_language)
  end

  #セーブを行う際は原文で保存
  #プラグインを外した際に変に翻訳文が残ることを避ける。
  def self.make_save_contents

    data_temp = Marshal.dump($game_actors)

    actors = $game_actors.instance_variable_get(:@data)
    actors.each.with_index do |actor,i|
      next unless actor #空アクターの場合はnil
      actor_id = actor.instance_variable_get(:@actor_id)
      $game_actors[actor_id].name     = Langscore.fetch_original_text(actor.name, $ls_actor_tr)
      $game_actors[actor_id].nickname = Langscore.fetch_original_text(actor.nickname, $ls_actor_tr)
    end

    #セーブ本処理
    result = ls_make_save_contents

    $game_actors = Marshal.load(data_temp)

    result
  end

  #セーブデータは原文で保存されているため、起動時の言語設定で置き換える。
  #これを省くと中国語で起動した際に、再度翻訳を適用するまで日本語のまま……といった事が起きる。
  def self.extract_save_contents(contents)
    ls_extract_save_contents(contents)

    Langscore.changeLanguage($langscore_current_language)
  end

  #セーブデータロード時のマップ更新はversion_idに依存しているが、
  #version_idの変更タイミングが謎なのでMapが更新されたりされなかったりするように見える。
  #Langscoreの場合$game_map.setupが行われないと翻訳文が読み込まれないため、
  #reload_map_if_updatedの分岐に関わらずロードするようにする。
  def self.reload_map_if_updated
    ls_reload_map_if_updated
    $game_map.load_langscore_file($game_map.map_id)
  end

eval

Cache::module_eval <<-eval

  class << Cache
    alias ls_base_load_bitmap load_bitmap
  end
  
  def self.load_bitmap(folder_name, filename, hue = 0)
    
    path = folder_name+filename
    ts_path = Langscore.translate(path, $ls_graphics_tr)
    if ts_path != path
      filename = ts_path.sub(folder_name, "")
    else
      #翻訳テキスト内で明示的に画像が指定されていない場合、ファイル名検索
      #filenameは元から拡張子なしなのでそのまま結合
      new_filename = filename + '_' + $langscore_current_language
      
      has_key = $ls_graphic_cache.has_key?(filename)
      if has_key == false
        begin
          bitmap = ls_base_load_bitmap(folder_name, new_filename, hue)
          $ls_graphic_cache[filename] = true
          return bitmap
        rescue => e
          $ls_graphic_cache[filename] = false
        end
      end

      if $ls_graphic_cache[filename]
        filename = new_filename
      end
    end

    ls_base_load_bitmap(folder_name, filename, hue)
  end
  #ロード時の変換処理も必須
eval

module Langscore

  %{SYSTEM1}%
  %{SYSTEM2}%

module UI
class Window_Langscore < Window_Command

  attr_accessor :recreate

  def initialize
    super(0,0)
    self.x = (Graphics.width - self.window_width ) / 2
    self.y = (Graphics.height - self.window_height) / 2
    @_recreate = false
    
    SUPPORT_LANGUAGE.each.with_index do |l, i|
      select(i) if $langscore_current_language == l
    end

    open
  end

  def window_width
    return 210
  end

  def make_command_list
    SUPPORT_LANGUAGE.each.with_index do |l, i|
      add_command(SYSTEM2[l], l.to_sym)
    end
  end

  def cursor_up(wrap = false)
    super wrap
    call_handler(:ls_update)
  end

  def cursor_down(wrap = false)
    super wrap
    call_handler(:ls_update)
  end

end


class Window_Langscore_OKCancel < Window_HorzCommand
  def initialize(y)

    @gray_color = $game_system.window_tone
    @gray_color.gray = 255
    @def_color = $game_system.window_tone

    super((Graphics.width - self.window_width ) / 2, y)
    select(1)

    deactivate
    open
  end

  def make_command_list
    add_command("OK",       :ok)
    add_command("Reselect", :select)
    add_command("Cancel",   :cancel)
  end
  
  def window_width
    return 380
  end

  def col_max
    return 3
  end

  def activate
    self.tone.set(@def_color)
    super
  end

  def deactivate
    self.tone.set(@gray_color)
    super
  end

end

class Scene_Language < Scene_MenuBase
  def start
    super
    @before_lang = $langscore_current_language
    create_help_window
    create_lsmain_window
    create_ok_cancel_window

    @okcancel_window.deactivate

  end

  def terminate
    super
    @command_window.dispose
    Langscore.changeLanguage($langscore_current_language)
    LSSetting.save()
  end
  

  def create_lsmain_window    
    @command_window.dispose unless @command_window.nil?
    @command_window = Window_Langscore.new
    @command_window.viewport = @viewport

    @command_window.set_handler(:ok, method(:to_okcancel))
    @command_window.set_handler(:ls_update, method(:select_lang))
    @command_window.set_handler(:cancel, method(:return_scene))
    @help_window.set_text(Langscore.translate(SYSTEM1, $langscore_system))
  end

  def update_basic
    super
    if @command_window != nil && @command_window.recreate
      @help_window.dispose unless @help_window.nil?
      create_help_window
      create_lsmain_window
    end
  end

  def select_lang
    return if @command_window.index == -1
    $langscore_current_language = SUPPORT_LANGUAGE[@command_window.index]
    @help_window.set_text(Langscore.translate(SYSTEM1, $langscore_system))
    if Langscore.updateFont($langscore_current_language)
      @command_window.recreate = true
    end
  end
  
  def to_okcancel
    @command_window.deactivate
    @okcancel_window.activate
  end

  def create_ok_cancel_window
    @okcancel_window.dispose unless @okcancel_window.nil?
    y_pos = @command_window.y + @command_window.height + 8
    @okcancel_window = Window_Langscore_OKCancel.new(y_pos)
    @okcancel_window.viewport = @viewport

    @okcancel_window.set_handler(:select, method(:reselect))
    @okcancel_window.set_handler(:ok, method(:accept))
    @okcancel_window.set_handler(:cancel, method(:reject))
  end
  
  def reselect
    @command_window.activate
    @okcancel_window.deactivate
  end

  def accept
    self.return_scene
  end

  def reject
    $langscore_current_language = @before_lang
    self.return_scene
  end
end

end #module UI


class LSSetting
  GetPrivateProfileString = Win32API.new('kernel32', 'GetPrivateProfileString', %w(p p p p l p), 'L')
  WritePrivateProfileString = Win32API.new('kernel32', 'WritePrivateProfileString', %w(p p p p), 'i')

  BufferSize = 16+1

  def self.load
    result = ' ' * BufferSize
    r = GetPrivateProfileString.call("Langscore", "Lang", "ja", result, BufferSize, "./Game.ini")
    if r == BufferSize-2
      $langscore_current_language = "ja"
    else
      $langscore_current_language = result.slice(0...r)
      p "Langscore Load ini : #{$langscore_current_language}"
    end
  end

  def self.save
    WritePrivateProfileString.call("Langscore", "Lang", $langscore_current_language, "./Game.ini")
  end
end

LSSetting.load()

def self.saveIni
  LSSetting.save()
end

def self.loadIni
  LSSetting.load()
end

end # module Langscore
