// Langscore_RemoteHub.js (デバッグ用プラグイン) の結合テスト。
// 実行方法: test/plugin で `npm run test-remotehub-mv` / `npm run test-remotehub-mz`
// テスト用プロジェクト (mv_test / mz_test) を一時フォルダへコピーし、NW.js のテストプレイを模して起動する。
// 翻訳ファイルを書き換えるテストがあるため、テストデータそのものは使わない。
//
// 確認する内容 (優先度順)
//  1. ゲームのプロジェクトを書き換えないこと
//  2. 壊れた翻訳ファイルや書き込みの失敗でゲームが止まらないこと
//  3. Langscore のプロジェクトでは Langscore_RemoteHub フォルダ以外を書き換えないこと
//  4. 表示中の文章・選択肢・カーソルの項目の書き出しと、翻訳の読み直し
const { expect } = require('chai');
const path = require('path');
const fs = require('fs');
const os = require('os');
const http = require('http');
const crypto = require('crypto');
const { JSDOM } = require('jsdom');
const { requirePluginProject } = require('./test_data_root');

const IS_MV = process.env.IS_MV === 'true';
const IS_MZ = process.env.IS_MZ === 'true';
const PROJ_NAME = process.env.PROJ_NAME || '';
const SOURCE_ROOT = requirePluginProject(PROJ_NAME);
const RESOURCE = path.join(__dirname, '..', '..', 'resource');
const PORT = 8181;

if (!IS_MV && !IS_MZ) {
  throw new Error('IS_MV / IS_MZ と PROJ_NAME が未指定です。package.json の npm scripts 経由で実行してください。');
}

//---------------------------------------------------------------
// 一時フォルダのゲームプロジェクト
//   <tmp>/<proj>            ゲーム
//   <tmp>/<proj>_langscore  Langscore のプロジェクト (Langscore_RemoteHub フォルダの置き場所)
//---------------------------------------------------------------
const TEMP_ROOT = fs.mkdtempSync(path.join(os.tmpdir(), 'ls_remotehub_'));
const GAME_ROOT = path.join(TEMP_ROOT, PROJ_NAME);
const LANGSCORE_ROOT = path.join(TEMP_ROOT, PROJ_NAME + '_langscore');
const HUB_DIR = path.join(LANGSCORE_ROOT, 'Langscore_RemoteHub');
const TRANSLATE_DIR = path.join(GAME_ROOT, 'data', 'translate');

function setupTempProject() {
  fs.cpSync(SOURCE_ROOT, GAME_ROOT, { recursive: true });
  fs.mkdirSync(LANGSCORE_ROOT);
  fs.copyFileSync(path.join(SOURCE_ROOT, '..', PROJ_NAME + '_langscore', 'config.json'), path.join(LANGSCORE_ROOT, 'config.json'));

  const pluginsDir = path.join(GAME_ROOT, 'js', 'plugins');
  fs.copyFileSync(path.join(RESOURCE, 'Langscore_RemoteHub.js'), path.join(pluginsDir, 'Langscore_RemoteHub.js'));
  // Langscore_test.js はテストプレイではない扱いにしているため、テストプレイとして起動する
  fs.writeFileSync(path.join(pluginsDir, 'Langscore_RemoteHub_testplay.js'), "Utils.isOptionValid = function(name){ return name === 'test'; };\n");

  const pluginsJsPath = path.join(GAME_ROOT, 'js', 'plugins.js');
  const pluginsJs = fs.readFileSync(pluginsJsPath, 'utf-8');
  const start = pluginsJs.indexOf('['), end = pluginsJs.lastIndexOf(']');
  const plugins = JSON.parse(pluginsJs.slice(start, end + 1));
  plugins.push({ name: 'Langscore_RemoteHub_testplay', status: true, description: '', parameters: {} });
  plugins.push({ name: 'Langscore_RemoteHub', status: true, description: '', parameters: {} });
  fs.writeFileSync(pluginsJsPath, pluginsJs.slice(0, start) + JSON.stringify(plugins) + pluginsJs.slice(end + 1));
}

// フォルダ以下の全ファイルの内容のハッシュ。キーはフォルダからの相対パス
function snapshot(root) {
  const result = {};
  (function walk(dir) {
    for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
      const full = path.join(dir, entry.name);
      if (entry.isDirectory()) { walk(full); continue; }
      result[path.relative(root, full).split(path.sep).join('/')] = crypto.createHash('sha1').update(fs.readFileSync(full)).digest('hex');
    }
  })(root);
  return result;
}

function changedFiles(before, after) {
  const keys = new Set([...Object.keys(before), ...Object.keys(after)]);
  return [...keys].filter(k => before[k] !== after[k]).sort();
}

//---------------------------------------------------------------
// ゲームの起動 (Langscore_mvmz.test.js と同じ方法)
//---------------------------------------------------------------
const MIME = {
  '.html': 'text/html', '.js': 'text/javascript', '.json': 'application/json', '.csv': 'text/plain',
  '.css': 'text/css', '.png': 'image/png', '.ttf': 'font/ttf', '.woff': 'font/woff', '.wasm': 'application/wasm'
};

class FontFaceStub {
  constructor(family, source) { this.family = family; this.source = source; this.status = 'loaded'; }
  load() { return Promise.resolve(this); }
}
class WorkerStub {
  postMessage() {}
  terminate() {}
  addEventListener() {}
  removeEventListener() {}
}

function waitForCondition(conditionFunction, timeout = 5000, interval = 50) {
  return new Promise((resolve, reject) => {
    const startTime = Date.now();
    (function check() {
      let ok = false;
      try { ok = conditionFunction(); } catch (e) { ok = false; }
      if (ok) { resolve(); }
      else if (Date.now() - startTime >= timeout) { reject(new Error('Timeout waiting for condition')); }
      else { setTimeout(check, interval); }
    })();
  });
}

const sleep = (ms) => new Promise(resolve => setTimeout(resolve, ms));

let server, dom, window;
const pageErrors = [];

async function initializeRPGMaker() {
  const html = fs.readFileSync(path.join(GAME_ROOT, 'index.html'), 'utf-8');
  const { VirtualConsole } = require('jsdom');
  const virtualConsole = new VirtualConsole();
  virtualConsole.on('jsdomError', (e) => {
    const message = String(e && e.message || e);
    // テスト用のプロジェクトには画像が無いため、画像の読み込みの失敗は除く
    if (message.startsWith('Could not load img')) { return; }
    pageErrors.push(message);
  });
  dom = new JSDOM(html, {
    url: `http://localhost:${PORT}/`,
    runScripts: 'dangerously',
    resources: 'usable',
    pretendToBeVisual: true,
    virtualConsole,
    beforeParse(win) {
      // NW.js 実行を模倣する。process.pid は Langscore_RemoteHub が pid ファイルに書く
      win.process = { env: { IS_NWJS: 'true' }, pid: process.pid, mainModule: { filename: path.join(GAME_ROOT, 'index.html'), path: GAME_ROOT } };
      win.require = require;
      win.FontFace = FontFaceStub;
      win.Worker = WorkerStub;
      if (!win.document.fonts) { win.document.fonts = { add() {}, check() { return true; } }; }
    }
  });
  window = dom.window;
  await new Promise(resolve => window.addEventListener('load', resolve));
  await waitForCondition(() => window.langscoreFinishTickEnd || window.langscoreBootError, 30000);
  if (window.langscoreBootError) {
    throw new Error(`ゲームの起動に失敗: ${window.langscoreBootError}`);
  }
}

function readCurrent() {
  return JSON.parse(fs.readFileSync(path.join(HUB_DIR, 'current.json'), 'utf-8'));
}

// current.json が条件を満たすまで待つ。書き出しは 100ms 遅らせている
async function waitForCurrent(condition) {
  await waitForCondition(() => condition(readCurrent()));
  return readCurrent();
}

// MV はウィンドウを座標で、MZ は Rectangle で作る
function rect() { return new window.Rectangle(0, 0, 600, 400); }

function newMessageWindow() {
  return IS_MZ ? new window.Window_Message(rect()) : new window.Window_Message();
}

function newChoiceWindow(messageWindow) {
  if (IS_MV) { return new window.Window_ChoiceList(messageWindow); }
  const choice = new window.Window_ChoiceList();
  choice.setMessageWindow(messageWindow);
  return choice;
}

// Scene の update で、操作中のウィンドウのカーソルの項目を書き出す
function updateScene(windows) {
  const scene = new window.Scene_Base();
  scene._windowLayer = { children: windows };
  scene.update();
}

function activate(w) {
  w.openness = 255;
  w.visible = true;
  w.activate();
  return w;
}

const originalTranslateFiles = {};
function writeTranslateFile(name, content) {
  const file = path.join(TRANSLATE_DIR, name);
  if (!(name in originalTranslateFiles)) { originalTranslateFiles[name] = fs.existsSync(file) ? fs.readFileSync(file) : null; }
  fs.writeFileSync(file, content);
}
function restoreTranslateFiles() {
  for (const [name, content] of Object.entries(originalTranslateFiles)) {
    const file = path.join(TRANSLATE_DIR, name);
    if (content === null) { fs.rmSync(file, { force: true }); }
    else { fs.writeFileSync(file, content); }
    delete originalTranslateFiles[name];
  }
}
function readTranslateFile(name) {
  return fs.readFileSync(path.join(TRANSLATE_DIR, name), 'utf-8');
}

//---------------------------------------------------------------
let gameBefore, langscoreBefore;

describe('Langscore_RemoteHub', function () {
  this.timeout(20000);

  before(async function () {
    this.timeout(60000);
    setupTempProject();
    gameBefore = snapshot(GAME_ROOT);
    langscoreBefore = snapshot(LANGSCORE_ROOT);
    await new Promise(resolve => {
      server = http.createServer((req, res) => {
        const pathname = decodeURIComponent(new URL(req.url, 'http://localhost').pathname);
        const file = path.join(GAME_ROOT, pathname);
        fs.readFile(file, (err, data) => {
          if (err) { res.writeHead(404); res.end(); return; }
          res.writeHead(200, { 'Content-Type': MIME[path.extname(file)] || 'application/octet-stream' });
          res.end(data);
        });
      }).listen(PORT, resolve);
    });
    await initializeRPGMaker();
    // マップの文章の翻訳は、読み込んだマップの分だけ持っている
    window.DataManager.loadMapData(1);
    await waitForCondition(() => window.DataManager.isMapLoaded() && window._langscore.ls_current_map[1] instanceof window.Map, 10000);
    window.$gameMap.setup(1);
    window.$gameMap._interpreter._mapId = 1;
    window._langscore.changeLanguage('en', true);
  });

  after(function () {
    if (dom) { dom.window.close(); }
    if (server) { server.close(); }
    fs.rmSync(TEMP_ROOT, { recursive: true, force: true });
  });

  afterEach(function () {
    restoreTranslateFiles();
  });


  describe('起動', function () {
    it('Langscore のプロジェクトに pid を書き、ゲームのフォルダには何も作らないこと', function () {
      expect(fs.readFileSync(path.join(HUB_DIR, 'pid'), 'utf-8')).to.equal(String(process.pid));
      expect(fs.existsSync(path.join(GAME_ROOT, 'Langscore_RemoteHub'))).to.be.false;
    });

    it('起動中にエラーが出ないこと', function () {
      expect(window.langscoreBootError).to.be.undefined;
      expect(pageErrors).to.deep.equal([]);
    });
  });

  describe('表示中の文章の書き出し', function () {
    it('メッセージの原文・訳文・翻訳ファイルを書き出すこと', async function () {
      const message = newMessageWindow();
      const translated = message.convertEscapeCharacters('通常のテキストです');
      expect(translated).to.equal('Normal text.');
      const current = await waitForCurrent(c => c.texts.some(t => t.original === '通常のテキストです'));
      expect(current.language).to.equal('en');
      const shown = current.texts.find(t => t.original === '通常のテキストです');
      expect(shown).to.include({ window: 'Window_Message', file: 'Map001.csv', translated: 'Normal text.', choice: false, selected: false });
    });

    it('翻訳ファイルに無い文章は、翻訳ファイルを空にして書き出すこと', async function () {
      const message = newMessageWindow();
      expect(message.convertEscapeCharacters('翻訳ファイルに無い文章')).to.equal('翻訳ファイルに無い文章');
      const current = await waitForCurrent(c => c.texts.some(t => t.original === '翻訳ファイルに無い文章'));
      expect(current.texts.find(t => t.original === '翻訳ファイルに無い文章').file).to.equal('');
    });

    it('新しいメッセージが始まったら、前の文章を消すこと', async function () {
      const message = newMessageWindow();
      message.convertEscapeCharacters('通常のテキストです');
      await waitForCurrent(c => c.texts.length > 0);
      window.$gameMessage.add('改行を含む\nテキストです');
      try { message.startMessage(); } catch (e) { /* 描画に必要なウィンドウが無い。文章の消去は startMessage の最初に行う */ }
      window.$gameMessage.clear();
      message.convertEscapeCharacters('改行を含む\nテキストです');
      const current = await waitForCurrent(c => c.texts.some(t => t.original === '改行を含む\nテキストです'));
      expect(current.texts.map(t => t.original)).to.not.include('通常のテキストです');
    });

    it('選択肢はカーソルの当たっているものに印を付けること', async function () {
      const message = newMessageWindow();
      const choice = newChoiceWindow(message);
      window.$gameMessage.setChoices(['ラフィーナ', 'エルーシェ', 'やめる'], 0, -1);
      choice.start();
      let current = await waitForCurrent(c => c.texts.filter(t => t.choice).length === 3 && c.texts.some(t => t.selected));
      expect(current.texts.filter(t => t.selected).map(t => t.original)).to.deep.equal(['ラフィーナ']);
      expect(current.texts.find(t => t.original === 'ラフィーナ').file).to.equal('Map001.csv');

      choice.select(2);
      current = await waitForCurrent(c => c.texts.some(t => t.selected && t.original === 'やめる'));
      expect(current.texts.filter(t => t.selected).map(t => t.original)).to.deep.equal(['やめる']);

      // カーソルが無くなったら外す
      choice.select(-1);
      current = await waitForCurrent(c => c.texts.every(t => !t.selected));
      expect(current.texts.filter(t => t.choice).length).to.equal(3);
      window.$gameMessage.clear();
    });
  });

  describe('カーソルの当たっているデータベースの項目', function () {
    it('アイテムの一覧は、アイテム名の原文と Items.csv を書き出すこと', async function () {
      const list = activate(IS_MZ ? new window.Window_ItemList(rect()) : new window.Window_ItemList(0, 0, 600, 400));
      list._data = [window.$dataItems[1]];
      list.select(0);
      updateScene([list]);
      const current = await waitForCurrent(c => c.texts.some(t => t.focus));
      const focused = current.texts.find(t => t.focus);
      expect(focused).to.include({ file: 'Items.csv', original: 'ポーション', translated: 'portion', selected: true });
      expect(current.texts.filter(t => t.selected).length).to.equal(1);

      // 操作中のウィンドウが無くなったら外す
      list.deactivate();
      updateScene([list]);
      await waitForCurrent(c => c.texts.every(t => !t.focus));
    });

    it('コマンドは、コマンド名の原文と System.csv を書き出すこと', async function () {
      const command = activate(IS_MZ ? new window.Window_MenuCommand(rect()) : new window.Window_MenuCommand(0, 0));
      const index = command._list.findIndex(c => c.symbol === 'item');
      expect(index).to.be.at.least(0);
      command.select(index);
      updateScene([command]);
      const current = await waitForCurrent(c => c.texts.some(t => t.focus));
      expect(current.texts.find(t => t.focus)).to.include({ file: 'System.csv', original: 'アイテム', translated: 'item' });
      command.deactivate();
      updateScene([command]);
      await waitForCurrent(c => c.texts.every(t => !t.focus));
    });

    it('アクターの選択は、アクター名の原文と Actors.csv を書き出すこと', async function () {
      const status = activate(IS_MZ ? new window.Window_MenuStatus(rect()) : new window.Window_MenuStatus(0, 0));
      status.select(0);
      updateScene([status]);
      const actor = window.$gameParty.members()[0];
      const current = await waitForCurrent(c => c.texts.some(t => t.focus));
      const focused = current.texts.find(t => t.focus);
      expect(focused).to.include({ file: 'Actors.csv', translated: actor.name() });
      expect(window._langscore.ls_actors_tr.has(focused.original)).to.be.true;
      status.deactivate();
      updateScene([status]);
      await waitForCurrent(c => c.texts.every(t => !t.focus));
    });

    it('データベースに無い項目では何も書き出さず、エラーにならないこと', async function () {
      const list = activate(IS_MZ ? new window.Window_ItemList(rect()) : new window.Window_ItemList(0, 0, 600, 400));
      list._data = [{ name: 'データベースに無いアイテム', iconIndex: 0, description: '' }];
      list.select(0);
      expect(() => updateScene([list])).to.not.throw();
      await sleep(300);
      expect(readCurrent().texts.some(t => t.focus)).to.be.false;
    });

    it('項目の取得でエラーが出ても、ゲームを止めないこと', function () {
      const list = activate(IS_MZ ? new window.Window_ItemList(rect()) : new window.Window_ItemList(0, 0, 600, 400));
      list._data = [window.$dataItems[1]];
      list.select(0);
      list.item = function () { throw new Error('broken window'); };
      expect(() => updateScene([list])).to.not.throw();
    });
  });

  describe('翻訳の読み直し', function () {
    it('データベースの翻訳ファイルを書き換えると、アイテム名が変わること', async function () {
      writeTranslateFile('Items.csv', readTranslateFile('Items.csv').replace(',portion', ',portion edited'));
      await waitForCondition(() => window.$dataItems[1].name === 'portion edited');
      // 元に戻すと戻る
      restoreTranslateFiles();
      await waitForCondition(() => window.$dataItems[1].name === 'portion');
    });

    it('マップの翻訳ファイルを書き換えると、文章の訳が変わること', async function () {
      const message = newMessageWindow();
      writeTranslateFile('Map001.csv', readTranslateFile('Map001.csv').replace('Normal text.', 'Normal text edited.'));
      await waitForCondition(() => message.convertEscapeCharacters('通常のテキストです') === 'Normal text edited.');
    });

    it('書き込み途中の空の翻訳ファイルを読んでも、書き終わった内容が反映されること', async function () {
      const edited = readTranslateFile('Items.csv').replace(',portion', ',portion after write');
      writeTranslateFile('Items.csv', '');
      await sleep(500);
      expect(() => window.$dataItems[1].name).to.not.throw();
      writeTranslateFile('Items.csv', edited);
      await waitForCondition(() => window.$dataItems[1].name === 'portion after write');
    });

    it('壊れた翻訳ファイルでもゲームを止めないこと', async function () {
      writeTranslateFile('Items.csv', 'original,ja,en\n"閉じていない引用符,a\n\u0000\u0001,,,,\n');
      writeTranslateFile('Map001.csv', '"');
      await sleep(800);
      const message = newMessageWindow();
      expect(() => message.convertEscapeCharacters('通常のテキストです')).to.not.throw();
      expect(typeof window.$dataItems[1].name).to.equal('string');
      expect(pageErrors).to.deep.equal([]);
    });

    it('翻訳ファイルを消しても、ゲームを止めないこと', async function () {
      writeTranslateFile('Weapons.csv', readTranslateFile('Weapons.csv'));
      fs.rmSync(path.join(TRANSLATE_DIR, 'Weapons.csv'));
      await sleep(800);
      expect(typeof window.$dataWeapons[1].name).to.equal('string');
      expect(pageErrors).to.deep.equal([]);
    });

    it('翻訳ファイル以外のファイルは読み直さないこと', async function () {
      const before = window.$dataItems[1].name;
      writeTranslateFile('memo.txt', 'portion,changed');
      await sleep(800);
      expect(window.$dataItems[1].name).to.equal(before);
    });
  });

  describe('書き出しの失敗', function () {
    it('current.json に書けなくても、文章の表示を止めないこと', async function () {
      const currentJson = path.join(HUB_DIR, 'current.json');
      fs.rmSync(currentJson, { force: true });
      fs.mkdirSync(currentJson);
      try {
        const message = newMessageWindow();
        expect(message.convertEscapeCharacters('通常のテキストです')).to.equal('Normal text.');
        await sleep(300);
        expect(pageErrors).to.deep.equal([]);
      }
      finally {
        fs.rmSync(currentJson, { recursive: true, force: true });
      }
    });
  });

  describe('プロジェクトを書き換えないこと', function () {
    it('ゲームのプロジェクトの内容が変わらないこと', async function () {
      restoreTranslateFiles();
      await sleep(500);
      // セーブフォルダはツクールの起動処理 (設定の保存) が書くため除く
      const changed = changedFiles(gameBefore, snapshot(GAME_ROOT)).filter(f => !f.startsWith('save/'));
      expect(changed).to.deep.equal([]);
    });

    it('Langscore のプロジェクトでは Langscore_RemoteHub フォルダ以外を書き換えないこと', function () {
      const changed = changedFiles(langscoreBefore, snapshot(LANGSCORE_ROOT));
      expect(changed.filter(f => !f.startsWith('Langscore_RemoteHub/'))).to.deep.equal([]);
      expect(changed).to.include('Langscore_RemoteHub/pid');
    });
  });
});
