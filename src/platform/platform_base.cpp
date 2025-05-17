#include "platform_base.h"
#include "reader/csvreader.h"
#include "writer/uniquerowcsvwriter.hpp"
#include "reader/speciftranstext.hpp"
#include <iostream>
#include <map>
#include <set>
#include <future>

#include <unicode/utypes.h>
#include <unicode/unistr.h>
#include <unicode/ucnv.h>
#include <unicode/schriter.h>
#include <ft2build.h>
#include FT_FREETYPE_H

using namespace langscore;
using namespace std::literals::string_literals;
namespace fs = std::filesystem;



static std::u32string utf8ToUtf32(const std::u8string& utf8Text) {
    // ICUのUnicodeStringを作成
    icu::UnicodeString unicodeString = icu::UnicodeString::fromUTF8(std::string(reinterpret_cast<const char*>(utf8Text.c_str())));

    // UTF-32文字列に変換
    int32_t utf32Length = unicodeString.countChar32();
    std::u32string utf32String(utf32Length, U'\0');

    for(int32_t i = 0, j = 0; i < unicodeString.length(); ) {
        UChar32 c;
        U16_NEXT(unicodeString.getBuffer(), i, unicodeString.length(), c);
        utf32String[j++] = static_cast<char32_t>(c);
    }

    return utf32String;
}

static void OutputError(auto path, auto type, auto errorSummary, auto lang, auto str, size_t row) 
{
    nlohmann::json j;
    j["Type"] = type;             
    j["Summary"] = errorSummary;  
    j["Language"] = lang;         
    j["Details"] = str;
    j["File"] = utility::cnvStr<std::string>(path.u8string());
    j["Row"] = row;

    std::cout << j.dump() << std::endl;
}

static void OutputErrorWithWidth(auto path, auto type, auto errorSummary, auto lang, auto str, auto width, size_t row)
{
    nlohmann::json j;
    j["Type"] = type;
    j["Summary"] = errorSummary;
    j["Language"] = lang;
    j["Details"] = str;
    j["Width"] = width;
    j["File"] = utility::cnvStr<std::string>(path.u8string());
    j["Row"] = row;

    std::cout << j.dump() << std::endl;
}


std::unordered_map<std::u8string, std::u8string> platform_base::Help_Text = {
	{ u8"en"s,    u8"The currently selected language is displayed."s },
	{ u8"es"s,    u8"Se muestra el idioma actualmente seleccionado."s },
	{ u8"de"s,    u8"Die aktuell ausgewählte Sprache wird angezeigt."s },
	{ u8"fr"s,    u8"La langue actuellement sélectionnée s'affiche."s },
	{ u8"it"s,    u8"Viene visualizzata la lingua attualmente selezionata."s },
	{ u8"ja"s,    u8"現在選択中の言語が表示されます。"s },
	{ u8"ko"s,    u8"현재 선택한 언어가 표시됩니다."s },
	{ u8"ru"s,    u8"Отображается текущий выбранный язык."s },
	{ u8"zh-cn"s, u8"显示当前选择的语言。"s },
	{ u8"zh-tw"s, u8"顯示當前選擇的語言。"s },
};

std::unordered_map<std::u8string, std::u8string> platform_base::Language_Items = {
	{ u8"en"s,    u8"English"s },
	{ u8"es"s,    u8"Español"s },
	{ u8"de"s,    u8"Deutsch"s },
	{ u8"fr"s,    u8"Français"s },
	{ u8"it"s,    u8"Italiano"s },
	{ u8"ja"s,    u8"日本語"s },
	{ u8"ko"s,    u8"한국어"s },
	{ u8"ru"s,    u8"Русский язык"s },
	{ u8"zh-cn"s, u8"中文(簡体)"s },
	{ u8"zh-tw"s, u8"中文(繁体)"s },
};

std::filesystem::copy_options langscore::platform_base::convertCopyOption(MergeTextMode mode)
{
	switch(mode)
	{
		case MergeTextMode::Both:
			return std::filesystem::copy_options::overwrite_existing;
			break;
		case MergeTextMode::AcceptSource:
			return std::filesystem::copy_options::skip_existing;
			break;
		case MergeTextMode::MergeKeepSource:
			return std::filesystem::copy_options::overwrite_existing;
			break;
		case MergeTextMode::MergeKeepTarget:
			return std::filesystem::copy_options::overwrite_existing;
			break;
	}
	return std::filesystem::copy_options::none;
}

std::vector<std::filesystem::path> langscore::platform_base::exportFolderPath(std::filesystem::path fileName)
{
    config config;
    std::u8string exportPath;
    auto pathList = config.exportDirectory(exportPath);

    if(std::filesystem::exists(exportPath) == false) {
        std::filesystem::create_directories(exportPath);
    }

    std::vector<std::filesystem::path> result;
    for(auto& path : pathList)
    {
        result.emplace_back(path / fileName);
        if(std::filesystem::exists(path) == false) {
            std::filesystem::create_directories(path);
        }
    }

    return result;
}

void langscore::platform_base::copyFonts(fs::path fontDestPath)
{
    //フォントのコピー
    config config;
    const auto lsProjectPath = fs::path(config.langscoreProjectPath());

    auto globalFonts = config.globalFontList();
    auto localFonts = config.localFontList();

    if(fs::exists(fontDestPath) == false) {
        fs::create_directory(fontDestPath);
    }

    if(globalFonts.empty() == false || localFonts.empty() == false)
    {

        auto globalFontFolder = this->appPath.parent_path() / "../resources/fonts";
        for(auto& relativePath : globalFonts)
        {
            auto path = globalFontFolder / relativePath;
            if(fs::exists(path) == false) {
                std::cerr << "Warning! : not found global font file : " << path << std::endl;
                continue;
            }
            fs::copy_file(path, fontDestPath / relativePath, fs::copy_options::skip_existing);
        }
        auto localFontFolder = lsProjectPath / "Fonts";
        for(auto& relativePath : localFonts)
        {
            auto path = localFontFolder / relativePath;
            if(fs::exists(path) == false) {
                std::cerr << "Warning! : not found local font file : " << path << std::endl;
                continue;
            }
            fs::copy_file(path, fontDestPath / relativePath, fs::copy_options::skip_existing);
        }
    }
    else
    {
        auto languages = config.languages();
        for(auto& language : languages)
        {
            if(language.font.file.empty()) {
                continue;
            }
            auto destPath = fontDestPath / language.font.file.filename();
            //appPathはbin/divisi.exeとなる。
            //また、Langscore.exeから呼び出されることを想定しているため、
            //2つ階層を上に上げるとLangscore.exeと同階層になり、resourceフォルダが存在する。
            //そのため素のデバッグ実行では正しく確認できない。
            auto global = this->appPath.parent_path().parent_path() / language.font.file;
            if(fs::exists(global)) {
                fs::copy_file(global, destPath, fs::copy_options::skip_existing);
            }
            else if(fs::exists(lsProjectPath / language.font.file)) {
                fs::copy_file(lsProjectPath / language.font.file, destPath, fs::copy_options::skip_existing);
            }
        }
    }
}

utility::u8stringlist platform_base::GetScriptFileName(config& config, utility::u8stringlist scriptNameList)
{
	const auto deserializeOutPath = config.langscoreAnalyzeDirectorty();
	auto scriptCsv = plaincsvreader{deserializeOutPath + u8"/Scripts/_list.csv"s}.getPlainCsvTexts();
	utility::u8stringlist result;
	for(const auto& name : scriptNameList)
	{
		auto scriptListResult = std::find_if(scriptCsv.cbegin(), scriptCsv.cend(), [&name](const auto& x)
		{
			if(x.size() < 2){ return false; }
			const auto& scriptName = x[1];
			return scriptName == name;
		});
		if(scriptListResult != scriptCsv.cend()){
			result.emplace_back((*scriptListResult)[0]);
		}
		else{
			result.emplace_back(u8"");
		}
	}
	return result;
}

void platform_base::writeFixedGraphFileNameData()
{
    std::cout << "writeFixedGraphFileNameData" << std::endl;

    config config;
    std::u8string root;
    auto mergeTextMode = MergeTextMode::MergeKeepSource;
    auto mergeTextModeRaw = config.globalWriteMode();
    if(0 <= mergeTextModeRaw && mergeTextModeRaw <= 4) {
        mergeTextMode = static_cast<MergeTextMode>(mergeTextModeRaw);
    }

    auto ignorePictures = config.ignorePictures();
    std::vector<TranslateText> transTextList;
    for(auto& f : graphicFileList) {
        auto result = std::find(ignorePictures.cbegin(), ignorePictures.cend(), f.generic_u8string());
        if(result != ignorePictures.cend()) { continue; }
        transTextList.emplace_back(f.generic_u8string(), supportLangs);
    }

    if(config.enableLanguagePatch())
    {
        // 言語パッチモードが有効な場合、言語ごとに分けて出力
        const auto pair = config.exportDirectoryWithLang(root);

        for(auto& [lang, translateFolder] : pair)
        {
            auto outputPath = fs::path(translateFolder) / "Graphics.csv";
            std::cout << "Write Graphics : " << outputPath << std::endl;

            // 言語ごとの翻訳テキストを作成
            speciftranstext transText{std::vector<std::u8string>{lang}, transTextList};
            writeFixedTranslateText<uniquerowcsvwriter>(outputPath, transText, this->defaultLanguage, std::vector<std::u8string>{lang}, mergeTextMode);
        }
    }
    else
    {
        // 通常モードでは全言語を同じCSVに出力
        auto csvPathList = exportFolderPath("Graphics.csv");
        speciftranstext transText{supportLangs, std::move(transTextList)};

        for(auto& csvPath : csvPathList) {
            std::cout << "Write Graphics : " << csvPath << std::endl;
            writeFixedTranslateText<uniquerowcsvwriter>(csvPath, transText, this->defaultLanguage, this->supportLangs, mergeTextMode);
        }
    }

    std::cout << "Finish." << std::endl;
}

//void platform_base::writeFixedGraphFileNameData()
//{
//	std::cout << "writeFixedGraphFileNameData" << std::endl;
//
//	config config;
//	auto mergeTextMode = MergeTextMode::MergeKeepSource;
//	auto mergeTextModeRaw = config.globalWriteMode();
//	if(0 <= mergeTextModeRaw && mergeTextModeRaw <= 4){
//		mergeTextMode = static_cast<MergeTextMode>(mergeTextModeRaw);
//	}
//	auto ignorePictures = config.ignorePictures();
//	std::vector<TranslateText> transTextList;
//	for(auto& f : graphicFileList){
//		auto result = std::find(ignorePictures.cbegin(), ignorePictures.cend(), f.generic_u8string());
//		if(result != ignorePictures.cend()){ continue; }
//		transTextList.emplace_back(f.generic_u8string(), supportLangs);
//	}
//	speciftranstext transText{supportLangs, std::move(transTextList)};
//
//	auto csvPathList = exportFolderPath("Graphics.csv");
//    for(auto& csvPath : csvPathList) {
//        std::cout << "Write Graphics : " << csvPath << std::endl;
//        writeFixedTranslateText<uniquerowcsvwriter>(csvPath, transText, this->defaultLanguage, this->supportLangs, mergeTextMode);
//    }
//	std::cout << "Finish." << std::endl;
//}


bool FindNewlineCR(fs::path path)
{
    std::fstream file(path, std::ios::binary | std::ios::in);
    if(file.bad()) { return false; }

    char c = 0;
    bool findCr = false;
    while(file.get(c))
    {
        if(c == '\r') {
            findCr = true;
        }
        else if(findCr) {
            if(c == '\n') {
                std::cout << "find : " << file.tellg() << std::endl;
                return true;
            }
            else {
                findCr = false;
            }
        }

    }
    return false;
}

bool platform_base::validateTranslateFileList(std::vector<ValidateFileInfo> csvPathList) const
{
    config config;
    bool result = true;
    
    for(auto& fileInfo : csvPathList)
    {
        auto _path = fileInfo.csvPath;
        const auto fileName = _path.filename().stem().string();
        auto wroteCsvReader = csvreader{this->supportLangs, {_path}};
        
        result &= validateCsvFormat(fileInfo, wroteCsvReader);

        auto translateTexts = std::move(wroteCsvReader).currentTexts();

        //ツクールのテキストで使用する制御文字を検出。
        //[]で括る必要のある制御文字と、単体で完結する制御文字の二種類。
        auto validateTextInfoList = convertValidateTextInfo(fileName, translateTexts);

        result &= validateTextFormat(validateTextInfoList, _path);


        if(fileInfo.textValidateInfos.empty() == false)
        {
            result &= validateTexts(validateTextInfoList, fileInfo.textValidateInfos, std::move(_path));
        }
    }
    return result;
}

bool langscore::platform_base::validateCsvFormat(ValidateFileInfo& fileInfo, const csvreader& wroteCsvReader) const
{
    bool result = true;
    const auto& _path = fileInfo.csvPath;
    const auto fileName = _path.filename().stem().string();
    //マップは\r\nだとバグるのでチェックする。
    if(fileName.find("Map") != std::string::npos && FindNewlineCR(_path)) {
        OutputError(_path, ValidateErrorType::Warning, IncludeCR, ""s, ""s, 0);
        return false;
    }


    auto csvUseLangs = wroteCsvReader.curerntUseLangList();
    if(this->supportLangs.size() != csvUseLangs.size()) {
        OutputError(_path, ValidateErrorType::Warning, NotEQLang, ""s, ""s, 0);
    }
    else {
        auto lang = this->supportLangs;
        std::sort(lang.begin(), lang.end());
        std::sort(csvUseLangs.begin(), csvUseLangs.end());
        if(false == std::equal(lang.begin(), lang.end(), csvUseLangs.begin(), csvUseLangs.end())) {
            OutputError(_path, ValidateErrorType::Warning, NotEQLang, ""s, ""s, 0);
        }
    }

    {
        //csvが正しいかどうかのチェック。
        //currentTextsにすると不正な状態でも空として保持されてしまうため、
        //plaincsvreaderを使用する。
        auto plainTexts = plaincsvreader{_path}.getPlainCsvTexts();
        auto header = plainTexts.front();
        plainTexts.erase(plainTexts.begin());
        if(header.size() != csvUseLangs.size() + 1) {
            OutputError(_path, ValidateErrorType::Warning, NotEQLang, ""s, ""s, 0);
            result = false;
        }

        int rowCount = 0;
        for(auto& row : plainTexts)
        {
            if(rowCount == plainTexts.size() - 1) {
                //末行の末列は配列に追加されないため、-1までを許容する。
                if(row.size() < header.size() - 1) {
                    auto csvText = csvwriter::convertCsvText(row[0]);
                    auto text = row.empty() == false ? utility::cnvStr<std::string>(csvText) : ""s;
                    OutputError(_path, ValidateErrorType::Error, InvalidCSV, ""s, text, rowCount);
                    result = false;
                    break;
                }
            }
            else
            {
                if(row.size() != header.size()) {
                    auto csvText = csvwriter::convertCsvText(row[0]);
                    auto text = row.empty() == false ? utility::cnvStr<std::string>(csvText) : ""s;
                    OutputError(_path, ValidateErrorType::Error, InvalidCSV, ""s, text, rowCount);
                    result = false;
                    break;
                }
            }
            rowCount++;
        }
    }

    return result;
}

bool platform_base::validateTextFormat(const std::vector<ValidateTextInfo>& textsInfos, std::filesystem::path path) const
{
    //const auto OutputError = [&path](auto type, auto errorSummary, auto lang, auto str, size_t row) {
    //    auto result = utility::join({type, std::to_string(errorSummary), lang, str, path.string(), std::to_string(row)}, ","s);
    //    std::cout << result << std::endl;
    //};
    size_t row = 1;
    bool result = true;
    for(auto& textInfo : textsInfos)
    {
        //原文が空の場合はエラー(かなりやばい)
        if(textInfo.origin.original.empty()) {
            OutputError(path, ValidateErrorType::Error, EmptyCol, "original"s, ""s, row);
            result = false;
        }

        std::vector<std::string> emptyTextLangs;
        for(const auto& [lang, translatedText] : textInfo.origin.translates)
        {
            if(translatedText.empty()) {
                emptyTextLangs.emplace_back(utility::cnvStr<std::string>(lang));
                result = false;
                continue;
            }

            //制御文字の検出
            auto escStr = ""s;
            for(const auto& esc : textInfo.escWithValueChars) {
                if(translatedText.find(esc) == translatedText.npos) {
                    escStr += utility::cnvStr<std::string>(esc) + " "s;
                    result = false;
                }
            }
            for(const auto& esc : textInfo.escChars) {
                if(translatedText.find(esc) == translatedText.npos) {
                    escStr += utility::cnvStr<std::string>(esc) + " "s;
                    result = false;
                }
            }

            //原文と比較したとき、制御文字が不足している文章はエラー
            if(escStr.empty() == false) {
                OutputError(path, ValidateErrorType::Error, NotFoundEsc, utility::cnvStr<std::string>(lang), escStr, row);
            }
        }

        //翻訳文が空の言語がある場合は警告
        if(emptyTextLangs.empty() == false) {
            OutputError(path, ValidateErrorType::Warning, EmptyCol, utility::join(emptyTextLangs, " "s), ""s, row);
        }
        row++;
    }

    //Map以外はここで結果を返す
    const auto fileName = path.filename().stem();
    if(fileName.string().find("Map") != 0) {
        return result;
    }

    //Mapの場合は改行のミスを検出する
    bool exit = false;
    for(const auto& infos : textsInfos)
    {
        for(const auto& trans : infos.origin.translates)
        {
            //文章毎に出力するメリットをあまり感じないので、ファイル単位で出力する。
            if(trans.second.find(u8"\r\n") != std::u8string::npos) {
                OutputError(path, ValidateErrorType::Error, IncludeCR, "", "", 0);
                exit = true;
                break;
            }
        }
        if(exit) { break; }
    }

    return result;
}

// エスケープ文字を削除する関数
void removeEscapeCharacters(std::u8string& text, const std::vector<std::u8string>& escWithValueChars, const std::vector<std::u8string>& escChars) 
{
    for(const auto& esc : escWithValueChars) {
        auto pos = text.find(esc);
        while(pos != std::u8string::npos) {
            auto endPos = text.find(u8']', pos);
            if(endPos != std::u8string::npos) {
                endPos++;
                text.erase(pos, endPos - pos);
            }
            else {
                break;
            }
            pos = text.find(esc, pos);
        }
    }

    for(const auto& esc : escChars) {
        auto pos = text.find(esc);
        while(pos != std::u8string::npos) {
            text.erase(pos, esc.length());
            pos = text.find(esc, pos);
        }
    }
}

std::vector<platform_base::ValidateTextInfo> platform_base::convertValidateTextInfo(std::string fileName, const std::vector<TranslateText>& texts) const
{

    // テキストから右端の改行コードを削除するヘルパー関数
    const auto FormatText = [](const auto& text) {
        return utility::right_trim(text, u8"\r\n"s);
    };

    // 設定を読み込み
    config config;
    // 分析ディレクトリのパスを取得
    auto analyzeDirPath = fs::path(config.langscoreAnalyzeDirectorty());
    // 対応するCSVファイルを読み込む（テキストタイプ情報を含む）
    auto csvContents = plaincsvreader{analyzeDirPath / (fileName + ".csv")}.getPlainCsvTexts();
    // 原文とテキストタイプのマッピングを格納するマップ
    std::unordered_map<std::u8string, std::u8string> textMap;
    if(csvContents.empty() == false) {
        std::set<std::u8string> nameTypeList;
        csvContents.erase(csvContents.begin()); // ヘッダー行を削除

        // テキストリストから全ての原文をマップのキーとして登録
        for(auto& transText : texts) {
            textMap[FormatText(transText.original)] = u8"";
        }

        // CSVの内容を解析して、テキストタイプを対応する原文に関連付ける
        for(const auto& row : csvContents) {
            auto it = textMap.find(FormatText(row[0]));
            if(it != textMap.end() && row.size() > 1) {
                it->second = row[1];  // テキストタイプを設定
            }
        }
    }

    // 結果を格納するリスト
    std::vector<ValidateTextInfo> resultLists;
    // 翻訳テキストリストを処理
    for(auto& transText : texts)
    {
        ValidateTextInfo result;
        result.origin = transText;  // 原文と翻訳を含むオリジナルデータをコピー

        // テキストマップから該当するタイプを取得して設定
        auto it = textMap.find(FormatText(transText.original));
        if(it != textMap.end()) {
            result.origin.textType.emplace_back(it->second);
        }
        // 空の原文はスキップ
        if(result.origin.original.empty()) { continue; }

        // 制御文字（エスケープシーケンス）を検出
        detectConstrolChar(result);

        // 表示用のテキストを作成（制御文字を除去したもの）
        result.display.original = result.origin.original;
        result.display.translates = result.origin.translates;
        // 原文から制御文字を削除
        removeEscapeCharacters(result.display.original, result.escWithValueChars, result.escChars);

        // 各言語の翻訳テキストからも同様に制御文字を削除
        for(auto& [lang, text] : result.display.translates) {
            removeEscapeCharacters(text, result.escWithValueChars, result.escChars);
        }

        // 結果リストに追加
        resultLists.emplace_back(std::move(result));
    }

    return resultLists;
}


static const std::vector<std::u8string> escWithValueChars = {
    u8"\\v[", u8"\\n[", u8"\\p[", u8"\\c[", u8"\\l[", u8"\\r[",
    u8"\\V[", u8"\\N[", u8"\\P[", u8"\\C[", u8"\\L[", u8"\\R["
};
static const std::vector<std::u8string> escChars = {
    u8"\\g", u8"\\G", u8"\\{", u8"\\}", u8"\\$", u8"\\.", u8"\\|",
    u8"\\!", u8"\\>", u8"\\<", u8"\\^", u8"\\\\"
};
void langscore::platform_base::detectConstrolChar(ValidateTextInfo& validateInfo) const
{
    auto text = validateInfo.origin.original;
    for(const auto& c : escWithValueChars)
    {
        auto pos = text.find(c);
        auto offset = 0;
        for(; pos != text.npos; pos = text.find(c, offset))
        {
            auto endPos = text.find(u8']', pos);
            if(endPos == text.npos) {
                break;
            }
            endPos++;
            auto substr_result = text.substr(pos, endPos - pos);
            offset = endPos;
            validateInfo.escWithValueChars.emplace_back(std::move(substr_result));
        }
    }
    std::sort(validateInfo.escWithValueChars.begin(), validateInfo.escWithValueChars.end());
    validateInfo.escWithValueChars.erase(std::unique(validateInfo.escWithValueChars.begin(), validateInfo.escWithValueChars.end()), validateInfo.escWithValueChars.end());

    for(const auto& c : escChars)
    {
        auto pos = text.find(c);
        for(; pos != text.npos; pos = text.find(c)) {
            validateInfo.escChars.emplace_back(text.substr(pos, c.length()));
            break;
        }
    }
    std::sort(validateInfo.escChars.begin(), validateInfo.escChars.end());
    validateInfo.escChars.erase(std::unique(validateInfo.escChars.begin(), validateInfo.escChars.end()), validateInfo.escChars.end());

    config config;
    auto exCharList = config.extendControlCharList();
    for(const auto& ctrlChar : exCharList)
    {
        auto c = u8"\\"s + ctrlChar;
        auto pos = text.find(c);
        if(c.find(u8"[") != std::u8string::npos)
        {
            auto offset = 0;
            for(; pos != text.npos; pos = text.find(c, offset))
            {
                auto endPos = text.find(u8']', pos);
                if(endPos == text.npos) {
                    break;
                }
                endPos++;
                auto substr_result = text.substr(pos, endPos - pos);
                offset = endPos;
                validateInfo.escWithValueChars.emplace_back(std::move(substr_result));
            }
        }
        else
        {
            for(; pos != text.npos; pos = text.find(c)) {
                validateInfo.escChars.emplace_back(text.substr(pos, c.length()));
                break;
            }
        }
    }
}

std::uint64_t langscore::platform_base::countNumTexts(const std::u8string& multilineText) const
{
    auto texts = utility::split(multilineText, u8"\n"s);
    int maxNumTexts = 0;
    for(auto&& text : texts)
    {
        //アイコンサイズを考慮する。
        //文字の拡大・縮小時のサイズは考慮しない。(width側では考慮する)
        //アイコンサイズは1文字としてカウントする？

        auto textStr = utility::cnvStr<std::string>(std::move(text));
        auto icuStr = icu::UnicodeString::fromUTF8(textStr.c_str());
        icu::StringCharacterIterator it(icuStr);
        int count = 0;
        //コードユニットが2つ以上の文字を考慮するため、next32を使用する。(🌏などの絵文字)
        for(UChar32 cp = it.first32(); it.hasNext(); cp = it.next32()) {
            count++;
        }
        if(maxNumTexts < count) {
            maxNumTexts = count;
        }
    }
    return maxNumTexts;
}

bool langscore::platform_base::validateTexts(const std::vector<ValidateTextInfo>& translateList, const config::TextValidateTypeMap& validateInfoList, std::filesystem::path path) const
{
    config config;
    auto langs = config.languages();
    std::map<std::u8string, config::Language> langMap;
    for(auto& lang : langs)
    {
        langMap[utility::cnvStr<std::u8string>(lang.name)] = lang;
    }

    if(validateInfoList.empty()) {
        return true;
    }

    auto fontDir = this->getGameProjectFontDirectory();
    size_t row = 1; //表示側は1開始なのでrowは1から開始する。
    for(auto& translate : translateList)
    {
        for(const auto& textType : translate.origin.textType)
        {
            if(validateInfoList.find(textType) == validateInfoList.end()) {
                continue;
            }

            const auto& validateLangMap = validateInfoList.at(textType);

            for(const auto& [langText, textLines] : translate.display.translates)
            {
                if(textLines.find(u8"Did you come here to see") != std::u8string::npos) {
                    std::cout << "debug" << std::endl;
                }
                if(validateLangMap.find(langText) == validateLangMap.end()) {
                    continue;
                }

                auto& lang = langMap[langText];
                const auto& validateInfo = validateLangMap.at(langText);
                if(validateInfo.mode == config::ValidateTextMode::Ignore) {
                    continue;
                }
                else if(validateInfo.mode == config::ValidateTextMode::TextCount)
                {
                    auto maxNumTexts = countNumTexts(textLines);

                    if(validateInfo.count < maxNumTexts) {
                        OutputErrorWithWidth(path, ValidateErrorType::Warning, OverTextCount, lang.name, utility::cnvStr<std::string>(csvwriter::convertCsvText(textLines)), maxNumTexts, row);
                    }
                    else {
                        //該当のテキストタイプが見つからない。更新忘れ？エラーを出す。
                    }
                }
                else if(validateInfo.mode == config::ValidateTextMode::TextWidth)
                {
                    auto fontName = lang.font.file.filename();
                    auto textInfos = measureTextWidth(textLines, (fontDir / fontName).u8string(), lang.font.size);
                    if(textInfos.empty()) {
                        continue;
                    }
                    //textInfosの末尾から検索し、detectWidthListの値を超えるものがあればエラーを出力する。
                    auto width_result = std::max_element(textInfos.begin(), textInfos.end(), [](auto& a, auto& b) { return a.second.right < b.second.right; });
                    int width = 0;
                    if(width_result != textInfos.end()) {
                        width = width_result->second.right;
                    }

                    if(validateInfo.width < width) {
                        OutputErrorWithWidth(path, ValidateErrorType::Warning, PartiallyClipped, lang.name, utility::cnvStr<std::string>(csvwriter::convertCsvText(textLines)), width, row);
                    }
                    else {
                        //該当のテキストタイプが見つからない。更新忘れ？エラーを出す。
                    }
                }
            }
        }
        row++;
    }
    return true;
}

//freeTypeを使用して、文字の横幅を計測する。
std::vector<std::pair<std::u8string, platform_base::TextHorizontalLength>> platform_base::measureTextWidth(std::u8string baseText, std::u8string fontPath, int fontSize) const
{
    FT_Library library;
    FT_Face face;

    // FreeTypeライブラリを初期化
    if(FT_Init_FreeType(&library)) {
        std::cerr << "Could not initialize FreeType library" << std::endl;
        return {};
    }

    // フォントファイルをロード (例: Arial.ttf)
    auto pathStr = utility::cnvStr<std::string>(fontPath);
    if(FT_New_Face(library, pathStr.c_str(), 0, &face)) {
        std::cerr << "Could not load font" << std::endl;
        FT_Done_FreeType(library);
        return {};
    }

    // フォントサイズを設定 (単位: 1/64ピクセル)
    FT_Set_Pixel_Sizes(face, 0, fontSize); // 高さ48ピクセルに設定


    //icuでtextを1文字ずつ取り出してstd::vector<char32_t>に格納する。
    auto texts = utility::split(baseText, u8"\n"s);
    std::vector<std::pair<std::u8string, TextHorizontalLength>> characters;

    //VXAce: フォントサイズは 8～72 の範囲を取る。拡縮の制御文字を適用すると+-8ずつ変化する。
    //MV/MZ: フォントサイズは12～108の範囲を取る。拡縮の制御文字を適用すると+-12ずつ変化する。
    for(auto&& text : texts)
    {
        auto textStr = utility::cnvStr<std::string>(std::move(text));
        auto icuStr = icu::UnicodeString::fromUTF8(textStr.c_str());
        icu::StringCharacterIterator it(icuStr);

        int left = 0;

        for(UChar32 cp = it.first32(); it.hasNext(); cp = it.next32())
        {
            icu::UnicodeString charStr(cp);

            //std::u32string utf32Text = utf8ToUtf32(utf8Char);
            if(FT_Load_Char(face, charStr.char32At(0), FT_LOAD_RENDER)) {
                std::cerr << "Could not load character " << int(cp) << std::endl;
                continue;
            }

            int right = left + (face->glyph->advance.x >> 6);  // 1/64ピクセルをピクセルに変換

            std::string strTmp;
            charStr.toUTF8String(strTmp);
            std::u8string utf8Char = utility::cnvStr<std::u8string>(strTmp);
            characters.emplace_back(
                std::make_pair(utf8Char, TextHorizontalLength{left, right})
            );
            left = right;
        }
    }

    // リソースを解放
    FT_Done_Face(face);
    FT_Done_FreeType(library);

    return characters;
}