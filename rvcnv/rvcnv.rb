# encoding: UTF-8
Encoding.default_external = 'UTF-8'

require 'zlib'
require 'csv'
require 'optparse'
require 'json'
require_relative '../resource/lscsv.rb'


module RPG
  module ToJson
    def to_json(*a)
      result = {'class' => self.class.name}
      self.instance_variables.each do |var|
        result[var] = self.instance_variable_get var
      end
      result.to_json(*a)
    end
  end
end

class Table
  include RPG::ToJson
  attr_accessor :xsize 
  attr_accessor :ysize 
  attr_accessor :zsize 

  def initialize(d)
  end
  
  def self._load(obj)
    Table.new(obj.unpack("VVVVVv*"))
  end
end

class Tone  
  include RPG::ToJson
  def initialize(d)
  end

  attr_accessor :ref 
  attr_accessor :green 
  attr_accessor :blue 
  attr_accessor :gray 

  def self._load(obj)
    Tone.new(obj.unpack("ssss"))
  end
end

class Color  
  include RPG::ToJson
  def initialize(d)
  end

  attr_accessor :ref 
  attr_accessor :green 
  attr_accessor :blue 
  attr_accessor :alpha 

  def self._load(obj)
    Color.new(obj.unpack("ssss"))
  end
end


class RPG::Event
  attr_accessor :id
  attr_accessor :name
  attr_accessor :x
  attr_accessor :y
  attr_accessor :pages
end

class RPG::Event::Page
  attr_accessor :condition
  attr_accessor :graphic
  attr_accessor :move_type
  attr_accessor :move_speed
  attr_accessor :move_frequency
  attr_accessor :move_route
  attr_accessor :walk_anime
  attr_accessor :step_anime
  attr_accessor :direction_fix
  attr_accessor :through
  attr_accessor :priority_type
  attr_accessor :trigger
  attr_accessor :list
end

class RPG::Event::Page::Condition
  attr_accessor :switch1_valid
  attr_accessor :switch2_valid
  attr_accessor :variable_valid
  attr_accessor :self_switch_valid
  attr_accessor :item_valid
  attr_accessor :actor_valid
  attr_accessor :switch1_id
  attr_accessor :switch2_id
  attr_accessor :variable_id
  attr_accessor :variable_value
  attr_accessor :self_switch_ch
  attr_accessor :item_id
  attr_accessor :actor_id
end

class RPG::Event::Page::Graphic
  attr_accessor :tile_id
  attr_accessor :character_name
  attr_accessor :character_index
  attr_accessor :direction
  attr_accessor :pattern
end

class RPG::CommonEvent
  attr_accessor :id
  attr_accessor :name
  attr_accessor :trigger
  attr_accessor :switch_id
  attr_accessor :list
end

class RPG::EventCommand
  attr_accessor :code
  attr_accessor :indent
  attr_accessor :parameters
end

class RPG::MoveRoute
  attr_accessor :repeat
  attr_accessor :skippable
  attr_accessor :wait
  attr_accessor :list
end

class RPG::MoveCommand
  attr_accessor :code
  attr_accessor :parameters
end

class RPG::BaseItem
  attr_accessor :id
  attr_accessor :name
  attr_accessor :icon_index
  attr_accessor :description
  attr_accessor :features
  attr_accessor :note
end

class RPG::Class < RPG::BaseItem
  attr_accessor :exp_params
  attr_accessor :params
  attr_accessor :learnings
end

class RPG::Class::Learning
  attr_accessor :level
  attr_accessor :skill_id
  attr_accessor :note
end

class RPG::Actor < RPG::BaseItem
  attr_accessor :nickname
  attr_accessor :class_id
  attr_accessor :initial_level
  attr_accessor :max_level
  attr_accessor :character_name
  attr_accessor :character_index
  attr_accessor :face_name
  attr_accessor :face_index
  attr_accessor :equips
end

class RPG::UsableItem < RPG::BaseItem

  attr_accessor :scope
  attr_accessor :occasion
  attr_accessor :speed
  attr_accessor :animation_id
  attr_accessor :success_rate
  attr_accessor :repeats
  attr_accessor :tp_gain
  attr_accessor :hit_type
  attr_accessor :damage
  attr_accessor :effects
end

class RPG::Skill < RPG::UsableItem
  attr_accessor :stype_id
  attr_accessor :mp_cost
  attr_accessor :tp_cost
  attr_accessor :message1
  attr_accessor :message2
  attr_accessor :required_wtype_id1
  attr_accessor :required_wtype_id2
end

class RPG::Item < RPG::UsableItem
  attr_accessor :itype_id
  attr_accessor :price
  attr_accessor :consumable
end

class RPG::UsableItem::Damage
  attr_accessor :type
  attr_accessor :element_id
  attr_accessor :formula
  attr_accessor :variance
  attr_accessor :critical
end

class RPG::UsableItem::Effect
  attr_accessor :code
  attr_accessor :data_id
  attr_accessor :value1
  attr_accessor :value2
end

class RPG::EquipItem < RPG::BaseItem
  attr_accessor :price
  attr_accessor :etype_id
  attr_accessor :params
end

class RPG::Weapon < RPG::EquipItem
  attr_accessor :wtype_id
  attr_accessor :animation_id
end

class RPG::Armor < RPG::EquipItem
  attr_accessor :atype_id
end

class RPG::Enemy < RPG::BaseItem
  attr_accessor :battler_name
  attr_accessor :battler_hue
  attr_accessor :params
  attr_accessor :exp
  attr_accessor :gold
  attr_accessor :drop_items
  attr_accessor :actions
end

class RPG::Enemy::DropItem
  attr_accessor :kind
  attr_accessor :data_id
  attr_accessor :denominator
end

class RPG::Enemy::Action
  attr_accessor :skill_id
  attr_accessor :condition_type
  attr_accessor :condition_param1
  attr_accessor :condition_param2
  attr_accessor :rating
end

class RPG::State < RPG::BaseItem
  attr_accessor :restriction
  attr_accessor :priority
  attr_accessor :remove_at_battle_end
  attr_accessor :remove_by_restriction
  attr_accessor :auto_removal_timing
  attr_accessor :min_turns
  attr_accessor :max_turns
  attr_accessor :remove_by_damage
  attr_accessor :chance_by_damage
  attr_accessor :remove_by_walking
  attr_accessor :steps_to_remove
  attr_accessor :message1
  attr_accessor :message2
  attr_accessor :message3
  attr_accessor :message4
end

class RPG::BaseItem::Feature
  attr_accessor :code
  attr_accessor :data_id
  attr_accessor :value
end

class RPG::MapInfo
  attr_accessor :name
  attr_accessor :parent_id
  attr_accessor :order
  attr_accessor :expanded
  attr_accessor :scroll_x
  attr_accessor :scroll_y
end

class RPG::Map
  attr_accessor :display_name
  attr_accessor :tileset_id
  attr_accessor :width
  attr_accessor :height
  attr_accessor :scroll_type
  attr_accessor :specify_battleback
  attr_accessor :battleback1_name
  attr_accessor :battleback2_name
  attr_accessor :autoplay_bgm
  attr_accessor :bgm
  attr_accessor :autoplay_bgs
  attr_accessor :bgs
  attr_accessor :disable_dashing
  attr_accessor :encounter_list
  attr_accessor :encounter_step
  attr_accessor :parallax_name
  attr_accessor :parallax_loop_x
  attr_accessor :parallax_loop_y
  attr_accessor :parallax_sx
  attr_accessor :parallax_sy
  attr_accessor :parallax_show
  attr_accessor :note
  attr_accessor :data
  attr_accessor :events
end

class RPG::Map::Encounter
  attr_accessor :troop_id
  attr_accessor :weight
  attr_accessor :region_set
end


class RPG::Troop
  attr_accessor :id
  attr_accessor :name
  attr_accessor :members
  attr_accessor :pages
end

class RPG::Troop::Member
  attr_accessor :enemy_id
  attr_accessor :x
  attr_accessor :y
  attr_accessor :hidden
end

class RPG::Troop::Page
  attr_accessor :condition
  attr_accessor :span
  attr_accessor :list
end

class RPG::Troop::Page::Condition
  attr_accessor :turn_ending
  attr_accessor :turn_valid
  attr_accessor :enemy_valid
  attr_accessor :actor_valid
  attr_accessor :switch_valid
  attr_accessor :turn_a
  attr_accessor :turn_b
  attr_accessor :enemy_index
  attr_accessor :enemy_hp
  attr_accessor :actor_id
  attr_accessor :actor_hp
  attr_accessor :switch_id
end

class RPG::Animation
  attr_accessor :id
  attr_accessor :name
  attr_accessor :animation1_name
  attr_accessor :animation1_hue
  attr_accessor :animation2_name
  attr_accessor :animation2_hue
  attr_accessor :position
  attr_accessor :frame_max
  attr_accessor :frames
  attr_accessor :timings
end

class RPG::Animation::Frame
  attr_accessor :cell_max
  attr_accessor :cell_data
end

class RPG::Animation::Timing
  attr_accessor :frame
  attr_accessor :se
  attr_accessor :flash_scope
  attr_accessor :flash_color
  attr_accessor :flash_duration
end

class RPG::Tileset
  attr_accessor :id
  attr_accessor :mode
  attr_accessor :name
  attr_accessor :tileset_names
  attr_accessor :flags
  attr_accessor :note
end


class RPG::System
  attr_accessor :game_title
  attr_accessor :version_id
  attr_accessor :japanese
  attr_accessor :party_members
  attr_accessor :currency_unit
  attr_accessor :skill_types
  attr_accessor :weapon_types
  attr_accessor :armor_types
  attr_accessor :elements
  attr_accessor :switches
  attr_accessor :variables
  attr_accessor :boat
  attr_accessor :ship
  attr_accessor :airship
  attr_accessor :title1_name
  attr_accessor :title2_name
  attr_accessor :opt_draw_title
  attr_accessor :opt_use_midi
  attr_accessor :opt_transparent
  attr_accessor :opt_followers
  attr_accessor :opt_slip_death
  attr_accessor :opt_floor_death
  attr_accessor :opt_display_tp
  attr_accessor :opt_extra_exp
  attr_accessor :window_tone
  attr_accessor :title_bgm
  attr_accessor :battle_bgm
  attr_accessor :battle_end_me
  attr_accessor :gameover_me
  attr_accessor :sounds
  attr_accessor :test_battlers
  attr_accessor :test_troop_id
  attr_accessor :start_map_id
  attr_accessor :start_x
  attr_accessor :start_y
  attr_accessor :terms
  attr_accessor :battleback1_name
  attr_accessor :battleback2_name
  attr_accessor :battler_name
  attr_accessor :battler_hue
  attr_accessor :edit_map_id
end

class RPG::System::Vehicle
  attr_accessor :character_name
  attr_accessor :character_index
  attr_accessor :bgm
  attr_accessor :start_map_id
  attr_accessor :start_x
  attr_accessor :start_y
end

class RPG::System::Terms
  attr_accessor :basic
  attr_accessor :params
  attr_accessor :etypes
  attr_accessor :commands
end

class RPG::System::TestBattler
  attr_accessor :actor_id
  attr_accessor :level
  attr_accessor :equips
end



class RPG::AudioFile
  attr_accessor :name
  attr_accessor :volume
  attr_accessor :pitch
end

class RPG::BGM < RPG::AudioFile
  attr_accessor :pos
end

class RPG::BGS < RPG::AudioFile
  attr_accessor :pos
end

class RPG::ME < RPG::AudioFile
end

class RPG::SE < RPG::AudioFile
end

class ScriptData
  attr_accessor :id
  attr_accessor :name
  attr_accessor :data
end



ObjectSpace.each_object(Class) do |c|
  if c.name.nil? == false && c.name.include?("RPG::")
    c.class_eval do
      include RPG::ToJson
    end
  end
end

#処理を変更した場合、lscsv_test.rb側の内容も追従させること。
def fix_newlines_in_csv(input_texts)
  #Map以外のデータの改行コードを修正する処理。
  #\nだけの場合は\r\nに変換する。

  nl_unique_token = "___LaNgScOrE_NEW_LINE___"
  header = LSCSV.fetch_header(input_texts)
  rows = LSCSV.parse_col(header, LSCSV.parse_row(input_texts))

  #""括りを行うかどうかの判定。
  rows[1...rows.size].each do |row|
    row.map! do |field|
      is_add_dq = false
      
      protected_text = field
      if protected_text.include?("\n")
        protected_text  = field.gsub("\r\n", nl_unique_token)
        is_add_dq = true
      end

      if protected_text.include?("\n")
        #文章中の改行コードが\nだけだった場合、\r\nに変換。
        protected_text.gsub!("\n", "\r\n")
        is_add_dq = true
      elsif protected_text.include?("\"")
        protected_text.gsub!("\"", "\"\"")
        is_add_dq = true
      elsif protected_text.include?(",")
        is_add_dq = true
      end

      field = protected_text.gsub(nl_unique_token, "\r\n")

      if is_add_dq
        field = "\"" + field + "\""
      else
        field
      end
    end
  end

  result = ""
  result += header.join(",") + "\n"
  rows[1...rows.size].each do |r|
    result += r.join(",") + "\n"
  end
  return result.chomp("\n")
end


#================================================
opt = OptionParser.new
Version = "1.0.2"

p "RVCNV Version #{Version}"

input_folder_path = ""
opt.on_head('-i PROJPATH', '--input PROJPATH'){ |v| 
  input_folder_path = File.absolute_path(v)
  input_folder_path.gsub!("\\", "/")
  input_folder_path.chop! if input_folder_path.end_with?('/')
}

output_folder = ""
opt.on('-o OUTPUTPATH', '--output OUTPUTPATH'){ |v| 
  output_folder = File.absolute_path(v)
  output_folder.chop! if output_folder.end_with?('/')

  Dir.mkdir(output_folder) unless File.exist?(output_folder)
}

compress = false
opt.on('-c'){|v| compress = v }
packing = false
opt.on('-p'){|v| packing = v }

opt.parse!(ARGV)

#パースでブロック内の処理が評価されるため、パース後に代入すること
data_folder   = input_folder_path+'/Data'
script_folder = output_folder+'/Scripts'

#================================================

SCRIPTLIST_ID       = 0
SCRIPTLIST_PATH     = 1
SCRIPTLIST_NAME     = 2
SCRIPTLIST_DATA     = 3
CSV_SCRIPTLIST_ID   = 0
CSV_SCRIPTLIST_NAME = 1
EMPTY_SCRIPT_NAME   = "_NONAME_"

if packing
  p "Packing..."
  class LsDumpData
    attr_accessor :data
  end
  read_dir = input_folder_path
  p "Check #{read_dir}"
  Dir.glob('*.csv', base: read_dir).each do |fileName|
  
    origin = LsDumpData.new
    File.open(input_folder_path + "/" + fileName, 'rb:utf-8:utf-8') do |file|
      texts = file.readlines().join()
      if fileName.match?(/Map[0-9]{3}/)
        #Map系に\r\nが含まれていたら\nに変換
        texts = texts.gsub(/\r\n/, "\n")
      elsif fileName.include?("Scripts")
        #スクリプト系は一旦無加工にする。
      else
        #その他のデータ内の文字列は改行文字が\r\nとなるため、
        #\nのみの改行だった場合は\r\nに修正して埋める。
        texts = fix_newlines_in_csv(texts)
      end
      origin.data = texts
    end
    d = Marshal.dump(origin)
    output_path = output_folder + "/" + File.basename(fileName, ".csv") + ".rvdata2"
    File.open(output_path, "wb") do | dumpFile |
      p "packing"
      p "  Input  : #{input_folder_path + "/" + fileName}"
      p "  Output : #{output_path}"
      dumpFile.write(d)
    end
  end
  exit
end

if compress
  p "Compress..."
  compressData = []
  exported_script_folder = input_folder_path+'_langscore/analyze/Scripts'
  script_list_path = exported_script_folder + '/_list.csv'
  CSV.foreach(script_list_path) do | row |
    id         = row[CSV_SCRIPTLIST_ID]
    scriptname = row[CSV_SCRIPTLIST_NAME]
    scriptname = "" if scriptname.include?(EMPTY_SCRIPT_NAME)
    filepath   = exported_script_folder+'/'+id.to_s+'.rb'
    if File.exist?(filepath) == false
      compressData.push([id, scriptname, Zlib::Deflate.deflate("", Zlib::DEFAULT_COMPRESSION )])
    else 
      File.open(filepath, 'rb:utf-8:utf-8') do |file|
        contents = file.readlines().join()
        compressed = Zlib::Deflate.deflate(contents, Zlib::DEFAULT_COMPRESSION );
        compressData.push([id, scriptname, compressed])
      end
    end
  end

  File.open(data_folder+'/Scripts.rvdata2', 'wb') do |file|
    Marshal.dump(compressData, file)
  end

  p "Compress Complete."
  exit
end

is_exit = false
if input_folder_path.empty?
  p "Need Project Folder" 
  is_exit = true
end

if output_folder.empty?
  p "Need Output Folder" 
  is_exit = true
end

p "Data Folder #{data_folder}"
p "Script Folder #{script_folder}"

if is_exit
  exit -1
end

#スクリプトの展開
script_list = []
if File.exist?(data_folder+"/Scripts.rvdata2")
  p "Extending Scripts.rvdata2"
  File.open(data_folder+"/Scripts.rvdata2", 'rb') do |file|
    Marshal.load(file.read).each do |id, name, script|
      d = Zlib::Inflate.inflate(script)
      
      name = EMPTY_SCRIPT_NAME if name.empty?

      script_path = script_folder+'/'+id.to_s+'.rb'
      script_list.push([id.to_s, script_path, name, d])
    end
  end
end


#再圧縮のためにリストを用意しておく
Dir.mkdir(script_folder) unless File.exist?(script_folder)

script_list_path = script_folder+'/_list.csv'
File.delete(script_list_path) if File.exist?(script_list_path)

CSV.open(script_list_path, 'w') do |file|
  script_list.each do |r|
    #ID, NAME
    file.puts([r[SCRIPTLIST_ID], r[SCRIPTLIST_NAME]])
    
    File.open(r[SCRIPTLIST_PATH], "wb") do |out|
      out.write(r[SCRIPTLIST_DATA]) 
    end
  end
end

if File.exist?(data_folder)
  # rvdata_list = [
  #   'Actors.rvdata2', 'Animations.rvdata2', 'Armors.rvdata2',
  #   'Classes.rvdata2','CommonEvents.rvdata2','Enemies.rvdata2',
  #   'Items.rvdata2',
  #   *Dir.glob('Map[0-9][0-9][0-9].rvdata2', base: data_folder),
  #   'Skills.rvdata2', 'States.rvdata2', 'System.rvdata2',
  #   'Troops.rvdata2', 'Weapons.rvdata2'
  # ]
  # Dir.foreach(data_folder) do |item|
  #   next if item == '.' or item == '..'
  #   puts item
  # end
  rvdata_list = []
  Dir.foreach(data_folder) do |filename|
    next if filename == '.' || filename == '..'
    
    if filename =~ /\A(Actors|Armors|Classes|CommonEvents|Enemies|Items|Map[0-9]{3}|Skills|States|System|Troops|Weapons)\.rvdata2\z/
      p filename
      rvdata_list << filename
    end
    
  end
  rvdata_list.each do |rvdata|
    data = ''
    File.open(data_folder+"/"+File.basename(rvdata), 'rb') do |file|
      begin 
        data = Marshal.load(file.read)
        File.open(output_folder+"/"+File.basename(rvdata, ".rvdata2")+".json", "wb") do |out|
          out.write(data.to_json)
        end
      rescue => e
        p "Load : #{data_folder+"/"+rvdata}"
        p e
      end
    end
  end

  #マップ名を取得するためにMapInfosを解析。
  #Jsonは手間なのでCSVで直接出力する [マップID, オーダー, マップ名]
  File.open(data_folder+"/MapInfos.rvdata2", 'rb') do |info|
    begin 
      csvData = []
      data = Marshal.load(info.read).each do |id, mapInfo|
        csvData.push([id, mapInfo.order, mapInfo.name])
      end

      CSV.open(output_folder+"/MapInfos.csv", 'w') do |file|
        csvData.each do |r| 
          file.puts([r[0],r[1],r[2]]) 
        end
      end
    rescue => e
      p "Load : #{data_folder+"/"+rvdata}"
      p e
    end
  end

end