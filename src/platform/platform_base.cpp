#include "platform_base.h"
#include "reader/csvreader.h"
#include "writer/uniquerowcsvwriter.hpp"
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

void langscore::platform_base::copyFonts()
{
    //フォントのコピー
    config config;
    const auto lsProjectPath = fs::path(config.langscoreProjectPath());

    auto globalFonts = config.globalFontList();
    auto localFonts = config.localFontList();

    if(globalFonts.empty() && localFonts.empty()){ return; }

    auto fontDestPath = fs::path(config.gameProjectPath()) / u8"Fonts"s;
    if(fs::exists(fontDestPath) == false){
        fs::create_directory(fontDestPath);
    }

    auto globalFontFolder = this->appPath.parent_path() / "../resources/fonts";
    for(auto& relativePath : globalFonts)
    {
        auto path = globalFontFolder / relativePath;
        if(fs::exists(path) == false){
            std::cerr << "Warning! : not found global font file : " << path << std::endl;
            continue;
        }
        fs::copy_file(path, fontDestPath / relativePath, fs::copy_options::skip_existing);
    }
    auto localFontFolder = lsProjectPath / "Fonts";
    for(auto& relativePath : localFonts)
    {
        auto path = localFontFolder / relativePath;
        if(fs::exists(path) == false){
            std::cerr << "Warning! : not found local font file : " << path << std::endl;
            continue;
        }
        fs::copy_file(path, fontDestPath / relativePath, fs::copy_options::skip_existing);
    }
}

utility::u8stringlist platform_base::GetScriptFileName(config& config, utility::u8stringlist scriptNameList)
{
	const auto deserializeOutPath = config.langscoreAnalyzeDirectorty();
	csvreader scriptCsvReader;
	auto scriptCsv = scriptCsvReader.parsePlain(deserializeOutPath + u8"/Scripts/_list.csv"s);
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

	auto csvPath = exportFolderPath("Graphics.csv");
	std::cout << "Write Graphics : " << csvPath << std::endl;
	writeFixedTranslateText<uniquerowcsvwriter>(csvPath, transTextList, mergeTextMode);
	std::cout << "Finish." << std::endl;
}