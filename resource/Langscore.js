//---------------------------------------------------------------
// 
// Langscore CoreScript "Unison" 
// Version 0.9.12
// Written by BreezeSinfonia 來奈津
// 
// 注意：このスクリプトは自動生成されました。編集は非推奨です。
//---------------------------------------------------------------
 /*:en
 * @target MV MZ
 * @plugindesc Translate plugin Langscore for RPG Tskool MV/MZ.
 * @author BreezeSinfonia Kunatsu
 * @url https://github.com/MUkoutyan/langscore-app/releases
 * 
 * @help
 * Note: This script is auto-generated, so editing is deprecated.
 * If you want to change supported languages.exe., please edit it using Langscore.exe.
 * 
 * [Instructions for MV]
 * Plugin commands
 * Specify language character for #lang. Possible characters are %{SUPPORT_LANGUAGE_STR}%.
 * Langscore changeLanguage #lang 
 * 
 * @command changeLanguage
 * @text changeLanguage
 * @desc Specify language character.
 * 
 * @arg language
 * @text language to change
 * @desc The characters that can be specified are %{SUPPORT_LANGUAGE_STR}%.
 * 
 * @param Default Language
 * @text Default language.
 * @desc Language to be applied "at first launch" of the game. Specify the language in which the game was created.
 * @default ja
 * 
 * @param MustBeIncludedImage
 * @desc 
 * @default Always specify files to be included even when "Do not include unused files" is checked during deployment.
 * @require 1
 * @dir img/
 * @type file[]
 */
 /*:ja
 * @target MV MZ
 * @plugindesc 翻訳アプリLangscoreのRPGツクールMV/MZ用プラグインです。
 * @author BreezeSinfonia 來奈津
 * @url https://github.com/MUkoutyan/langscore-app/releases
 * 
 * @help
 * 注意：このスクリプトは自動生成されています。Langscore.exeを使用すると更新されるため、編集は非推奨です。
 * サポート言語等を変更する場合、Langscore.exeを使用して編集してください。
 * 
 * [MV向けの説明]
 * プラグインコマンド
 * #langに言語文字を指定します。指定可能な文字は%{SUPPORT_LANGUAGE_STR}%です。
 * ダブルクォーテーションは含まないで下さい。
 * Langscore changeLanguage #lang 
 * 
 * [MZ向けの説明]
 * プラグインコマンド
 * コマンド名「言語の変更」を選び、変更する言語の項目に言語文字を指定します。
 * 指定可能な文字は%{SUPPORT_LANGUAGE_STR}%です。
 * ダブルクォーテーションは含まないで下さい。
 *  
 * @command changeLanguage
 * @text 言語の変更
 * @desc 言語文字を指定します。
 * 
 * @arg language
 * @text 変更する言語
 * @desc 指定可能な文字は%{SUPPORT_LANGUAGE_STR}%です。
 * 
 * @param Default Language
 * @text デフォルト言語
 * @desc ゲーム初回起動時に適用する言語です。ゲームを作成した際の言語を指定してください。
 * @default %{DEFAULT_LANGUAGE}%
 * 
 * @param Must Be Included Image
 * @desc デプロイメント時に「未使用ファイルを含まない」をチェックした際も、必ず含めるファイルを指定します。
 * @default
 * @require 1
 * @dir img/
 * @type file[]
 * 
 * %{REQUIRED_ASSETS}%
 * 
 */

 
%{UNISON_LSCSV}%

var Langscore = class
{


  static isNull(obj){
    return obj === null || obj === undefined;
  }

  
  static isMV(){
    return Utils.RPGMAKER_NAME === 'MV';
  }
  static isMZ(){
    return Utils.RPGMAKER_NAME === 'MZ';
  }


  constructor()
  {
    this._lscsv = new LSCSV();
    this._updateMethods = [];

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

    this.ls_should_throw_for_debug = false;

    if(StorageManager.isLocalMode()){
        this.fs = require('fs');
        this.path = require('path');
        this.basePath = this.path.dirname(process.mainModule.filename);
    }
  }

  handleError(message)
  {
    if (this.ls_should_throw_for_debug) {
      throw new Error(message);
    } else {
        console.error(message);
    }
  }

  isLoadedTranslateFiles()
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

  translate(text, langscore_map, lang = Langscore.langscore_current_language)
  {
    if(Langscore.isNull(langscore_map)){
      this.handleError("Langscore Error(translate): langscore_map is null")
      return text;
    }
    
    var key = text;

    var translatedList = langscore_map[key];
    if(!translatedList){ 
      this.handleError("Langscore Error(translate): not found translatedList")
      return text; 
    }
    var t = translatedList[lang];
    if(t){
      text = t;
    }
    return text;
  }

  translate_for_map(text) 
  {
    if(!this.ls_current_map){ 
      this.handleError("Langscore Error(translate_for_map): Invalid ls_current_map")
      return text; 
    }
    
    var parent = this;
    var currentMapId = 0;
    //会話などで処理中のMapIDが指定されている場合はそちらのIDを使用する。
    if($gameMap._interpreter.isOnCurrentMap() === false){
      currentMapId = $gameMap._interpreter._mapId;
    }
    else{
      currentMapId = $gameMap._mapId;
    }
    var currentMapTranslatedmap = this.ls_current_map[currentMapId];
    if(!currentMapTranslatedmap){ 
      this.handleError(`Langscore Error(translate_for_map): not found currentMapId(${currentMapId})`)
      return text; 
    }

    var translate_result = parent.translate(text, currentMapTranslatedmap);
    if(translate_result !== text){
      return translate_result;
    }
    return text;
  }

  translate_for_script(text){
    return this.translate(text, this.ls_scripts_tr);
  };

  fetch_original_text(transed_text, langscore_map) 
  {
    var origin = transed_text;
    for (const originText of Object.keys(langscore_map)) {
      var transMap = langscore_map[originText];
      for (let transText of Object.values(transMap)) {
        if (transText === transed_text) {
            return originText;
        }
      }
    }
    
    return transed_text;
  }

  translate_list_reset()
  {
    this.ls_actors_tr.clear();
    this.ls_system_tr.clear();
    this.ls_classes_tr.clear();
    this.ls_skills_tr.clear();
    this.ls_states_tr.clear();
    this.ls_weapons_tr.clear();
    this.ls_armors_tr.clear();
    this.ls_items_tr.clear();
    this.ls_enemies_tr.clear();
    this.ls_graphics_tr = this._lscsv.to_map("Graphics")
    this.ls_scripts_tr = this._lscsv.to_map("Scripts")
    this.ls_troops_tr = this._lscsv.to_map("Troops")
    this.ls_common_event = this._lscsv.to_map("CommonEvents")
  
    changeLanguage(Langscore.langscore_current_language)
  }

  changeLanguage(lang, forceUpdate = false)
  {
    if(forceUpdate === false && Langscore.langscore_current_language === lang){
      return;
    }

    var enablePatch = false;
    if(Langscore.EnablePathMode)
    {
      if(StorageManager.isLocalMode() === false)
      {
        throw new Error('LangscoreのパッチモードはWebブラウザではサポートしていません。');
      }
      else{
        enablePatch = true;
      }
    }

    if(!Langscore.Support_Language.includes(lang)){
      return;
    }
    
    Langscore.langscore_current_language = lang;
    this.updatePluginParameters();
    this.updateFont(lang);
  
    this.updateSkills();
    this.updateClasses();
    this.updateStates();
    this.updateEnemies();
    this.updateItems();
    this.updateArmors();
    this.updateWeapons();
    this.updateSystem();
    this.updateActor();
    
    if(enablePatch === true){
      if($gameMap){
        //$gameMap === nullは起動直後に起こり得る。
        this.loadMapData($gameMap.mapId());
      }
    }

    this._updateMethods.forEach(function(method) {
      method();
    });
    this.Langscore_PluginCustom();
    
    this.ls_graphic_cache = {};
    this.ls_graphic_cache.clear
    
    if(Langscore.isFirstLoaded === false){
      //changeLanguageの初回呼び出しはConfigManagerのロード後の、保存されていたlangscore_current_languageを
      //使用して呼び出されるため、ConfigManager.save()は呼び出さない。
      Langscore.isFirstLoaded = true;
    }else{
      ConfigManager.save();
    }
  }

  updateFont(lang) {

    if(Langscore.isNull(Langscore.FontList)){
      return;
    }

    Langscore.currentFont = Langscore.FontList[lang];
    if(!Langscore.currentFont){
      console.error(`Langscore: No font is set for ${lang}. Skip font update.`)
      return;
    }
    var currentFontName = Langscore.currentFont["name"];
    
    if(currentFontName === ""){
      Langscore.currentFont = undefined;
      return;
    }


    if(Langscore.isMV())
    {
      //デフォルトのM+1フォントの場合、GameFontとしてロード&定義済みなのでそちらを使う。
      //M+ 1m regularとするとフォントサイズがやたらと小さくなる現象が起こる。調査しづらいので暫定でこの対処。
      if(currentFontName.toLowerCase() === "m+ 1m regular"){
        Langscore.currentFont.name = "GameFont";
      }

      if(Langscore.currentFont["isLoaded"] === false){
        Graphics.loadFont(currentFontName,`fonts/${Langscore.currentFont.fileName}`)
        Langscore.currentFont["isLoaded"] = true;
      }
    }
    else if(Langscore.isMZ())
    {      
      if(currentFontName.toLowerCase() === "m+ 1m regular"){
        Langscore.currentFont.name = "rmmz-mainfont";
        Langscore.currentFont.fileName = "mplus-1m-regular.woff";
      }
      FontManager.load(Langscore.currentFont.name, Langscore.currentFont.fileName);
    }
  };

  updateForNameAndDesc(data_list, tr_list) 
  {
    const elm_trans =(el) => {
      return this.translate(el, tr_list);
    }
    data_list.forEach(function(obj,i){
      if(data_list[i] === null){ return; }
      data_list[i].name        = elm_trans(_langscore.fetch_original_text(obj.name, tr_list));
      data_list[i].description = elm_trans(_langscore.fetch_original_text(obj.description, tr_list));
    });
  };

  updateForName(data_list, tr_list) 
  {
    const elm_trans =(el) => {
      return this.translate(el, tr_list);
    }
    data_list.forEach(function(obj,i){
      if (data_list[i] === null) { return; }
      data_list[i].name = elm_trans(_langscore.fetch_original_text(obj.name, tr_list));
    });
  };

  updateActor()
  {
    const elm_trans =(el) => {
      return this.translate(el, this.ls_actors_tr);
    }

    if($dataActors === null){
      return;
    }
    //大元のデータベースを更新。Game_Actor作成時に使用されるため必要。
    var _this = this;
    $dataActors.forEach(function(obj,i)
    {
      if($dataActors[i] === null){ return; }
      $dataActors[i].name        = elm_trans(_langscore.fetch_original_text(obj.name, _this.ls_actors_tr));
      $dataActors[i].nickname    = elm_trans(_langscore.fetch_original_text(obj.nickname, _this.ls_actors_tr));
      $dataActors[i].profile     = elm_trans(_langscore.fetch_original_text(obj.profile, _this.ls_actors_tr));
    });
    
    //起動時の初回コールの場合はgameActorsがnullになっている。
    if($gameActors === null){
      return;
    }
    //既にGame_Actorが作成されている場合、インスタンス側も更新。
    //他のデータベースと同様に初期化を行うと、パラメータ値等も全部初期化されるので、名前以外の内容は保持する。
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
      var profile = this.fetch_original_text(actor._profile, this.ls_actors_tr);
      if(profile){
        $gameActors.actor(i)._profile = elm_trans(profile);
      }
    }
  };

  //配列の全要素に対してmodifyFunctionを適用するヘルパー関数
  internal_modifyArray(arr, modifyFunction) 
  {
    arr.forEach((elem, index) => {
      var origin_text = this.fetch_original_text(elem, this.ls_system_tr);
      if(origin_text){
        arr[index] = modifyFunction(origin_text);
      }
    });
    return arr;
  };

  updateSystem()
  { 
    this.internal_modifyArray($dataSystem.terms.params, (el) => el = this.translate(el, this.ls_system_tr) );
    this.internal_modifyArray($dataSystem.terms.commands, (el) => el = this.translate(el, this.ls_system_tr) );
    this.internal_modifyArray($dataSystem.terms.basic, (el) => el = this.translate(el, this.ls_system_tr) );
    Object.keys($dataSystem.terms.messages).forEach(key => {
      var value = $dataSystem.terms.messages[key];
      var origin_text = this.fetch_original_text(value, this.ls_system_tr);
      if(origin_text){  
        $dataSystem.terms.messages[key] = this.translate(origin_text, this.ls_system_tr);
      }
    });

    this.internal_modifyArray($dataSystem.skillTypes, (el) => el = this.translate(el, this.ls_system_tr) )

    $dataSystem.currencyUnit = this.translate($dataSystem.currencyUnit, this.ls_system_tr);

  };

  updateClasses(){
    this.updateForName($dataClasses, this.ls_classes_tr);
  };

  updateSkills(){

    const elm_trans =(el) => {
      return this.translate(el, this.ls_skills_tr);
    };
    $dataSkills.forEach(function(skill,i){
      if($dataSkills[i] === null){ return; }
      $dataSkills[i].name        = elm_trans(_langscore.fetch_original_text(skill.name, _langscore.ls_skills_tr));
      $dataSkills[i].description = elm_trans(_langscore.fetch_original_text(skill.description, _langscore.ls_skills_tr));
      $dataSkills[i].message1    = elm_trans(_langscore.fetch_original_text(skill.message1, _langscore.ls_skills_tr));
      $dataSkills[i].message2    = elm_trans(_langscore.fetch_original_text(skill.message2, _langscore.ls_skills_tr));
    });
  };

  updateStates(){
    
    const elm_trans =(el) => {
      return this.translate(el, this.ls_states_tr);
    };
    $dataStates.forEach(function(state,i){
      if($dataStates[i] === null){ return; }
      $dataStates[i].name      = elm_trans(state.name);
      $dataStates[i].message1  = elm_trans(state.message1);
      $dataStates[i].message2  = elm_trans(state.message2);
      $dataStates[i].message3  = elm_trans(state.message3);
      $dataStates[i].message4  = elm_trans(state.message4);
    });
  };

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

  replaceNestedJSON(jsonData, path, newValue) 
  {
    // "/"区切りで指定されたパスの先の値を書き換えるメソッド
    // 値が文字列の場合
    const keys = path.split('/');
    let current = jsonData;
    const stackJSONValues = [];

    for (let i = 0; i < keys.length - 1; i++) {
      try {
        if (typeof current[keys[i]] === 'string') {
          try {
            current[keys[i]] = JSON.parse(current[keys[i]]);
            //JSONとして解析できたもののみを積んでいく
            stackJSONValues.push(current[keys[i]]);
          } catch (e) {
          }
        }
      } catch(e){
      }
      current = current[keys[i]];
    }

    if(!current){ 
      console.error("Langscore Error", `Script.csvの${path}が解釈できません。Langscore.exeで再度翻訳ファイルを出力してください。`);
      return;
    }
    const lastKey = keys[keys.length - 1];
    current[lastKey] = newValue;

    //積んだJSONのデータを文字列に変換し、
    //ひとつ上のstackの値に置き換える。
    for(let i=keys.length-2; i>0; i--)
    {
      var data = JSON.stringify(stackJSONValues.pop());
      stackJSONValues[stackJSONValues.length-1][keys[i]] = data;
    }

    //ツクールの場合、渡ってくるjsonDataは既に辞書化されているため、
    //最後にpathの先頭のキーで置き換え
    var result = JSON.stringify(stackJSONValues.pop());
    jsonData[keys[0]] = result;

    return jsonData;
  }

  updatePluginParameters()
  {
    if(!this.ls_scripts_tr){ return; }
    var parent = this;
    Object.keys(this.ls_scripts_tr).forEach(function(key){
      var infos = key.split(':');
      if(infos.length <= 1 || 2 < infos.length){ return; }
      var params = PluginManager.parameters(infos[0]);
      if(!params || Object.keys(params).length === 0 && params.constructor === Object){ return; }

      //パスの場合の処理
      if(infos[1].includes("/")){
        var trans = parent.ls_scripts_tr[key];
        if(trans){
            var text = trans[Langscore.langscore_current_language];
            if(text){
                // JSON文字列である可能性があるため、replaceNestedJSONを呼び出す
                params = parent.replaceNestedJSON(params, infos[1], text);
                // 更新されたパラメータを再設定
                PluginManager._parameters[infos[0].toLowerCase()] = params;
            }
        }
      }
      else{
        //通常の文字列の場合の処理
        var param = params[infos[1]];
        var trans = parent.ls_scripts_tr[key];
        if(param && trans){
          var text = trans[Langscore.langscore_current_language];
          if(text){
            PluginManager._parameters[infos[0].toLowerCase()][infos[1]] = text;
          }
        }
      }
    });
  }

  loadSystemDataFile(varName, fileName) {
    var xhr = new XMLHttpRequest();
    var url = 'data/translate/'
    if(Langscore.EnablePathMode){
      url += Langscore.langscore_current_language + '/'
    }
    url += fileName;
    
    var parent = this;
    xhr.open('GET', url);
    xhr.overrideMimeType('text/plain');
    xhr.onload = function() {
      if (xhr.status < 400) {
        parent[varName] = parent._lscsv.to_map(xhr.responseText, varName);
      }
      else{
        parent[varName] = {};
      }
    };
    xhr.onerror = parent._mapLoader || function() {
      //無限ループにさせないようにnullではなく空にしておく。
      parent[varName] = {};
      console.error(`File ${DataManager._errorUrl || url} could not be read.`);
      // Graphics.csv等は存在しない場合があるため、クリティカルなエラーではなくログ表示のみに留める。
      // DataManager._errorUrl = DataManager._errorUrl || url;
    };
    parent[varName] = null;
    xhr.send();
  };

  
  loadMapDataFile(mapID) {
    var xhr = new XMLHttpRequest();
    var url = 'data/translate/'
    
    if(Langscore.EnablePathMode){
      url += Langscore.langscore_current_language + '/'
    }
    url += 'Map%1.csv'.format(mapID.padZero(3));

    var parent = this;
    xhr.open('GET', url);
    xhr.overrideMimeType('text/plain');
    
    xhr.onload = function() {
      if (xhr.status < 400) {
        parent.ls_current_map[mapID] = parent._lscsv.to_map(xhr.responseText, url);
      }
      else{
        parent.ls_current_map[mapID] = {};
      }
    };
    xhr.onerror = parent._mapLoader || function() {
      //無限ループにさせないようにnullではなく空にしておく。
      parent.ls_current_map[mapID] = {};
      console.error(`File ${DataManager._errorUrl || url} could not be read.`);
      // イベントによるマップ遷移時の場合、遷移先マップにメッセージイベントが無くCSVが出力されていない場合があるのでエラーとしない。
      // DataManager._errorUrl = DataManager._errorUrl || url;
    };
    parent.ls_current_map[mapID] = null;
    xhr.send();
  };

  
  loadMapData(mapId)
  {
    if(Langscore.isMV())
      {
        if(mapId > 0){
          var fileName = 'Map%1.csv'.format(mapId.padZero(3));
          this.mapLoader = ResourceHandler.createLoader('data/translate/' + fileName, _langscore.loadMapDataFile.bind(this, mapId));
          this.loadMapDataFile(mapId);
        }
      }
      else if(Langscore.isMZ())
      {
        if(mapId > 0){
          var fileName = 'Map%1.csv'.format(mapId.padZero(3));
          this.loadMapDataFile(mapId);
        }
      }
  }

  registerUpdateMethodAtLanguageUpdate(method) {
    if (typeof method === "function") {
        this._updateMethods.push(method);
    }
  }
  
  Langscore_PluginCustom(){
    //Langscore_Customで上書きされるメソッド
  }

} //class Langscore

_lscsv = new LSCSV();

Langscore.isFirstLoaded = false;

//MV向けのクラス変数定義
Langscore.Langscore_Parameters = PluginManager.parameters('Langscore');
%{SUPPORT_LANGUAGE}%;
Langscore.Default_Language = String(Langscore.Langscore_Parameters['Default Language']);
Langscore.EnablePathMode   = Boolean(Langscore.Langscore_Parameters['Enable Language Patch Mode'] === "true");

%{SUPPORT_FONTS}%

Langscore.langscore_current_language = String(Langscore.Langscore_Parameters['Default Language']);
Langscore.currentFont = Langscore.FontList[Langscore.langscore_current_language];

_langscore = new Langscore();

(function() {
  'use strict';


//MV向けの対応
if(Langscore.isMV())
{
  //モジュールの上書き
  //シーン遷移に関わらない翻訳ファイルは初期化時に読み込み
  //戦闘テスト用は未対応
  var DataManager_isDatabaseLoaded = DataManager.isDatabaseLoaded;
  DataManager.isDatabaseLoaded = function(){
    var result = DataManager_isDatabaseLoaded.call(this);
    if(result){
      if(_langscore.isLoadedTranslateFiles() === false){
        return false;
      }
      _langscore.changeLanguage(Langscore.langscore_current_language, true);
    }
    return result;
  }

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
}

//MZ向けの対応
if(Langscore.isMZ())
{
  PluginManager.registerCommand('Langscore', "changeLanguage", args => {
    _langscore.changeLanguage(args['language']);
  });
  
  var Scene_Boot_onDatabaseLoaded = Scene_Boot.prototype.onDatabaseLoaded;
  Scene_Boot.prototype.onDatabaseLoaded = function(){
    Scene_Boot_onDatabaseLoaded.call(this);
    //Langscoreの読み込みに必要な基本ファイルが読み込まれた後に、初回更新を行う。
    const checkLoadFlag = setInterval(() => {
      if (ImageManager.isReady() && FontManager.isReady() && ConfigManager.isLoaded()) {
        clearInterval(checkLoadFlag);
        _langscore.changeLanguage(Langscore.langscore_current_language, true);
      }
    }, 16);
  }
}


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
  //TODO: ロード時等に二回呼ばれて、動作に問題はないが無駄。
  //      消しても問題ないかが把握しきれていないので、一旦残す。
  _langscore.loadMapData(mapId);
};


//-----------------------------------------------------

Game_System.prototype.isJapanese = function() {
  return Langscore.langscore_current_language ? Langscore.langscore_current_language === "ja" : false;
};

Game_System.prototype.isChinese = function() {
  return Langscore.langscore_current_language ? Langscore.langscore_current_language.match(/^zh/) : false;
};

Game_System.prototype.isKorean = function() {
  return Langscore.langscore_current_language ? Langscore.langscore_current_language === "ko" : false;
};

Game_System.prototype.isCJK = function() {
  return Langscore.langscore_current_language ? Langscore.langscore_current_language.match(/^(ja|zh|ko)/) : false;
};

Game_System.prototype.isRussian = function() {
  return Langscore.langscore_current_language ? Langscore.langscore_current_language === "ru" : false;
};

//名前入力ウィンドウのキャラ名が崩れる不具合への対応
var Window_NameEdit_prototype_charWidth = Window_NameEdit.prototype.charWidth;
Window_NameEdit.prototype.charWidth = function()
{
  //入力中に名前が空になるとアンカーが消えるため、空なら既存処理を呼び出す。(※既存処理は$gameSystem.isJapanese()依存)
  if(this._name == null || this._name == ''){
    return Window_NameEdit_prototype_charWidth.call(this);
  }
  let maxWidth = 0;
  for(let char of this._name)
  {
    let w = this.textWidth(char);
    if(maxWidth < w){
      maxWidth = w;
    }
  }
  return maxWidth;
};

//アクター名の変更
if(Langscore.isMV())
{
  var Game_Interpreter_command320 = Game_Interpreter.prototype.command320;
  Game_Interpreter.prototype.command320 = function() {
    var result = Game_Interpreter_command320.call(this);
    _langscore.updateActor();
    return result;  //戻り値は元のコマンドに合わせること。適切に値が返らないと入力の反映が止まる。
  };

  //二つ名の変更
  var Game_Interpreter_command324 = Game_Interpreter.prototype.command324;
  Game_Interpreter.prototype.command324 = function() {
    var result = Game_Interpreter_command324.call(this);
    _langscore.updateActor();
    return result;
  };

  //プロフィールの変更
  var Game_Interpreter_command325 = Game_Interpreter.prototype.command325;
  Game_Interpreter.prototype.command325 = function() {
    var result = Game_Interpreter_command325.call(this);
    _langscore.updateActor();
    return result; 
  };

  var Window_Base_standardFontFace = Window_Base.prototype.standardFontFace;
  Window_Base.prototype.standardFontFace = function() 
  {
    return Langscore.currentFont ? Langscore.currentFont["name"] : Window_Base_standardFontFace.call(this);
  };

  var Window_Base_standardFontSize = Window_Base.prototype.standardFontSize;
  Window_Base.prototype.standardFontSize = function() {
    return Langscore.currentFont ? Langscore.currentFont["size"] : Window_Base_standardFontSize.call(this);
  };

}
else if(Langscore.isMZ())
{
  var Game_Interpreter_command320 = Game_Interpreter.prototype.command320;
  Game_Interpreter.prototype.command320 = function(params) {
    var result = Game_Interpreter_command320.call(this, params);
    _langscore.updateActor();
    return result;  //戻り値は元のコマンドに合わせること。適切に値が返らないと入力の反映が止まる。
  };

  //二つ名の変更
  var Game_Interpreter_command324 = Game_Interpreter.prototype.command324;
  Game_Interpreter.prototype.command324 = function(params) {
    var result = Game_Interpreter_command324.call(this, params);
    _langscore.updateActor();
    return result;
  };

  //プロフィールの変更
  var Game_Interpreter_command325 = Game_Interpreter.prototype.command325;
  Game_Interpreter.prototype.command325 = function(params) {
    var result = Game_Interpreter_command325.call(this, params);
    _langscore.updateActor();
    return result; 
  };

  var Game_System_prototype_mainFontFace = Game_System.prototype.mainFontFace;
  Game_System.prototype.mainFontFace = function() {
    return Langscore.currentFont ? Langscore.currentFont.name + ", " + $dataSystem.advanced.fallbackFonts : Game_System_prototype_mainFontFace.call(this);
  };
  var Game_System_prototype_mainFontSize = Game_System.prototype.mainFontSize;
  Game_System.prototype.mainFontSize = function() {
    return Langscore.currentFont ? Langscore.currentFont.size : Game_System_prototype_mainFontSize.call(this);
  };

}


var Window_Base_convertEscapeCharacters = Window_Base.prototype.convertEscapeCharacters;
Window_Base.prototype.convertEscapeCharacters = function(text) 
{
  if (text.length === 0) {
    return Window_Base_convertEscapeCharacters.call(this, text);
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

  if(Langscore.isMZ()){
    //MZでは名前ボックスが存在している。
    //名前ボックスの場合でMap側に翻訳文が見つからない場合、Actors.csv側の翻訳文を検索する。
    if(this instanceof Window_NameBox){
      result = _langscore.translate(text, _langscore.ls_actors_tr);
      if(result && result !== text){
        return Window_Base_convertEscapeCharacters.call(this, result);
      }
    }
  }
  
  return Window_Base_convertEscapeCharacters.call(this, text);
}

//セーブを行う際は原文で保存
//プラグインを外した際に変に翻訳文が残ることを避ける。

if(Langscore.isMV())
{
  var DataManager_saveGameWithoutRescue = DataManager.saveGameWithoutRescue;
  DataManager.saveGameWithoutRescue = function(savefileId)
  {

    for (var i = 0; i < $dataActors.length; ++i) {
      var actor = $gameActors.actor(i);
      if (!actor){ continue; }

      let name = _langscore.fetch_original_text(actor._name, _langscore.ls_actors_tr);
      if(name){
        $gameActors.actor(i).setName(name);
      }
      var nickname = _langscore.fetch_original_text(actor._nickname, _langscore.ls_actors_tr);
      if(nickname){
        $gameActors.actor(i).setNickname(nickname);
      }
      var profile = _langscore.fetch_original_text(actor._profile, _langscore.ls_actors_tr);
      if(profile){
        $gameActors.actor(i).setProfile(profile);
      }
    }

    for (var i = 0; i < $dataClasses.length; ++i) {
      
      var classData = $dataClasses[i];
      if(classData){
        var className = _langscore.fetch_original_text(classData.name, _langscore.ls_classes_tr);
        if(className){
          $dataClasses[i].name = className;
        }
      }
    }

    //=================== セーブ本処理 ===================== 
    var result = DataManager_saveGameWithoutRescue.apply(this, arguments);
    //===================================================== 

    //$gameActors等をコピーして再代入する方法を試していたが、コピー前の[]の配列がコピー後にArray{}に変わるなど、
    //元のオブジェクトと完全に一致するディープコピーが上手くいかない。
    //.forEachを行っている箇所が多いため、Array{}には変更したくない。
    //また、$gameActors内プロパティの型自体が変化してしてしまうため、これも論外。
    //さらに、コピー対象のオブジェクト数が多すぎるため、正直updateActor等で部分的に更新した方が速そう。
    _langscore.updateActor();
    _langscore.updateClasses();

    return result;
  };
}
else if(Langscore.isMZ())
{
  var DataManager_saveGame = DataManager.saveGame;
  DataManager.saveGame = function(savefileId)
  {

    for (var i = 0; i < $dataActors.length; ++i) {
      var actor = $gameActors.actor(i);
      if (!actor){ continue; }

      let name = _langscore.fetch_original_text(actor._name, _langscore.ls_actors_tr);
      if(name){
        $gameActors.actor(i).setName(name);
      }
      var nickname = _langscore.fetch_original_text(actor._nickname, _langscore.ls_actors_tr);
      if(nickname){
        $gameActors.actor(i).setNickname(nickname);
      }
      var profile = _langscore.fetch_original_text(actor._profile, _langscore.ls_actors_tr);
      if(profile){
        $gameActors.actor(i).setProfile(profile);
      }
    }

    for (var i = 0; i < $dataClasses.length; ++i) {
      
      var classData = $dataClasses[i];
      if(classData){
        var className = _langscore.fetch_original_text(classData.name, _langscore.ls_classes_tr);
        if(className){
          $dataClasses[i].name = className;
        }
      }
    }

    //=================== セーブ本処理 ===================== 
    var result = DataManager_saveGame.apply(this, arguments);
    //===================================================== 

    //$gameActors等をコピーして再代入する方法を試していたが、コピー前の[]の配列がコピー後にArray{}に変わるなど、
    //元のオブジェクトと完全に一致するディープコピーが上手くいかない。
    //.forEachを行っている箇所が多いため、Array{}には変更したくない。
    //また、$gameActors内プロパティの型自体が変化してしてしまうため、これも論外。
    //さらに、コピー対象のオブジェクト数が多すぎるため、正直updateActor等で部分的に更新した方が速そう。
    _langscore.updateActor();
    _langscore.updateClasses();

    return result;
  };
}

//セーブデータは原文で保存されているため、起動時の言語設定で置き換える。
//これを省くと中国語で起動した際に、再度翻訳を適用するまで日本語のまま……といった事が起きる。
var DataManager_extractSaveContents = DataManager.extractSaveContents;
DataManager.extractSaveContents = function(contents) {
  DataManager_extractSaveContents.call(this, contents);
  _langscore.changeLanguage(Langscore.langscore_current_language, true)
};

var ImageManager_loadBitmap = ImageManager.loadBitmap;
ImageManager.loadBitmap = function(folder_name, filename, hue = 0) 
{    
  if(_langscore.ls_graphic_cache === null || filename === ""){
    return ImageManager_loadBitmap.call(this, folder_name, filename, hue);
  }
  var path = folder_name+filename;
  var ts_path = _langscore.translate(path, _langscore.ls_graphics_tr);
  if(ts_path !== path){
    filename = ts_path.replace(folder_name, "")
  }
  else if(StorageManager.isLocalMode() && (Langscore.langscore_current_language !== Langscore.Default_Language))
  {
    //ブラウザ実行の場合、ファイルの存在確認をする術が無いため、
    //ファイル名による画像切替はデスクトップアプリ実行時のみにする。

    var new_filename = filename + '_' + Langscore.langscore_current_language;
    var has_key = _langscore.ls_graphic_cache[filename];

    if(has_key === undefined)
    {
      var searchPath = _langscore.path.join(_langscore.basePath, folder_name + new_filename + ".png")
      //MVの仕様に合わせてpngのみ対応にする。
      _langscore.ls_graphic_cache[filename] = _langscore.fs.existsSync(searchPath);
    }

    if (_langscore.ls_graphic_cache[filename]) {
      filename = new_filename;
    }
  }

  return ImageManager_loadBitmap.call(this, folder_name, filename, hue);
}


var SceneManager_initialize = SceneManager.initialize;
SceneManager.initialize = function() {
  SceneManager_initialize.call(this);
  _langscore.updatePluginParameters();
};

var Scene_Boot_isReady = Scene_Boot.prototype.isReady;
Scene_Boot.prototype.isReady = function() {
  var result = Scene_Boot_isReady.call(this);
  return result && Langscore.isFirstLoaded;
};


var ConfigManager_makeData = ConfigManager.makeData;
ConfigManager.makeData = function() {
  var config = ConfigManager_makeData.call(this);
  config.currentLanguage = Langscore.langscore_current_language;
  return config;
};


var ConfigManager_applyData = ConfigManager.applyData;
ConfigManager.applyData = function(config) 
{
  Langscore.langscore_current_language = Langscore.Default_Language;
  
  ConfigManager_applyData.apply(this, arguments);
  var lang = config["currentLanguage"];
  if(lang !== undefined && Langscore.Support_Language.includes(lang)){
    Langscore.langscore_current_language = lang;
  }

  //Support_Language以外の言語が指定されると、起動直後にisFirstLoadedがtrueにならずハングする。
  //対応言語を手動編集した場合に発生しうるので未想定の動作だが、念のためのチェック。
  if(!Langscore.Support_Language.includes(Langscore.langscore_current_language)){
    Langscore.langscore_current_language = Langscore.Default_Language;
    //厳重にチェック
    if(!Langscore.Support_Language.includes(Langscore.langscore_current_language)){
      //Support_Languageが空はヤバいのでクラッシュさせる。チェックしない。
      Langscore.langscore_current_language = Langscore.Support_Language[0];
    }
  }
};

})(); //'use strict';
