Utils.isNwjs = function() 
{
    var result = typeof require === 'function' && typeof process === 'object';
    return result;
};

Graphics.printError = function(name, message){
    console.log(`Error : ${name} ${message}`);
}
Graphics.setLoadingImage = function(){
    this._loadingImage = new Image();
}

class LangscoreTestEvent {

};

Scene_Boot.prototype.isGameFontLoaded = function() {
    return true;
};

Scene_Boot.prototype.loadSystemWindowImage = function() {
};
Scene_Boot.loadSystemImages = function(){
}

Scene_Title.prototype.createBackground = function(){
    this._backSprite1 = new Sprite(ImageManager.loadEmptyBitmap());
    this._backSprite2 = new Sprite(ImageManager.loadEmptyBitmap());
    this.addChild(this._backSprite1);
    this.addChild(this._backSprite2);
}
Scene_Title.prototype.createForeground = function(){
}
Window_Base.prototype.loadWindowskin = function() {
    this.windowskin = ImageManager.loadEmptyBitmap();
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
