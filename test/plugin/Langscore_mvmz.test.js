// Langscore.js (RPGツクールMV/MZ用プラグイン) の結合テスト。
// 実行方法: test/plugin で `npm test` (MV/MZ × NW.js/ブラウザ の4パターンを順に実行)
// 前提: test/test_data.zip を展開して mv_test / mz_test が存在すること。
//       js/plugins/Langscore.js は pretest (sync_plugin.js) が resource/ から生成する。
const { expect } = require('chai');
const path = require('path');
const fs = require('fs');
const http = require('http');
const { JSDOM } = require('jsdom');

const IS_MV = process.env.IS_MV === 'true';
const IS_MZ = process.env.IS_MZ === 'true';
const IS_NWJS = process.env.IS_NWJS === 'true';
const PROJ_ROOT = path.join(__dirname, process.env.PROJ_NAME || '');
const PORT = 8180;

if (!IS_MV && !IS_MZ) {
  throw new Error('IS_MV / IS_MZ と PROJ_NAME が未指定です。package.json の npm scripts 経由で実行してください。');
}

//---------------------------------------------------------------
// ゲームプロジェクトを配信する静的サーバー。
// jsdom は index.html 内の <script> や data/*.json, data/translate/*.csv を XHR で取得するため必要。
//---------------------------------------------------------------
const MIME = {
  '.html': 'text/html', '.js': 'text/javascript', '.json': 'application/json', '.csv': 'text/plain',
  '.css': 'text/css', '.png': 'image/png', '.ttf': 'font/ttf', '.woff': 'font/woff', '.wasm': 'application/wasm'
};
// テストごとに Langscore のプラグインパラメータを差し替えるため、js/plugins.js は配信時に書き換える。
let pluginParamOverrides = {};
function applyPluginParamOverrides(pluginsJs) {
  const start = pluginsJs.indexOf('['), end = pluginsJs.lastIndexOf(']');
  const plugins = JSON.parse(pluginsJs.slice(start, end + 1));
  Object.assign(plugins.find(p => p.name === 'Langscore').parameters, pluginParamOverrides);
  return pluginsJs.slice(0, start) + JSON.stringify(plugins) + pluginsJs.slice(end + 1);
}

let server;
before(function (done) {
  server = http.createServer((req, res) => {
    const pathname = decodeURIComponent(new URL(req.url, 'http://localhost').pathname);
    const file = path.join(PROJ_ROOT, pathname);
    fs.readFile(file, (err, data) => {
      if (err) { res.writeHead(404); res.end(); return; }
      if (pathname === '/js/plugins.js') { data = applyPluginParamOverrides(data.toString('utf-8')); }
      res.writeHead(200, { 'Content-Type': MIME[path.extname(file)] || 'application/octet-stream' });
      res.end(data);
    });
  }).listen(PORT, done);
});
after(function () { if (server) { server.close(); } });

// jsdom に無い API の最小スタブ。フォントの実ロードと音声デコード (MZ の vorbisdecoder.js が Worker を要求) はテスト対象外。
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

function waitForCondition(conditionFunction, interval = 50, timeout = 10000) {
  return new Promise((resolve, reject) => {
    const startTime = Date.now();
    (function check() {
      if (conditionFunction()) { resolve(); }
      else if (Date.now() - startTime >= timeout) { reject(new Error('Timeout waiting for condition')); }
      else { setTimeout(check, interval); }
    })();
  });
}

let dom, window;
// options.pluginParams: js/plugins.js 内の Langscore パラメータへの上書き (例: {'Enable Language Patch Mode': 'true'})
async function initializeRPGMaker(options = {}) {
  pluginParamOverrides = options.pluginParams || {};
  const html = fs.readFileSync(path.join(PROJ_ROOT, 'index.html'), 'utf-8');
  dom = new JSDOM(html, {
    url: `http://localhost:${PORT}/`,
    runScripts: 'dangerously',
    resources: 'usable',
    pretendToBeVisual: true,
    beforeParse(win) {
      if (IS_NWJS) {
        // NW.js 実行を模倣する。プラグインは process.mainModule.filename からプロジェクトルートを求め、
        // fs で直接ファイルを読む (Utils.isNwjs は js/plugins/Langscore_test.js が require/process の有無で判定)。
        win.process = { env: { IS_NWJS: 'true' }, mainModule: { filename: path.join(PROJ_ROOT, 'index.html'), path: PROJ_ROOT } };
        win.require = require;
      }
      win.FontFace = FontFaceStub;
      win.Worker = WorkerStub;
      if (!win.document.fonts) { win.document.fonts = { add() {}, check() { return true; } }; }
    }
  });
  window = dom.window;
  await new Promise(resolve => window.addEventListener('load', resolve));
  // js/plugins/Langscore_test.js がタイトル画面到達時に langscoreFinishTickEnd を立てて SceneManager を止める。
  // 起動中の例外は Graphics.printError 経由で langscoreBootError に記録される。
  await waitForCondition(() => window.langscoreFinishTickEnd || window.langscoreBootError);
  if (window.langscoreBootError) {
    throw new Error(`ゲームの起動に失敗: ${window.langscoreBootError}`);
  }
}

async function saveContents(saveFileId) {
  if (IS_MV) { window.DataManager.saveGame(saveFileId); }
  else { await window.DataManager.saveGame(saveFileId); }
}

async function loadContents(saveFileId) {
  if (IS_MV) {
    return window.JsonEx.parse(window.StorageManager.load(saveFileId));
  }
  const saveName = window.DataManager.makeSavename(saveFileId);
  return await window.StorageManager.loadObject(saveName);
}

//---------------------------------------------------------------
// 期待値。<proj>/data/translate/*.csv の ja/en 列と対応している。
//---------------------------------------------------------------
const testActors = [{'ja': 'エルーシェ', 'en': 'eluche'}, {'ja': '雑用係', 'en': 'Compassionate'}, {'ja': 'ラフィーナ', 'en': 'Rafina'}, {'ja': '傲慢ちき', 'en': 'arrogant'}, {'ja': 'ケスティニアスの雑用係。\nそんなに仕事は無い。', 'en': "Kestinius' scullery maid.\nNot that much work."}, {'ja': 'チビのツンデレウーマン。\n魔法が得意。', 'en': 'Tiny tsundere woman.\nHe is good at magic.'}];
const testArmors = [{'ja': '盾', 'en': 'Shield'}, {'ja': '帽子', 'en': 'Had'}, {'ja': '服', 'en': 'Wear'}, {'ja': '指輪', 'en': 'Ring'}];
const testClasses = [{'ja': '勇者', 'en': 'brave'}, {'ja': '戦士', 'en': 'warrior'}, {'ja': '魔術師', 'en': 'magician'}, {'ja': '僧侶', 'en': 'monk'}];
const testCommonevents = [{'ja': '顧問です', 'en': 'Advisor.'}, {'ja': ' ', 'en': ''}];
const testEnemies = [{'ja': 'こうもり', 'en': 'umbrella'}, {'ja': 'スライム', 'en': 'slime'}, {'ja': 'オーク', 'en': 'oak'}, {'ja': 'ミノタウロス', 'en': 'Minotaur'},
                     {'ja': '鳥', 'en': 'bird'}, {'ja': 'ゾンビ', 'en': 'zombie'}, {'ja': 'ツボ', 'en': 'pot'}, {'ja': '木', 'en': 'tree'}];  // 後半は mz_test の敵
const testItems = [{'ja': 'ポーション', 'en': 'portion'}, {'ja': 'マジックウォーター', 'en': 'magic water'}, {'ja': 'ディスペルハーブ', 'en': 'dispersal herb'}, {'ja': 'スティミュラント', 'en': 'formula'}];
const testMap001 = [{'ja': '通常のテキストです', 'en': 'Normal text.'}, {'ja': '改行を含む\nテキストです', 'en': 'Includes line breaks\nText.'}, {'ja': 'カンマを含む,テキストです', 'en': 'Including comma, text.'}, {'ja': '"タ"フ"ルクォーテーションを含むテキストです"', 'en': '"The text contains a t"ouh"le quotation."'}, {'ja': '"""Hello, World"",\nそれはプログラムを書く際の","""謎の呪文""(Mystery spell)―――"', 'en': '"""Hello, World"","\nIt is a ",""Mystery spell""(Mystery spell)---""" when writing a program.'}, {'ja': '名前変えるよ', 'en': "I'll change the name."}, {'ja': '1番の変数の値は \\V[1] です。', 'en': 'The value of variable 1 is \\V[1].'}, {'ja': '1番のアクターの名前は \\N[1] です。', 'en': 'The name of the 1 actor is \\N[1].'}, {'ja': '1番のパーティーメンバーの名前は \\P[1] です。', 'en': 'The name of party member 1 is \\P[1].'}, {'ja': '現在の所持金は \\G です。', 'en': 'The current amount of money you have is \\G'}, {'ja': 'この文字は \\C[2] 赤色 \\C[0] 通常色 です。', 'en': 'This letter is \\C[2] red \\C[0] normal color.'}, {'ja': 'これはアイコン \\I[64] の表示です。', 'en': 'This is the display of the icon \\I[64].'}, {'ja': '文字サイズを \\{ 大きく \\} 小さくします。', 'en': 'The font size will be reduced to \\{large \\}smaller.'}, {'ja': '\\$ 所持金ウィンドウを表示します。', 'en': '\\$ Display the money window.'}, {'ja': '文章の途中で 1/4 秒間のウェイトを \\. します。', 'en': 'It will display a 1/4 second wait \\. in the middle of a sentence.'}, {'ja': '文章の途中で 1 秒間のウェイトを \\| します。', 'en': 'The text is weighted for one second \\| in the middle of a sentence.'}, {'ja': '文章の途中でボタンの入力待ちを \\! します。', 'en': 'Waiting for button input in the middle of a sentence \\! The button is pressed.'}, {'ja': '\\>この行の文字を一瞬で表示します。\\<', 'en': '\\>Display the characters of this line in a moment. \\<'}, {'ja': '\\^文章表示後の入力待ちを行いません。', 'en': '\\^Do not wait for input after displaying a sentence.'}, {'ja': 'バックスラッシュの表示は \\\\ です。', 'en': 'The backslash is displayed at \\\\'}, {'ja': '複合させます\n\\{\\C[2]\\N[2]\\I[22]', 'en': 'Compounding\n\\{\\C[2]\\N[2]\\I[22]'}, {'ja': 'ラフィーナ', 'en': 'Rafina.'}, {'ja': 'エルーシェ', 'en': 'eluche'}, {'ja': 'やめる', 'en': 'leave'}, {'ja': '二つ名変えるよ', 'en': "I'll change my name."}, {'ja': '雑用係', 'en': 'Compassionate'}, {'ja': '傲慢ちき', 'en': 'arrogant'}, {'ja': '無くす', 'en': 'lose something'}, {'ja': 'プロフィール変えるよ', 'en': "I'll change my profile."}, {'ja': 'チビのツンデレウーマン。\n魔法が得意。', 'en': 'Tiny tsundere woman.\nShe is good at magic.'}, {'ja': 'ケスティニアスの雑用係。\nそんなに仕事は無い。', 'en': "Kestinius' scullery maid.\nNot that much work."}, {'ja': '何があったのかよく思い出せない。\n\n辺りを包む木々。天を覆う新緑の葉。\n\n小鳥のせせらぎと、風の音だけが聴こえる。\n\n彼方には、見慣れたような、しかしそうでないような、\n懐かしさを僅かに感じる家並み———\n\nふと、目の前の景色が遠ざかる。\n何かに後ろに引っ張られるように、\n辺りを俯瞰するような景色が広がっていく。\n\n"アタシ"は後ろを振り向いた。\n\n肩まで行かない赤髪を垂らし、\n見開く緑目は"私"を見つめ———', 'en': 'I can\'t quite remember what happened.\n\nTrees surrounding the area. Fresh green leaves covering the heavens.\n\nOnly the rustling of birds and the sound of the wind can be heard.\n\nIn the distance, the houses look familiar, but not so familiar,\nhouses, which seem familiar but not so familiar...\n\nSuddenly, the view in front of you fades away.\nAs if something is pulling you back,\nthe view of the area spreads out like a bird\'s eye view.\n\n"Atashi" turned around.\n\nHer red hair hangs down to her shoulders,\nGreen eyes wide open, staring at "me"...'}, {'ja': '勝ち', 'en': 'win'}, {'ja': '逃げ犬', 'en': 'fugitive dog'}, {'ja': '負け犬', 'en': 'dejected loser'}, {'ja': '\\{言語\\}変える\\}よ', 'en': "\\}I'm going to \\{change \\{Language\\}."}, {'ja': '日本語', 'en': 'Japanese (language)'}, {'ja': '英語', 'en': 'English (language)'}, {'ja': '中国語', 'en': 'Chinese Language'}, {'ja': 'やっぱやめる', 'en': 'give up'}, {'ja': 'フィールドを移動します', 'en': 'Move field'}, {'ja': '行って帰る', 'en': 'go and come back'}, {'ja': '行ったきり', 'en': 'going and leaving'}, {'ja': '移動後のメッセージです。', 'en': 'This is the message after the move.'}, {'ja': '更に移動した際のメッセージです', 'en': "Here's the message when we move further"}];
const testMap002 = [{'ja': '帰れ！', 'en': 'Go Home!'}];
const testMap003 = [{'ja': '誰……？', 'en': 'Who ......?.'}];
const testScripts = [{'ja': 'CustomizeConfigItem', 'en': ''}, {'ja': 'NumberOptions', 'en': ''}, {'ja': '数値項目', 'en': ''}, {'ja': 'StringOptions', 'en': ''}, {'ja': '文字項目', 'en': ''}, {'ja': 'SwitchOptions', 'en': ''}, {'ja': 'スイッチ項目', 'en': ''}, {'ja': 'VolumeOptions', 'en': ''}, {'ja': '音量項目', 'en': ''}, {'ja': 'Number', 'en': ''}, {'ja': 'Boolean', 'en': ''}, {'ja': 'String', 'en': ''}, {'ja': 'Volume', 'en': ''}, {'ja': 'no item', 'en': ''}, {'ja': 'shift', 'en': ''}, {'ja': 'NumberOptions', 'en': ''}, {'ja': '数値項目', 'en': ''}, {'ja': 'StringOptions', 'en': ''}, {'ja': '文字項目', 'en': ''}, {'ja': 'SwitchOptions', 'en': ''}, {'ja': 'スイッチ項目', 'en': ''}, {'ja': 'VolumeOptions', 'en': ''}, {'ja': '音量項目', 'en': ''}, {'ja': '', 'en': 'Number'}, {'ja': '', 'en': 'Boolean'}, {'ja': '', 'en': 'String'}, {'ja': '', 'en': 'Volume'}, {'ja': '', 'en': 'shift'}, {'ja': 'alwaysDash', 'en': ''}, {'ja': 'スイッチ項目1', 'en': 'Switch Item 1'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': '数値項目1', 'en': 'Numeric Item 1'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': '数値項目2', 'en': 'Numeric Item 2'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': '難易度', 'en': 'difficulty'}, {'ja': '', 'en': ''}, {'ja': '易しい', 'en': 'Easy'}, {'ja': '難しい', 'en': 'Difficult'}, {'ja': '地獄', 'en': 'Hell'}, {'ja': '', 'en': ''}, {'ja': '音量項目1', 'en': 'Volume Item 1'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': '音量項目2', 'en': 'Volume Item 2'}, {'ja': '', 'en': ''}, {'ja': 'TMLogWindow', 'en': ''}, {'ja': ' / ', 'en': ''}, {'ja': 'ok', 'en': ''}, {'ja': 'cancel', 'en': ''}, {'ja': '', 'en': 'YED_Hospital'}, {'ja': '', 'en': ','}, {'ja': '', 'en': 'Actor Help (Healthy)'}, {'ja': '', 'en': 'Actor Help (Treat)'}, {'ja': '', 'en': 'healOne'}, {'ja': '', 'en': 'healAll'}, {'ja': '', 'en': 'cancel'}, {'ja': '', 'en': 'Heal One Help'}, {'ja': '', 'en': 'Heal All Help (Healthy)'}, {'ja': '', 'en': 'Heal All Help (Treat)'}, {'ja': '', 'en': 'Exit Help'}, {'ja': '', 'en': 'healOne'}, {'ja': '', 'en': 'healAll'}, {'ja': '', 'en': 'cancel'}, {'ja': '', 'en': 'ok'}, {'ja': '', 'en': 'cancel'}, {'ja': '%1 does not need to be recovered', 'en': '%1は回復の必要はない'}, {'ja': 'Recover one.', 'en': '1人を回復する'}, {'ja': '%1needs to be recovered.', 'en': '%1は回復が必要だ'}, {'ja': 'Pay%1G to recover all members', 'en': '%1Gを払ってメンバー全員を回復させます'}, {'ja': 'Exit', 'en': 'Exit'}, {'ja': 'Exit.', 'en': '外に出る'}, {'ja': 'Recover all members.', 'en': '全員を回復する'}, {'ja': 'There is no member who needs to be healed.', 'en': '回復が必要なメンバーが居ません'}, {'ja': 'Heal each member individually', 'en': 'メンバーを個別に回復させます'}, {'ja': 'People4, 1', 'en': 'People4, 1'}, {'ja': 'Hello, how can I help you today?', 'en': 'こんにちは、今日はどうされましたか？'}, {'ja': 'Loli', 'en': 'Loli'}, {'ja': '', 'en': 'center'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': 'en:Advanced Array 1', 'en': 'Advanced Array 1'}, {'ja': 'en:Advanced Array 2', 'en': 'Advanced Array 2'}, {'ja': 'Advanced Array 1', 'en': 'アドバンス Array 1'}, {'ja': 'Advanced Array 2', 'en': 'アドバンス Array 2'}, {'ja': 'Advanced Array 3', 'en': 'アドバンス Array 3'}, {'ja': 'Advanced Array 4', 'en': 'アドバンス Array 4'}, {'ja': '', 'en': 'Map001'}, {'ja': '', 'en': ''}, {'ja': 'Choice 1', 'en': 'チョイス 1'}, {'ja': 'Choice 2', 'en': 'チョイス 2'}, {'ja': '', 'en': '<dynamicNote:custom>'}, {'ja': '', 'en': ''}, {'ja': '', 'en': ''}, {'ja': 'Dynamic 1', 'en': 'Dynamic 1'}, {'ja': 'Inner Value 1', 'en': 'Inner Value 1'}, {'ja': 'Inner Value 2', 'en': 'Inner Value 2'}];
const testSkills = [{'ja': 'の攻撃！', 'en': 'attack!'}, {'ja': '攻撃', 'en': 'Attack'}, {'ja': 'は身を守っている。', 'en': 'is protecting itself.'}, {'ja': '防御', 'en': 'Guard'}, {'ja': '連続攻撃', 'en': 'Continuous attack'}, {'ja': '２回攻撃', 'en': '2 attacks'}, {'ja': '3回攻撃', 'en': '3 attacks'}, {'ja': 'は逃げてしまった。', 'en': 'has fled.'}, {'ja': '逃げる', 'en': 'escape'}, {'ja': 'は様子を見ている。', 'en': 'is looking at the situation.'}, {'ja': '様子を見る', 'en': 'wait and see'}, {'ja': 'は%1を唱えた！', 'en': 'chanted %1!'}, {'ja': 'ヒール', 'en': 'heel'}, {'ja': 'ファイア', 'en': 'fire'}, {'ja': 'スパーク', 'en': 'spark'}, {'ja': 'お手軽デバフセット！', 'en': ''}];
const testStates = [{'ja': 'は倒れた！', 'en': 'fell down!'}, {'ja': 'を倒した！', 'en': 'defeated!'}, {'ja': 'は立ち上がった！', 'en': 'stood up!'}, {'ja': '戦闘不能', 'en': 'inability to fight'}, {'ja': '予防的', 'en': 'precautionary'}, {'ja': '不死身', 'en': 'immortal body'}, {'ja': 'は毒にかかった！', 'en': 'was poisoned!'}, {'ja': 'に毒をかけた！', 'en': 'poisoned the'}, {'ja': 'の毒が消えた！', 'en': 'The poison of the "Mere Old Man" disappeared!'}, {'ja': '毒', 'en': 'Poison'}, {'ja': 'は暗闇に閉ざされた！', 'en': 'was closed in darkness!'}, {'ja': 'を暗闇に閉ざした！', 'en': 'closed to the darkness!'}, {'ja': 'の暗闇が消えた！', 'en': 'of darkness disappeared!'}, {'ja': '暗闇', 'en': 'the dark'}, {'ja': 'は沈黙した！', 'en': 'was silent!'}, {'ja': 'を沈黙させた！', 'en': 'was silenced!'}, {'ja': 'の沈黙が解けた！', 'en': 'The silence of the "Mere Old Man" was broken!'}, {'ja': '鳴く', 'en': 'purr'}, {'ja': 'は激昂した！', 'en': 'was agitated!'}, {'ja': 'を激昂させた！', 'en': 'The first time the company was involved in the project, it was aggravated by the'}, {'ja': 'は我に返った！', 'en': 'came back to himself!'}, {'ja': '憤慨', 'en': 'aroused indignation'}, {'ja': 'は混乱した！', 'en': 'was confused!'}, {'ja': 'を混乱させた！', 'en': 'confused!'}, {'ja': '混乱', 'en': 'confusion'}, {'ja': 'は魅了された！', 'en': 'was fascinated!'}, {'ja': 'を魅了した！', 'en': 'The event fascinated the'}, {'ja': '魅了', 'en': 'fascinate'}, {'ja': 'は眠った！', 'en': 'slept!'}, {'ja': 'を眠らせた！', 'en': 'He put the "Mere Old Man" to sleep!'}, {'ja': 'は眠っている。', 'en': 'is sleeping.'}, {'ja': 'は目を覚ました！', 'en': 'woke up!'}, {'ja': '睡眠', 'en': 'sleep'}];
const testSystem = [{'ja': '一般防具', 'en': 'General Defense'}, {'ja': '魔法防具', 'en': 'Magic Defense'}, {'ja': '軽装防具', 'en': 'Lightweight Defense'}, {'ja': '重装防具', 'en': 'Heavy Duty Defense'}, {'ja': '小型盾', 'en': 'shields'}, {'ja': '大型盾', 'en': 'large shield'}, {'ja': 'ゴル', 'en': 'Gol'}, {'ja': '物理', 'en': 'physiotherapy'}, {'ja': '炎', 'en': 'Frame'}, {'ja': '氷', 'en': 'Ice'}, {'ja': '雷', 'en': 'Thunder'}, {'ja': '水', 'en': 'Water'}, {'ja': '土', 'en': 'Mud'}, {'ja': '風', 'en': 'Wind'}, {'ja': '光', 'en': 'Light'}, {'ja': '闇', 'en': 'Dark'}, {'ja': '武器', 'en': 'weaponry'}, {'ja': '盾', 'en': 'Shield'}, {'ja': '頭', 'en': 'Head'}, {'ja': '身体', 'en': 'body'}, {'ja': '装飾品', 'en': 'ornament'}, {'ja': 'Langdcore_Test', 'en': 'Langdcore_Test'}, {'ja': '魔法', 'en': 'magic'}, {'ja': '必殺技', 'en': 'specia'}, {'ja': 'レベル', 'en': 'level'}, {'ja': 'Lv', 'en': 'Lv'}, {'ja': 'ＨＰ', 'en': 'ＨＰ'}, {'ja': 'HP', 'en': 'HP'}, {'ja': 'ＭＰ', 'en': 'ＭＰ'}, {'ja': 'MP', 'en': 'MP'}, {'ja': 'ＴＰ', 'en': 'ＴＰ'}, {'ja': 'TP', 'en': 'TP'}, {'ja': '経験値', 'en': 'XP'}, {'ja': 'EXP', 'en': 'EXP'}, {'ja': '戦う', 'en': 'fight'}, {'ja': '逃げる', 'en': 'escape'}, {'ja': '攻撃', 'en': 'attack'}, {'ja': '防御', 'en': 'precautionary'}, {'ja': 'アイテム', 'en': 'item'}, {'ja': 'スキル', 'en': 'skill'}, {'ja': '装備', 'en': 'equipment'}, {'ja': 'ステータス', 'en': 'status'}, {'ja': '並び替え', 'en': 'sort'}, {'ja': 'セーブ', 'en': 'curbing'}, {'ja': 'ゲーム終了', 'en': 'end of game'}, {'ja': 'オプション', 'en': 'option'}, {'ja': '防具', 'en': 'protective gear'}, {'ja': '大事なもの', 'en': 'Important.'}, {'ja': '最強装備', 'en': 'the most powerful equipment'}, {'ja': '全て外す', 'en': 'remove all of them'}, {'ja': 'ニューゲーム', 'en': 'new game'}, {'ja': 'コンティニュー', 'en': 'continue'}, {'ja': 'タイトルへ', 'en': 'Go to Title.'}, {'ja': 'やめる', 'en': 'leave'}, {'ja': '購入する', 'en': 'Buy Now'}, {'ja': '売却する', 'en': 'Sold to.'}, {'ja': '%1には効かなかった！', 'en': 'It did not work for %1!'}, {'ja': '%1は %2 のダメージを受けた！', 'en': '%1 took %2 damage!'}, {'ja': '%1は%2を %3 奪われた！', 'en': '%1 %2 was %3 robbed!'}, {'ja': '%1の%2が %3 増えた！', 'en': 'The %2 of %1 increased by %3!'}, {'ja': '%1の%2が %3 減った！', 'en': 'The %2 of %1 was reduced by %3!'}, {'ja': '%1はダメージを受けていない！', 'en': '%1 is not damaged!'}, {'ja': 'ミス！\u3000%1はダメージを受けていない！', 'en': 'Miss!\u3000%1 is not damaged!'}, {'ja': '%1の%2が %3 回復した！', 'en': 'The %2 of %1 recovered %3!'}, {'ja': '常時ダッシュ', 'en': 'dash at any time'}, {'ja': 'BGM 音量', 'en': 'BGM Volume'}, {'ja': 'BGS 音量', 'en': 'BGS Volume'}, {'ja': '%1の%2が上がった！', 'en': 'The %2 of %1 was raised!'}, {'ja': '%1の%2が元に戻った！', 'en': 'The %1 %2 is back to normal!'}, {'ja': 'コマンド記憶', 'en': 'command storage'}, {'ja': '%1の反撃！', 'en': 'Counterattack at %1!'}, {'ja': '痛恨の一撃！！', 'en': 'A painful blow!'}, {'ja': '会心の一撃！！', 'en': 'A blow to the heart!'}, {'ja': '%1の%2が下がった！', 'en': 'The %2 of %1 dropped!'}, {'ja': '%1は戦いに敗れた。', 'en': '%1 lost the battle.'}, {'ja': '%1が出現！', 'en': '%1 appears!'}, {'ja': '%1に %2 のダメージを与えた！', 'en': '%2 damage to %1!'}, {'ja': '%1の%2を %3 奪った！', 'en': "I took %3 of %1's %2!"}, {'ja': '%1にダメージを与えられない！', 'en': 'No damage to %1!'}, {'ja': 'ミス！\u3000%1にダメージを与えられない！', 'en': 'Miss!\u3000No damage to %1!'}, {'ja': 'しかし逃げることはできなかった！', 'en': 'But there was no escape!'}, {'ja': '%1は逃げ出した！', 'en': '%1 escaped!'}, {'ja': '%1は攻撃をかわした！', 'en': '%1 dodged the attack!'}, {'ja': '次の%1まで', 'en': 'Next %1 to'}, {'ja': '現在の%1', 'en': 'Current%1'}, {'ja': 'ファイル', 'en': 'file'}, {'ja': '%1は%2 %3 に上がった！', 'en': '%1 went up to %2 %3!'}, {'ja': 'どのファイルをロードしますか？', 'en': 'Which files do you want to load?'}, {'ja': '%1は魔法を打ち消した！', 'en': 'The %1 cancelled the magic!'}, {'ja': '%1は魔法を跳ね返した！', 'en': 'The %1 bounced back the magic!'}, {'ja': 'ME 音量', 'en': 'ME Volume'}, {'ja': '%1 の%2を獲得！', 'en': 'Gained %2 of %1!'}, {'ja': 'お金を %1\\G 手に入れた！', 'en': 'I got %1\\G of money!'}, {'ja': '%1を手に入れた！', 'en': 'I got %1!'}, {'ja': '%1を覚えた！', 'en': 'I learned %1!'}, {'ja': '%1たち', 'en': 'The %1s.'}, {'ja': '持っている数', 'en': 'Number of items you have'}, {'ja': '%1は先手を取った！', 'en': '%1 took the lead!'}, {'ja': 'どのファイルにセーブしますか？', 'en': 'Which file do you want to save to?'}, {'ja': 'SE 音量', 'en': 'SE Volume'}, {'ja': '%1が%2をかばった！', 'en': '%1 covered for %2!'}, {'ja': '%1は不意をつかれた！', 'en': 'The %1 caught me off guard!'}, {'ja': '%1は%2を使った！', 'en': '%1 used %2!'}, {'ja': '%1の勝利！', 'en': '%1 win!'}, {'ja': '最大ＨＰ', 'en': 'Maximum HP'}, {'ja': '最大ＭＰ', 'en': 'Maximum MP'}, {'ja': '攻撃力', 'en': 'offensive ability'}, {'ja': '防御力', 'en': 'defensive strength'}, {'ja': '魔法力', 'en': 'magic power'}, {'ja': '魔法防御', 'en': 'Magic Defense'}, {'ja': '敏捷性', 'en': 'agility'}, {'ja': '運', 'en': 'Luck'}, {'ja': '命中率', 'en': 'hit rate'}, {'ja': '回避率', 'en': 'evasion rate'}, {'ja': '短剣', 'en': 'stiletto'}, {'ja': '剣', 'en': 'Sword'}, {'ja': 'フレイル', 'en': 'frail'}, {'ja': '斧', 'en': 'Axe'}, {'ja': 'ムチ', 'en': 'whip'}, {'ja': '杖', 'en': 'Staff'}, {'ja': '弓', 'en': 'Bow'}, {'ja': 'クロスボウ', 'en': 'crossbow'}, {'ja': '銃', 'en': 'Gun'}, {'ja': '爪', 'en': 'Claw'}, {'ja': 'グローブ', 'en': 'glove'}, {'ja': '槍', 'en': 'Lance'}];
const testTroops = [{'ja': 'おおおおおおいおいおぃおぉいぃぉぃぉぃぉぉぉぃぃ', 'en': 'oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh oh.'}];
const testWeapons = [{'ja': '剣', 'en': 'Sword'}, {'ja': '斧', 'en': 'Axe'}, {'ja': '杖', 'en': 'Staff'}, {'ja': '弓', 'en': 'Bow'}];

describe('Langscore', function()
{
  before(() => initializeRPGMaker());

  beforeEach(() => {
    window._langscore.changeLanguage("en", true);
  });

  it('テキストを正しく翻訳すること', function()
  {
    const pairs = [
      ["通常のテキストです", "Normal text."],
      ["改行を含む\nテキストです", "Includes line breaks\nText."],
      ["\"\"タ\"\"フ\"\"ルクォーテーションを含むテキストです\"\"", "The text contains a t\"\"ouh\"\"le quotation."],
    ];
    // translate は instanceof Map で判定するため、jsdom 側 (window) の Map で作る必要がある
    const testMap = new window.Map(pairs.map(([ja, en]) => [ja, new window.Map([["ja", ja], ["en", en]])]));
    for (const [ja, en] of pairs) {
      expect(window._langscore.translate(ja, testMap, "en")).to.equal(en);
      expect(window._langscore.translate(ja, testMap, "ja")).to.equal(ja);
    }
    // Map 以外 (Object) を渡した場合は原文をそのまま返す
    expect(window._langscore.translate("通常のテキストです", { "通常のテキストです": { en: "x" } }, "en")).to.equal("通常のテキストです");
  });

  it('翻訳が利用できない場合、元のテキストを返すこと', function() {
    const testMap = {
      "Hello": { "ja": "こんにちは" },
    };
    const result = window._langscore.translate("Goodbye", testMap, "ja");
    expect(result).to.equal("Goodbye");
  });

  it('翻訳文から原文テキストを正しく取得すること', function() {
    const testMap = {
      "Hello": { "ja": "こんにちは", "en": "Hello" },
      "Goodbye": { "ja": "さようなら" }
    };
    const result = window._langscore.fetch_original_text("こんにちは", testMap);
    expect(result).to.equal("Hello");
  });

  it('言語の変更が行えている', function()
  {
    expect(window.Langscore.langscore_current_language).to.equal('en');

    window._langscore.changeLanguage("ja", true);
    expect(window.Langscore.langscore_current_language).to.equal('ja');
  });

  it('画像ファイルを正しくロードすること', function()
  {
    if(window.StorageManager.isLocalMode())
    {
      //画像パスの自動置き換え
      const path = window.ImageManager.loadBitmap('img/pictures/', 'nantoka8').url;
      expect(path).to.equal('img/pictures/nantoka8_en.png');
    }
    {
      //CSVから画像パスを読み込み
      const path = window.ImageManager.loadBitmap('img/pictures/', 'nantoka10').url;
      expect(path).to.equal('img/pictures/nantoka8.png');
    }
  });

  it('言語を正しく変更すること', function() {
    window._langscore.changeLanguage("en");
    expect(window.Langscore.langscore_current_language).to.equal("en");
  });

  it('セーブデータが正しく保存されること', async function()
  {
    {
      window._langscore.changeLanguage("ja");

      await saveContents(1);

      var contents = await loadContents(1);
      actors = contents.actors;
      // セーブデータが正しく保存されているかの確認
      expect(actors.actor(1).name()).to.equal('エルーシェ');
      expect(actors.actor(1).nickname()).to.equal('雑用係');
      expect(actors.actor(1).profile()).to.equal('ケスティニアスの雑用係。\nそんなに仕事は無い。');
      expect(actors.actor(2).name()).to.equal('ラフィーナ');
      expect(actors.actor(2).nickname()).to.equal('傲慢ちき');
      expect(actors.actor(2).profile()).to.equal('チビのツンデレウーマン。\n魔法が得意。');

      expect(window.$gameActors.actor(1).name()).to.equal('エルーシェ');
      expect(window.$gameActors.actor(1).currentClass().name).to.equal('戦士');
      expect(window.$gameActors.actor(1).nickname()).to.equal('雑用係');
      expect(window.$gameActors.actor(1).profile()).to.equal('ケスティニアスの雑用係。\nそんなに仕事は無い。');
      expect(window.$gameActors.actor(2).name()).to.equal('ラフィーナ');
      expect(window.$gameActors.actor(2).currentClass().name).to.equal('魔術師');
      expect(window.$gameActors.actor(2).nickname()).to.equal('傲慢ちき');
      expect(window.$gameActors.actor(2).profile()).to.equal('チビのツンデレウーマン。\n魔法が得意。');
    }

    {
      window._langscore.changeLanguage("en");

      // セーブデータの作成
      await saveContents(1);

      var contents = await loadContents(1);
      actors = contents.actors;

      // セーブデータが正しく保存されているかの確認
      expect(actors.actor(1).name()).to.equal('エルーシェ');
      expect(actors.actor(1).nickname()).to.equal('雑用係');
      expect(actors.actor(1).profile()).to.equal('ケスティニアスの雑用係。\nそんなに仕事は無い。');
      expect(actors.actor(2).name()).to.equal('ラフィーナ');
      expect(actors.actor(2).nickname()).to.equal('傲慢ちき');
      expect(actors.actor(2).profile()).to.equal('チビのツンデレウーマン。\n魔法が得意。');

      expect(window.$gameActors.actor(1).name()).to.equal('eluche');
      expect(window.$gameActors.actor(1).currentClass().name).to.equal('warrior');
      expect(window.$gameActors.actor(1).nickname()).to.equal('Compassionate');
      expect(window.$gameActors.actor(1).profile()).to.equal('Kestinius\' scullery maid.\nNot that much work.');
      expect(window.$gameActors.actor(2).name()).to.equal('Rafina');
      expect(window.$gameActors.actor(2).currentClass().name).to.equal('magician');
      expect(window.$gameActors.actor(2).nickname()).to.equal('arrogant');
      expect(window.$gameActors.actor(2).profile()).to.equal('Tiny tsundere woman.\nShe is good at magic.');

      expect(window.$dataClasses[1].name).to.equal('brave');
      expect(window.$dataClasses[2].name).to.equal('warrior');
      expect(window.$dataClasses[3].name).to.equal('magician');
      expect(window.$dataClasses[4].name).to.equal('monk');

    }
  });

  it('アクター名が正しく更新されること', function()
  {
    window._langscore.changeLanguage("ja", true);
    const actor = window.$gameActors.actor(1);
    expect(actor.name()).to.equal("エルーシェ");
    expect(actor.nickname()).to.equal("雑用係");
    expect(actor.profile()).to.equal('ケスティニアスの雑用係。\nそんなに仕事は無い。');

    window._langscore.changeLanguage("en", true);
    expect(actor.name()).to.equal("eluche");
    expect(actor.nickname()).to.equal("Compassionate");
    expect(actor.profile()).to.equal(`Kestinius' scullery maid.\nNot that much work.`);

    window._langscore.changeLanguage("ja", true);
    expect(actor.name()).to.equal("エルーシェ");
    expect(actor.nickname()).to.equal("雑用係");
    expect(actor.profile()).to.equal('ケスティニアスの雑用係。\nそんなに仕事は無い。');
  });

  // data_list の各フィールドが ja→en→ja の言語変更で期待値通りに置き換わり、かつ元に戻ることを確認する。
  function expectDatabaseTranslated(label, list, pairs, fields) {
    const dict = new Map(pairs.filter(p => p.en).map(p => [p.ja, p.en]));
    window._langscore.changeLanguage("ja", true);
    const jaValues = list.map(obj => obj && fields.map(f => obj[f]));

    window._langscore.changeLanguage("en", true);
    let checked = 0;
    list.forEach((obj, i) => {
      if (!obj) { return; }
      fields.forEach((f, j) => {
        const en = dict.get(jaValues[i][j]);
        if (en) { expect(obj[f], `${label}[${i}].${f}`).to.equal(en); checked++; }
      });
    });
    expect(checked, `${label}: translated fields`).to.be.above(0);

    window._langscore.changeLanguage("ja", true);
    list.forEach((obj, i) => {
      if (obj) { fields.forEach((f, j) => expect(obj[f], `${label}[${i}].${f}`).to.equal(jaValues[i][j])); }
    });
  }

  it('スキルが正しく更新されること', function() {
    expectDatabaseTranslated('skills', window.$dataSkills, testSkills, ['name', 'description', 'message1', 'message2']);
  });

  it('アイテム・武器・防具・職業・敵・ステートが正しく更新されること', function() {
    expectDatabaseTranslated('items',   window.$dataItems,   testItems,   ['name', 'description']);
    expectDatabaseTranslated('weapons', window.$dataWeapons, testWeapons, ['name', 'description']);
    expectDatabaseTranslated('armors',  window.$dataArmors,  testArmors,  ['name', 'description']);
    expectDatabaseTranslated('classes', window.$dataClasses, testClasses, ['name']);
    expectDatabaseTranslated('enemies', window.$dataEnemies, testEnemies, ['name']);
    expectDatabaseTranslated('states',  window.$dataStates,  testStates,  ['name', 'message1', 'message2', 'message3', 'message4']);
  });

  it('システム用語と通貨単位が正しく更新されること', function() {
    window._langscore.changeLanguage("en", true);
    expect(window.TextManager.fight).to.equal('fight');
    expect(window.$dataSystem.currencyUnit).to.equal('Gol');
    expect(window.$dataSystem.elements).to.include('Thunder');
    window._langscore.changeLanguage("ja", true);
    expect(window.TextManager.fight).to.equal('戦う');
    expect(window.$dataSystem.currencyUnit).to.equal('ゴル');
    expect(window.$dataSystem.elements).to.include('雷');
  });

  it('言語変更が Langscore_ObserverBridge 経由で通知されること', function() {
    const received = [];
    const observer = lang => received.push(lang);
    window._registerLangscoreObserver(observer);
    try {
      window._langscore.changeLanguage("ja", true);
      window._langscore.changeLanguage("en", true);
      window._langscore.changeLanguage("en");        // 同じ言語への変更は通知されない
    } finally {
      window._LangscoreObserverQueue.splice(window._LangscoreObserverQueue.indexOf(observer), 1);
    }
    expect(received).to.deep.equal(['ja', 'en']);
  });

  it('Scripts.csv によりプラグインパラメータが翻訳されること', function() {
    // CustomizeConfigItem は mv_test にのみ導入されている
    const params = window.PluginManager.parameters('CustomizeConfigItem');
    if (!params['スイッチ項目']) { this.skip(); }
    const switchItemName = () => JSON.parse(JSON.parse(params['スイッチ項目'])[0]).Name;
    expect(switchItemName()).to.equal('Switch Item 1');
    window._langscore.changeLanguage("ja", true);
    expect(switchItemName()).to.equal('スイッチ項目1');
  });

  it('プラグインコマンドで言語変更と言語選択メニューの表示ができること', function() {
    const interpreter = new window.Game_Interpreter();
    const call = (command, args) => {
      if (IS_MV) { interpreter.pluginCommand('Langscore', [command, args.language]); }
      else { window.PluginManager.callCommand(interpreter, 'Langscore', command, args); }
    };
    call('changeLanguage', { language: 'ja' });
    expect(window.Langscore.langscore_current_language).to.equal('ja');
    call('changeLanguage', { language: 'xx' });   // 未サポート言語は無視
    expect(window.Langscore.langscore_current_language).to.equal('ja');

    window.SceneManager._nextScene = null;
    call('displayLanguageMenu', {});
    const scene = window.SceneManager._nextScene;
    expect(scene && scene.constructor.name).to.equal('Scene_LanguageSelect');

    // 言語選択ウィンドウ: 対応言語 + 確定/キャンセルが並び、現在の言語が選択されている
    scene.createWindowLayer();
    scene.createLanguageWindow();
    const win = scene._languageWindow;
    win.setHandler('ok', () => {});
    win.setHandler('cancel', () => {});
    const langs = win._list.filter(c => c.symbol === 'language');
    expect(langs.map(c => c.ext)).to.deep.equal(['ja', 'en']);
    expect(langs.map(c => c.name)).to.deep.equal(['► 日本語 ◄', 'English']);
    expect(win._list.map(c => c.symbol)).to.include.members(['ok', 'cancel']);
    expect(win.currentExt()).to.equal('ja');

    // English を選んで確定すると言語が変わる
    win.select(win._list.findIndex(c => c.ext === 'en'));
    win.processOk();
    expect(win.currentExt()).to.equal('en');
    win.select(win._list.findIndex(c => c.symbol === 'ok'));
    win.processOk();
    expect(window.Langscore.langscore_current_language).to.equal('en');

    window.SceneManager._nextScene = null;
    window.SceneManager._stack = [];
  });

});



describe('Langscore for Map', function()
{

  before(async () => {
    await initializeRPGMaker();
    var mapID = 1;
    window.DataManager.loadMapData(mapID);
    await waitForCondition(() => window.DataManager.isMapLoaded() === true);

    window.$gameMap.setup(mapID);
    window.$gameMap._interpreter._mapId = mapID;
  });

  it('翻訳文を正しく取得できるか', function()
  {
    window._langscore.changeLanguage("en", true);

    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.en;
      const result = window._langscore.translate_for_map(inText);
      expect(result).to.equal(outText);
    });
  });


  it('テキストの変換に成功するか', function()
  {
    window._langscore.changeLanguage("en", true);

    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.en;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });
  });

  //メモ
  //名前変更: 320
  //クラス変更: 321
  //二つ名変更: 324
  //プロフィール変更: 325
  it('アクター名の変更イベントが正しく反映されているか', function()
  {
    window._langscore.changeLanguage("en", true);

    //Game_Interpreter.update()内部で_nextSceneのチェックをしているため必須。
    window.SceneManager._nextScene = null;
    var eventId = 2;
    var evList = window.$gameMap.events();
    var ev = evList.filter((event) => { return event.eventId() == eventId; })[0];
    {
      //ダミーのインタプリタを作成。
      var interpreter = new window.Game_Interpreter();
      interpreter.setup(ev.list(), eventId);
      ev.start();
      while(interpreter.isRunning())
      {
        interpreter.update();

        //コマンド実行前の場合は配列アクセスインデックス。
        //ただし、選択処理はコマンド実行後のインデックス値なので、+1する。
        if(interpreter._index == 3){
          //["ラフィーナ","エルーシェ","やめる"]
          window.$gameMessage.onChoice(0);
          window.$gameMessage.clear();
        }
      }
    }
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("Rafina");
      window._langscore.changeLanguage("ja", true);
      expect(actor.name()).to.equal("ラフィーナ");
    }

    window._langscore.changeLanguage("en", true);
    {
      //ダミーのインタプリタを作成。
      var interpreter = new window.Game_Interpreter();
      interpreter.setup(ev.list(), eventId);
      ev.start();
      while(interpreter.isRunning())
      {
        interpreter.update();

        //コマンド実行前の場合は配列アクセスインデックス。
        //ただし、選択処理はコマンド実行後のインデックス値なので、+1する。
        if(interpreter._index == 3){
          //["ラフィーナ","エルーシェ","やめる"]
          window.$gameMessage.onChoice(1);
          window.$gameMessage.clear();
        }
      }
    }

    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("eluche");
      window._langscore.changeLanguage("ja", true);
      expect(actor.name()).to.equal("エルーシェ");
    }

  });

  it('二つ名の変更イベントが正しく反映されているか', function()
  {
    window._langscore.changeLanguage("en", true);

    //Game_Interpreter.update()内部で_nextSceneのチェックをしているため必須。
    window.SceneManager._nextScene = null;
    var eventId = 3;
    var evList = window.$gameMap.events();
    var ev = evList.filter((event) => { return event.eventId() == eventId; })[0];
    {
      //ダミーのインタプリタを作成。
      var interpreter = new window.Game_Interpreter();
      interpreter.setup(ev.list(), eventId);
      ev.start();
      while(interpreter.isRunning())
      {
        interpreter.update();

        //コマンド実行前の場合は配列アクセスインデックス。
        //ただし、選択処理はコマンド実行後のインデックス値なので、+1する。
        if(interpreter._index == 3){
          //["雑用係","傲慢ちき","無くす","やめる"]
          window.$gameMessage.onChoice(1);
          window.$gameMessage.clear();
        }
      }
    }
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.nickname()).to.equal("arrogant");
      window._langscore.changeLanguage("ja", true);
      expect(actor.nickname()).to.equal("傲慢ちき");
    }

    window._langscore.changeLanguage("en", true);
    {
      //ダミーのインタプリタを作成。
      var interpreter = new window.Game_Interpreter();
      interpreter.setup(ev.list(), eventId);
      ev.start();
      while(interpreter.isRunning())
      {
        interpreter.update();

        //コマンド実行前の場合は配列アクセスインデックス。
        //ただし、選択処理はコマンド実行後のインデックス値なので、+1する。
        if(interpreter._index == 3){
          //["雑用係","傲慢ちき","無くす","やめる"]
          window.$gameMessage.onChoice(0);
          window.$gameMessage.clear();
        }
      }
    }

    {
      const actor = window.$gameActors.actor(1);
      expect(actor.nickname()).to.equal("Compassionate");
      window._langscore.changeLanguage("ja", true);
      expect(actor.nickname()).to.equal("雑用係");
    }

  });

  it('プロフィールの変更イベントが正しく反映されているか', function()
  {
    window._langscore.changeLanguage("en", true);

    //Game_Interpreter.update()内部で_nextSceneのチェックをしているため必須。
    window.SceneManager._nextScene = null;
    var eventId = 4;
    var evList = window.$gameMap.events();
    var ev = evList.filter((event) => { return event.eventId() == eventId; })[0];
    {
      //ダミーのインタプリタを作成。
      var interpreter = new window.Game_Interpreter();
      interpreter.setup(ev.list(), eventId);
      ev.start();
      while(interpreter.isRunning())
      {
        interpreter.update();

        //コマンド実行前の場合は配列アクセスインデックス。
        //ただし、選択処理はコマンド実行後のインデックス値なので、+1する。
        if(interpreter._index == 3){
          //["ラフィーナ","エルーシェ","やめる"]
          window.$gameMessage.onChoice(0);
          window.$gameMessage.clear();
        }
      }
    }
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.profile()).to.equal("Tiny tsundere woman.\nShe is good at magic.");
      window._langscore.changeLanguage("ja", true);
      expect(actor.profile()).to.equal("チビのツンデレウーマン。\n魔法が得意。");
    }

    window._langscore.changeLanguage("en", true);
    {
      //ダミーのインタプリタを作成。
      var interpreter = new window.Game_Interpreter();
      interpreter.setup(ev.list(), eventId);
      ev.start();
      while(interpreter.isRunning())
      {
        interpreter.update();

        //コマンド実行前の場合は配列アクセスインデックス。
        //ただし、選択処理はコマンド実行後のインデックス値なので、+1する。
        if(interpreter._index == 3){
          //["ラフィーナ","エルーシェ","やめる"]
          window.$gameMessage.onChoice(1);
          window.$gameMessage.clear();
        }
      }
    }

    {
      const actor = window.$gameActors.actor(1);
      expect(actor.profile()).to.equal("Kestinius' scullery maid.\nNot that much work.");
      window._langscore.changeLanguage("ja", true);
      expect(actor.profile()).to.equal("ケスティニアスの雑用係。\nそんなに仕事は無い。");
    }

  });

  it('ショップの商品名が翻訳されているか', function()
  {
    // イベント6: アイテム1-4, 武器1-2, 防具1-2 を扱うショップ (command 302/605)
    const goodsNames = () => {
      window.SceneManager._nextScene = null;
      const ev = window.$gameMap.events().find(event => event.eventId() === 6);
      const interpreter = new window.Game_Interpreter();
      interpreter.setup(ev.list(), 6);
      interpreter.update();   // ショップ呼び出しでシーン遷移待ちになるため1回で止まる
      const scene = window.SceneManager._nextScene;
      expect(scene).to.be.instanceOf(window.Scene_Shop);
      const tables = [window.$dataItems, window.$dataWeapons, window.$dataArmors];
      const names = scene._goods.map(([type, id]) => tables[type][id].name);
      window.SceneManager._nextScene = null;
      window.SceneManager._stack = [];
      return names;
    };
    window._langscore.changeLanguage("en", true);
    expect(goodsNames()).to.deep.equal(['portion', 'magic water', 'dispersal herb', 'formula', 'Sword', 'Axe', 'Shield', 'Had']);
    window._langscore.changeLanguage("ja", true);
    expect(goodsNames()).to.deep.equal(['ポーション', 'マジックウォーター', 'ディスペルハーブ', 'スティミュラント', '剣', '斧', '盾', '帽子']);
  });

  it('セーブファイルの書き込みとロード', async function()
  {
    window._langscore.changeLanguage("ja", true);
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("エルーシェ");
    }

    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.ja;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });
    //日本語を指定した状態でセーブ
    await saveContents(1);

    //英語に変更
    window._langscore.changeLanguage("en", true);
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("eluche");
    }
    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.en;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });

    //日本語を指定したときのデータをロード
    await loadContents(1);

    //言語設定はセーブデータに影響を受けないので、英語になっていることが正しい。

    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("eluche");
    }
    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.en;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });
  });


  it('2024/06報告不具合', async function()
  {
    //ex.)ja -> enへの言語変更後にセーブを行うと、一部の制御文字を使用した
    //    文章が日本語で表示される。期待する言語は英語。
    window._langscore.changeLanguage("ja", true);
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("エルーシェ");
    }

    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.ja;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });

    //英語に変更
    window._langscore.changeLanguage("en", true);
    //日本語を指定した状態でセーブ
    await saveContents(2);

    //不具合が発生する場合、一部が日本語になるためtext.enと一致しない。
    {
      const actor = window.$gameActors.actor(1);
      expect(actor.name()).to.equal("eluche");
    }
    testMap001.map(text => {
      var inText   = text.ja;
      var outText  = text.en;
      var dymmy_window = new window.Window_Base(new window.Rectangle(0,0,0,0));
      const _actual = dymmy_window.convertEscapeCharacters(inText);
      const _expect = dymmy_window.convertEscapeCharacters(outText);
      expect(_actual).to.equal(_expect);
    });
  });

  it('2024/10報告不具合 名前入力が常に英語になる', async function()
  {
    //isJapanese系の関数が正しい値を返していない。langscore_current_language変数の名前を間違えていた。(.currentLanguage)

    var dummy_window = new window.Window_NameInput(new window.Rectangle(0,0,0,0));

    expect(window.Langscore.langscore_current_language).to.not.equal(undefined)
    window._langscore.changeLanguage("ja", true);
    expect(window.$gameSystem.isJapanese()).to.true;
    expect(dummy_window.table()[0][0]).to.equal("あ");
    window._langscore.changeLanguage("en", true);
    expect(window.$gameSystem.isJapanese()).to.not.true;
    expect(dummy_window.table()[0][0]).to.equal("A");
  });

});

describe('Language State Variable', function()
{
  before(() => initializeRPGMaker({ pluginParams: { 'Language State Variable': '1' } }));

  it('対応言語の利用可否が指定変数から順に格納されること', function() {
    // System_Allowed_Languages の順 (ja, en, zh-cn, ...) に、変数1から 1/0 が入る
    const allowed = window.Langscore.System_Allowed_Languages;
    expect(allowed.slice(0, 3)).to.deep.equal(['ja', 'en', 'zh-cn']);
    const values = allowed.map((_, i) => window.$gameVariables.value(1 + i));
    expect(values).to.deep.equal(allowed.map(lang => window.Langscore.Support_Language.includes(lang) ? 1 : 0));
  });
});

describe('Enable Translation For Default Language', function()
{
  // convertEscapeCharacters がデフォルト言語 (ja) でも翻訳処理を通すかどうか
  function countMapTranslateCalls() {
    const original = window._langscore.translate_for_map;
    let calls = 0;
    window._langscore.translate_for_map = function() { calls++; return original.apply(this, arguments); };
    try {
      window._langscore.changeLanguage("ja", true);
      new window.Window_Base(new window.Rectangle(0, 0, 0, 0)).convertEscapeCharacters('通常のテキストです');
    } finally {
      window._langscore.translate_for_map = original;
    }
    return calls;
  }

  describe('false (既定)', function() {
    before(() => initializeRPGMaker());
    it('デフォルト言語では翻訳処理を通さないこと', function() {
      expect(countMapTranslateCalls()).to.equal(0);
    });
  });

  describe('true', function() {
    before(() => initializeRPGMaker({ pluginParams: { 'Enable Translation For Default Language': 'true' } }));
    it('デフォルト言語でも翻訳処理を通すこと', function() {
      expect(countMapTranslateCalls()).to.be.above(0);
    });
  });
});

describe('Language Patch Mode', function()
{
  // data/translate/<lang>/*.csv を言語ごとに読み込むモード。NW.js (ローカル実行) 専用。
  describe('NW.js', function() {
    before(async function() {
      if (!IS_NWJS) { this.skip(); }
      await initializeRPGMaker({ pluginParams: { 'Enable Language Patch Mode': 'true' } });
      window.DataManager.loadMapData(1);
      await waitForCondition(() => window.DataManager.isMapLoaded());
      window.$gameMap.setup(1);
      window.$gameMap._interpreter._mapId = 1;   // translate_for_map はインタプリタのマップIDを参照する
    });

    it('data/translate 配下のフォルダから選択可能言語が決まること', function() {
      // ja/ と en/ が存在する。zh-cn/ は mz_test にのみ存在するが Support Language 外なので含まれない。
      expect(window._langscore.current_language_list).to.have.members(['ja', 'en']);
    });

    it('言語フォルダの翻訳が適用されること', async function() {
      // 言語変更時に現在マップの Map###.csv を言語フォルダから読み直す (非同期) ので完了を待つ
      const mapHasLanguage = lang => {
        const map = window._langscore.ls_current_map[1];
        return map instanceof window.Map && map.get('通常のテキストです') && map.get('通常のテキストです').has(lang);
      };
      window._langscore.changeLanguage("en", true);
      await waitForCondition(() => mapHasLanguage('en'));
      expect(window.$gameActors.actor(1).name()).to.equal('eluche');
      expect(window.TextManager.fight).to.equal('fight');
      expect(window._langscore.translate_for_map('通常のテキストです')).to.equal('Normal text.');

      window._langscore.changeLanguage("ja", true);
      await waitForCondition(() => mapHasLanguage('ja'));
      expect(window.$gameActors.actor(1).name()).to.equal('エルーシェ');
      expect(window._langscore.translate_for_map('通常のテキストです')).to.equal('通常のテキストです');
    });
  });

  describe('ブラウザ', function() {
    before(async function() {
      if (IS_NWJS) { this.skip(); }
      await initializeRPGMaker();
    });

    it('パッチモードでの言語変更はエラーになること', function() {
      window.Langscore.EnablePathMode = true;
      try {
        expect(() => window._langscore.changeLanguage("en", true)).to.throw(/Webブラウザ/);
      } finally {
        window.Langscore.EnablePathMode = false;
      }
    });
  });
});

describe('入力値の検証', function()
{
  // 翻訳ファイルのパスはCSV/フォルダ名から組み立てるため、外部入力として検証している。
  before(() => initializeRPGMaker());

  it('言語フォルダ名は英数字・ハイフン・アンダースコアのみ許可すること', function() {
    for (const ok of ['ja', 'en', 'zh-cn', 'pt_BR']) {
      expect(window._langscore.isValidLanguageFolder(ok), ok).to.be.true;
    }
    for (const ng of ['..', '.', 'ja/en', 'ja\\en', 'ja.csv', 'ja en', '', 'toolongfoldername', 'CON', 'nul']) {
      expect(window._langscore.isValidLanguageFolder(ng), ng).to.be.false;
    }
  });

  it('言語コードは許可リストにあるものだけを受け付けること', function() {
    const allowed = window.Langscore.System_Allowed_Languages;
    expect(allowed.length).to.be.above(0);
    for (const lang of allowed) {
      expect(window._langscore.isValidLanguageCode(lang), lang).to.be.true;
    }
    for (const ng of ['xx', '', '../ja', 'JA']) {
      expect(window._langscore.isValidLanguageCode(ng), ng).to.be.false;
    }
  });

  it('翻訳ファイル名はパストラバーサルを弾くこと', function() {
    expect(window._langscore.isValidFileName('Map001.csv')).to.be.true;
    for (const ng of ['../Map001.csv', 'a/Map001.csv', 'a\\Map001.csv', 'Map001.json', 'Map<1>.csv', '', null, undefined]) {
      expect(window._langscore.isValidFileName(ng), String(ng)).to.be.false;
    }
  });

  it('ファイル名のサニタイズで危険な文字が除去されること', function() {
    expect(window._langscore.sanitizeFileName('../Map001.csv')).to.equal('..Map001.csv');
    expect(window._langscore.sanitizeFileName('Map<1>|?.csv')).to.equal('Map1.csv');
    expect(window._langscore.sanitizeFileName('Map_001-a.csv')).to.equal('Map_001-a.csv');
  });

  it('サポート外の言語への変更は無視されること', function() {
    window._langscore.changeLanguage("ja", true);
    window._langscore.changeLanguage("xx");
    expect(window.Langscore.langscore_current_language).to.equal("ja");
  });
});

describe('言語判定とフォント', function()
{
  before(() => initializeRPGMaker());

  it('Game_System の言語判定が現在の言語に追従すること', function() {
    const gameSystem = new window.Game_System();

    window._langscore.changeLanguage("ja", true);
    expect(gameSystem.isJapanese()).to.be.true;
    expect(gameSystem.isCJK()).to.be.ok;
    expect(gameSystem.isKorean()).to.be.false;
    expect(gameSystem.isRussian()).to.be.false;
    expect(gameSystem.isChinese()).to.be.null;

    window._langscore.changeLanguage("en", true);
    expect(gameSystem.isJapanese()).to.be.false;
    expect(gameSystem.isCJK()).to.be.null;
    expect(gameSystem.isChinese()).to.be.null;
  });

  it('言語変更時に該当言語のフォント定義へ切り替わること', function() {
    for (const lang of ["ja", "en"]) {
      window._langscore.changeLanguage(lang, true);
      expect(window.Langscore.currentFont, lang).to.exist;
      expect(window.Langscore.currentFont.size, lang).to.equal(window.Langscore.FontList[lang].size);

      // MV/MZともデフォルトのM+1フォントは組み込みのフォント名に差し替えられる
      const expectedName = IS_MZ ? "rmmz-mainfont" : "GameFont";
      expect(window.Langscore.currentFont.name, lang).to.equal(expectedName);
    }
  });

  it('ウィンドウのフォント設定にLangscoreのフォントが反映されること', function() {
    window._langscore.changeLanguage("en", true);
    const font = window.Langscore.currentFont;

    if (IS_MZ) {
      const gameSystem = new window.Game_System();
      expect(gameSystem.mainFontFace()).to.include(font.name);
      expect(gameSystem.mainFontSize()).to.equal(font.size);
    }
    else {
      const window_base = new window.Window_Base(0, 0, 100, 100);
      expect(window_base.standardFontFace()).to.equal(font.name);
      expect(window_base.standardFontSize()).to.equal(font.size);
    }
  });
});
