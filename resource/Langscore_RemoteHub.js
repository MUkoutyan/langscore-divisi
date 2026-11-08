//---------------------------------------------------------------
//
// Langscore DebugScript "RemoteHub"
// Version 1.0.0
// Written by BreezeSinfonia 來奈津
//
//---------------------------------------------------------------
 /*:en
 * @target MV MZ
 * @plugindesc Debug plugin for Langscore. Reloads the translation files while test playing. Place it below Langscore.
 * @author BreezeSinfonia Kunatsu
 * @url https://mukoutyan.github.io/langscore-manual/
 *
 * @help
 * Functions only during test play. 
 * ・This plugin connects to the Langscore app while the game is running, allowing you to apply edited translations. 
 * ・Please remove this plugin before the final product release. 
 *
 * Translations for messages currently on screen will be updated starting from the next message. 
 * Windows, such as those used for menus, will be redrawn after the data is reloaded.
 */
 /*:ja
 * @target MV MZ
 * @plugindesc Langscoreのデバッグ用プラグインです。テストプレイ中に翻訳ファイルの変更を反映します。Langscoreより下に配置して下さい。
 * @author BreezeSinfonia 來奈津
 * @url https://mukoutyan.github.io/langscore-manual/
 *
 * @help
 * テストプレイ中のみ動作します。
 * ・ゲーム実行中にLangscoreアプリと連携し、編集した翻訳文を反映させるためのプラグインです。
 * ・製品リリース時にはこのプラグインは削除して下さい。
 *
 * 表示中のメッセージは次のメッセージから新しい翻訳になります。
 * メニューなどのウィンドウは読み直した後に再描画します。
 */

(function(){
  'use strict';

  if(!Utils.isOptionValid('test') || !Utils.isNwjs()){ return; }
  if(typeof _langscore === 'undefined'){
    console.warn("Langscore_RemoteHub: Langscoreプラグインが見つかりません。Langscoreより下に配置して下さい。");
    return;
  }

  const fs   = require('fs');
  const path = require('path');
  const basePath     = path.dirname(process.mainModule.filename);
  const translateDir = path.join(basePath, 'data', 'translate');
  //Langscoreのプロジェクトはゲームのフォルダーの隣にある。見つからなければゲームのフォルダーに作る
  const langscoreDir = path.join(path.dirname(basePath), path.basename(basePath) + '_langscore');
  const hubDir       = path.join(fs.existsSync(langscoreDir) ? langscoreDir : basePath, 'remote');

  //アプリがゲームの起動中を確かめられるよう、プロセスIDを書いておく
  try
  {
    if(!fs.existsSync(hubDir)){ 
      fs.mkdirSync(hubDir); 
    }
    fs.writeFileSync(path.join(hubDir, 'pid'), String(process.pid));
  }
  catch(e){
    console.warn("Langscore_RemoteHub: failed to write pid", e);
  }

  //データベースの翻訳は、画面に出ている訳文から原文を逆引きして訳し直すため、changeLanguageが必要
  const textOnlyFiles = ['Troops.csv', 'CommonEvents.csv'];

  //---------------------------------------------------------------
  // 翻訳ファイルの読み直し
  //---------------------------------------------------------------

  const readCsv = function(fullPath){
    return _langscore._lscsv.to_map(fs.readFileSync(fullPath, 'utf8'), fullPath);
  };

  //パッチモードでは言語ごとのフォルダのCSVを1つのMapにまとめているため、変わった言語の分だけ上書きする
  const nextMap = function(oldMap, fullPath){
    const newMap = readCsv(fullPath);
    if(!Langscore.EnablePathMode || !(oldMap instanceof Map)){ 
      return newMap; 
    }
    const merged = new Map();
    for(const [key, value] of oldMap){ 
      merged.set(key, new Map(value)); 
    }
    _langscore.mergeMapToMap(merged, newMap);
    return merged;
  };

  //古い訳文からも原文を引けるようにする。引けないと画面上の古い訳文が残る
  const replaceDatabaseMap = function(varName, next)
  {
    const reverse = new Map(_langscore._createReverseMap(_langscore[varName]));
    for(const [text, original] of _langscore._createReverseMap(next)){ 
      reverse.set(text, original); 
    }
    _langscore._reverseOriginalTextCache.set(next, reverse);
    _langscore[varName] = next;
  };

  const reloadFile = function(relativePath){
    const fileName = path.basename(relativePath);
    const fullPath = path.join(translateDir, relativePath);
    if(!fs.existsSync(fullPath)){ return false; }

    const mapMatch = fileName.match(/^Map(\d+)\.csv$/);
    if(mapMatch)
    {
      const mapId = Number(mapMatch[1]);
      //読み込んでいないマップは、移動したときに新しいCSVが読まれる
      if(!(_langscore.ls_current_map[mapId] instanceof Map)){ 
        return false; 
      }
      _langscore.ls_current_map[mapId] = nextMap(_langscore.ls_current_map[mapId], fullPath);
      return false;
    }

    const file = _langscore._databaseFiles.find(f => f.src === fileName);
    if(!file){ return false; }

    if(textOnlyFiles.includes(fileName)){
      _langscore[file.name] = nextMap(_langscore[file.name], fullPath);
      return false;
    }
    replaceDatabaseMap(file.name, nextMap(_langscore[file.name], fullPath));
    return true;
  };

  const refreshWindows = function()
  {
    const scene = SceneManager._scene;
    if(!scene || !scene._windowLayer){ return; }

    scene._windowLayer.children.forEach(function(w)
    {
      if(typeof w.refresh !== 'function' || !w.visible || (w.isOpen && !w.isOpen())){ 
        return; 
      }
      try{ 
        w.refresh(); 
      } catch(e) { 
        console.warn("Langscore_RemoteHub: refresh failed", w.constructor.name, e); 
      }
    });
  };

  const pendingFiles = new Set();
  let reloadTimer = null;

  const applyPendingFiles = function(){
    reloadTimer = null;
    let needChangeLanguage = false;
    for(const relativePath of pendingFiles)
    {
      try{
        needChangeLanguage = reloadFile(relativePath) || needChangeLanguage;
        console.log("Langscore_RemoteHub: reloaded " + relativePath);
      }
      catch(e){
        //書き込み途中のCSVを読んだ場合などは、次の変更で読み直される
        console.warn("Langscore_RemoteHub: failed to reload " + relativePath, e);
      }
    }
    pendingFiles.clear();
    if(needChangeLanguage){
      _langscore.changeLanguage(Langscore.langscore_current_language, true);
    }
    refreshWindows();
  };

  if(fs.existsSync(translateDir))
  {
    fs.watch(translateDir, { recursive: true }, function(eventType, relativePath)
    {
      if(!relativePath || !relativePath.endsWith('.csv')){ return; }
      pendingFiles.add(relativePath);
      //書き出しは複数のファイルを続けて書くため、まとめて読み直す
      clearTimeout(reloadTimer);
      reloadTimer = setTimeout(applyPendingFiles, 300);
    });
  }

  //---------------------------------------------------------------
  // 表示中の文章の書き出し
  //---------------------------------------------------------------

  const translateFileName = function(map)
  {
    const file = _langscore._databaseFiles.find(f => _langscore[f.name] === map);
    if(file){ return file.src; }
    for(const mapId in _langscore.ls_current_map){
      if(_langscore.ls_current_map[mapId] === map){ 
        return 'Map' + String(mapId).padStart(3, '0') + '.csv'; 
      }
    }
    return '';
  };

  //convertEscapeCharacters の中で、最初に原文が見つかった翻訳ファイルを控える
  let capture = null;
  const _translate = _langscore.translate;
  _langscore.translate = function(text, langscore_map, lang)
  {
    const result = _translate.apply(this, arguments);
    if(capture && capture.file === '' && langscore_map instanceof Map && langscore_map.has(text)){
      capture.file = translateFileName(langscore_map);
      capture.translated = result;
    }
    return result;
  };

  const isMessageWindow = function(w){
    return w instanceof Window_Message || w instanceof Window_ChoiceList || w instanceof Window_ScrollText
        || (typeof Window_NameBox !== 'undefined' && w instanceof Window_NameBox);
  };

  let shownTexts = [];
  let writeTimer = null;

  const writeCurrent = function()
  {
    writeTimer = null;
    const data = {
      language: Langscore.langscore_current_language,
      mapId: $gameMap ? $gameMap.mapId() : 0,
      texts: shownTexts
    };
    try
    {
      if(!fs.existsSync(hubDir)){ 
        fs.mkdirSync(hubDir); 
      }
      fs.writeFileSync(path.join(hubDir, 'current.json'), JSON.stringify(data, null, 1));
    }
    catch(e){
      console.warn("Langscore_RemoteHub: failed to write current.json", e);
    }
  };

  const Window_Base_convertEscapeCharacters = Window_Base.prototype.convertEscapeCharacters;
  Window_Base.prototype.convertEscapeCharacters = function(text){
    if(!isMessageWindow(this) || !text){
      return Window_Base_convertEscapeCharacters.call(this, text);
    }
    capture = { window: this.constructor.name, file: '', original: text, translated: '', choice: this instanceof Window_ChoiceList, selected: false };
    const shown = capture;
    let result;
    try{ 
      result = Window_Base_convertEscapeCharacters.call(this, text); 
    }
    finally{ capture = null; }

    //選択肢は幅の計算でも呼ばれるため、同じ文章は1回だけ記録する
    if(!shownTexts.some(t => t.window === shown.window && t.original === shown.original))
    {
      shownTexts.push(shown);
      if(!writeTimer){ writeTimer = setTimeout(writeCurrent, 100); }
    }
    return result;
  };

  //カーソルの当たっている選択肢に印を付ける。カーソルが無ければ外す
  const Window_ChoiceList_select = Window_ChoiceList.prototype.select;
  Window_ChoiceList.prototype.select = function(index)
  {
    Window_ChoiceList_select.call(this, index);
    const original = 0 <= index ? $gameMessage.choices()[index] : undefined;
    let changed = false;
    for(const t of shownTexts)
    {
      const selected = t.choice && t.original === original;
      if(t.selected !== selected){ t.selected = selected; changed = true; }
    }
    if(changed && !writeTimer){ 
      writeTimer = setTimeout(writeCurrent, 100); 
    }
  };

  //---------------------------------------------------------------
  // カーソルの当たっているデータベースの文章
  // データベースは言語を変えたときに訳文で書き換えているため、表示している訳文から原文を引く
  //---------------------------------------------------------------

  //訳文の原文と翻訳ファイルを探す。preferred の翻訳ファイルから先に探す
  const findDatabaseText = function(text, preferred)
  {
    if(!text){ return null; }
    const files = _langscore._databaseFiles
                  .filter(f => f.src === preferred)
                  .concat(_langscore._databaseFiles.filter(f => f.src !== preferred));
    for(const f of files)
    {
      const map = _langscore[f.name];
      if(!(map instanceof Map)){ continue; }

      const original = map.has(text) ? text : _langscore._createReverseMap(map).get(text);
      if(original !== undefined){ 
        return { file: f.src, original: original }; 
      }
    }
    return null;
  };

  //ウィンドウのカーソルの当たっている項目の訳文と、探す翻訳ファイル
  const focusedText = function(w)
  {
    const index = w.index();
    if(index < 0){ return null; }

    //アイテム・スキル・装備の一覧
    if(typeof w.item === 'function')
    {
      const item = w.item();
      if(item && item.name)
      {
        let file = 'Items.csv';
        if(DataManager.isSkill(item)){ file = 'Skills.csv'; }
        else if(DataManager.isWeapon(item)){ file = 'Weapons.csv'; }
        else if(DataManager.isArmor(item)){ file = 'Armors.csv'; }
        return [item.name, file];
      }
    }
    if(typeof w.enemy === 'function')
    {
      const enemy = w.enemy();
      if(enemy){ return [enemy.originalName(), 'Enemies.csv']; }
    }
    let actor = null;

    if(w instanceof Window_MenuStatus){ 
      actor = $gameParty.members()[index]; 
    }else if(w instanceof Window_BattleStatus){ 
      actor = $gameParty.battleMembers()[index]; 
    }
    
    if(actor){ 
      return [actor.name(), 'Actors.csv']; 
    }
    //メニューや戦闘のコマンド
    if(typeof w.commandName === 'function'){ 
      return [w.commandName(index), 'System.csv']; 
    }
    return null;
  };

  //カーソルの当たっている項目が変わったら書き出す。メッセージや選択肢より後に動いたものを優先する
  let focusKey = null;
  const updateFocus = function(scene)
  {
    let found = null;
    const windows = scene && scene._windowLayer ? scene._windowLayer.children : [];
    for(const w of windows)
    {
      if(!(w instanceof Window_Selectable) || w instanceof Window_ChoiceList){ 
        continue; 
      }
      if(!w.active || !w.visible || !w.isOpen()){ continue; }
      
      try{ found = focusedText(w); }
      catch(e){ found = null; }
      
      if(found){ 
        found.push(w.constructor.name); break; 
      }
    }

    const key = found ? found.join('\n') : null;
    if(key === focusKey){ return; }
    focusKey = key;

    shownTexts = shownTexts.filter(t => !t.focus);
    const db = found ? findDatabaseText(found[0], found[1]) : null;
    if(db){
      shownTexts.forEach(t => { t.selected = false; });
      shownTexts.push({ 
        window:     found[2], 
        file:       db.file, 
        original:   db.original, 
        translated: found[0], 
        choice: false, selected: true, focus: true 
      });
    }
    if(!writeTimer){ writeTimer = setTimeout(writeCurrent, 100); }
  };

  const Scene_Base_update = Scene_Base.prototype.update;
  Scene_Base.prototype.update = function(){
    Scene_Base_update.call(this);
    updateFocus(this);
  };

  //新しいメッセージが始まったら、前の画面の文章は消す
  const Window_Message_startMessage = Window_Message.prototype.startMessage;
  Window_Message.prototype.startMessage = function(){
    shownTexts = [];
    focusKey = null;
    Window_Message_startMessage.call(this);
  };
})();
