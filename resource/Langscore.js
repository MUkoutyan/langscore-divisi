//---------------------------------------------------------------
//
// Langscore CoreScript "Unison"
// Version 0.10.3
// Written by BreezeSinfonia 來奈津
//
// 注意：このスクリプトは自動生成されました。編集は非推奨です。
//---------------------------------------------------------------
 /*:en
 * @target MV MZ
 * @plugindesc Translate plugin Langscore for RPG Tskool MV/MZ.
 * @author BreezeSinfonia Kunatsu
 * @url https://mukoutyan.github.io/langscore-manual/
 * 
 * @help
 * Note: This script is auto-generated, so editing is deprecated.
 * If you want to change supported languages.exe., please edit it using Langscore.exe.
 * 
 * [description for MV]
 * Plug-in commands
 *
 * [Change language].
 * Specify language character for #lang. The character that can be specified is %{SUPPORT_LANGUAGE_STR}%.
 * Do not include double quotation marks.
 * Langscore changeLanguage #lang
 *
 * [Display language change menu]
 * langscore displayLanguageMenu
 *
 * [Description for MZ] 
 * langscore changeLanguage
 * Plugin commands
 *
 * [Change Language]
 * Select the command name “Change Language” and specify the language character in the field for the language to be changed.
 * The character that can be specified is %{SUPPORT_LANGUAGE_STR}%.
 * Do not include double quotation marks.
 *
 * [Show language change menu item]
 * Select the command name “Show Language Change Menu”.
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
 * @param Enable Translation For Default Language
 * @text Enable translation for default language
 * @desc Choose whether to search the translation CSV when the default language is selected.
 * @type boolean
 * @on Enable translation
 * @off Disable translation
 * @default true
 * 
 * @param Enable Language Patch Mode
 * @type boolean
 * @on Enable language patch mode
 * @off Disable language patch mode
 * @default false
 * 
 * @param Language State Variable
 * @desc When using language patch mode, specify the first variable to store language availability. Ensure at least 10 free variables from this point. Languages are stored in order: %{ALLOWED_LANGUAGE}%.
 * @type variable
 * @default -1
 * 
 * @param Must Be Included Image
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
 * @url https://mukoutyan.github.io/langscore-manual/
 * 
 * @help
 * 注意：このスクリプトは自動生成されています。Langscore.exeを使用すると更新されるため、編集は非推奨です。
 * サポート言語等を変更する場合、Langscore.exeを使用して編集してください。
 * 
 * [MV向けの説明]
 * プラグインコマンド
 * 
 * 【言語の変更】
 * #langに言語文字を指定します。指定可能な文字は%{SUPPORT_LANGUAGE_STR}%です。
 * ダブルクォーテーションは含まないで下さい。
 * Langscore changeLanguage #lang 
 * 
 * 【言語変更メニューの表示】
 * langscore displayLanguageMenu
 * 
 * [MZ向けの説明]
 * プラグインコマンド
 * 
 * 【言語の変更】
 * コマンド名「言語の変更」を選び、変更する言語の項目に言語文字を指定します。
 * 指定可能な文字は%{SUPPORT_LANGUAGE_STR}%です。
 * ダブルクォーテーションは含まないで下さい。
 * 
 * 【言語変更メニューの表示】
 * コマンド名「言語変更メニューの表示」を選びます。
 *  
 * @command changeLanguage
 * @text 言語の変更
 * @desc 言語文字を指定します。
 * 
 * @arg language
 * @text 変更する言語
 * @desc 指定可能な文字は%{SUPPORT_LANGUAGE_STR}%です。
 * 
 * @command displayLanguageMenu
 * @text 言語変更メニューの表示
 * @desc 言語変更メニューを表示します。
 * 
 * @param Default Language
 * @text デフォルト言語
 * @desc ゲーム初回起動時に適用する言語です。ゲームを作成した際の言語を指定してください。
 * @default %{DEFAULT_LANGUAGE}%
 * 
 * @param Enable Translation For Default Language
 * @text デフォルト言語時に翻訳処理を行う
 * @desc デフォルト言語が選択されている場合、翻訳CSVから検索をするかどうかを選択できます。
 * @type boolean
 * @on 翻訳処理を行う
 * @off 翻訳処理を行わない
 * @default true
 * 
 * @param Enable Language Patch Mode
 * @text 言語パッチモードを有効にする
 * @desc パッチモードは言語毎に翻訳CSVを用意する方法です。翻訳CSVの読み込み場所が変わるため注意して下さい。
 * @type boolean
 * @on 有効
 * @off 無効
 * @default false
 * 
 * @param Language State Variable
 * @text 言語の利用可否フラグを格納する先頭変数
 * @desc 言語パッチモード使用時に、言語が選択可能かを格納する先頭の変数を指定して下さい。変数は指定した箇所から10個以上の空きを設けて下さい。格納される言語は上から順に%{ALLOWED_LANGUAGE}%となります。
 * @type variable
 * @default -1
 * 
 * @param Must Be Included Image
 * @text デプロイメントしても必ず含めるファイル
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
    
    this.ls_actors_tr     = null;
    this.ls_system_tr     = null;
    this.ls_classes_tr    = null;
    this.ls_skills_tr     = null;
    this.ls_states_tr     = null;
    this.ls_weapons_tr    = null;
    this.ls_armors_tr     = null;
    this.ls_items_tr      = null;
    this.ls_enemies_tr    = null;
    this.ls_graphics_tr   = null;
    this.ls_scripts_tr    = null;
    this.ls_troops_tr     = null;
    this.ls_common_event  = null;
    
    this.ls_current_map = new Map;
    this.ls_graphic_cache = {};

    this._reverseOriginalTextCache = new Map;
    this.ls_should_throw_for_debug = false;

    this.current_language_list = []

    if(StorageManager.isLocalMode()){
      if(!this.fs) {this.fs = require('fs'); }
      if(!this.path) {this.path = require('path'); }
      if(!this.basePath) {this.basePath = this.path.dirname(process.mainModule.filename); }
    }

    this.updateTranslateLanguageList();
  }

  updateTranslateLanguageList()
  {
    if(Langscore.EnablePathMode && StorageManager.isLocalMode()){

      // セキュリティ強化: パスの正規化と検証
      const translatePath = this.path.resolve(this.path.join(this.basePath, 'data', 'translate'));
      
      // ベースパス外へのアクセスを防ぐ
      if (!translatePath.startsWith(this.path.resolve(this.basePath))) {
        console.error('Langscore Security Error: Invalid path detected');
        return;
      }

      try {
        const items = this.fs.readdirSync(translatePath);
        for (const item of items) {
          // ファイル名の検証
          if (!this.isValidLanguageFolder(item)) {
            continue;
          }
          
          const itemPath = this.path.resolve(this.path.join(translatePath, item));
          
          // パストラバーサル攻撃を防ぐ
          if (!itemPath.startsWith(translatePath)) {
            console.warn(`Langscore Security Warning: Suspicious path detected: ${item}`);
            continue;
          }
          
          if (this.fs.statSync(itemPath).isDirectory() === false) {
            continue;
          }
          if(Langscore.Support_Language.includes(item)){
            this.current_language_list.push(item);
          }
          else{
            console.log('Langscore Warning: サポートされていない言語フォルダ' + item +'が含まれています。:');
          }
        }
      } catch (error) {
        console.log('Langscore Error: Failed to read translate folder:', error);
      }

      //this.current_language_listを空にするとcsvの読み込みリクエストすら送られない。
      //そのため、ls_~系の変数がnullになり続けるため、起動チェックが常にfalseとなり初回ロード時に無限ループとなる。
      //回避するためにcurrent_language_listはデフォルト言語を埋める。意味合いとしても問題はない。
      if(!this.current_language_list.includes(Langscore.Default_Language)){
        this.current_language_list.push(Langscore.Default_Language)
      }

    }
    else{
      this.current_language_list = Langscore.Support_Language;
    }
  }

  // セキュリティ検証メソッドを追加
  isValidLanguageFolder(folderName) {
    // 許可する文字のみ（英数字、ハイフン、アンダースコア）
    const validPattern = /^[a-zA-Z0-9_-]+$/;
    
    // 長さ制限
    if (folderName.length > 10) {
      return false;
    }
    
    // パターンマッチング
    if (!validPattern.test(folderName)) {
      return false;
    }
    
    // 危険な文字列の除外
    const dangerousPatterns = ['..', '.', 'CON', 'PRN', 'AUX', 'NUL'];
    if (dangerousPatterns.some(pattern => folderName.toUpperCase().includes(pattern))) {
      return false;
    }
    
    return true;
  }

  // 言語コードの厳密な検証
  isValidLanguageCode(lang) {
    //以下の言語のみを許容。
    return Langscore.System_Allowed_Languages.includes(lang);
  }

  handleError(message)
  {
    if (this.ls_should_throw_for_debug) {
      throw new Error(message);
    } else {
      //console.error(message);
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
    if((langscore_map instanceof Map) === false){
      this.handleError("Langscore Error(translate): langscore_map is not map")
      return text;
    }
    
    var key = text;

    var translatedList = langscore_map.get(key);
    if(!translatedList){ 
      this.handleError("Langscore Error(translate): not found translatedList")
      return text; 
    }
    var t = translatedList.get(lang);
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
    if(!transed_text){
      //特に空文字の場合はCSVが不完全だと、最初に見つけた空文字を検出してしまい、
      //異なる行の内容が返される問題がある。空文字は処理しない。
      return transed_text;
    }

    // Mapの場合
    if (langscore_map instanceof Map) {
      for (const [originText, transMap] of langscore_map.entries()) {
        if (transMap instanceof Map) {
          for (const [lang, transText] of transMap.entries()) {
            if (transText === transed_text) {
              return originText;
            }
          }
        }
      }
    }
    // Objectの場合
    else if (typeof langscore_map === "object" && langscore_map !== null) {
      for (const originText of Object.keys(langscore_map)) {
        const transMap = langscore_map[originText];
        if (typeof transMap === "object" && transMap !== null) {
          for (const lang of Object.keys(transMap)) {
            if (transMap[lang] === transed_text) {
              return originText;
            }
          }
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

    //未サポートの言語なら弾く。
    //未サポートの言語はパッチモードを使用している場合に渡されうる。
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

    window._LangscoreObserverQueue.forEach(function(method) {
      method(Langscore.langscore_current_language);
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

  //原文取得用のキャッシュを追加
  _createReverseMap(tr_list) 
  {
    const reverseMap = new Map();
    if(!tr_list || !(tr_list instanceof Map)){ return reverseMap; }
    
    if (this._reverseOriginalTextCache.has(tr_list)) {
      return this._reverseOriginalTextCache.get(tr_list);
    }
    
    // 逆引きマップを構築
    for (const [originText, transMap] of tr_list.entries()) {
      for (const [lang, transText] of transMap.entries()) {
        if (transText) {
          reverseMap.set(transText, originText);
        }
      }
    }
    
    this._reverseOriginalTextCache.set(tr_list, reverseMap);
    return reverseMap;
  }

  updateForNameAndDesc(data_list, tr_list) 
  {
    if (!data_list || !tr_list){ return; }

    // 処理時間計測開始
    // const startTime = performance.now();
    // console.log('[Langscore] updateForNameAndDesc 開始 - データ数:', data_list ? data_list.length : 0);

    const reverseMap = this._createReverseMap(tr_list);
    
    for (let i = 0; i < data_list.length; i++) {
      if (data_list[i] === null){ continue; }
      
      const obj = data_list[i];
      
      // 直接処理
      if (obj.name) {
        const originalName = reverseMap.get(obj.name) || obj.name;
        obj.name = this.translate(originalName, tr_list);
      }
      
      if (obj.description) {
        const originalDesc = reverseMap.get(obj.description) || obj.description;
        obj.description = this.translate(originalDesc, tr_list);
      }
      
      if (obj.note) {
        const originalNote = reverseMap.get(obj.note) || obj.note;
        obj.note = this.translate(originalNote, tr_list);
      }
    }

    this.updateMetaData(data_list);
    
    // 処理時間計測終了
    // const endTime = performance.now();
    // const processingTime = endTime - startTime;
    // console.log('[Langscore] updateForNameAndDesc 完了 - 処理時間:', processingTime.toFixed(2), 'ms');

  };

  updateForName(data_list, tr_list) 
  {
    const reverseMap = this._createReverseMap(tr_list);
    for (let i = 0; i < data_list.length; i++) 
    {
      if (data_list[i] === null){ continue; }
      const obj = data_list[i];

      if (obj.name) {
        const originalName = reverseMap.get(obj.name) || obj.name;
        obj.name = this.translate(originalName, tr_list);
      }
      if (obj.note) {
        const originalNote = reverseMap.get(obj.note) || obj.note;
        obj.note = this.translate(originalNote, tr_list);
      }
    }
  };

  updateActor()
  {
    if($dataActors === null){
      return;
    }
    
    const reverseMap = this._createReverseMap(this.ls_actors_tr);

    //大元のデータベースを更新。Game_Actor作成時に使用されるため必要。
    for (let i = 0; i < $dataActors.length; i++) 
    {
      if ($dataActors[i] === null){ continue; }
      const obj = $dataActors[i];

      if (obj.name) {
        const originalName = reverseMap.get(obj.name) || obj.name;
        obj.name = this.translate(originalName, this.ls_actors_tr);
      }
      
      if (obj.nickname) {
        const originalNickname = reverseMap.get(obj.nickname) || obj.nickname;
        obj.nickname = this.translate(originalNickname, this.ls_actors_tr);
      }
      
      if (obj.profile) {
        const originalProfiler = reverseMap.get(obj.profile) || obj.profile;
        obj.profile = this.translate(originalProfiler, this.ls_actors_tr);
      }
    }
    
    //起動時の初回コールの場合はgameActorsがnullになっている。
    if($gameActors === null){
      return;
    }
    //既にGame_Actorが作成されている場合、インスタンス側も更新。
    //他のデータベースと同様に初期化を行うと、パラメータ値等も全部初期化されるので、名前以外の内容は保持する。
    for (var i = 0; i < $dataActors.length; ++i) {
      var actor = $gameActors.actor(i);
      if (!actor){ continue; }

      let name = reverseMap.get(actor._name);
      if(name){
        $gameActors.actor(i)._name = this.translate(name, this.ls_actors_tr);
      }
      var nickname = reverseMap.get(actor._nickname);
      if(nickname){
        $gameActors.actor(i)._nickname = this.translate(nickname, this.ls_actors_tr);
      }
      var profile = reverseMap.get(actor._profile);
      if(profile){
        $gameActors.actor(i)._profile = this.translate(profile, this.ls_actors_tr);
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
    this.internal_modifyArray($dataSystem.weaponTypes, (el) => el = this.translate(el, this.ls_system_tr) );
    this.internal_modifyArray($dataSystem.equipTypes, (el) => el = this.translate(el, this.ls_system_tr) );
    this.internal_modifyArray($dataSystem.elements, (el) => el = this.translate(el, this.ls_system_tr) );
    this.internal_modifyArray($dataSystem.armorTypes, (el) => el = this.translate(el, this.ls_system_tr) );
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

  updateSkills()
  {
    const reverseMap = this._createReverseMap(this.ls_skills_tr);
    for (let i = 0; i < $dataSkills.length; i++) 
    {
      if ($dataSkills[i] === null){ continue; }
      const obj = $dataSkills[i];

      if (obj.name) {
        const origin = reverseMap.get(obj.name) || obj.name;
        obj.name = this.translate(origin, this.ls_skills_tr);
      }
      if (obj.description) {
        const origin = reverseMap.get(obj.description) || obj.description;
        obj.description = this.translate(origin, this.ls_skills_tr);
      }
      if (obj.message1) {
        const origin = reverseMap.get(obj.message1) || obj.message1;
        obj.message1 = this.translate(origin, this.ls_skills_tr);
      }
      if (obj.message2) {
        const origin = reverseMap.get(obj.message2) || obj.message2;
        obj.message2 = this.translate(origin, this.ls_skills_tr);
      }
      if (obj.note) {
        const origin = reverseMap.get(obj.note) || obj.note;
        obj.note = this.translate(origin, this.ls_skills_tr);
      }
    }
  };

  updateStates()
  {
    const reverseMap = this._createReverseMap(this.ls_states_tr);
    for (let i = 0; i < $dataStates.length; i++) 
    {
      if ($dataStates[i] === null){ continue; }
      const obj = $dataStates[i];

      if (obj.name) {
        const origin = reverseMap.get(obj.name) || obj.name;
        obj.name = this.translate(origin, this.ls_states_tr);
      }
      if (obj.message1) {
        const origin = reverseMap.get(obj.message1) || obj.message1;
        obj.message1 = this.translate(origin, this.ls_states_tr);
      }
      if (obj.message2) {
        const origin = reverseMap.get(obj.message2) || obj.message2;
        obj.message2 = this.translate(origin, this.ls_states_tr);
      }
      if (obj.message3) {
        const origin = reverseMap.get(obj.message3) || obj.message3;
        obj.message3 = this.translate(origin, this.ls_states_tr);
      }
      if (obj.message4) {
        const origin = reverseMap.get(obj.message4) || obj.message4;
        obj.message4 = this.translate(origin, this.ls_states_tr);
      }
      if (obj.note) {
        const origin = reverseMap.get(obj.note) || obj.note;
        obj.note = this.translate(origin, this.ls_states_tr);
      }
    }
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

  updateMetaData(object)
  {
    //DataManager.onLoadから抜粋
    //一部の処理は不要なため除外
    var array;
    if (object === $dataMap) {
        DataManager.extractMetadata(object);
        array = object.events;
    } else {
        array = object;
    }
    if (Array.isArray(array)) {
        for (var i = 0; i < array.length; i++) {
            var data = array[i];
            if (data && data.note !== undefined) {
                DataManager.extractMetadata(data);
            }
        }
    }
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
    for (const [key, trans] of Object.entries(this.ls_scripts_tr)) {
      var infos = key.split(':');
      if(infos.length <= 1 || 2 < infos.length){ continue; }
      var params = PluginManager.parameters(infos[0]);
      if(!params || Object.keys(params).length === 0 && params.constructor === Object){ continue; }

      //パスの場合の処理
      if(infos[1].includes("/")){
        if(trans){
            var text = trans.get(Langscore.langscore_current_language);
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
        if(param && trans){
          var text = trans.get(Langscore.langscore_current_language);
          if(text){
            PluginManager._parameters[infos[0].toLowerCase()][infos[1]] = text;
          }
        }
      }
    }
  }

  // Map同士のマージ処理を関数化
  mergeMapToMap(existingMap, newMap) {
    for (const [key, value] of newMap) {
      if (!existingMap.has(key)) {
        // 新しいキーの場合は追加
        existingMap.set(key, value);
        continue;
      }

      // 既存のキーがある場合の処理
      const existingValue = existingMap.get(key);
      if (existingValue instanceof Map && value instanceof Map) {
        // 両方ともMapの場合は内部要素をマージ
        for (const [innerKey, innerValue] of value) {
          existingValue.set(innerKey, innerValue);
        }
      } else {
        // Mapでない場合は上書き
        existingMap.set(key, value);
      }
    }
  }

  // ObjectからMapへのマージ処理を関数化
  mergeObjectToMap(existingMap, newObject) {
    Object.keys(newObject).forEach(key => {
      if (!existingMap.has(key)) {
        // 新しいキーの場合は追加
        existingMap.set(key, newObject[key]);
        return;
      }

      // 既存のキーがある場合の処理
      const existingValue = existingMap.get(key);
      if (existingValue instanceof Map) {
        Object.keys(newObject[key]).forEach(innerKey => {
          existingValue.set(innerKey, newObject[key][innerKey]);
        });
      } else {
        existingMap.set(key, newObject[key]);
      }
    });
  }

  loadSystemDataFile(varName, fileName, langFolder = "") 
  {
    var xhr = new XMLHttpRequest();
    var url = 'data/translate/'

    if(langFolder !== ""){
      url += (langFolder + "/")
    }

    // パス正規化とエスケープ処理
    const sanitizedFileName = this.sanitizeFileName(fileName);
    url += sanitizedFileName;
  
    
    var parent = this;
    xhr.open('GET', url);
    xhr.overrideMimeType('text/plain');
    xhr.onload = function() 
    {
      // エラーレスポンスの場合は早期リターン
      if (xhr.status >= 400) {
        parent[varName] = {};
        return;
      }

      const newMap = parent._lscsv.to_map(xhr.responseText, varName);
      
      // 初回の場合は新しいMapを代入して終了
      if (!(parent[varName] instanceof Map)) {
        parent[varName] = newMap;
        return;
      }

      // 既存のMapが存在する場合のマージ処理
      if (newMap instanceof Map) {
        parent.mergeMapToMap(parent[varName], newMap);
      } else {
        parent.mergeObjectToMap(parent[varName], newMap);
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

  isValidFileName(fileName) {
    // ファイル名の基本検証
    if (!fileName || typeof fileName !== 'string') {
      return false;
    }
    
    // 危険な文字の除外
    const dangerousChars = /[<>:"|?*\x00-\x1f]/;
    if (dangerousChars.test(fileName)) {
      return false;
    }
    
    // パストラバーサルの防止
    if (fileName.includes('..') || fileName.includes('/') || fileName.includes('\\')) {
      return false;
    }
    
    // 拡張子の検証
    if (!fileName.endsWith('.csv')) {
      return false;
    }
    
    return true;
  }

  sanitizeFileName(fileName) {
    // ファイル名のサニタイズ
    return fileName.replace(/[^a-zA-Z0-9._-]/g, '');
  }
  
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
    
    xhr.onload = function() 
    {
            // エラーレスポンスの場合は早期リターン
      if (xhr.status >= 400) {
        parent.ls_current_map[mapID] = {};
        return;
      }

      const newMap = parent._lscsv.to_map(xhr.responseText, url);
      
      // 初回の場合は新しいMapを代入して終了
      if (!(parent.ls_current_map[mapID] instanceof Map)) {
        parent.ls_current_map[mapID] = newMap;
        return;
      }

      // 既存のMapが存在する場合のマージ処理
      if (newMap instanceof Map) {
        parent.mergeMapToMap(parent.ls_current_map[mapID], newMap);
      } else {
        parent.mergeObjectToMap(parent.ls_current_map[mapID], newMap);
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

  updateLanguageStateVariables()
  {
    //$gameVariablesの初期化後に実行したいため、
    //新規ゲーム時のDataManager.setupNewGame内または
    //ロード直後のDataManager.extractSaveContentsで呼び出す。
    if(!Langscore.Language_StateStartVariable){
      return;
    }
    
    const startVarId = parseInt(Langscore.Language_StateStartVariable);
    if(!startVarId || startVarId !== -1)
    {
      // 各言語の状態を変数に格納
      Langscore.System_Allowed_Languages.forEach((lang, index) => {
        const varId = startVarId + index;
        let isAvailable = 0; // デフォルトは利用不可
        
        // パッチモード時：フォルダが存在するかチェック
        if(this.current_language_list && this.current_language_list.includes(lang)){
          isAvailable = 1;
        }
        // 非パッチモード時：サポート言語に含まれているかチェック
        else if(!Langscore.EnablePathMode && Langscore.Support_Language.includes(lang)){
          isAvailable = 1;
        }
        
        // 変数に値を設定（ゲーム開始前の場合は$dataSystemを使用）
        if($dataSystem && $dataSystem.variables && varId < $dataSystem.variables.length){
          if($gameVariables){
            $gameVariables.setValue(varId, isAvailable);
          }
        }
      });
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
Langscore.System_Allowed_Languages = [%{ALLOWED_LANGUAGE}%];
%{SUPPORT_LANGUAGE}%;
Langscore.Default_Language = String(Langscore.Langscore_Parameters['Default Language']);
Langscore.EnablePathMode   = Boolean(Langscore.Langscore_Parameters['Enable Language Patch Mode'] === "true");

%{SUPPORT_FONTS}%;

Langscore.Language_StateStartVariable = Langscore.Langscore_Parameters['Language State Variable']
Langscore.Enable_Translation_For_DefLang = Langscore.Langscore_Parameters['Enable Translation For Default Language']

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
          else if(args[0].toUpperCase() === 'DISPLAYLANGUAGEMENU'){
            SceneManager.push(Scene_LanguageSelect);
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
  PluginManager.registerCommand('Langscore', "displayLanguageMenu", args => {
    SceneManager.push(Scene_LanguageSelect);
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


var DataManager_setupNewGame = DataManager.setupNewGame;
DataManager.setupNewGame = function() 
{
  DataManager_setupNewGame.call(this);
  _langscore.updateLanguageStateVariables();
}

var DataManager_loadDatabase = DataManager.loadDatabase;
DataManager.loadDatabase = function() 
{
  DataManager_loadDatabase.call(this);

  if(Langscore.EnablePathMode && StorageManager.isLocalMode())
  {
    _langscore.current_language_list.forEach(lang => 
    {
  for (var i = 0; i < _langscore._databaseFiles.length; i++) {
    var varName = _langscore._databaseFiles[i].name;
    var fileName = _langscore._databaseFiles[i].src;
        
        _langscore.loadSystemDataFile(varName, fileName, lang);
      }
    })
  }
  else
  {
    for (var i = 0; i < _langscore._databaseFiles.length; i++) {
      var varName = _langscore._databaseFiles[i].name;
      var fileName = _langscore._databaseFiles[i].src;
      
      _langscore.loadSystemDataFile(varName, fileName);
    }
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
  //デフォルト言語での翻訳処理が無効かつデフォルト言語なら、翻訳処理を無視。
  if(Langscore.Enable_Translation_For_DefLang === false && Langscore.langscore_current_language === Langscore.Default_Language){
    return Window_Base_convertEscapeCharacters.call(this, text);
  }

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
  _langscore.updateLanguageStateVariables();
  _langscore.changeLanguage(Langscore.langscore_current_language, true)
};

var ImageManager_loadBitmap = ImageManager.loadBitmap;
ImageManager.loadBitmap = function(folder_name, filename, hue = 0) 
{    
  if(Langscore.Enable_Translation_For_DefLang === false && Langscore.langscore_current_language === Langscore.Default_Language){
    return ImageManager_loadBitmap.call(this, folder_name, filename, hue);
  }

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

//-----------------------------------------------------------------------------
// Window_LanguageSelect
//
// 言語選択用のコマンドウィンドウクラス
//-----------------------------------------------------------------------------

function Window_LanguageSelect() {
    this.initialize.apply(this, arguments);
}

Window_LanguageSelect.prototype = Object.create(Window_Command.prototype);
Window_LanguageSelect.prototype.constructor = Window_LanguageSelect;

Window_LanguageSelect.prototype.initialize = function() {
    this.clearCommandList();
    this._languageList = [];
    this.last_selected_language = Langscore.langscore_current_language;
    this.makeLanguageList();

    // 現在の言語に応じた文言を設定
    this.setupLanguageTexts();

    var width = this.windowWidth();
    var height = this.windowHeight();

    if(Langscore.isMV()){
    Window_Command.prototype.initialize.call(this, 0, 0);
    }
    else if(Langscore.isMZ()){
      const rect = { x: 0, y: 0, width: width, height: height };
      Window_Command.prototype.initialize.call(this, rect);
    }
    
    width = this.windowWidth();
    height = this.windowHeight();
    var x = (Graphics.boxWidth - width) / 2;
    var y = (Graphics.boxHeight - height) / 2;
    this.move(x, y, width, height);
    
    this.selectCurrentLanguage();
    this.activate();
};


Window_LanguageSelect.prototype.setupLanguageTexts = function() {
  // 各言語の文言を定義
  this._texts = {
    'ja': {
        title: '言語選択',
        ok: '確定',
        cancel: 'キャンセル'
    },
    'en': {
        title: 'Language Select',
        ok: 'OK',
        cancel: 'Cancel'
    },
    'zh-cn': {
        title: '语言选择',
        ok: '确定',
        cancel: '取消'
    },
    'zh-tw': {
        title: '語言選擇',
        ok: '確定',
        cancel: '取消'
    },
    'ko': {
        title: '언어 선택',
        ok: '확인',
        cancel: '취소'
    },
    'fr': {
        title: 'Sélection de langue',
        ok: 'OK',
        cancel: 'Annuler'
    },
    'de': {
        title: 'Sprachauswahl',
        ok: 'OK',
        cancel: 'Abbrechen'
    },
    'es': {
        title: 'Selección de idioma',
        ok: 'OK',
        cancel: 'Cancelar'
    },
    'ru': {
        title: 'Выбор языка',
        ok: 'OK',
        cancel: 'Отмена'
    }
  };
  
  // 現在の言語の文言を取得（存在しない場合は日本語をデフォルト）
  this._currentTexts = this._texts[Langscore.langscore_current_language] || this._texts['ja'];
};

Window_LanguageSelect.prototype.windowWidth = function() {
    return 480;
};

Window_LanguageSelect.prototype.windowHeight = function() {
    return this.fittingHeight(Math.min(this.maxItems(), 12)); // +3はタイトル、区切り線、確定/キャンセル用
};


if(Langscore.isMZ())
{
Window_LanguageSelect.prototype.contentsHeight = function() {
    return this.fittingHeight(Math.min(this.maxItems(), 12)); // +3はタイトル、区切り線、確定/キャンセル用
};
}

Window_LanguageSelect.prototype.makeLanguageList = function() {
    this._languageList = [];
    
    // current_language_listが存在する場合はそれを使用
    if (_langscore && _langscore.current_language_list && _langscore.current_language_list.length > 0) {
        this._languageList = _langscore.current_language_list.slice();
    } else {
        // フォールバック: Support_Languageを使用
        this._languageList = Langscore.Support_Language.slice();
    }
};

Window_LanguageSelect.prototype.makeCommandList = function() {
    // タイトル（選択不可）
    this.addCommand(this._currentTexts.title, 'title', false);
    
    // 区切り線（選択不可）
    this.addCommand('──────────────────', 'separator', false);
    
    // 言語リスト
    for (var i = 0; i < this._languageList.length; i++) {
        var langCode = this._languageList[i];
        var displayName = this.getLanguageDisplayName(langCode);
        var enabled = _langscore.isValidLanguageCode(langCode);
        
        // 現在選択中の言語をマーク
        if (langCode === this.last_selected_language) {
            displayName = '► ' + displayName + ' ◄';
        }
        
        this.addCommand(displayName, 'language', enabled, langCode);
    }
    
    // 区切り線（選択不可）
    this.addCommand('──────────────────', 'separator2', false);
    
    // 確定・キャンセルコマンド
    this.addCommand(this._currentTexts.ok, 'ok', true);
    this.addCommand(this._currentTexts.cancel, 'cancel', true);
};

Window_LanguageSelect.prototype.getLanguageDisplayName = function(langCode) {
    // 言語コードを表示名に変換
    var languageNames = {
        'ja': '日本語',
        'en': 'English',
        'zh-cn': '简体中文',
        'zh-tw': '繁體中文',
        'ko': '한국어',
        'fr': 'Français',
        'de': 'Deutsch',
        'es': 'Español',
        'ru': 'Русский'
    };
    
    return languageNames[langCode] || langCode;
};

Window_LanguageSelect.prototype.selectCurrentLanguage = function() {
    
    // 言語コマンドの中から現在の言語を探す
    for (var i = 0; i < this._list.length; i++) {
        if (this._list[i].symbol === 'language' && this._list[i].ext === this.last_selected_language) {
            this.select(i);
            return;
        }
    }
    
    // 見つからない場合は最初の有効な言語コマンドを選択
    for (var j = 0; j < this._list.length; j++) {
        if (this._list[j].symbol === 'language' && this._list[j].enabled) {
            this.select(j);
            return;
        }
    }
};

Window_LanguageSelect.prototype.drawItem = function(index) 
{    
  if(Langscore.isMV()){
    var rect = this.itemRectForText(index);
  }
  else if(Langscore.isMZ()){
    var rect = this.itemLineRect(index);
  }
    var command = this._list[index];
    var align = 'center';
    
    // タイトルと区切り線の場合
    if (command.symbol === 'title') {
        this.changeTextColor(this.systemColor());
        align = 'center';
    } else if (command.symbol === 'separator' || command.symbol === 'separator2') {
        this.changeTextColor(this.normalColor());
        this.contents.paintOpacity = 128;
        align = 'center';
    } else if (command.symbol === 'language') {
        // 現在選択中の言語の場合
        if (command.ext === this.last_selected_language) {
            this.changeTextColor(this.systemColor());
        } else {
            this.resetTextColor();
        }
        align = 'center';
    } else {
        // 確定・キャンセルコマンド
        this.resetTextColor();
        align = 'center';
    }
    
    this.changePaintOpacity(command.enabled);
    this.drawText(command.name, rect.x, rect.y, rect.width, align);
    this.changePaintOpacity(true);
    this.contents.paintOpacity = 255;
};

Window_LanguageSelect.prototype.normalColor = function() 
{
  if(Langscore.isMV()){
    return Window_Command.prototype.normalColor.call(this);
  }
  else if(Langscore.isMZ()){
    return ColorManager.normalColor();
  }
};

Window_LanguageSelect.prototype.systemColor = function() 
{
  if(Langscore.isMV()){
    return Window_Command.prototype.systemColor.call(this);
  }
  else if(Langscore.isMZ()){
    return ColorManager.systemColor();
  }
};

Window_LanguageSelect.prototype.isOkEnabled = function() {
    return true;
};

Window_LanguageSelect.prototype.processOk = function() {
    var symbol = this.currentSymbol();
    var ext = this.currentExt();
    
    switch (symbol) {
        case 'language':
            if (!ext || ext !== this.last_selected_language) {
                this.last_selected_language = ext
                // リストを再構築して表示を更新
                this.playOkSound();
                this.refresh();
                this.selectCurrentLanguage();
            } else {
                this.playBuzzerSound();
            }
            break;
            
        case 'ok':
            // 確定処理
            Window_Selectable.prototype.processOk.call(this);
            _langscore.changeLanguage(this.last_selected_language);
            break;
            
        case 'cancel':
            // キャンセル処理
            this.processCancel();
            break;
            
        default:
            this.playBuzzerSound();
            break;
    }
};

Window_LanguageSelect.prototype.changeLanguage = function(language) 
{    
    // リストを再構築して表示を更新
    this.refresh();
    this.selectCurrentLanguage();
};

Window_LanguageSelect.prototype.refresh = function() {
    this.clearCommandList();
    this.makeCommandList();
    this.createContents();
    Window_Command.prototype.refresh.call(this);
};

Window_LanguageSelect.prototype.cursorDown = function(wrap) {
    var index = this.index();
    var maxItems = this.maxItems();
    
    // 選択不可な項目をスキップ
    do {
        index = (index + 1) % maxItems;
    } while (!this.isCommandEnabled(index) && index !== this.index());
    
    if (index !== this.index()) {
        this.select(index);
    }
};

Window_LanguageSelect.prototype.cursorUp = function(wrap) {
    var index = this.index();
    var maxItems = this.maxItems();
    
    // 選択不可な項目をスキップ
    do {
        index = (index + maxItems - 1) % maxItems;
    } while (!this.isCommandEnabled(index) && index !== this.index());
    
    if (index !== this.index()) {
        this.select(index);
    }
};

//-----------------------------------------------------------------------------
// Window_LanguageCommand
//
// 言語選択のメインコマンドウィンドウ（簡易版）
//-----------------------------------------------------------------------------

function Window_LanguageCommand() {
    this.initialize.apply(this, arguments);
}

Window_LanguageCommand.prototype = Object.create(Window_Command.prototype);
Window_LanguageCommand.prototype.constructor = Window_LanguageCommand;

Window_LanguageCommand.prototype.initialize = function() {
    Window_Command.prototype.initialize.call(this, 0, 0);
    this.updatePlacement();
};

Window_LanguageCommand.prototype.windowWidth = function() {
    return 240;
};

Window_LanguageCommand.prototype.updatePlacement = function() {
    this.x = (Graphics.boxWidth - this.width) / 2;
    this.y = (Graphics.boxHeight - this.height) / 2;
};

Window_LanguageCommand.prototype.makeCommandList = function() {
    this.addCommand('言語選択', 'language');
    this.addCommand('戻る', 'cancel');
};

//-----------------------------------------------------------------------------
// Scene_LanguageSelect
//
// 言語選択シーン（更新版）
//-----------------------------------------------------------------------------

function Scene_LanguageSelect() {
    this.initialize.apply(this, arguments);
}

Scene_LanguageSelect.prototype = Object.create(Scene_MenuBase.prototype);
Scene_LanguageSelect.prototype.constructor = Scene_LanguageSelect;

Scene_LanguageSelect.prototype.initialize = function() {
    Scene_MenuBase.prototype.initialize.call(this);
};

Scene_LanguageSelect.prototype.create = function() {
    Scene_MenuBase.prototype.create.call(this);
    this.createLanguageWindow();
};

Scene_LanguageSelect.prototype.createLanguageWindow = function() {
    this._languageWindow = new Window_LanguageSelect();
    this._languageWindow.setHandler('ok', this.popScene.bind(this));
    this._languageWindow.setHandler('cancel', this.popScene.bind(this));
    this.addWindow(this._languageWindow);
};

Scene_LanguageSelect.prototype.onLanguageOk = function() {
    // 確定ボタンが押された時の処理
    this._languageWindow.deactivate();
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
  if(lang !== undefined && _langscore.current_language_list.includes(lang)){
    Langscore.langscore_current_language = lang;
  }

  //Support_Language以外の言語が指定されると、起動直後にisFirstLoadedがtrueにならずハングする。
  //対応言語を手動編集した場合に発生しうるので未想定の動作だが、念のためのチェック。
  if(!_langscore.current_language_list.includes(Langscore.langscore_current_language)){
    Langscore.langscore_current_language = Langscore.Default_Language;
    //厳重にチェック
    if(!_langscore.current_language_list.includes(Langscore.langscore_current_language)){
      //Support_Languageが空はヤバいのでクラッシュさせる。チェックしない。
      Langscore.langscore_current_language = Langscore.Support_Language[0];
    }
  }
};

})(); //'use strict';
