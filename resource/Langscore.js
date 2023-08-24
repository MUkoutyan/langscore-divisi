//---------------------------------------------------------------
// 
// Langscore CoreScript "Unison" 
// Version 0.9.4
// Written by BreezeSinfonia 來奈津
// 
// 注意：このスクリプトは自動生成されました。編集は非推奨です。
//---------------------------------------------------------------

 /*:
 * @plugindesc v0.9.4 LangscoreのMV用プラグインです。
 * 注意：このスクリプトは自動生成されているため、編集は非推奨です。
 * @author BreezeSinfonia 來奈津
 * 
 * @param Default Language
 * @desc ゲーム初回起動時に適用する言語です。
 * @default ja
 * 
 * @help
 * プラグインコマンド
 * #langに言語文字を指定します。指定可能な文字は%{SUPPORT_LANGUAGE_STR}です。
 * Langscore changeLanguage lang 
 */

 (() => {
  'use strict';
/*
%{SUPPORT_LANGUAGE}%
%{DEFAULT_LANGUAGE}%

%{SUPPORT_FONTS}%

%{TRANSLATE_FOLDER}%

*/
//ハッシュそのものからハッシュを格納するハッシュに変更。
//イベント中にマップ移動が行われた場合、翻訳内容がクリアされる問題への対応のため。


var Langscore_Parameters = PluginManager.parameters('Langscore');

//-----------------------------------------------------

//%{UNISON_LSCSV}%

//-----------------------------------------------------

class Langscore 
{
  // static Support_Language = ["ja", "en", "zh-cn"];
  static Support_Language = ["ja", "en"];
  static Default_Language = String(Langscore_Parameters['Default Language']);
  static Translate_Folder = "Data/Translate"

  static FontList = [
    {lang: "ja", font:  {name: "VL Gothic", size: 22}},
    {lang: "en", font:  {name: "VL Gothic", size: 22}},
    // {lang: "zh-cn", font : {name: "Source Han Sans CN Regular", size: 22}},
  ]

  static langscore_current_language = String(Langscore_Parameters['Default Language']);
  static currentFont = Langscore.FontList[Langscore.langscore_current_language];

  static isNull(obj){
    return obj === null || obj === undefined;
  }

  constructor()
  {
    this._databaseFiles = [
      { name: 'ls_actors_tr', src: 'Actors.csv' },
      { name: 'ls_classes_tr', src: 'Classes.csv' },
      { name: 'ls_skills_tr', src: 'Skills.csv' },
      { name: 'ls_items_tr', src: 'Items.csv' },
      { name: 'ls_weapons_tr', src: 'Weapons.csv' },
      { name: 'ls_armors_tr', src: 'Armors.csv' },
      { name: 'ls_enemies_tr', src: 'Enemies.csv' },
      { name: 'ls_troops_tr', src: 'Troops.csv' },
      { name: 'ls_states_tr', src: 'States.csv' },
      { name: 'ls_system_tr', src: 'System.csv' },
      { name: 'ls_graphics_tr', src: 'Graphics.csv' },
      { name: 'ls_scripts_tr', src: 'Scripts.csv' },
      { name: 'ls_common_event', src: 'CommonEvents.csv' },
    ];
    
    this.ls_actors_tr = null;
    this.ls_system_tr = null;
    this.ls_classes_tr = null;
    this.ls_skills_tr = null;
    this.ls_states_tr = null;
    this.ls_weapons_tr = null;
    this.ls_armors_tr = null;
    this.ls_items_tr = null;
    this.ls_enemies_tr = null;
    this.ls_graphics_tr = null;
    this.ls_scripts_tr = null;
    this.ls_troops_tr = null;
    this.ls_common_event = null;
    
    this.ls_current_map = new Map;
    this.ls_graphic_cache = {};
  }

  isLoaded()
  {
    for (var i = 0; i < this._databaseFiles.length; i++) {
      if (!this[this._databaseFiles[i].name]) {
          return false;
      }
    }
    return true;
  }

  lstrans(line_info, ...args) {
    text = this.translate_for_script(line_info);
    // return sprintf(text, *args)
  }

  translate(text, langscore_hash, lang = Langscore.langscore_current_language)
  {
    if(Langscore.isNull(langscore_hash)){
      return text;
    }
    
    var key = text;

    var translatedList = langscore_hash[key];
    if(!translatedList){ return text; }
    var t = translatedList[lang];
    if(t){
      text = t;
    }
    return text;
  }

  translate_for_map(text) 
  {
    if(!this.ls_current_map){ return text; }
    
    var parent = this;
    var currentMapId = $gameMap._interpreter._mapId;
    var currentMapTranslatedHash = this.ls_current_map[currentMapId];
    if(!currentMapTranslatedHash){ return text; }

    var translate_result = parent.translate(text, currentMapTranslatedHash);
    if(translate_result !== text){
      return translate_result;
    }
    return text;
  }

  translate_for_script(text){
    return this.translate(text, this.ls_scripts_tr);
  };

  fetch_original_text(transed_text, langscore_hash) 
  {
    var origin = transed_text;
    var a = langscore_hash.entries();
    for (const [originText, transList] of a) {
      for (let transText of transList.values()) {
          if (transText === origin) {
              return originText;
          }
      }
  }
    return origin;
  }

  translate_list_reset()
  {
    if(test === false){
      return;
    }
    this.ls_actors_tr.clear();
    this.ls_system_tr.clear();
    this.ls_classes_tr.clear();
    this.ls_skills_tr.clear();
    this.ls_states_tr.clear();
    this.ls_weapons_tr.clear();
    this.ls_armors_tr.clear();
    this.ls_items_tr.clear();
    this.ls_enemies_tr.clear();
    this.ls_graphics_tr = _lscsv.to_hash("Graphics")
    this.ls_scripts_tr = _lscsv.to_hash("Scripts")
    this.ls_troops_tr = _lscsv.to_hash("Troops")
    this.ls_common_event = _lscsv.to_hash("CommonEvents")
  
    changeLanguage($langscore_current_language)
  }

  changeLanguage(lang)
  {
    if(Langscore.langscore_current_language === lang){
      return;
    }

    if(!Langscore.Support_Language.includes(lang)){
      return;
    }
    
    Langscore.langscore_current_language = lang;
    this.updateFont(lang);
  
    this.updateSkills();
    this.updateClasses();
    this.updateStates();
    this.updateEnemies();
    this.updateItems();
    this.updateArmors();
    this.updateWeapons();
    this.updateSystem();
  
    //Game_Actors.newをしているため並列処理から除外
    this.updateActor();
    
    this.ls_graphic_cache = {};
    this.ls_graphic_cache.clear
  }

  updateFont(lang) {

    if(Langscore.isNull(this.FontList)){
      return;
    }

    if(Langscore.isNull(this.currentFont) === false){
      console.log(this.currentFont);
      this.beforeFontName = this.currentFont["name"];
    }
    if (this.FontList.find(item => item.lang === lang)) {
      return this.beforeFontName !== this.currentFont["name"];
    }
    this.currentFont = this.FontList[lang];
  };


  updateForNameAndDesc(data_list, tr_list) 
  {
    const elm_trans =(el) => {
      el = this.translate(el, tr_list);
    }
    data_list.forEach(function(obj,i){
      if(data_list[i] === null){ return; }
      data_list[i].name        = elm_trans(obj.name);
      data_list[i].description = elm_trans(obj.description);
    });
  };

  updateForName(data_list, tr_list) 
  {
    const elm_trans =(el) => {
      el = this.translate(el, tr_list);
    }
    data_list.forEach(function(obj,i){
      if (data_list[i] === null) { return; }
      data_list[i].name = elm_trans(obj.name);
    });
  };

  updateActor()
  {

    //大元のデータベースを更新。Game_Actor作成時に使用されるため必要。
    this.updateForNameAndDesc($dataActors, this.ls_actors_tr);
    
    const elm_trans =(el) => {
        el = this.translate(el, this.ls_actors_tr);
    }

    //既にGame_Actorが作成されている場合、インスタンス側も更新。
    //他のデータベースと同様に初期化を行うと、パラメータ値等も全部初期化されるので、名前以外の内容は保持する。

    if($dataActors === null || $gameActors === null){
      return;
    }
    for (var i = 0; i < $dataActors.length; ++i) {
      var actor = $gameActors.actor(i);
      if (!actor){ continue; }

      let name = this.fetch_original_text(actor._name, this.ls_actors_tr);
      if(name){
        $gameActors.actor(i)._name     = elm_trans(name);
      }
      var nickname = this.fetch_original_text(actor._nickname, this.ls_actors_tr);
      if(nickname){
        $gameActors.actor(i)._nickname = elm_trans(nickname);
      }
    }
  };

  updateSystem(){ 
    const elm_trans =(el) => {
      el = this.translate(el, this.ls_system_tr);
    }
    // $dataSystem.elements.map(elm_trans);
    // $dataSystem.skillTypes.map(elm_trans);
    $dataSystem.terms.params.map(elm_trans);
    $dataSystem.terms.commands.map(elm_trans);
    $dataSystem.terms.basic.map(elm_trans);
    Object.keys($dataSystem.terms.messages).forEach(key => {
      var value = $dataSystem.terms.messages[key];
      $dataSystem.terms.messages[key] = this.translate(value, this.ls_system_tr);
    });

    $dataSystem.currencyUnit = this.translate($dataSystem.currencyUnit, this.ls_system_tr);
  }

  updateClasses(){
    this.updateForName($dataClasses, this.ls_classes_tr);
  }

  updateSkills(){

    const elm_trans =(el) => {
      el = this.translate(el, this.ls_skills_tr);
    };
    $dataSkills.forEach(function(skill,i){
      if($dataSkills[i] === null){ return; }
      $dataSkills[i].name         = elm_trans(skill.name);
      $dataSkills[i].description = elm_trans(skill.description);
      $dataSkills[i].message1   = elm_trans(skill.message1);
      $dataSkills[i].message2   = elm_trans(skill.message2);
    });
  }

  updateStates(){
    
    const elm_trans =(el) => {
      el = this.translate(el, this.ls_states_tr);
    };
    $dataStates.forEach(function(skill,i){
      if($dataStates[i] === null){ return; }
      $dataStates[i].name        = elm_trans(skill.name);
      $dataStates[i].message1  = elm_trans(skill.message1);
      $dataStates[i].message2  = elm_trans(skill.message2);
      $dataStates[i].message3  = elm_trans(skill.message3);
      $dataStates[i].message4  = elm_trans(skill.message4);
    });
  }

  updateWeapons(){
    this.updateForNameAndDesc($dataWeapons, this.ls_weapons_tr);
  }

  updateArmors(){
    this.updateForNameAndDesc($dataArmors, this.ls_armors_tr);
  }

  updateItems(){
    this.updateForNameAndDesc($dataItems, this.ls_items_tr);
  }

  updateEnemies(){
    this.updateForName($dataEnemies, this.ls_enemies_tr);
  }



  loadSystemDataFile(varName, fileName) {
    var xhr = new XMLHttpRequest();
    var url = 'data/translates/' + fileName;
    var parent = this;
    xhr.open('GET', url);
    xhr.overrideMimeType('application/json');
    xhr.onload = function() {
      if (xhr.status < 400) {
        parent[varName] = _lscsv.to_hash(xhr.responseText, varName);
      }
      else{
        parent[varName] = {};
      }
    };
    xhr.onerror = parent._mapLoader || function() {
      //無限ループにさせないようにnullではなく空にしておく。
      parent[varName] = {};
      DataManager._errorUrl = DataManager._errorUrl || url;
    };
    parent[varName] = null;
    xhr.send();
  };

  
  loadMapDataFile(mapID) {
    var xhr = new XMLHttpRequest();
    var url = 'data/translates/Map%1.csv'.format(mapID.padZero(3));
    var parent = this;
    xhr.open('GET', url);
    xhr.overrideMimeType('application/json');
    
    xhr.onload = function() {
      if (xhr.status < 400) {
        parent.ls_current_map[mapID] = _lscsv.to_hash(xhr.responseText, url);
      }
      else{
        parent.ls_current_map[mapID] = {};
      }
    };
    xhr.onerror = parent._mapLoader || function() {
      //無限ループにさせないようにnullではなく空にしておく。
      parent.ls_current_map[mapID] = {};
      DataManager._errorUrl = DataManager._errorUrl || url;
    };
    parent.ls_current_map[mapID] = null;
    xhr.send();
  };

} //class Langscore

const _langscore = new Langscore();

const Game_Interpreter_pluginCommand = Game_Interpreter.prototype.pluginCommand;
Game_Interpreter.prototype.pluginCommand = function( command, args ) {
  Game_Interpreter_pluginCommand.call( this, command, args );

    switch(command.toUpperCase())
    {
      case 'LANGSCORE':
        if(args[0].toUpperCase() === 'CHANGELANGUAGE'){
          _langscore.changeLanguage(args[1]);
        }
        break;
    }
};


var DataManager_loadDatabase = DataManager.loadDatabase;
DataManager.loadDatabase = function() 
{
  DataManager_loadDatabase.call(this);

  for (var i = 0; i < _langscore._databaseFiles.length; i++) {
    var varName = _langscore._databaseFiles[i].name;
    var fileName = _langscore._databaseFiles[i].src;
    _langscore.loadSystemDataFile(varName, fileName);
  }
};

var DataManager_loadMapData = DataManager.loadMapData;
DataManager.loadMapData = function(mapId) 
{
  DataManager_loadMapData.call(this, mapId);
  if(mapId > 0){
    var fileName = 'Map%1.csv'.format(mapId.padZero(3));
    _langscore.mapLoader = ResourceHandler.createLoader('data/translates/' + fileName, _langscore.loadMapDataFile.bind(this, mapId));
    _langscore.loadMapDataFile(mapId);
  }
};

/*
//-----------------------------------------------------

class Game_Map

  def Game_Map.ls_finalize
    proc {
      this.ls_current_map.delete(@map_id)
    }
  }
  
  alias ls_base_setup setup
  function setup(map_id) {
      ls_base_setup(map_id)
    load_langscore_file(map_id)
  }

  function load_langscore_file(map_id) {
      file_name  = sprintf("Map%03d", @map_id)

    return if this.ls_current_map.include?(@map_id)
    this.ls_current_map[@map_id] = _lscsv.to_hash(file_name)
    //メモリ節約のためファイナライズで翻訳内容を消す。GC頼りなのでおまじない程度の気持ち。
    //会話中に何故かクラッシュした場合は真っ先に外す。
    ObjectSpace.define_finalizer(this, Game_Map.ls_finalize)
  }
}
*/
//アクター名の変更
var Game_Interpreter_command_320 = Game_Interpreter.command_320;
Game_Interpreter.command_320 = function() {
  Game_Interpreter_command_320.call(this);
  _langscore.updateActor();
};

//二つ名の変更
var Game_Interpreter_command_320 = Game_Interpreter.command_324;
Game_Interpreter.command_324 = function() {
  Game_Interpreter_command_324.call(this);
  _langscore.updateActor();
};

var Window_Base_convertEscapeCharacters = Window_Base.prototype.convertEscapeCharacters;
Window_Base.prototype.convertEscapeCharacters = function(text) 
{
  if (text.length === 0) {
    return Window_Base_convertEscapeCharacters(text)
  }

  let result = _langscore.translate_for_map(text);
  if(result && result !== text){
    return Window_Base_convertEscapeCharacters.call(this, result);
  }
  result = _langscore.translate(text, _langscore.ls_troops_tr);
  if(result && result !== text){
    return Window_Base_convertEscapeCharacters.call(this, result);
  }
  result = _langscore.translate(text, _langscore.ls_common_event);
  if(result && result !== text){
    return Window_Base_convertEscapeCharacters.call(this, result);
  }
  
  return Window_Base_convertEscapeCharacters(text);
}


var Window_Base_standardFontFace = Window_Base.prototype.standardFontFace;
Window_Base.prototype.standardFontFace = function() {
  return Langscore.isNull(_langscore.currentFont) === false ? _langscore.currentFont.name : Window_Base_standardFontFace.call(this);
};

var Window_Base_standardFontSize = Window_Base.prototype.standardFontSize;
Window_Base.prototype.standardFontSize = function() {
  return Langscore.isNull(_langscore.currentFont) === false ? _langscore.currentFont.size : Window_Base_standardFontSize.call(this);
};


//モジュールの上書き
//シーン遷移に関わらない翻訳ファイルは初期化時に読み込み
//戦闘テスト用は未対応
var DataManager_isDatabaseLoaded = DataManager.isDatabaseLoaded;
DataManager.isDatabaseLoaded = function(){
  var result = DataManager_isDatabaseLoaded.call(this);
  if(result){
    if(_langscore.isLoaded() === false){
      return false;
    }
    _langscore.changeLanguage(Langscore.langscore_current_language);
  }
  return result;
}

//セーブを行う際は原文で保存
//プラグインを外した際に変に翻訳文が残ることを避ける。
var DataManager_makeSaveContents = DataManager.makeSaveContents;
DataManager.makeSaveContents = function(){

  DataManager_makeSaveContents.call(this);
  // var data_temp = Marshal.dump($gameActors)
  // for (var i = 0; i < $dataActors.length; ++i) {
  //   var actor = $gameActors.actor(i);
  //   if (!actor){ continue; }
    
  //   $gameActors[i].name       = this.fetch_original_text(actor.name, this.ls_actors_tr)
  //   $gameActors[i].nickname = this.fetch_original_text(actor.nickname, this.ls_actors_tr)
  //   if (actor.equips().contains(item)) return true;
  // }

  // //セーブ本処理
  // result = ls_make_save_contents

  // $gameActors = Marshal.load(data_temp)

  // return result;
};

//セーブデータは原文で保存されているため、起動時の言語設定で置き換える。
//これを省くと中国語で起動した際に、再度翻訳を適用するまで日本語のまま……といった事が起きる。
var DataManager_extractSaveContents = DataManager.extractSaveContents;
DataManager.extractSaveContents = function(contents) {
  DataManager_extractSaveContents.call(this, contents);
  _langscore.changeLanguage($langscore_current_language)
};

//セーブデータロード時のマップ更新はversion_idに依存しているが、
//version_idの変更タイミングが謎なのでMapが更新されたりされなかったりするように見える。
//Langscoreの場合$game_map.setupが行われないと翻訳文が読み込まれないため、
//reload_map_if_updatedの分岐に関わらずロードするようにする。
// def this.reload_map_if_updated
//   ls_reload_map_if_updated
//   $game_map.load_langscore_file($game_map.map_id)
// }

var ImageManager_loadMitmap = ImageManager.loadBitmap;
ImageManager.loadBitmap = function(folder_name, filename, hue = 0) 
{    
  if(_langscore.ls_graphic_cache === null){
    return ImageManager_loadMitmap.call(this, folder_name, filename, hue);
  }
  var path = folder_name+filename;
  var ts_path = _langscore.translate(path, this.ls_graphics_tr);
  if(ts_path !== path){
    filename = ts_path.sub(folder_name, "")
  }
  else
  {
    //翻訳テキスト内で明示的に画像が指定されていない場合、ファイル名検索
    //filenameは元から拡張子なしなのでそのまま結合
    var new_filename = filename + '_' + Langscore.langscore_current_language;
    
    var has_key = _langscore.ls_graphic_cache[filename];
    if(has_key === false){
      try
      {
        bitmap = ImageManager_loadMitmap.call(this, folder_name, new_filename, hue);
        _langscore.ls_graphic_cache.set(filename, true);
        return bitmap;
      }
      catch(e)
      {
        _langscore.ls_graphic_cache.set(filename, false);
      }
    }

    if (_langscore.ls_graphic_cache[filename]) {
      filename = new_filename;
    }
  }

  return ImageManager_loadMitmap.call(this, folder_name, filename, hue);
}


})(); //'use strict';