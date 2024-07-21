#include "platform_base.h"
#include "reader/csvreader.h"
#include "writer/uniquerowcsvwriter.hpp"
#include "reader/speciftranstext.hpp"
#include <iostream>

using namespace langscore;
using namespace std::literals::string_literals;
namespace fs = std::filesystem;

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
        writeFixedTranslateText<uniquerowcsvwriter>(csvPath, transText, mergeTextMode);
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


bool platform_base::validateTranslateFileList(utility::filelist csvPathList) const
{
    const auto OutputError = [](auto path, auto type, auto errorSummary, auto lang, auto str, size_t row) {
        auto result = utility::join({type, std::to_string(errorSummary), lang, str, path.string(), std::to_string(row)}, ","s);
        std::cout << result << std::endl;
        };
    bool result = true;
    for(auto& _path : csvPathList)
    {
        const auto fileName = _path.filename().stem().string();
        //マップは\r\nだとバグるのでチェックする。
        if(fileName.find("Map") != std::string::npos && FindNewlineCR(_path)) {
            OutputError(_path, "Warning"s, IncludeCR, ""s, ""s, 0);
            continue;
        }
        auto csvReader = csvreader{this->supportLangs, {_path}};
        auto texts = csvReader.curerntTexts();

        auto csvUseLangs = csvReader.curerntUseLangList();
        if(this->supportLangs.size() != csvUseLangs.size()) {
            OutputError(_path, "Warning"s, NotEQLang, ""s, ""s, 0);
        }
        else {
            auto lang = this->supportLangs;
            std::sort(lang.begin(), lang.end());
            std::sort(csvUseLangs.begin(), csvUseLangs.end());
            if(false == std::equal(lang.begin(), lang.end(), csvUseLangs.begin(), csvUseLangs.end())) {
                OutputError(_path, "Warning"s, NotEQLang, ""s, ""s, 0);
            }
        }

        result &= validateTranslateList(std::move(texts), std::move(_path));
    }
    return result;
}

bool platform_base::validateTranslateList(std::vector<TranslateText> texts, std::filesystem::path path) const
{
    const auto OutputError = [&path](auto type, auto errorSummary, auto lang, auto str, size_t row) {
        auto result = utility::join({type, std::to_string(errorSummary), lang, str, path.string(), std::to_string(row)}, ","s);
        std::cout << result << std::endl;
    };
    size_t row = 1;
    bool result = true;
    for(auto& text : texts)
    {
        if(text.original.empty()) {
            OutputError("Error"s, EmptyCol, "original"s, ""s, row);
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
                OutputError("Error"s, NotFoundEsc, utility::cnvStr<std::string>(trans.first), escStr, row);
            }
        }
        if(emptyTextLangs.empty() == false) {
            OutputError("Warning"s, EmptyCol, utility::join(emptyTextLangs, " "s), ""s, row);
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
                OutputError("Error"s, IncludeCR, "", "", 0);
                exit = true;
                break;
            }
        }
        if(exit) { break; }
    }

    return result;
}

std::tuple<std::vector<std::u8string>, std::vector<std::u8string>> platform_base::findRPGMakerEscChars(std::u8string originalText) const
{
    static const std::vector<std::u8string> escWithValueChars = {
        u8"\\v[", u8"\\n[", u8"\\p[", u8"\\c[", u8"\\l[", u8"\\r["
        u8"\\V[", u8"\\N[", u8"\\P[", u8"\\C[", u8"\\L[", u8"\\R["
    };
    static const std::vector<std::u8string> escChars = {
        u8"\\g", u8"\\G", u8"\\{", u8"\\}", u8"\\$", u8"\\.", u8"\\|",
        u8"\\!", u8"\\>", u8"\\<", u8"\\^", u8"\\\\"
    };

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
