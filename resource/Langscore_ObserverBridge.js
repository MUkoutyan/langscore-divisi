//---------------------------------------------------------------
//
// Langscore CoreScript "Unison-ObserverBridge"
// Version 0.1.0
// Written by BreezeSinfonia 來奈津
//
//---------------------------------------------------------------
 /*:en
 * @target MV MZ
 * @plugindesc This is an auxiliary plugin for the Langscore translation application. Be sure to place it at the top of all plug-ins.
 * @author BreezeSinfonia Kunatsu
 * @url https://mukoutyan.github.io/langscore-manual/
 * 
 * @help
 * 
 */
 /*:ja
 * @target MV MZ
 * @plugindesc 翻訳アプリLangscoreの補助プラグインです。必ず全てのプラグインの先頭に配置して下さい。
 * @author BreezeSinfonia 來奈津
 * @url https://mukoutyan.github.io/langscore-manual/
 * 
 * @help
 * Langscoreで言語が変更された際に呼び出すメソッドを登録するプラグインです。
 * 必ず全てのプラグインの先頭に配置して下さい。
 * 
 * [プラグイン開発者向けの説明]
 * Langscoreで言語が変更された際に通知を受け取ることが出来ます。
 * プラグイン側で_registerLangscoreObserver関数に、第1引数のみ受け取る関数を渡して下さい。
 * 第1引数には変更後の言語の文字列が格納されます。
 * 
 * 通知のタイミングは、全てのデーターベース内の文字列を翻訳適用した後に呼び出されます。
 * 
 * 登録例)
 *  if (typeof _registerLangscoreObserver === 'function') {
 *      _registerLangscoreObserver(function(changed_language) {
 *          console.log("言語が " + changed_language + " に変更されました");
 *      });
 *  }
 * 
 */

(function(){
    'use strict';

    window._LangscoreObserverQueue = window._LangscoreObserverQueue || [];

    window._registerLangscoreObserver = function(callback) {
        if (typeof callback === 'function') {
            window._LangscoreObserverQueue.push(callback);
        }
    };
})();