#include "divisi_vxace.h"
#include "config.h"

#include "../writer/rbscriptwriter.h"
#include "../writer/csvwriter.h"
#include "../reader/csvreader.h"

#include <nlohmann/json.hpp>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <random>

using namespace langscore;
using namespace std::literals::string_literals;
namespace fs = std::filesystem;

namespace
{
    constexpr char8_t Script_File_Name[] = u8"langscore";
    constexpr char8_t Custom_Script_File_Name[] = u8"langscore_custom";

    std::unordered_map<std::u8string, std::u8string> Help_Text = {
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

    std::unordered_map<std::u8string, std::u8string> Language_Items = {
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

    const auto nl = u8"\n"s;
    const auto tab = u8"\t"s;

    utility::u8stringlist GetScriptFileName(config& config, utility::u8stringlist scriptNameList)
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

void divisi_vxace::setProjectPath(std::filesystem::path path){
    this->invoker.setProjectPath(invoker::ProjectType::VXAce, std::move(path));
}

std::filesystem::path langscore::divisi_vxace::exportFolderPath(std::filesystem::path fileName, std::filesystem::path dir)
{
    config config;
    if(dir.empty()){
        dir = config.outputTranslateFilePathForRPGMaker();
    }
    fs::path projectPath = config.projectPath();
    fs::path to = projectPath / dir;

    if(std::filesystem::exists(to) == false){
        std::filesystem::create_directories(to);
    }

    return to /= fileName;
}

ErrorStatus divisi_vxace::analyze()
{
    auto runResult = this->invoker.analyze();
    if(runResult.val() != 0){
        std::cerr << runResult.toStr() << std::endl;
        return runResult;
    }

    fetchFilePathList();

    this->writeAnalyzedData();
    this->writeAnalyzedRvScript();

    std::cout << "AnalyzeProject Done." << std::endl;
    return Status_Success;
}

ErrorStatus langscore::divisi_vxace::write()
{
    std::cout << "Write..." << std::endl;
    config config;
    auto exportFolderList = config.exportDirectory();
    for(auto& folder : exportFolderList)
    {
        if(fs::exists(folder)){ continue; }
        fs::create_directories(folder);
    }

    fetchFilePathList(); 

    writeFixedData();
    writeFixedRvScript();
    writeFixedGraphFileNameData();

    const auto lsAnalyzePath = fs::path(config.langscoreAnalyzeDirectorty());
    
    //フォントのコピー
    auto fontDestPath = config.projectPath() + u8"/Fonts"s;
    auto globalFonts = config.globalFontList();
    auto globalFontFolder = appPath / "../resources/fonts";
    for(auto& relativePath : globalFonts)
    {
        auto path = globalFontFolder / relativePath;
        if(fs::exists(path)){
            std::cerr << "Warning! : not found global font file : " << path << std::endl;
            continue;
        }
        fs::copy_file(path, fontDestPath + relativePath);
    }
    auto localFonts = config.localFontList();
    auto localFontFolder = lsAnalyzePath / "Fonts";
    for(auto& relativePath : localFonts)
    {
        auto path = localFontFolder / relativePath;
        if(fs::exists(path)){
            std::cerr << "Warning! : not found local font file : " << path << std::endl;
            continue;
        }
        fs::copy_file(path, fontDestPath + relativePath);
    }

    std::cout << "Export script files." << std::endl;
    rewriteScriptList();
    std::cout << "Done." << std::endl;

    std::cout << "Compress." << std::endl;
    auto runResult = this->invoker.recompressVXAce();
    if(runResult.val() != 0){
        return runResult;
    }
    std::cout << "Done." << std::endl;

    std::cout << "Write Translate File Done." << std::endl;
    return Status_Success;
}

void langscore::divisi_vxace::fetchFilePathList()
{
    config config;
    const auto deserializeOutPath = config.langscoreAnalyzeDirectorty();
    fs::recursive_directory_iterator dataItr(deserializeOutPath);
    size_t numScripts = 0;
    for(const auto& f : fs::recursive_directory_iterator{deserializeOutPath+u8"/Scripts"}){
        auto extension = f.path().extension();
        if(extension == ".rb"){ numScripts++; }
    }
    this->scriptFileList.resize(numScripts);

    csvreader scriptCsvReader;
    auto scriptCsv = scriptCsvReader.parsePlain(deserializeOutPath + u8"/Scripts/_list.csv"s);
    for(auto& f : fs::recursive_directory_iterator{deserializeOutPath})
    {
        auto extension = f.path().extension();
        if(extension == ".json"){
            this->dataFileList.emplace_back(f.path());
        }
        else if(extension == ".rb")
        {
            auto fileName = f.path().filename();
            auto itr = std::find_if(scriptCsv.cbegin(), scriptCsv.cend(), [name = fileName.stem()](const auto& x){
                return name == x[0];
            });

            if(itr != scriptCsv.cend())
            {
                if(2 < itr->size()){
                    if((*itr)[1] == ::Script_File_Name) { continue; }
                    else if((*itr)[1] == ::Custom_Script_File_Name){ continue; }
                }
            }

            auto pos = std::distance(scriptCsv.cbegin(), itr);
            if(0 <= pos && pos < numScripts){
                this->scriptFileList[pos] = f.path();
            }
            else{
                this->scriptFileList.emplace_back(f.path());
            }
        }
    }

    auto projectPath = fs::path(config.projectPath());
    auto graphicsPath = projectPath / "Graphics";
    fs::recursive_directory_iterator graphItr(graphicsPath);
    for(auto& f : graphItr)
    {
        if(f.is_directory()){ continue; }
        //パス区切り文字は\\ではなく/に統一(\\はRubyで読み取れない)
        auto relative_path = "./"/f.path().lexically_relative(graphicsPath);
        this->graphicFileList.emplace_back(relative_path);
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
    rbscriptwriter scriptWriter(this->supportLangs, scriptFileList);
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
    
    csvreader csvReader;
    auto scriptFileNameMap = csvReader.parsePlain(config.langscoreAnalyzeDirectorty()+u8"/Scripts/_list.csv"s);
    const auto GetScriptName = [&scriptFileNameMap](std::u8string scriptName)
    {
        for(const auto& row : scriptFileNameMap){
            if(row[1] == scriptName){ return row[2]; }
        }
        return u8""s;
    };

    auto& scriptTrans = scriptWriter.getScriptTexts();
    for(auto& pathPair : scriptTrans)
    {
        auto scriptFileName = std::get<0>(pathPair);
        auto scriptName = GetScriptName(scriptFileName);
        if(scriptName != u8"Vocab"s){ continue; }

        const auto searchFunc = [&](const auto& x){
            return x.memo.find(scriptFileName) != std::u8string::npos;
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

    const auto deserializeOutPath = fs::path(config.langscoreAnalyzeDirectorty());
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
    writeAnalyzeTranslateText<csvwriter>(outputPath, transTexts, OverwriteTextMode::LeaveOldNonBlank, false);

    std::cout << "Finish." << std::endl;
}


void langscore::divisi_vxace::writeFixedData()
{
    std::cout << "writeFixedData" << std::endl;

    config config;
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

    auto ignoreScripts = config.vxaceBasicData();
    for(auto& path : dataFileList)
    {
        auto result = std::find_if(ignoreScripts.cbegin(), ignoreScripts.cend(), [f = path.filename()](const auto& x){
            return x.ignore && x.filename == f.u8string();
        });
        if(result != ignoreScripts.cend()){ 
            continue; 
        }

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
    auto scriptInfoList = config.vxaceScripts();
    auto scriptList = scriptFileList;
    {
        auto rm_result = std::remove_if(scriptList.begin(), scriptList.end(), [&scriptInfoList](const auto& path){
            auto osPath = path.stem();
            auto result = std::find_if(scriptInfoList.cbegin(), scriptInfoList.cend(), [&osPath](const auto& script){
                return script.filename == osPath && script.ignore;
            });
            return result != scriptInfoList.cend();
        });
        scriptList.erase(rm_result, scriptList.end());
    }

    //スクリプトの翻訳を書き込むCSVの書き出し
    rbscriptwriter scriptWriter(this->supportLangs, scriptList);
    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
    auto transTexts = scriptWriter.curerntTexts();
    transTexts = scriptWriter.acceptIgnoreScripts(scriptInfoList, std::move(transTexts));

#ifdef _DEBUG
    for(auto& txt : transTexts)
    {
        auto origin = txt.translates[def_lang];
        for(auto& tl : txt.translates)
        {
            auto t = tl.second;
            if(def_lang != tl.first)
            {
                t = origin;
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

    const auto translateFolderList = config.exportDirectory();
    for(auto& translateFolder : translateFolderList){
        std::cout << "Write Fix Script CSV : " << translateFolder / fs::path{"Scripts.csv"} << std::endl;
        writeFixedTranslateText<csvwriter>(translateFolder / fs::path{"Scripts.csv"}, transTexts, OverwriteTextMode::LeaveOldNonBlank);
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
                auto lang = utility::cnvStr<std::u8string>(pair.name);
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
        auto result = std::find(ignorePictures.cbegin(), ignorePictures.cend(), f.generic_u8string());
        if(result != ignorePictures.cend()){ continue; }
        transTextList.emplace_back(f.generic_u8string(), supportLangs);
    }

    auto csvPath = exportFolderPath("Graphics.csv");
    std::cout << "Write Graphics : " << csvPath << std::endl;
    writeFixedTranslateText<csvwriter>(csvPath, transTextList, OverwriteTextMode::LeaveOld);
    std::cout << "Finish." << std::endl;
}

void divisi_vxace::rewriteScriptList()
{
    config config;
    const auto lsAnalyzePath = fs::path(config.langscoreAnalyzeDirectorty());

    csvreader scriptList;
    auto scriptListCsv = scriptList.parsePlain(lsAnalyzePath / "Scripts/_list.csv");
    const auto GetID = [&scriptListCsv]()
    {
        std::vector<size_t> idList;
        idList.reserve(scriptListCsv.size());
        for(auto& row : scriptListCsv){ idList.emplace_back(std::stoll(utility::cnvStr<std::string>(row[0]))); }

        std::mt19937 mt(std::random_device{}());
        size_t num = 0;
        do{
            num = 10000ll + size_t(double(mt()) * double(99999999 - 10000) + 1.0) / (1.0 + double(mt.max()));
        } while(std::find(idList.cbegin(), idList.cend(), num) != idList.cend());
        return num;
    };

    //_list.csvの編集　追加する位置を見つける
    const auto FindInsertPos = [&](const utility::u8stringlist& scriptList){
        auto scriptListInsertPos = std::find_if(scriptListCsv.cbegin(), scriptListCsv.cend(), [&scriptList](const auto& x)
        {
            if(x.size() < 2){ return false; }
            const auto& scriptName = x[1];
            if(std::find(scriptList.cbegin(), scriptList.cend(), scriptName) != scriptList.cend()){
                return true;
            }
            return false;
        });
        return scriptListInsertPos;
    };
    //Langscoreスクリプトの追加
    //_list.csvにlsのスクリプトがあるかチェック。なければCSVに追加
    auto scriptFileNameList = GetScriptFileName(config, {::Script_File_Name, ::Custom_Script_File_Name});

    if(scriptFileNameList[1] == u8""){
        auto lsCustomScriptID = utility::cnvStr<std::u8string>(std::to_string(GetID()));
        scriptFileNameList[1] = lsCustomScriptID;
        auto itr = FindInsertPos({u8"( ここに追加 )"s, u8"▼ メイン"s, u8"Main"s});
        if(itr == scriptListCsv.cend()){
            itr = scriptListCsv.cbegin();
        }
        else{
            itr--;  //ヒットした位置の一つ手前に挿入
        }
        scriptListCsv.insert(itr, {lsCustomScriptID, ::Custom_Script_File_Name});
    }
    if(scriptFileNameList[0] == u8""){
        auto lsScriptID = utility::cnvStr<std::u8string>(std::to_string(GetID()));
        scriptFileNameList[0] = lsScriptID;
        auto itr = FindInsertPos({u8"VXAce_SP1"s, u8"▼ 素材"s});
        if(itr == scriptListCsv.cend()){
            itr = scriptListCsv.cend() - 1;
        }
        else{
            itr++;  //ヒットした位置の次に挿入
        }
        scriptListCsv.insert(itr, {lsScriptID, ::Script_File_Name});
    }

    csvwriter::writePlain(lsAnalyzePath / "Scripts/_list.csv", scriptListCsv, OverwriteTextMode::OverwriteNew);

    auto outputPath = lsAnalyzePath / "Scripts";
    if(fs::exists(outputPath) == false){
        fs::create_directories(outputPath);
    }
    //langscore_custom.rbの出力
    auto lsCustomScriptPath = outputPath / (scriptFileNameList[1] + u8".rb"s);
    bool replaceLsCustom = fs::exists(lsCustomScriptPath) == false;
    if(replaceLsCustom == false){
        replaceLsCustom = config.overwriteLangscoreCustom();
    }
    if(replaceLsCustom){
        std::cout << "Write langscore_custom : " << outputPath / ::Custom_Script_File_Name << std::endl;
        writeFixedTranslateText<rbscriptwriter>(lsCustomScriptPath, scriptFileList, langscore::OverwriteTextMode::OverwriteNew);
    }

    //langscore.rbの出力
    auto outputScriptFilePath = outputPath / (scriptFileNameList[0] + u8".rb"s);
    bool replaceLs = fs::exists(outputScriptFilePath)==false;
    if(replaceLs == false){
        replaceLs = config.overwriteLangscore();
    }
    if(replaceLs){
        auto resourceFolder = this->appPath.parent_path() / "resource";
        const auto langscoreScriptFilePath = resourceFolder / (::Script_File_Name + u8".rb"s);
        std::cout << "Copy langscore : From " << langscoreScriptFilePath << " To : " << outputScriptFilePath << std::endl;
        fs::copy(langscoreScriptFilePath, outputScriptFilePath, fs::copy_options::overwrite_existing);
        std::cout << "Done." << std::endl;

        //現在の設定を元にlangscore.rbのカスタマイズ
        auto fileLines = formatSystemVariable(outputScriptFilePath);

        if(fs::exists(outputScriptFilePath))
        {
            std::cout << "Replace langscore : " << outputScriptFilePath << std::endl;
            std::ofstream outScriptFile(outputScriptFilePath, std::ios_base::trunc);
            for(const auto& l : fileLines){
                outScriptFile << utility::toString(l) << "\n";
            }
        }
    }

}