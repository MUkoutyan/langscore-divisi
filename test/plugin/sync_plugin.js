// resource/Langscore.js (テンプレート) を divisi と同じ規則で展開し、
// テスト用プロジェクト (mv_test / mz_test) の js/plugins へ書き出す。
// 展開規則は src/platform/divisi_mvmz.cpp の formatSystemVariable() に合わせること。
const fs = require('fs');
const path = require('path');

const RESOURCE = path.join(__dirname, '..', '..', 'resource');
const PROJECTS = ['mv_test', 'mz_test'];

const BASIC_DATA = [
    'Actors', 'Animations', 'Armors', 'Classes', 'CommonEvents', 'Enemies', 'Items',
    'Map001', 'Map002', 'Map003', 'MapInfos', 'Skills', 'States', 'System', 'Tilesets', 'Troops', 'Weapons'
];

function quoteList(list) {
    return list.map(s => `"${s}"`).join(',');
}

function render(template, config) {
    const allLangs = config.Languages.map(l => l.LanguageName);
    const enabled = config.Languages.filter(l => l.Enable);
    const supportLangs = enabled.map(l => l.LanguageName);
    const lscsv = fs.readFileSync(path.join(RESOURCE, 'lscsv.js'), 'utf-8')
        .split(/\r?\n/).filter(l => !l.includes('module.exports = LSCSV;')).join('\n');

    return template.split(/\r?\n/).map(line => {
        if (line.includes('%{DEFAULT_LANGUAGE}%')) {
            return line.replace('%{DEFAULT_LANGUAGE}%', config.DefaultLanguage);
        }
        if (line.includes('%{SUPPORT_LANGUAGE}%')) {
            return `Langscore.Support_Language = [${quoteList(supportLangs)}]`;
        }
        if (line.includes('%{SUPPORT_LANGUAGE_STR}%')) {
            return line.replace('%{SUPPORT_LANGUAGE_STR}%', quoteList(supportLangs));
        }
        if (line.includes('%{ALLOWED_LANGUAGE}%')) {
            return line.replace('%{ALLOWED_LANGUAGE}%', quoteList(allLangs));
        }
        if (line.includes('%{SUPPORT_FONTS}%')) {
            const rows = enabled.map(l =>
                `\t"${l.LanguageName}": {name:"${l.FontName}", size:${l.FontSize}, fileName: "${path.basename(l.FontPath)}", isLoaded : false }, `);
            return ['Langscore.FontList = {', ...rows, '}', ''].join('\n');
        }
        if (line.includes('%{UNISON_LSCSV}%')) {
            return lscsv;
        }
        if (line.includes('%{REQUIRED_ASSETS}%')) {
            return [...BASIC_DATA, 'Graphics', 'Scripts'].map(n => ` * @requiredAssets data/translates/${n}.csv`).join('\n');
        }
        return line;
    }).join('\n');
}

for (const proj of PROJECTS) {
    const dest = path.join(__dirname, proj, 'js', 'plugins');
    if (!fs.existsSync(dest)) {
        console.log(`skip ${proj}: ${dest} not found (extract test_data.zip)`);
        continue;
    }
    const config = JSON.parse(fs.readFileSync(path.join(__dirname, `${proj}_langscore`, 'config.json'), 'utf-8'));
    const template = fs.readFileSync(path.join(RESOURCE, 'Langscore.js'), 'utf-8');
    fs.writeFileSync(path.join(dest, 'Langscore.js'), render(template, config));
    fs.copyFileSync(path.join(RESOURCE, 'Langscore_ObserverBridge.js'), path.join(dest, 'Langscore_ObserverBridge.js'));
    console.log(`synced Langscore.js -> ${proj} (${quoteList(config.Languages.filter(l => l.Enable).map(l => l.LanguageName))})`);
}
