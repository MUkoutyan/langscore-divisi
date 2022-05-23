#-----------------------------------------------------
module Langscore
  
  %{SUPPORT_LANGUAGE}%
  %{DEFAULT_LANGUAGE}%

  %{SUPPORT_FONTS}%

  %{TRANSLATE_FOLDER}%

  $langscore_current_language = Langscore::DEFAULT_LANGUAGE
  $ls_current_map = nil
 
end

#-----------------------------------------------------
String.class_eval <<-eval
  def lstrans line_info
    Langscore.translate_for_script(line_info)
  end
eval

%{UNISON_LSCSV}%

#-----------------------------------------------------

module Langscore
  def self.translate(text, langscore_hash, lang = $langscore_current_language)

    return text if langscore_hash == nil

    key = text 

    translist = langscore_hash[key]
    if translist != nil
      t = translist[lang]
      if t != nil && t != ""
        text = t
      end
    end
    text
  end

  def self.translate_array(text, langscore_array, lang = $langscore_current_language)

    return text if langscore_array == nil

    transhash = langscore_array.select do |l|
      l.value?(text)
    end.first

    return text if transhash.nil? || transhash.empty?

    t = transhash[lang]
    if t != nil && t != ""
      text = t
    end
    text
  end

  def self.translate_for_script(text)
    result = self.translate(text, $data_langscore_scripts)
    result
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
    $data_langscore_graphics = LSCSV.to_hash("Graphics.lscsv")
    $data_langscore_scripts = LSCSV.to_hash("Scripts.lscsv")

    changeLanguage($langscore_current_language)
  end

  def self.changeLanguage(lang)

    $langscore_current_language = lang
    Langscore.Translate_Script_Text
    Langscore.updateFont(lang)

    functions = [
      lambda{Langscore.updateSkills},
      lambda{Langscore.updateClasses},
      lambda{Langscore.updateStates},
      lambda{Langscore.updateEnemies},
      lambda{Langscore.updateItems},
      lambda{Langscore.updateArmors},
      lambda{Langscore.updateWeapons},
      lambda{Langscore.updateSystem}
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
  end

  private
  def self.updateFont(lang)

    beforeFontName = Font.default_name
    if LS_FONT[lang] != nil
      Font.default_name = LS_FONT[lang][:name]
      Font.default_size = LS_FONT[lang][:size]
    else
      Font.default_name = LS_FONT["default"][:name]
      Font.default_size = LS_FONT["default"][:size]
    end
    return beforeFontName != Font.default_name
  end
  
  def self.updateForNameAndDesc(data_list, tr_list)
    elm_trans = lambda do |el|
      el = Langscore.translate_array(el, tr_list)
    end
    data_list.each.with_index do |obj,i|
      next if data_list[i].nil?
      data_list[i].name        = elm_trans.call(obj.name)
      data_list[i].description = elm_trans.call(obj.description)
    end
  end
  
  def self.updateForName(data_list, tr_list)
    elm_trans = lambda do |el|
      el = Langscore.translate_array(el, tr_list)
    end
    data_list.each.with_index do |obj,i|
      next if data_list[i].nil?
      data_list[i].name        = elm_trans.call(obj.name)
    end
  end

  def self.updateActor
    $ls_actor_tr ||= LSCSV.to_array_without_origin("Actors.lscsv")
    $game_actors = Game_Actors.new  #アクター情報をクリアして次回取得時に翻訳されたテキストが入るようにする
    updateForNameAndDesc($data_actors, $ls_actor_tr)
  end

  def self.updateSystem
    $ls_system_tr ||= LSCSV.to_array_without_origin("System.lscsv")

    elm_trans = lambda do |el| 
      el = Langscore.translate_array(el, $ls_system_tr)
    end
    $data_system.elements.map! &elm_trans
    $data_system.skill_types.map! &elm_trans
    $data_system.terms.params.map! &elm_trans
    $data_system.terms.etypes.map! &elm_trans
    $data_system.terms.commands.map! &elm_trans

    $data_system.currency_unit = Langscore.translate_array($data_system.currency_unit, $system_tr)
  end

  def self.updateClasses
    $ls_classes_tr ||= LSCSV.to_array_without_origin("Classes.lscsv")
    updateForName($data_classes, $ls_classes_tr)
  end

  def self.updateSkills
    $ls_skill_tr ||= LSCSV.to_array_without_origin("Skills.lscsv")

    elm_trans = lambda do |el|
      el = Langscore.translate_array(el, $ls_skill_tr)
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
    $ls_states_tr ||= LSCSV.to_array_without_origin("States.lscsv")
    
    elm_trans = lambda do |el|
      el = Langscore.translate_array(el, $ls_states_tr)
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
    $ls_weapons_tr ||= LSCSV.to_array_without_origin("Weapons.lscsv")
    updateForNameAndDesc($data_weapons, $ls_weapons_tr)
  end

  def self.updateArmors
    $ls_armors_tr ||= LSCSV.to_array_without_origin("Armors.lscsv")
    updateForNameAndDesc($data_armors, $ls_armors_tr)
  end
  
  def self.updateItems
    $ls_item_tr ||= LSCSV.to_array_without_origin("Items.lscsv")
    updateForNameAndDesc($data_items, $ls_item_tr)
  end

  def self.updateEnemies
    $ls_enemies_tr ||= LSCSV.to_array_without_origin("Enemies.lscsv")
    updateForName($data_enemies, $ls_enemies_tr)
  end

end

#-----------------------------------------------------

class Game_Map
  
  alias ls_base_setup setup
  def setup(map_id)
    ls_base_setup(map_id)
    
    file_name  = sprintf("Map%03d.lscsv", @map_id)

    $ls_current_map = LSCSV.to_hash(file_name)
    # p $ls_current_map
  end
end
  
class Window_Base < Window
  
  alias ls_base_convert_escape_characters convert_escape_characters
  def convert_escape_characters(text)
    if $ls_current_map == nil
      ls_base_convert_escape_characters(text)
    end

    text = Langscore.translate(text, $ls_current_map)

    ls_base_convert_escape_characters(text)
  end

  alias ls_base_draw_text draw_text
  def draw_text(*args)
    ls_base_draw_text(*args)
  end
end


#モジュールの上書き
#シーン遷移に関わらない翻訳ファイルは初期化時に読み込み
DataManager::module_eval <<-eval
  class << DataManager
    alias ls_base_load_normal_database load_normal_database
  end

  #戦闘テスト用は未対応
  def self.load_normal_database
    ls_base_load_normal_database
    $data_langscore_graphics ||= LSCSV.to_hash("Graphics.lscsv")
    p "Load Graphics.lscsv"# + $data_langscore_graphics.to_s
    $data_langscore_scripts ||= LSCSV.to_hash("Scripts.lscsv")
    p "Load Scripts.lscsv" if $data_langscore_scripts

    Langscore.changeLanguage($langscore_current_language)
  end

eval

Cache::module_eval <<-eval

  class << Cache
    alias ls_base_load_bitmap load_bitmap
  end
  
  def self.load_bitmap(folder_name, filename, hue = 0)
    path = folder_name + filename
    ts_path = Langscore.translate(path, $data_langscore_graphics)
    if ts_path != path
      filename = ts_path
    end

    ls_base_load_bitmap(folder_name, filename, hue)
  end
eval

module Langscore

  %{SYSTEM1}%
  %{SYSTEM2}%


class Window_Langscore < Window_Command

  attr_accessor :recreate

  def initialize lang
    @mark_lang = lang
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
      mark = (l == @mark_lang) ? "* " : ""
      add_command(mark + SYSTEM2[l], l.to_sym)
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

class Scene_Language < Scene_MenuBase
  def start
    super
    @before_lang = $langscore_current_language
    create_help_window
    create_lsmain_window
  end

  def terminate
    super
    @command_window.dispose
    if @before_lang != $langscore_current_language
      Langscore.changeLanguage($langscore_current_language)
      LSSetting.save()
    end
  end

  def create_lsmain_window    
    @command_window.dispose unless @command_window.nil?
    @command_window = Window_Langscore.new(@before_lang)
    @command_window.viewport = @viewport

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
end


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


end # module Langscore
