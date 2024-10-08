Encoding.default_external = Encoding::UTF_8
Encoding.default_internal = Encoding::UTF_8
require 'csv'
require 'rbconfig'
require 'Win32API'

module RPG
end


class Rect
  attr_accessor :x, :y, :width, :height
  @x = 0
  @y = 0
  @width = 0
  @height = 0

  def initialize(x = 0, y = 0, width = 0, height = 0)
    @x = x
    @y = y
    @width = width
    @height = height
  end

  def set(x = 0, y = 0, width = 0, height = 0)
    @x = x
    @y = y
    @width = width
    @height = height
  end
  
  def empty
    @x = 0
    @y = 0
    @width = 0
    @height = 0
  end

end

class Graphics
  @x = 0
  @y = 0
  @width = 1
  @height = 1

  class << self
    attr_accessor :x, :y, :width, :height
    def initialize()
    end

    def transition(frame, path = "", x = 0)
    end

    def width
      @width
    end

    def height
      @height
    end

    def fadeout(time)
    end

    def update
    end

    def frame_count=(f)
    end

    def frame_count()
    end
    def frame_rate()
      1
    end
    def frame_reset
    end
  end
end

class Tone  
  def initialize()
  end

  attr_accessor :ref 
  attr_accessor :green 
  attr_accessor :blue 
  attr_accessor :gray 

  def set(t, t2=0, t3=0)
  end

  def self._load(obj)
    Tone.new()
  end
end

class Color  
  def initialize(r = 0,g = 0,b = 0,a = 0)
  end

  attr_accessor :ref 
  attr_accessor :green 
  attr_accessor :blue 
  attr_accessor :alpha 

  def self._load(obj)
  end

  def set(r = 0,g = 0,b = 0,a = 0)
  end
end

class Font
  def initialize()
    @size = 1
    @default_bold = 1
    @default_italic = 1
    @default_size = 1
  end
  
  def size
    @size = 1
  end
  def size=(s)
    @size = s
  end
  
  def bold
  end
  def bold=(s)
  end

  def italic
  end
  def italic=(s)
  end

  def self.default_bold
    @default_bold = 1
  end
  def self.default_bold=(v)
  end
  def self.default_italic
    @default_italic = 1
  end
  def self.default_italic=(v)
  end
  def self.default_size
    @default_size = 1
  end
  def self.default_size=(v)
  end
  def self.default_name
    "VL Gothic"
  end
  def self.default_name=(f)
  end

  def color
    Color.new
  end
end


class Bitmap
  attr_accessor :image
  attr_accessor :width
  attr_accessor :height

  def initialize(w, h = 0)
    @font = Font.new
    if h == 0
      @image = image
      @width = 1
      @height = 1
    else
      @image = image
      @width = w
      @height = h
    end
  end

  def font
    @font
  end
  
  def draw_text(r, tr, s)
  end

  def disposed?
    false
  end

  def fill_rect(a,b,c=0,d=0,e=0)
  end

  def rect
    Rect.new
  end

  def gradient_fill_rect(r, c1, c2, f)
  end
  
end

class Object
  def dispose
  end
  def clear 
  end
  def font
    Font.new
  end

  def width
    1
  end
  def height
    1
  end

  def draw_text(*args)
  end
  def text_size(str)
  end

  def blt(*args)
  end
end

class Viewport
    
  attr_accessor :z
  attr_accessor :rect

  def initialize(rect = Rect.new)
    @rect = rect
  end

  def tone
    Tone.new
  end
  def oy=(y)
  end
  def oy
    1
  end
  def ox=(x)
  end
  def ox
    1
  end
  def color
    Color.new
  end
  def update
  end
end

class Sprite
  attr_accessor :bitmap
  attr_accessor :ox
  attr_accessor :oy
  attr_accessor :x
  attr_accessor :y
  attr_accessor :z
  
  def initialize(viewport = nil)
      @bitmap = Bitmap.new(1,1)
      @ox = 0
      @oy = 0
      @x = 0
      @y = 0
      @z = 0
  end

  def oy=(y)
  end
  def oy
    1
  end
  def ox=(x)
  end
  def ox
    1
  end

  def update
  end

  def src_rect
    Rect.new
  end

  def opacity=(v)
  end
  def blend_type=(v)
  end
  def bush_depth=(v)
  end
  def visible=(v)
  end
  def color
    Color.new
  end
end

class Tilemap
  attr_accessor :bitmaps
  attr_accessor :ox
  attr_accessor :oy
  def initialize(viewport)
    @bitmaps = Array.new
  end

  def map_data=(f)
  end

  def flags=(f)
  end

  def dispose
  end

  def oy=(y)
  end
  def oy
    1
  end
  def ox=(x)
  end
  def ox
    1
  end

  def update
  end
end

class Table
  attr_accessor :xsize 
  attr_accessor :ysize 
  attr_accessor :zsize 

  def initialize(d)
    @data = Hash.new()
  end
  
  def self._load(obj)
    Table.new(obj.unpack("VVVVVv*"))
  end

  def [](x, y, z)
    @data[[x, y, z]]
  end

  def []=(x, y, z, value)
    @data[[x, y, z]] = value
  end
end

class Plane < Viewport
  def initialize(viewport)
  end

  def bitmap
    Bitmap.new(1,1)
  end
  def bitmap=(b)
  end

  def ox=(x)
  end
  def oy=(x)
  end
  def ox
    1
  end
  def oy
    2
  end
end

class WindowSkin
  def get_pixel(w, x)
    Color.new(0,0,0)
  end
end

class Window < Sprite

  attr_accessor :x
  attr_accessor :y

  def initialize(x, y, width, height)
    @x = 0
    @y = 0
    @height = 1
    @width = 1
    @padding = 1
  end

  def windowskin=(w)
  end
  def windowskin
    WindowSkin.new
  end

  def contents
    Object.new
  end

  def active=(active)
  end
  def active
    true
  end

  def update
  end

  def cursor_rect
    Rect.new
  end

  def tone=(s)
  end
  def tone
    Tone.new
  end
  def padding=(s)
  end
  def contents=(s)
  end

  def x=(_x)
    @x = _x
  end
  def x
    @x
  end

  def y=(_y)
    @y = _y
  end
  def y
    @y
  end

  def height=(h)
    @height = h
  end
  def height
    @height
  end
  def width=(v)
    @width = v
  end
  def width
    @width
  end

  def padding=(p)
    @padding = p
  end
  def padding()
    @padding
  end

  def padding_bottom=(p)
    @padding = p
  end
  def padding_bottom()
    @padding
  end

  def openness=(x)
  end
  def openness
    1
  end

  def arrows_visible=(f)
  end
  def contents_opacity=(f)
  end

  def open?
    true
  end

  def disposed?
    true
  end
  
  def close
  end
  
  def close?
    true
  end

  def viewport=(v)
  end
  def viewport
    Viewport.new
  end
end


class Input
  class << self
    def update
    end
    def trigger?(k)
      false
    end
    def repeat?(k)
      false
    end
    def press?(k)
      false
    end
  end
end

class RPG::Event
  attr_accessor :id
  attr_accessor :name
  attr_accessor :x
  attr_accessor :y
  attr_accessor :pages

  def parallel?
    false
  end
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

  def parallel?
    false
  end

  def autorun?
    true
  end
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

class Params
  
  def [](x, y)
    0
  end

  def []=(x, y, value)
  end
end

class RPG::Class < RPG::BaseItem
  attr_accessor :exp_params
  attr_accessor :params
  attr_accessor :learnings

  def exp_for_level(l)
  end
  
  def params
    Params.new
  end
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

  def initialize
    @data = Hash.new(0)
  end

  def [](x, y, z)
    @data[[x, y, z]]
  end

  def []=(x, y, z, value)
    @data[[x, y, z]] = value
  end
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

  def flags
    Hash.new
  end
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
  
  def play
  end
  def stop
  end
  def self.play
  end
  def self.stop
  end
  def self.fade(t)
  end
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

  

def load_data(filename)
    File.open(Dir.pwd + "/vxace_test/" + filename, "rb") do |file|
        return Marshal.load(file)
    end
end
  
def rgss_main
    yield
    rescue RGSSReset
    retry
end

# CSVファイルのパス
csv_file_path = './vxace_test/Scripts/_list.csv'

# CSVファイルを読み込み、1列目のスクリプトファイル名を取得
script_files = []
CSV.foreach(csv_file_path, headers: false) do |row|
  if row[1].strip != "Main"
    path = ("./vxace_test/Scripts/" + row[1].strip + ".rb")
    script_files << path
  end
end

# スクリプトファイルを順番に読み込み
script_files.each do |file_path|
  begin
    # ファイルを読み込みUTF-8として評価する
    file_content = File.read(file_path, encoding: 'UTF-8')
    eval(file_content, binding, file_path)
  rescue => e
    # エラーが発生した場合の処理
    puts "Error in file: #{file_path}"
    puts "Error: #{e.message}"
    puts e.backtrace
  end
end