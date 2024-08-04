Utils.isNwjs = function() 
{
    var result = typeof require === 'function' && typeof process === 'object';
    return result;
};

Graphics.printError = function(name, message){
    console.log(`Error : ${name} ${message}`);
}

class LangscoreTestEvent {

};

Scene_Boot.prototype.isGameFontLoaded = function() {
    return true;
};

SceneManager.initAudio = function(){};
SceneManager.initNwjs = function() {};
SceneManager.onSceneCreate = function(){};
SceneManager.onSceneStart = function(){};
SceneManager.onSceneLoading = function() {};

var SceneManager_onSceneStart = SceneManager.onSceneStart;
SceneManager.onSceneStart = function() {
    SceneManager_onSceneStart.call(this);
    if(!window.langscoreFinishTickEnd){
        window.langscoreFinishTickEnd = new LangscoreTestEvent;
        this._stopped = true;
    }
};
