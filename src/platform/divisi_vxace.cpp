#include "divisi_vxace.h"
#include "config.h"

#include "../writer/rbscriptwriter.h"
#include "../writer/csvwriter.h"
#include "../reader/csvreader.h"

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>
#include <iostream>

using namespace langscore;
using namespace std::literals::string_literals;
namespace fs = std::filesystem;

namespace
{
    constexpr char Script_File_Name[] = "langscore.rb";
    constexpr char Custom_Script_File_Name[] = "langscore_custom.rb";

    std::unordered_map<std::u8string, std::u8string> Help_Text = {
        { u8"en",    u8"The currently selected language is displayed." },
        { u8"es",    u8"Se muestra el idioma actualmente seleccionado." },
        { u8"de",    u8"Die aktuell ausgewählte Sprache wird angezeigt." },
        { u8"fr",    u8"La langue actuellement sélectionnée s'affiche." },
        { u8"it",    u8"Viene visualizzata la lingua attualmente selezionata." },
        { u8"ja",    u8"現在選択中の言語が表示されます。" },
        { u8"ko",    u8"현재 선택한 언어가 표시됩니다." },
        { u8"ru",    u8"Отображается текущий выбранный язык." },
        { u8"zh-cn", u8"显示当前选择的语言。" },
        { u8"zh-tw", u8"顯示當前選擇的語言。" },
    };

    std::unordered_map<std::u8string, std::u8string> Language_Items = {
        { u8"en",    u8"English" },
        { u8"es",    u8"Español" },
        { u8"de",    u8"Deutsch" },
        { u8"fr",    u8"Français" },
        { u8"it",    u8"Italiano" },
        { u8"ja",    u8"日本語" },
        { u8"ko",    u8"한국어" },
        { u8"ru",    u8"Русский язык" },
        { u8"zh-cn", u8"中文(簡体)" },
        { u8"zh-tw", u8"中文(繁体)" },
    };

    const auto nl = u8"\n"s;
    const auto tab = u8"\t"s;
}

divisi_vxace::divisi_vxace()
    : platform_base()
{
    config config;
   
    for(auto langs = config.languages(); auto lang : langs){
        this->supportLangs.emplace_back(utility::cnvStr<std::u8string>(lang.name));
    }
}

divisi_vxace::~divisi_vxace(){}

void divisi_vxace::setProjectPath(std::filesystem::path path)
{
    this->invoker.setProjectPath(invoker::ProjectType::VXAce, std::move(path));

}

std::filesystem::path langscore::divisi_vxace::outputProjectDataPath(std::filesystem::path fileName, std::filesystem::path dir)
{
    config config;
    if(dir.empty()){
        dir = config.outputTranslateFilePathForRPGMaker();
    }
    const auto& projectPath = config.projectPath();

    fs::path to = projectPath / dir;

    if(std::filesystem::exists(to) == false){
        std::filesystem::create_directories(to);
    }

    return to /= fileName;
}

void divisi_vxace::analyze()
{
    auto runResult = this->invoker.analyze();
    if(runResult.val() != 0){
        std::cerr << runResult.toStr() << std::endl;
        return;
    }

    fetchFilePathList();

    this->writeAnalyzedData();
    this->writeAnalyzedRvScript();

    std::cout << "AnalyzeProject Done." << std::endl;
}

void langscore::divisi_vxace::write()
{
    std::cout << "Write..." << std::endl;
    fetchFilePathList(); 

    writeFixedData();
    writeFixedRvScript();
    writeFixedGraphFileNameData();

    config config;
    const auto deserializeOutPath = fs::path(config.tempDirectorty());
    auto outputPath = std::filesystem::path(deserializeOutPath / "Scripts");
    if(std::filesystem::exists(outputPath) == false){
        std::filesystem::create_directories(outputPath);
    }

    //unison_custom.rbの出力
    if(fs::exists(outputPath / ::Custom_Script_File_Name) == false){
        std::cout << "Write langscore_custom : " << outputPath / ::Custom_Script_File_Name << std::endl;
        writeFixedTranslateText<rbscriptwriter>(outputPath / ::Custom_Script_File_Name, scriptFileList, langscore::OverwriteTextMode::LeaveOldNonBlank);
    }

    //unison.rbの出力
    auto outputScriptFilePath = outputPath / Script_File_Name;
    auto resourceFolder = this->appPath.parent_path() / "resource";
    std::cout << "Copy langscore : From " << resourceFolder / Script_File_Name << " To : " << outputScriptFilePath << std::endl;
    fs::copy(resourceFolder / Script_File_Name, outputScriptFilePath, std::filesystem::copy_options::overwrite_existing);

    auto fileLines = formatSystemVariable(outputScriptFilePath);

    if(fs::exists(outputScriptFilePath))
    {
        std::cout << "Replace langscore : " << outputScriptFilePath << std::endl;
        std::ofstream outScriptFile(outputScriptFilePath, std::ios_base::trunc);
        for(const auto& l : fileLines){
            outScriptFile << utility::toString(l) << "\n";
        }
    }

    auto runResult = this->invoker.recompressVXAce();
    if(runResult.val() != 0){
        std::cerr << runResult.toStr() << std::endl;
        return;
    }

    std::cout << "Write Translate File Done." << std::endl;
}

void langscore::divisi_vxace::fetchFilePathList()
{
    config config;
    const auto deserializeOutPath = config.tempDirectorty();
    fs::recursive_directory_iterator dataItr(deserializeOutPath);
    for(auto& f : dataItr)
    {
        auto extension = f.path().extension();
        if(extension == ".json"){
            this->dataFileList.emplace_back(f.path());
        }
        else if(extension == ".rb")
        {
            auto fileName = f.path().filename();
            if(fileName == ::Script_File_Name){ continue; }
            else if(fileName == ::Custom_Script_File_Name){ continue; }

            this->scriptFileList.emplace_back(f.path());
        }
    }

    auto projectPath = fs::path(config.projectPath());
    auto graphicsPath = projectPath / "Graphics";
    fs::recursive_directory_iterator graphItr(graphicsPath);
    for(auto& f : graphItr)
    {
        if(f.is_directory()){ continue; }

        //パス区切り文字は\\ではなく/に統一(\\はRubyで読み取れない)
        auto relative_path = f.path().lexically_relative(projectPath);
        this->graphicFileList.emplace_back(relative_path.replace_extension(""));
    }
}

void divisi_vxace::writeAnalyzedData()
{
    std::cout << "writeAnalyzedData" << std::endl;
    for(auto& path : dataFileList)
    {
        std::ifstream loadFile(path);
        nlohmann::json json;
        loadFile >> json;

        auto csvFilePath = path;
        csvFilePath.make_preferred().replace_extension(".csv");
        std::cout << "Write CSV : " << csvFilePath << std::endl;

        writeAnalyzeTranslateText<csvwriter>(csvFilePath, json, langscore::OverwriteTextMode::Both);
    }
    std::cout << "Finish." << std::endl;
}

void divisi_vxace::writeAnalyzedRvScript()
{
    //解析直後の素のデータを書き出す。
    //無視リスト等は考慮せず、書き出し先はTempフォルダ以下になる。
    std::cout << "writeAnalyzedRvScript" << std::endl;

    //Rubyスクリプトを予め解析してテキストを生成しておく。
    auto scriptList = scriptFileList;
    rbscriptwriter scriptWriter(this->supportLangs, scriptList);
    auto& transTexts = scriptWriter.curerntTexts();

    config config;
    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
    for(auto& t : transTexts){
        if(t.translates.find(def_lang) == t.translates.end()){ continue; }
        t.translates[def_lang] = t.original;
    }

    //デフォルトスクリプトのVocab.rb内の文字列は予め対応
    auto resourceFolder = this->appPath.parent_path() / "resource";
    csvreader reader;
    auto vocabs = reader.parse(resourceFolder/"vocab.csv");
    auto& scriptTrans = scriptWriter.getScriptTexts();
    for(auto& pathPair : scriptTrans)
    {
        if(pathPair.first.filename() != "Vocab.rb"){ continue; }

        const auto searchFunc = [&](const auto& x){
            return x.memo.find(u8"Vocab.rb") != std::u8string::npos;
        };
        auto begin = std::find_if(transTexts.begin(), transTexts.end(), searchFunc);
        auto end = std::find_if(transTexts.rbegin(), transTexts.rend(), searchFunc).base();

        std::for_each(begin, end, [&](auto& texts)
        {
            auto result = std::find_if(vocabs.begin(), vocabs.end(), [&texts](const auto& x){
                return x.original == texts.original;
            });
            if(result == vocabs.end()){ return; }
            for(const auto& key : this->supportLangs){
                if(result->translates.find(key) != result->translates.end()){
                    texts.translates[key] = result->translates[key];
                }
                else if(key == u8"ja"){
                    texts.translates[key] = result->original;
                }
            }
        });
        break;
    }
    //=================================

    const auto deserializeOutPath = fs::path(config.tempDirectorty());
    auto outputPath = deserializeOutPath / "Scripts";
    if(std::filesystem::exists(outputPath) == false){
        std::filesystem::create_directories(outputPath);
    }

    //memoに行数が格納されているため入れ替え
    for(auto& t : transTexts){
        t.memo.swap(t.original);
        t.translates[u8"memo"s] = t.memo;
    }

    std::cout << "Write CSV : " << outputPath << std::endl;
    writeAnalyzeTranslateText<csvwriter>(outputPath, transTexts, OverwriteTextMode::LeaveOld, false);

    std::cout << "Finish." << std::endl;
}


void langscore::divisi_vxace::writeFixedData()
{
    std::cout << "writeFixedData" << std::endl;

    config config;
    //const auto translateFolder = fs::path{config.projectPath()} / fs::path{config.outputTranslateFilePathForRPGMaker()};
    const auto translateFolderList = config.exportDirectory();

    auto writeRvCsv = [this, &translateFolderList](fs::path inputPath)
    {
        std::ifstream loadFile(inputPath);
        nlohmann::json json;
        loadFile >> json;

        auto csvFilePath = inputPath.filename();
        csvFilePath.make_preferred().replace_extension(".csv");
        for(auto& translateFolder : translateFolderList){
            csvFilePath = translateFolder / csvFilePath;
            std::cout << "Write Fix Data CSV : " << csvFilePath << std::endl;
            writeFixedTranslateText<csvwriter>(csvFilePath, json, langscore::OverwriteTextMode::LeaveOldNonBlank);
        }
    };

    auto ignoreScripts = config.vxaceScripts();
    for(auto& path : dataFileList)
    {
        auto result = std::find_if(ignoreScripts.cbegin(), ignoreScripts.cend(), [f = path.filename()](const auto& x){
            return x.ignore && x.filename == f.u8string();
        });
        if(result != ignoreScripts.cend()){ continue; }

        writeRvCsv(path);
    }
    std::cout << "Finish." << std::endl;
}

void langscore::divisi_vxace::writeFixedRvScript()
{
    std::cout << "writeFixedRvScript" << std::endl;
    //無視リスト等を反映させて書き出す。
    //書き出し先はプロジェクト直下のTranslateになる。

    //Rubyスクリプトを予め解析してテキストを生成しておく。
    config config;
    auto ignoreScriptPath = config.vxaceScripts();
    auto scriptList = scriptFileList;
    {
        auto rm_result = std::remove_if(scriptList.begin(), scriptList.end(), [&ignoreScriptPath](const auto& path){
            auto osPath = path.stem();
            auto result = std::find_if(ignoreScriptPath.cbegin(), ignoreScriptPath.cend(), [&osPath](const auto& script){
                return script.filename == osPath && script.ignore;
            });
            return result != ignoreScriptPath.cend();
        });
        scriptList.erase(rm_result, scriptList.end());
    }

    rbscriptwriter scriptWriter(this->supportLangs, scriptList);
    auto& transTexts = scriptWriter.curerntTexts();

    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
    for(auto& t : transTexts){
        if(t.translates.find(def_lang) == t.translates.end()){ continue; }
        t.translates[def_lang] = t.original;
        t.original = t.memo;
    }

    //無視する行の判定
    utility::u8stringlist ignoreRowName;
    for(auto& scriptInfo : ignoreScriptPath)
    {
        //ファイルごと無視した場合は上で取り除いているので何もしない
        if(scriptInfo.ignore){ continue; }

        for(const auto& textInfo : scriptInfo.texts)
        {
            if(textInfo.disable){ continue; }
            if(textInfo.ignore){ continue; }
            auto name = scriptInfo.filename + u8".rb"s + utility::cnvStr<std::u8string>(":" + std::to_string(textInfo.row) + ":" + std::to_string(textInfo.col));
            ignoreRowName.emplace_back(std::move(name));
        }
    }
    {
        auto rm_result = std::remove_if(transTexts.begin(), transTexts.end(), [&ignoreRowName](const auto& t){
            return std::find(ignoreRowName.cbegin(), ignoreRowName.cend(), t.original) != ignoreRowName.cend();
        });
        transTexts.erase(rm_result, transTexts.end());
    }

#ifdef _DEBUG
    for(auto& txt : transTexts){
        for(auto& tl : txt.translates)
        {
            auto t = tl.second;
            if(def_lang != tl.first)
            {
                t = txt.memo;
                if(t[0] == u8'\"'){
                    t.insert(1, tl.first + u8"-");
                }
                else {
                    t = tl.first + u8"-" + t;
                }
            }
            tl.second = t;
        }
    }
#endif


    //const auto translateFolder = fs::path{config.projectPath()} / fs::path{config.outputTranslateFilePathForRPGMaker()};
    const auto translateFolderList = config.exportDirectory();

    for(auto& translateFolder : translateFolderList){
        std::cout << "Write Fix Script CSV : " << translateFolder / fs::path{"Scripts.csv"} << std::endl;
        writeFixedTranslateText<csvwriter>(translateFolder / fs::path{"Scripts.csv"}, transTexts, OverwriteTextMode::LeaveOld);
    }

    std::cout << "Finish." << std::endl;
}

utility::u8stringlist divisi_vxace::formatSystemVariable(std::filesystem::path path)
{
    std::ifstream inScriptFile(path);

    std::string _linTmp;
    utility::u8stringlist result;
    config config;
    auto defLanguage = config.defaultLanguage();

    const auto findStr = [](const std::u8string& _line, std::u8string_view str){
        return _line.find(str) != std::u8string::npos;
    };

    while(std::getline(inScriptFile, _linTmp))
    {
        std::u8string _line(_linTmp.begin(), _linTmp.end());
        if(findStr(_line, u8"%{SUPPORT_LANGUAGE}%"))
        {
            auto list = this->supportLangs;
            for(auto& t : list){ t = u8"\"" + t + u8"\""; }
            auto langs = utility::join(list, u8","s);
            _line = tab + u8"SUPPORT_LANGUAGE = [" + langs + u8"]";
        }
        else if(findStr(_line, u8"%{DEFAULT_LANGUAGE}%")){
            _line = tab + u8"DEFAULT_LANGUAGE = \"" + utility::cnvStr<std::u8string>(defLanguage) + u8"\"";
        }
        else if(findStr(_line, u8"%{SUPPORT_FONTS}%"))
        {
            auto fonts = config.languages();
            _line = tab + u8"LS_FONT = {" + nl;
            for(auto& pair : fonts)
            {
                auto lang = utility::cnvStr<std::u8string>(pair.font.name);
                auto sizeStr = utility::cnvStr<std::u8string>(std::to_string(pair.font.size));
                _line += tab + tab;
                _line += u8"\"" + lang + u8"\" => {:name => \"" + pair.font.name + u8"\", :size => " + sizeStr + u8"}," + nl;
            }
            _line += tab + u8"}\n";
        }
        else if(findStr(_line, u8"%{TRANSLATE_FOLDER}%"))
        {
            auto outPath = config.outputTranslateFilePathForRPGMaker();
            _line = tab + u8"TRANSLATE_FOLDER = \"./" + utility::cnvStr<std::u8string>(outPath) + u8"\"" + nl;
        }
        else if(findStr(_line, u8"%{UNISON_LSCSV}%"))
        {
            auto resourceFolder = this->appPath.parent_path() / "resource";
            std::stringstream ss;
            std::cout << "Include LSCSV : " << resourceFolder / "lscsv.rb" << std::endl;
            std::ifstream lscsv(resourceFolder / "lscsv.rb");
            assert(lscsv.good());
            ss << lscsv.rdbuf();
            lscsv.close();
            _line = utility::cnvStr<std::u8string>(ss.str());
        }
        else if(findStr(_line, u8"%{SYSTEM1}%"))
        {
            _line  = tab + u8"SYSTEM1 = \"" + ::Help_Text[u8"ja"] + u8"\"" + nl;
            _line += tab + u8"$langscore_system = {" + nl;
            _line += tab + tab + u8"SYSTEM1 => {" + nl;
            for(auto& l : this->supportLangs){
                if(::Help_Text.find(l) != ::Help_Text.end()){
                    _line += tab + tab + tab;
                    _line += u8"\"" + l + u8"\" => \"" + ::Help_Text[l] + u8"\"," + nl;
                }
            }
            _line += tab + tab + u8"}" + nl;
            _line += tab + u8"}";
        }
        else if(findStr(_line, u8"%{SYSTEM2}%"))
        {
            _line = u8"\tSYSTEM2 = {\n";
            for(auto& l : this->supportLangs){
                if(::Language_Items.find(l) != ::Language_Items.end()){
                    _line += u8"\t\t\"" + l + u8"\" => \"" + ::Language_Items[l] + u8"\",\n";
                }
            }
            _line += u8"\t}";
        }
        result.emplace_back(std::move(_line));
    }
    inScriptFile.close();

    return result;
}

void divisi_vxace::writeFixedGraphFileNameData()
{
    std::cout << "writeFixedGraphFileNameData" << std::endl;

    config config;
    auto ignorePictures = config.ignorePictures();
    std::vector<TranslateText> transTextList;
    for(auto& f : graphicFileList){
        auto result = std::find(ignorePictures.cbegin(), ignorePictures.cend(), f.u8string());
        if(result != ignorePictures.cend()){ continue; }
        transTextList.emplace_back(f.generic_u8string(), supportLangs);
    }

    const auto& projectPath = fs::path(config.projectPath());
    std::cout << "Write Graphics : " << projectPath / config.outputTranslateFilePathForRPGMaker() / "Graphics.csv" << std::endl;
    writeFixedTranslateText<csvwriter>(projectPath / config.outputTranslateFilePathForRPGMaker() / "Graphics.csv", transTextList, OverwriteTextMode::LeaveOld);
    std::cout << "Finish." << std::endl;
}

void divisi_vxace::copyDataToTemp()
{
    config config;
    const auto deserializeOutPath = fs::path(config.tempDirectorty());
    fs::directory_iterator it(deserializeOutPath);
    //翻訳ファイルのコピー
    for(auto& f : it)
    {
        auto srcPath = f.path().filename();
        if(f.is_directory() && srcPath == "Script"){
            continue;
        }
        if(srcPath.extension().string().find(csvwriter::extension) == std::string::npos){ continue; }

        fs::path to = outputProjectDataPath(srcPath);

        auto fsOption = convertCopyOption(langscore::OverwriteTextMode::OverwriteNew);
        std::cout << "Copy Translate File To : " << to << std::endl;
        fs::copy(f, to, fsOption);
    }

    //rbスクリプトのコピー
    fs::directory_iterator it2{deserializeOutPath / "Scripts"};
    for(auto& f : it2)
    {
        auto srcPath = f.path().filename();
        //unison以外は無視
        if(srcPath != ::Custom_Script_File_Name && srcPath != Script_File_Name){
            continue;
        }
        if(srcPath.extension().string().find(rbscriptwriter::extension) == std::string::npos){ continue; }

        fs::path to = outputProjectDataPath(srcPath, "Scripts");
        auto fsOption = convertCopyOption(langscore::OverwriteTextMode::OverwriteNew);
        fs::copy(f, to, fsOption);
    }
}

void langscore::divisi_vxace::copyData(langscore::OverwriteTextMode option)
{
    config config;
    const auto projectPath = fs::path(config.projectPath());
    const auto deserializeOutPath = projectPath / fs::path(config.outputTranslateFilePathForRPGMaker());
    fs::directory_iterator it(deserializeOutPath);
    //翻訳ファイルのコピー
    for(auto& f : it)
    {
        auto srcPath = f.path().filename();
        if(f.is_directory() && srcPath == "Script"){
            continue;
        }
        if(srcPath.extension().string().find(csvwriter::extension) == std::string::npos){ continue; }

        fs::path to = outputProjectDataPath(srcPath);

        auto fsOption = convertCopyOption(option);
        std::cout << "Copy Translate File To : " << to << std::endl;
        fs::copy(f, to, fsOption);
    }

    //rbスクリプトのコピー
    fs::directory_iterator it2{fs::path(config.tempDirectorty()) / "Scripts"};
    for(auto& f : it2)
    {
        auto srcPath = f.path().filename();
        //unison以外は無視
        if(srcPath != ::Custom_Script_File_Name && srcPath != Script_File_Name){
            continue;
        }
        if(srcPath.extension().string().find(rbscriptwriter::extension) == std::string::npos){ continue; }

        fs::path to = outputProjectDataPath(srcPath, "Scripts");
        auto fsOption = convertCopyOption(option);
        fs::copy(f, to, fsOption);
    }
}
