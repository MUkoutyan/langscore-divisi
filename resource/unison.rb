#-----------------------------------------------------
module Langscore
  
  %{SUPPORT_LANGUAGE}%
  %{DEFAULT_LANGUAGE}%

  %{SUPPORT_FONTS}%

  $langscore_current_language = Langscore::DEFAULT_LANGUAGE
  $langscore_current_transrate_file = nil
 
end


#-----------------------------------------------------
String.class_eval <<-eval
  def lstrans line_info
    Langscore.translate_for_script(line_info)
  end
eval


class LSCSV

  def self.to_hash(file_name)
    file = open(file_name)
    return {} if file == nil

    rows = parse_col(parse_row(file))

    varidate(rows)

    #To Hash
    header = rows[0]
    row_index = (1...header.size).select do |i|
      Langscore::SUPPORT_LANGUAGE.include?(header[i])
    end
 
    #改行コードを全てRGSS側の\r\nに統一
    result = {}
    rows[1...rows.size].each do |r|
      origin = r[0].gsub("\n\n", "\r\n")
      trans  = r[1...header.size]

      transhash = {}
      row_index.each do |i|
        transhash[header[i]] = r[i].gsub("\n\n", "\r\n")
      end

      result[origin] = transhash
    end

    result
  end
  

  def self.to_array_without_origin(file_name)
    hash = to_hash(file_name)
    return hash.values
  end

  private 

  def self.varidate(rows)
    size = rows[0].size
    mismatch_cells = rows.select{ |r| r.size != size }
    if mismatch_cells.empty? == false
      p "Error! : Missmatch Num Cells : #{mismatch_cells.to_s}" 
      p "Header size : #{size}, Languages : #{rows[0]}"
      raise "Error! : Missmatch Num Cells : #{mismatch_cells.to_s}" 
    end
  end

  def self.open(file_name)
    begin
      trans_file = load_data(file_name)
    rescue
      begin
        trans_file = File.open(file_name)
      rescue
        p "Warning : Not Found Transcript File #{file_name}"
        return nil
      end
    end
    trans_file
  end

  def self.parse_row(file)
    return if file == nil
    csv_text = file.readlines()
    lines = []
    line_buffer = ""
    csv_text.each do |l|
      dq_count = l.count('\"');

      if line_buffer != ""
        dq_count += 1
      end

      if (dq_count%2) == 0
        line_buffer += l
        lines.push(line_buffer)
        line_buffer = ""
      else
        line_buffer += l
      end
    end

    return lines
  end

  def self.parse_col(rows)
    return if rows == nil
    result = []
    cols = []
    find_quote = false
    rows.each do |r|
      col = ""
      r.each_char do |c|

        if c=="\""
          find_quote = !find_quote
          next
        end

        if find_quote
          col += c
          next
        end

        if c==","
          find_quote = false
          cols.push(col)
          col = ""
          next
        elsif c=="\n"
          find_quote = false
          col = ""
          break
        end

        col += c
      end
      
      if col.empty? == false
        cols.push(col)
      end

      result.push(cols)
      cols = []

    end
    if cols.empty? == false
      result.push(cols)
    end

    result
  end
end


#-----------------------------------------------------

module Langscore
  def self.translate(text, langscore_hash, lang = $langscore_current_language)

    return text if langscore_hash == nil

    key = text 
    key = text.chop if text[text.size-1]=="\n" #メッセージの場合、改行していなくても末尾に必ず改行が含まれるっぽい

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

    p "Search : #{text}"

    transhash = langscore_array.select do |l|
      p l.values
      l.value?(text)
    end.first

    p "Transhash : #{transhash}"
    return text if transhash.nil? || transhash.empty?

    t = transhash[lang]
    if t != nil && t != ""
      text = t
    end
    text
  end

  def self.translate_for_script(text)
    result = self.translate(text, $data_langscore_scripts)
    if result == text && $DEBUG 
      return "!!!Langscore : Invalid!!!"
    end
    result
  end

  def self.changeLanguage(lang)
    $langscore_current_language = lang
    Langscore.Translate_Script_Text
    Langscore.updateFont(lang)
    Langscore.updateActor
  end

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

  def self.updateActor
    $actor_tr ||= LSCSV.to_array_without_origin("./Translate/Actors.lscsv")

    $game_actors = Game_Actors.new
    $data_actors = $data_actors.map do |actor|
      next if actor.nil?
      actor.name = Langscore.translate_array(actor.name, $actor_tr)
      actor.description = Langscore.translate_array(actor.description, $actor_tr)
      p "Name : #{actor.name}, Desc : #{actor.description}"
      return actor
    end
  end

end
#-----------------------------------------------------

class Game_Map
  
  alias ls_base_setup setup
  def setup(map_id)
    ls_base_setup(map_id)
    
    file_name  = sprintf("./Translate/Map%03d.lscsv", @map_id)

    $langscore_current_transrate_file = LSCSV.to_hash(file_name)
    # p $langscore_current_transrate_file
  end
end
  
class Window_Base < Window
  
  alias ls_base_convert_escape_characters convert_escape_characters
  def convert_escape_characters(text)
    if $langscore_current_transrate_file == nil
      ls_base_convert_escape_characters(text)
    end

    text = Langscore.translate(text, $langscore_current_transrate_file)

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
    $data_langscore_graphics ||= LSCSV.to_hash("Translate/Graphics.lscsv")
    p "Load Graphics.lscsv"# + $data_langscore_graphics.to_s
    $data_langscore_scripts ||= LSCSV.to_hash("Translate/Scripts.lscsv")
    p "Load Scripts.lscsv" if $data_langscore_scripts
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

class Scene_Language < Scene_MenuBase
  def start
    super
    create_help_window
    create_lsmain_window
  end

  def terminate
    super
    @command_window.dispose
    Langscore.changeLanguage($langscore_current_language)
  end

  def create_lsmain_window    
    @command_window.dispose unless @command_window.nil?
    @command_window = Window_Langscore.new
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

end
