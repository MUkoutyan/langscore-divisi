#-----------------------------------------------------
module Langscore
  
  %{SUPPORT_LANGUAGE}%
  %{DEFAULT_LANGUAGE}%
  
  $langscore_current_language = Langscore::DEFAULT_LANGUAGE
  $langscore_current_transrate_file = nil

  def self.translate(text, langscore_hash)

    text if langscore_hash == nil

    key = text 
    key = text.chop if text[text.size-1]=="\n" #メッセージの場合、改行していなくても末尾に必ず改行が含まれるっぽい

    translist = langscore_hash[key]
    if translist != nil
      t = translist[$langscore_current_language]
      if t != nil && t != ""
        text = t
      end
    end
    text
  end

  def self.translate_for_script(text)
    self.translate(text, $data_langscore_scripts)
  end

  def self.changeLanguage(lang)
    $langscore_current_language = lang
    Langscore.Translate_Script_Text
  end
  
end


#-----------------------------------------------------
String.class_eval <<-eval
  def lstrans
    text = Langscore.translate(self, $data_langscore_scripts)
    # p text
    text 
    # caller_line = caller.first
    # if /^(.+?):(\d+)?/ =~ caller_line 
    #   p caller_line
    #   puts "#{$1}, #{$2.to_i}" 
    # end 
  end
eval

class LSCSV

  def self.to_hash(file_name)
    file = open(file_name)
    return {} if file == nil

    rows = parse_col(parse_row(file))

    #Varidate
    size = rows[0].size
    mismatch_cells = rows.select{ |r| r.size != size }
    if mismatch_cells.empty? == false
      raise "Error! : Missmatch Num Cells : #{file_name},#{mismatch_cells.to_s}" 
    end

    #To Hash
    header = rows[0]
    langs = header[1...header.size-1]

    result = {}
    rows[1...rows.size].each do |r|
      origin = r[0]
      trans  = r[1...header.size]

      transhash = {}
      langs.each.with_index do |l, i|
        transhash[l] = trans[i]
      end

      p transhash

      result[origin] = transhash
    end

    result
  end

  private 
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
          cols.push(col)
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

class Game_Map
  
  alias ls_base_setup setup
  def setup(map_id)
    ls_base_setup(map_id)
    
    file_name  = sprintf("./Data/Map%03d.lscsv", @map_id)

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
    $data_langscore_graphics ||= LSCSV.to_hash("Data/Graphics.lscsv")
    p "Load Graphics.lscsv"# + $data_langscore_graphics.to_s
    $data_langscore_scripts ||= LSCSV.to_hash("Data/Scripts.lscsv")
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
