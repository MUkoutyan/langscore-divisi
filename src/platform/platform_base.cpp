#include "platform_base.h"
#include "reader/csvreader.h"
#include "writer/uniquerowcsvwriter.hpp"
#include "reader/speciftranstext.hpp"
#include <iostream>

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
	auto mergeTextMode = MergeTextMode::MergeKeepSource;
	auto mergeTextModeRaw = config.globalWriteMode();
	if(0 <= mergeTextModeRaw && mergeTextModeRaw <= 4){
		mergeTextMode = static_cast<MergeTextMode>(mergeTextModeRaw);
	}
	auto ignorePictures = config.ignorePictures();
	std::vector<TranslateText> transTextList;
	for(auto& f : graphicFileList){
		auto result = std::find(ignorePictures.cbegin(), ignorePictures.cend(), f.generic_u8string());
		if(result != ignorePictures.cend()){ continue; }
		transTextList.emplace_back(f.generic_u8string(), supportLangs);
	}
	speciftranstext transText{supportLangs, std::move(transTextList)};

	auto csvPathList = exportFolderPath("Graphics.csv");
    for(auto& csvPath : csvPathList) {
        std::cout << "Write Graphics : " << csvPath << std::endl;
        writeFixedTranslateText<uniquerowcsvwriter>(csvPath, transText, this->defaultLanguage, this->supportLangs, mergeTextMode);
    }
	std::cout << "Finish." << std::endl;
}


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
    //const auto OutputError = [](auto path, auto type, auto errorSummary, auto lang, auto str, size_t row) {
    //    auto result = utility::join({type, std::to_string(errorSummary), lang, str, path.string(), std::to_string(row)}, ","s);
    //    std::cout << result << std::endl;
    //};
    bool result = true;
    config config;
    for(auto& fileInfo : csvPathList)
    {
        const auto& _path = fileInfo.csvPath;
        const auto fileName = _path.filename().stem().string();
        //マップは\r\nだとバグるのでチェックする。
        if(fileName.find("Map") != std::string::npos && FindNewlineCR(_path)) {
            OutputError(_path, ValidateErrorType::Warning, IncludeCR, ""s, ""s, 0);
            continue;
        }

        auto csvReader = csvreader{this->supportLangs, {_path}};

        auto csvUseLangs = csvReader.curerntUseLangList();
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

        auto& texts = csvReader.currentTexts();
        result &= validateTranslateList(texts, _path);

        if(fileInfo.textMode == config::ValidateTextMode::TextCount) {
            result &= validateTextCount(std::move(texts), fileInfo.textValidateSize, std::move(_path));
        }
        else if(fileInfo.textMode == config::ValidateTextMode::TextWidth) {
            result &= validateTextWidth(std::move(texts), fileInfo.textValidateSize, std::move(_path));
        }
    }
    return result;
}

bool platform_base::validateTranslateList(std::vector<TranslateText> texts, std::filesystem::path path) const
{
    //const auto OutputError = [&path](auto type, auto errorSummary, auto lang, auto str, size_t row) {
    //    auto result = utility::join({type, std::to_string(errorSummary), lang, str, path.string(), std::to_string(row)}, ","s);
    //    std::cout << result << std::endl;
    //};
    size_t row = 1;
    bool result = true;
    for(auto& text : texts)
    {
        if(text.original.empty()) {
            OutputError(path, ValidateErrorType::Error, EmptyCol, "original"s, ""s, row);
            result = false;
        }
        //ツクールのテキストで使用する制御文字を検出。
        //[]で括る必要のある制御文字と、単体で完結する制御文字の二種類。
        auto [withValEscList, EscList] = findRPGMakerEscChars(text.original);

        std::vector<std::string> emptyTextLangs;
        for(auto& trans : text.translates)
        {
            const auto& translatedText = trans.second;
            if(translatedText.empty()) {
                emptyTextLangs.emplace_back(utility::cnvStr<std::string>(trans.first));
                result = false;
                continue;
            }

            //制御文字の検出
            auto escStr = ""s;
            for(auto& esc : withValEscList) {
                if(translatedText.find(esc) == translatedText.npos) {
                    escStr += utility::cnvStr<std::string>(esc) + " "s;
                    result = false;
                }
            }
            for(auto& esc : EscList) {
                if(translatedText.find(esc) == translatedText.npos) {
                    escStr += utility::cnvStr<std::string>(esc) + " "s;
                    result = false;
                }
            }

            if(escStr.empty() == false) {
                OutputError(path, ValidateErrorType::Error, NotFoundEsc, utility::cnvStr<std::string>(trans.first), escStr, row);
            }
        }
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
    for(auto& text : texts)
    {
        for(auto& trans : text.translates)
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

static const std::vector<std::u8string> escWithValueChars = {
    u8"\\v[", u8"\\n[", u8"\\p[", u8"\\c[", u8"\\l[", u8"\\r["
    u8"\\V[", u8"\\N[", u8"\\P[", u8"\\C[", u8"\\L[", u8"\\R["
};
static const std::vector<std::u8string> escChars = {
    u8"\\g", u8"\\G", u8"\\{", u8"\\}", u8"\\$", u8"\\.", u8"\\|",
    u8"\\!", u8"\\>", u8"\\<", u8"\\^", u8"\\\\"
};
std::tuple<std::vector<std::u8string>, std::vector<std::u8string>> platform_base::findRPGMakerEscChars(std::u8string originalText) const
{
    std::vector<std::u8string> result1;
    std::vector<std::u8string> result2;
    if(originalText.empty()) { return std::forward_as_tuple(result1, result2); }

    auto text = originalText;
    std::transform(text.begin(), text.end(), text.data(), ::tolower);
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
            auto result = originalText.substr(pos, endPos - pos);
            offset = endPos;
            result1.emplace_back(std::move(result));
        }
    }
    std::sort(result1.begin(), result1.end());
    result1.erase(std::unique(result1.begin(), result1.end()), result1.end());

    for(const auto& c : escChars)
    {
        auto pos = text.find(c);
        for(; pos != text.npos; pos = text.find(c)) {
            result2.emplace_back(originalText.substr(pos, c.length()));
            break;
        }
    }
    std::sort(result2.begin(), result2.end());
    result2.erase(std::unique(result2.begin(), result2.end()), result2.end());

    return std::forward_as_tuple(result1, result2);
}

std::u8string langscore::platform_base::convertDisplayTexts(std::u8string originalText) const
{
    if(originalText.empty()) { return originalText; }

    auto text = originalText;
    std::transform(text.begin(), text.end(), text.data(), ::tolower);
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

            auto result = text.substr(pos, endPos - pos);
            text = utility::replace(text, result, u8""s);
        }
    } 

    for(const auto& c : escChars)
    {
        text = utility::replace(text, c, u8""s);
    }

    text = utility::replace(text, u8"\\\""s, u8"\""s);

    return text;
}

bool langscore::platform_base::validateTextCount(std::vector<TranslateText> translateList, std::vector<std::uint16_t> detectCountList, std::filesystem::path path) const
{
    //const auto OutputError = [&path](auto type, auto errorSummary, auto lang, auto str, size_t row) {
    //    auto result = utility::join({type, std::to_string(errorSummary), lang, str, path.string(), std::to_string(row)}, ","s);
    //    std::cout << result << std::endl;
    //};
    config config;
    auto langs = config.languages();
    std::map<std::u8string, config::Language> langMap;
    for(auto& lang : langs)
    {
        langMap[utility::cnvStr<std::u8string>(lang.name)] = lang;
    }

    if(detectCountList.empty()) {
        return true;
    }
    size_t row = 0;
    for(auto& translate : translateList)
    {
        for(auto& pair : translate.translates)
        {
            auto& lang = langMap[pair.first];

            auto texts = utility::split(pair.second, u8"\n"s);
            int maxNumTexts = 0;
            for(auto&& text : texts)
            {
                auto removed_esc_text = convertDisplayTexts(std::move(text));
                //アイコンサイズを考慮する。
                //文字の拡大・縮小時のサイズは考慮しない。(width側では考慮する)
                //アイコンサイズは1文字としてカウントする？

                auto textStr = utility::cnvStr<std::string>(std::move(removed_esc_text));
                auto icuStr = icu::UnicodeString::fromUTF8(textStr.c_str());
                icu::StringCharacterIterator it(icuStr);
                int count = 0;
                for(UChar32 cp = it.first32(); it.hasNext(); cp = it.next()) {
                    count++;
                }
                if(maxNumTexts < count) {
                    maxNumTexts = count;
                }
            }

            //if(translate.hasFaceGraphic) {
            //    if(2 <= detectCountList.size() && detectCountList[1] < maxNumTexts) {
            //        OutputError(path, ValidateErrorType::Warning, OverTextCount, utility::cnvStr<std::string>(pair.first), utility::cnvStr<std::string>(pair.second), row);
            //    }
            //}
            //else {
            //    if(1 <= detectCountList.size() && detectCountList[0] < maxNumTexts) {
            //        OutputError(path, ValidateErrorType::Warning, OverTextCount, utility::cnvStr<std::string>(pair.first), utility::cnvStr<std::string>(pair.second), row);
            //    }
            //}
        }
        row++;
    }
    return true;
}

bool langscore::platform_base::validateTextWidth(std::vector<TranslateText> texts, std::vector<std::uint16_t> detectWidthList, std::filesystem::path path) const
{
    //const auto OutputError = [&path](auto type, auto errorSummary, auto width, auto str, size_t row) {
    //    auto result = utility::join({type, std::to_string(errorSummary), width, str, path.string(), std::to_string(row)}, ","s);
    //    std::cout << result << std::endl;
    //};

    config config;
    auto langs = config.languages();
    std::map<std::u8string, config::Language> langMap;
    for(auto& lang : langs)
    {
        langMap[utility::cnvStr<std::u8string>(lang.name)] = lang;
    }

    if(detectWidthList.empty()) {
        return true;
    }
    auto gameProjDir = fs::absolute(fs::path(config.langscoreProjectPath()) / config.gameProjectPath());
    auto fontDir = this->getGameProjectFontDirectory();
    size_t row = 0;
    for(auto& text : texts)
    {
        for(auto& pair : text.translates)
        {
            const auto& lang = langMap[pair.first];
            auto fontName = lang.font.file.filename();
            auto textInfos = measureTextWidth(convertDisplayTexts(pair.second), (fontDir / fontName).u8string(), lang.font.size);
            if(textInfos.empty()) {
                continue;
            }
            //textInfosの末尾から検索し、detectWidthListの値を超えるものがあればエラーを出力する。
            auto width_result = std::max_element(textInfos.begin(), textInfos.end(), [](auto& a, auto& b) { return a.second.right < b.second.right; });
            int width = 0;
            if(width_result != textInfos.end()) {
                width = width_result->second.right;
            }
            
            //if(text.hasFaceGraphic) {
            //    if(2 <= detectWidthList.size() && detectWidthList[1] < width) {
            //        OutputErrorWithWidth(path, ValidateErrorType::Warning, PartiallyClipped, ""s, utility::cnvStr<std::string>(csvwriter::convertCsvText(pair.second)), width, row);
            //    }
            //}
            //else {
            //    if(1 <= detectWidthList.size() && detectWidthList[0] < width) {
            //        OutputErrorWithWidth(path, ValidateErrorType::Warning, PartiallyClipped, ""s, utility::cnvStr<std::string>(csvwriter::convertCsvText(pair.second)), width, row);
            //    }
            //}
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

        for(UChar32 cp = it.first32(); it.hasNext(); cp = it.next())
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