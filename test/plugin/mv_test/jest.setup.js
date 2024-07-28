const chai = require('chai');
const path = require('path');
const fs = require('fs');

const { JSDOM } = require("jsdom");

// index.htmlのパスを指定
const htmlFilePath = path.resolve(__dirname, 'index.html');
// HTMLファイルの内容を読み込む
const htmlContent = fs.readFileSync(htmlFilePath, 'utf-8');
let dom = new JSDOM(htmlContent, {
    // 必要に応じてURLや他のオプションを指定
    url: 'http://localhost:8180/',
    runScripts: 'dangerously', // JavaScriptの実行を許可
    resources: 'usable', // 外部リソース（スクリプト）の読み込みを許可
    pretendToBeVisual: true // オプション：仮想ブラウザとして振る舞う
});

window = dom.window;

// DOMContentLoadedの完了を待つ
await new Promise(resolve => {
    window.addEventListener('load', () => {
    resolve();
    });
});


function waitForCondition(conditionFunction, interval = 100, timeout = 5000) {
    return new Promise((resolve, reject) => {
    const startTime = Date.now();

    function checkCondition() {
        if (conditionFunction()) 
        {
        resolve();
        } else if (Date.now() - startTime >= timeout) {
        reject(new Error('Timeout waiting for condition'));
        } else {
        setTimeout(checkCondition, interval);
        }
    }

    checkCondition();
    });
}

await waitForCondition(() => window.SceneManager._stopped === true);

// windowオブジェクト内の全てのプロパティをglobalスコープに追加
Object.getOwnPropertyNames(window).forEach(prop => {
    if (typeof global[prop] === 'undefined') {
        global[prop] = window[prop];
    }
});

console.log("jest.setup.js done.");