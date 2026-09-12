// resource/langscore.rb (テンプレート) を divisi と同じ規則で展開し、
// テスト用プロジェクト (vxace_test) の Scripts へ書き出す。
// 展開規則は src/platform/divisi_vxace.cpp の createLangscoreScript() に合わせること。
const fs = require('fs');
const path = require('path');

const RESOURCE = path.join(__dirname, '..', '..', 'resource');
const PROJECT = 'vxace_test';

// src/platform/platform_base.cpp の Help_Text / Language_Items と同じ内容
const HELP_TEXT = {
    'en': 'The currently selected language is displayed.',
    'es': 'Se muestra el idioma actualmente seleccionado.',
    'de': 'Die aktuell ausgewählte Sprache wird angezeigt.',
    'fr': "La langue actuellement sélectionnée s'affiche.",
    'it': 'Viene visualizzata la lingua attualmente selezionata.',
    'ja': '現在選択中の言語が表示されます。',
    'ko': '현재 선택한 언어가 표시됩니다.',
    'ru': 'Отображается текущий выбранный язык.',
    'zh-cn': '显示当前选择的语言。',
    'zh-tw': '顯示當前選擇的語言。',
};
const LANGUAGE_ITEMS = {
    'en': 'English',
    'es': 'Español',
    'de': 'Deutsch',
    'fr': 'Français',
    'it': 'Italiano',
    'ja': '日本語',
    'ko': '한국어',
    'ru': 'Русский язык',
    'zh-cn': '中文(簡体)',
    'zh-tw': '中文(繁体)',
};

function quoteList(list) {
    return list.map(s => `"${s}"`).join(',');
}

function render(template, config) {
    const allLangs = config.Languages.map(l => l.LanguageName);
    const enabled = config.Languages.filter(l => l.Enable);
    const supportLangs = enabled.map(l => l.LanguageName);
    const write = config.Write || {};
    const lscsv = fs.readFileSync(path.join(RESOURCE, 'lscsv.rb'), 'utf-8');

    // config::outputTranslateFilePathForRPGMaker() はVXAceでは固定値を返す
    const translateFolder = 'Data/Translate';

    const table = (name, source) => [
        `\t${name} = {`,
        ...allLangs.filter(l => source[l] !== undefined).map(l => `\t\t"${l}" => "${source[l]}",`),
        '\t}',
    ].join('\n');

    return template.split(/\r?\n/).map(line => {
        if (line.includes('%{ALLOWED_LANGUAGE}%')) {
            return `\tSTSTEM_ALLOWED_LANGUAGES = [${quoteList(allLangs)}]`;
        }
        if (line.includes('%{SUPPORT_LANGUAGE}%')) {
            return `\tSUPPORT_LANGUAGE = [${quoteList(supportLangs)}]`;
        }
        if (line.includes('%{DEFAULT_LANGUAGE}%')) {
            return `\tDEFAULT_LANGUAGE = "${config.DefaultLanguage}"`;
        }
        if (line.includes('%{SUPPORT_FONTS}%')) {
            return [
                '\tLS_FONT = {',
                ...enabled.map(l => `\t\t"${l.LanguageName}" => {:name => "${l.FontName}", :size => ${l.FontSize}},`),
                '\t}',
            ].join('\n');
        }
        if (line.includes('%{TRANSLATE_FOLDER}%')) {
            return `\tTRANSLATE_FOLDER = "${translateFolder}"`;
        }
        if (line.includes('%{ENABLE_PATCH_MODE}%')) {
            return `\tENABLE_PATCH_MODE = ${write.EnableLanguagePatch ? 'true' : 'false'}`;
        }
        if (line.includes('%{ENABLE_TRANSLATION_FOR_DEFLANG}%')) {
            return `\tENABLE_TRANSLATION_FOR_DEFLANG = ${write.EnableTranslationDefLang ? 'true' : 'false'}`;
        }
        if (line.includes('%{UNISON_LSCSV}%')) {
            return lscsv;
        }
        if (line.includes('%{SYSTEM1}%')) {
            return table('SYSTEM1', HELP_TEXT);
        }
        if (line.includes('%{SYSTEM2}%')) {
            return table('SYSTEM2', LANGUAGE_ITEMS);
        }
        return line;
    }).join('\n');
}

const template = fs.readFileSync(path.join(RESOURCE, 'langscore.rb'), 'utf-8');
const configPath = path.join(__dirname, `${PROJECT}_langscore`, 'config.json');
const config = JSON.parse(fs.readFileSync(configPath, 'utf-8').replace(/^﻿/, ''));

const dest = path.join(__dirname, PROJECT, 'Scripts', 'langscore.rb');
fs.writeFileSync(dest, render(template, config), 'utf-8');
console.log(`rendered ${dest}`);
