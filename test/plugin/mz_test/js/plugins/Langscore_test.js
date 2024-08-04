Utils.isNwjs = function() 
{
    var result = typeof require === 'function' && typeof process === 'object';
    return result;
};
Utils.isOptionValid = function(name) {
    return false;
}

class IntervalTicker {
    constructor(interval = 16) {
      this.interval = interval;
      this.callbacks = [];
      this.timerId = null;
    }
  
    add(callback) {
      this.callbacks.push(callback);
    }
  
    start() {
      if (this.timerId === null) {
        this.timerId = setInterval(() => {
          for (let callback of this.callbacks) {
            callback();
          }
        }, this.interval);
      }
    }
  
    stop() {
      if (this.timerId !== null) {
        clearInterval(this.timerId);
        this.timerId = null;
      }
    }
  }

Graphics.printError = function(name, message){
    console.log(`Error : ${name} ${message}`);
}
Graphics.showRetryButton = function(){}

FontManager.load = function(){}

const intervalTicker = new IntervalTicker(16); // 60fps相当
class LangscoreTestEvent {
};

Scene_Boot.prototype.isGameFontLoaded = function() {
    return true;
};

intervalTicker.add(() => {
    SceneManager.update(16);
});

SceneManager.initialize = function() {
    this._stopped = false;
    intervalTicker.start();
    this.checkBrowser();
    this.checkPluginErrors();
    this.initGraphics();
    this.initAudio();
    this.initVideo();
    this.initInput();
    this.setupEventHandlers();
};

SceneManager.checkBrowser = function(){};
SceneManager.initGraphics = function(){};
SceneManager.initAudio = function(){};
SceneManager.initNwjs = function() {};
SceneManager.onSceneCreate = function(){};
SceneManager.onSceneStart = function(){};
SceneManager.onSceneLoading = function() {};

// var SceneManager_onSceneStart = SceneManager.onSceneStart;
SceneManager.onSceneStart = function() {
    // SceneManager_onSceneStart.call(this);
    if(!window.langscoreFinishTickEnd){
        window.langscoreFinishTickEnd = new LangscoreTestEvent;
        this._stopped = true;
    }
};

SceneManager_terminate = SceneManager.terminate;
SceneManager.terminate = function() {
    SceneManager_terminate.call(this);
}


SceneManager.updateMain = function() {
    if(!window.langscoreFinishTickEnd)
    {
        if(this._scene && this._scene.isReady()){  
            DataManager.setupNewGame();
            console.log("stop");
            window.langscoreFinishTickEnd = new LangscoreTestEvent;
            this._stopped = true;
            intervalTicker.stop();
        }
        else{
            this.changeScene();
            this.updateScene();
        }

    }
};
