#include "divisi_vxace.h"
#include "config.h"

#include "../writer/rbscriptwriter.h"
#include "../writer/uniquerowcsvwriter.hpp"
#include "../reader/vxacejsonreader.hpp"

#include <nlohmann/json.hpp>
#include <crc32.h>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <random>

/*
文章中の改行コードメモ
通常会話 : \n
スクロール文章の改行 : \n\n
装備やスキルの説明 : \r\n
*/

using namespace langscore;
using namespace std::literals::string_literals;
namespace fs = std::filesystem;

const static std::u8string nl = u8"\n"s;
const static std::u8string tab = u8"\t"s;

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

std::filesystem::path langscore::divisi_vxace::exportFolderPath(std::filesystem::path fileName)
{
    config config;
    std::u8string exportPath;
    auto pathList = config.exportDirectory(exportPath);

    if(std::filesystem::exists(exportPath) == false){
        std::filesystem::create_directories(exportPath);
    }

    for(auto& path : pathList)
    {
        if(std::filesystem::exists(path) == false){
            std::filesystem::create_directories(path);
        }
    }

    return fs::path(exportPath) /= fileName;
}

ErrorStatus divisi_vxace::analyze()
{
    auto runResult = this->invoker.analyze();
    if(runResult.val() != 0){
        std::cerr << runResult.toStr() << std::endl;
        return runResult;
    }

    //解析では言語を使用しない。 書き出されるCSVはオリジナル文のみを表示させる。
    this->supportLangs.clear();

    config config;
    const auto baseDirecotry = config.langscoreAnalyzeDirectorty();
    std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(baseDirecotry);

    this->writeAnalyzedBasicData();
    this->writeAnalyzedRvScript(baseDirecotry);

    std::cout << "AnalyzeProject Done." << std::endl;
    return Status_Success;
}

ErrorStatus langscore::divisi_vxace::update()
{
    config config;
    //変にマージしないように一旦全削除
    const auto updateDirPath = config.langscoreUpdateDirectorty();
    const auto analyzeDirPath = config.langscoreAnalyzeDirectorty();
    fs::remove_all(updateDirPath);

    auto runResult = this->invoker.update();
    if(runResult.val() != 0){
        std::cerr << runResult.toStr() << std::endl;
        return runResult;
    }

    //アップデートでも言語を使用しない。 書き出されるCSVはオリジナル文のみを表示させる。
    this->supportLangs.clear();

    std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(updateDirPath);

    //updateフォルダへCSVの書き出し
    this->writeAnalyzedBasicData();
    this->writeAnalyzedRvScript(updateDirPath);

    auto [analyzeScripts, analyzeDataList, analyzeGraphics] = fetchFilePathList(analyzeDirPath);

    //ファイルのリストアップ
    utility::filelist analyzeCsvList;
    for(const auto& f : fs::directory_iterator{analyzeDirPath}){
        auto extension = f.path().extension();
        if(extension == ".csv"){
            analyzeCsvList.emplace_back(f.path());
        }
    }
    utility::filelist updateCsvList;
    for(const auto& f : fs::directory_iterator{updateDirPath}){
        auto extension = f.path().extension();
        if(extension == ".csv"){
            updateCsvList.emplace_back(f.path());
        }
    }

    enum Type{ Add, Delete, Update };
    std::vector<std::pair<fs::path, Type>> messageList;
    utility::filelist throughCopyList;  //何もせずにコピーする

    //消されるファイルの列挙
    for(auto s : analyzeCsvList){
        auto result = std::find_if(updateCsvList.begin(), updateCsvList.end(), [&s](const auto& x){
            return x.filename() == s.filename();
        });
        if(result == updateCsvList.end()){
            messageList.emplace_back(std::make_pair(s.filename(), Type::Delete));
        }
    }
    //追加されるファイルの列挙
    for(auto s : updateCsvList){
        auto result = std::find_if(analyzeCsvList.begin(), analyzeCsvList.end(), [&s](const auto& x){
            return x.filename() == s.filename();
        });
        if(result == analyzeCsvList.end()){
            messageList.emplace_back(std::make_pair(s.filename(), Type::Add));
        }
    }

    //更新されるファイルのチェック    
    const auto CompareFileHash = [&messageList](std::filesystem::path path, const utility::filelist& files)
    {
        auto result = std::find_if(files.begin(), files.end(), [&path](const auto& x){
            return x.filename() == path.filename();
        });
        if(result == files.end()){
            return;
        }

        auto x_data = utility::getFileData(path);
        CRC32 x;
        auto x_hash = x(x_data.data(), x_data.size());

        auto y_data = utility::getFileData(*result);
        CRC32 y;
        auto y_hash = y(y_data.data(), y_data.size());

        if(x_hash != y_hash){
            messageList.emplace_back(std::make_pair(path.filename(), Type::Update));
        }
    };
    for(auto s : analyzeCsvList)
    {
        if(s.extension() != ".csv"){ continue; }
        CompareFileHash(std::move(s), updateCsvList);
    }

    //ファイルの並びが揃ってないので一旦揃える
    std::sort(messageList.begin(), messageList.end(), [](const auto& x, const auto& y){
        return x.first < y.first;
    });

    //メッセージを出しつつ、ファイルの移動

    for(auto& mes : messageList){
        if(mes.second == Type::Add){
            std::cout << "Add : " << mes.first << std::endl;
            fs::copy(updateDirPath / mes.first, analyzeDirPath / mes.first);
        }
        else if(mes.second == Type::Delete){
            std::cout << "Delete : " << mes.first << std::endl;
            fs::remove(analyzeDirPath / mes.first);
        }
        else if(mes.second == Type::Update){
            std::cout << "Update : " << mes.first << std::endl;
            fs::copy(updateDirPath / mes.first, analyzeDirPath / mes.first, fs::copy_options::overwrite_existing);
        }
    }

    for(const auto& f : fs::directory_iterator{updateDirPath}){
        auto extension = f.path().extension();
        if(extension == ".json"){
            auto filename = f.path().filename();
            fs::copy(updateDirPath / filename, analyzeDirPath / filename, fs::copy_options::overwrite_existing);
            fs::remove(f.path());
        }
    }

    std::cout << "UpdateProject Done." << std::endl;
    return Status_Success;
}

ErrorStatus langscore::divisi_vxace::write()
{
    std::cout << "Write..." << std::endl;
    config config;
    std::u8string rootPath;
    auto exportFolderList = config.exportDirectory(rootPath);
    for(auto& folder : exportFolderList)
    {
        if(fs::exists(folder)){ continue; }
        fs::create_directories(folder);
    }
    
    std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(config.langscoreAnalyzeDirectorty()); 

    writeFixedBasicData();
    writeFixedRvScript();
    writeFixedGraphFileNameData();

    copyFonts();

    std::cout << "Export script files." << std::endl;
    bool replaceScript = false;
    rewriteScriptList(replaceScript);
    std::cout << "Done." << std::endl;

    if(replaceScript){
        std::cout << "Compress." << std::endl;
        auto runResult = this->invoker.recompressVXAce();
        if(runResult.val() != 0){
            return runResult;
        }
        std::cout << "Done." << std::endl;
    }

    std::cout << "Write Translate File Done." << std::endl;
    return Status_Success;
}

ErrorStatus langscore::divisi_vxace::validate()
{
    config config;
    std::u8string root;
    const auto exportDirectory = config.exportDirectory(root);
    utility::filelist csvPathList;

    for(auto dir : exportDirectory)
    {
        for(const auto& f : fs::recursive_directory_iterator{dir}){
            auto extension = f.path().extension();
            if(extension == ".csv"){
                csvPathList.emplace_back(f.path());
            }
        }
    }

    //整合性チェック
    if(config.exportByLanguage())
    {
    }
    else
    {
        this->validateTranslateFileList(std::move(csvPathList));
    }

    return Status_Success;
}

ErrorStatus langscore::divisi_vxace::packing()
{
    std::cout << "Packing." << std::endl;
    auto runResult = this->invoker.packingVXAce();
    if(runResult.val() != 0){
        return runResult;
    }
    std::cout << "Done." << std::endl;

    return Status_Success;
}

std::tuple<utility::filelist, utility::filelist, utility::filelist> langscore::divisi_vxace::fetchFilePathList(std::u8string deserializeOutPath)
{
    size_t numScripts = 0;
    for(const auto& f : fs::recursive_directory_iterator{deserializeOutPath + u8"/Scripts"}){
        auto extension = f.path().extension();
        if(extension == ".rb"){ numScripts++; }
    }
    utility::filelist scripts;
    scripts.resize(numScripts);
    utility::filelist basicDataList;

    csvreader scriptCsvReader;
    auto scriptCsv = scriptCsvReader.parsePlain(deserializeOutPath + u8"/Scripts/_list.csv"s);
    for(auto& f : fs::recursive_directory_iterator{deserializeOutPath})
    {
        //Basic
        auto extension = f.path().extension();
        if(extension == ".json"){
            basicDataList.emplace_back(f.path());
        }
        else if(extension == ".rb")
        {
            //Script
            auto fileName = f.path().filename();
            auto itr = std::find_if(scriptCsv.cbegin(), scriptCsv.cend(), [name = fileName.stem()](const auto& x){
                return name == x[0];
            });

            if(itr != scriptCsv.cend())
            {
                if(2 < itr->size()){
                    if((*itr)[1] == Script_File_Name) { continue; }
                    else if((*itr)[1] == Custom_Script_File_Name){ continue; }
                }
            }

            auto pos = std::distance(scriptCsv.cbegin(), itr);
            if(0 <= pos && pos < numScripts){
                scripts[pos] = f.path();
            }
            else{
                scripts.emplace_back(f.path());
            }
        }
    }

    utility::filelist graphics;
    config config;
    auto gameProjectPath = fs::path(config.gameProjectPath());
    auto graphicsPath = gameProjectPath / "Graphics";
    fs::recursive_directory_iterator graphItr(graphicsPath);
    for(auto& f : graphItr)
    {
        if(f.is_directory()){ continue; }
        //パス区切り文字は\\ではなく/に統一(\\はRubyで読み取れない)
        const auto& path = f.path();
        const auto ext = path.stem();
        if(ext == ".jpg" || ext == ".png" || ext == ".bmp"){
            auto relative_path = "Graphics" / path.lexically_relative(graphicsPath);
            graphics.emplace_back(relative_path.parent_path() / ext);
        }
    }

    return std::forward_as_tuple(scripts, basicDataList, graphics);
}

void divisi_vxace::writeAnalyzedBasicData()
{
    std::cout << "writeAnalyzedBasicData" << std::endl;
    for(auto& path : this->basicDataFileList)
    {
        std::ifstream loadFile(path);
        nlohmann::json json;
        loadFile >> json;

        auto csvFilePath = path;
        csvFilePath.make_preferred().replace_extension(".csv");
        std::cout << "Write CSV : " << csvFilePath << std::endl;

        csvwriter writer(this->supportLangs, std::make_unique<vxace_jsonreader>(json));
        writer.write(csvFilePath, MergeTextMode::AcceptTarget);
    }
    std::cout << "Finish." << std::endl;
}

void divisi_vxace::writeAnalyzedRvScript(std::u8string baseDirectory)
{
    //解析直後の素のデータを書き出す。
    //無視リスト等は考慮せず、書き出し先はTempフォルダ以下になる。
    std::cout << "writeAnalyzedRvScript" << std::endl;

    config config;
    //Rubyスクリプトを予め解析してテキストを生成しておく。
    rbscriptwriter scriptWriter(this->supportLangs, this->scriptFileList);
    auto& transTexts = scriptWriter.curerntTexts();

    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
    for(auto& t : transTexts){
        if(t.translates.find(def_lang) == t.translates.end()){ continue; }
        t.translates[def_lang] = t.original;
    }

    //デフォルトスクリプトのVocab.rb内の文字列は予めScriptsの中に翻訳済みの内容を入れておく
    auto resourceFolder = this->appPath.parent_path() / "resource";
    csvreader reader;
    auto vocabs = reader.parse(resourceFolder/"vocab.csv");
    
    csvreader csvReader;
    auto scriptFileNameMap = csvReader.parsePlain(baseDirectory+u8"/Scripts/_list.csv"s);
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
            return x.scriptLineInfo.find(scriptFileName) != std::u8string::npos;
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

    const auto deserializeOutPath = fs::path(baseDirectory);
    auto outputPath = deserializeOutPath / "Scripts";
    if(std::filesystem::exists(outputPath) == false){
        std::filesystem::create_directories(outputPath);
    }

    //memoに行数が格納されているため入れ替え
    for(auto& t : transTexts){
        t.scriptLineInfo.swap(t.original);
        t.translates[u8"scriptLineInfo"s] = t.scriptLineInfo;
    }

    std::cout << "Write CSV : " << outputPath << std::endl;
    csvwriter writer(supportLangs, std::move(transTexts));
    writer.write(outputPath, MergeTextMode::AcceptTarget);

    std::cout << "Finish." << std::endl;
}

void langscore::divisi_vxace::writeFixedBasicData()
{
    std::cout << "writeFixedBasicData" << std::endl;

    config config;
    std::u8string root;
    const auto translateFolderList = config.exportDirectory(root);

    auto mergeTextMode = MergeTextMode::MergeKeepSource;
    auto mergeTextModeRaw = config.globalWriteMode();
    if(0 <= mergeTextModeRaw && mergeTextModeRaw <= 4){
        mergeTextMode = static_cast<MergeTextMode>(mergeTextModeRaw);
    }

    auto writeRvCsv = [this, &translateFolderList, mergeTextMode](fs::path inputPath)
    {
        std::ifstream loadFile(inputPath);
        nlohmann::json json;
        loadFile >> json;

        auto csvFilePath = inputPath.filename();
        csvFilePath.make_preferred().replace_extension(".csv");
        for(auto& translateFolder : translateFolderList){
            csvFilePath = translateFolder / csvFilePath;
            std::cout << "Write Fix Data CSV : " << csvFilePath << std::endl;
            writeFixedTranslateText<csvwriter>(csvFilePath, std::make_unique<vxace_jsonreader>(json), mergeTextMode);
        }
    };

    auto ignoreScripts = config.vxaceBasicData();
    for(auto& path : this->basicDataFileList)
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

    auto mergeTextMode = MergeTextMode::MergeKeepSource;
    auto mergeTextModeRaw = config.globalWriteMode();
    if(0 <= mergeTextModeRaw && mergeTextModeRaw <= 4){
        mergeTextMode = static_cast<MergeTextMode>(mergeTextModeRaw);
    }

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

//#ifdef _DEBUG
//    writerbase::ReplaceDebugTextByLang(transTexts, def_lang);
//#endif

    std::u8string root;
    const auto translateFolderList = config.exportDirectory(root);
    for(auto& translateFolder : translateFolderList){
        std::cout << "Write Fix Script CSV : " << translateFolder / fs::path{"Scripts.csv"} << std::endl;
        writeFixedTranslateText<csvwriter>(translateFolder / fs::path{"Scripts.csv"}, transTexts, mergeTextMode);
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
            _line = tab + u8"TRANSLATE_FOLDER = \"" + utility::cnvStr<std::u8string>(outPath) + u8"\"" + nl;
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
            _line  = tab + u8"SYSTEM1 = \"" + Help_Text[u8"ja"] + u8"\"" + nl;
            _line += tab + u8"$langscore_system = {" + nl;
            _line += tab + tab + u8"SYSTEM1 => {" + nl;
            for(auto& l : this->supportLangs){
                if(Help_Text.find(l) != Help_Text.end()){
                    _line += tab + tab + tab;
                    _line += u8"\"" + l + u8"\" => \"" + Help_Text[l] + u8"\"," + nl;
                }
            }
            _line += tab + tab + u8"}" + nl;
            _line += tab + u8"}";
        }
        else if(findStr(_line, u8"%{SYSTEM2}%"))
        {
            _line = u8"\tSYSTEM2 = {\n";
            for(auto& l : this->supportLangs){
                if(Language_Items.find(l) != Language_Items.end()){
                    _line += u8"\t\t\"" + l + u8"\" => \"" + Language_Items[l] + u8"\",\n";
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

void divisi_vxace::rewriteScriptList(bool& replaceScript)
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
    auto scriptFileNameList = GetScriptFileName(config, {Script_File_Name, Custom_Script_File_Name});

    bool updateScriptCsv = false;
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
        scriptListCsv.insert(itr, {lsCustomScriptID, Custom_Script_File_Name});
        updateScriptCsv = true;
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
        scriptListCsv.insert(itr, {lsScriptID, Script_File_Name});
        updateScriptCsv = true;
    }

    if(updateScriptCsv){
        csvwriter::writePlain(lsAnalyzePath / "Scripts/_list.csv", std::move(scriptListCsv), MergeTextMode::AcceptTarget);
    }

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
        std::cout << "Write langscore_custom : " << outputPath / scriptFileNameList[1] << std::endl;
        writeFixedTranslateText<rbscriptwriter>(rbscriptwriter{this->supportLangs, scriptFileList}, lsCustomScriptPath, langscore::MergeTextMode::AcceptTarget);
    }

    //langscore.rbの出力
    auto outputScriptFilePath = outputPath / (scriptFileNameList[0] + u8".rb"s);
    bool replaceLs = fs::exists(outputScriptFilePath)==false;
    if(replaceLs == false){
        replaceLs = config.overwriteLangscore();
    }
    if(replaceLs){
        auto resourceFolder = this->appPath.parent_path() / "resource";
        const auto langscoreScriptFilePath = resourceFolder / (Script_File_Name + u8".rb"s);
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
            replaceScript = true;
        }
    }

}

bool divisi_vxace::validateTranslateFileList(utility::filelist csvPathList) const
{
    csvreader csvreader;
    bool result = true;
    for(auto& _path : csvPathList)
    {
        auto texts = csvreader.parse(_path);
        result &= validateTranslateList(std::move(texts), std::move(_path));
    }
    return result;
}

bool divisi_vxace::validateTranslateList(std::vector<TranslateText> texts, std::filesystem::path path) const
{
    const auto OutputError = [&path](auto type, auto main, auto lang, auto str, size_t row){
        auto result = utility::join({type, main, lang, str, path.string(), std::to_string(row)}, ","s);
        std::cout << result << std::endl;
    };
    size_t row = 1;
    bool result = true;
    for(auto& text : texts)
    {
        if(text.original.empty()){
            OutputError("Error"s, "0"s, "original"s, ""s, row);
            result = false;
        }
        auto [withValEscList, EscList] = findEscChars(text.original);

        std::vector<std::string> emptyTextLangs;
        for(auto& trans : text.translates)
        {
            if(trans.second.empty()){
                emptyTextLangs.emplace_back(utility::cnvStr<std::string>(trans.first));
                result = false;
                continue;
            }

            auto escStr = ""s;
            for(auto& esc : withValEscList){
                if(trans.second.find(esc) == trans.second.npos){
                    escStr += utility::cnvStr<std::string>(esc) + " "s;
                    result = false;
                }
            }
            for(auto& esc : EscList){
                if(trans.second.find(esc) == trans.second.npos){
                    escStr += utility::cnvStr<std::string>(esc) + " "s;
                    result = false;
                }
            }

            if(escStr.empty() == false){
                OutputError("Error"s, "1"s, utility::cnvStr<std::string>(trans.first), escStr, row);
            }
        }
        if(emptyTextLangs.empty() == false){
            OutputError("Warning"s, "0"s, utility::join(emptyTextLangs, " "s), ""s, row);
        }
        row++;
    }

    return result;
}

std::tuple<std::vector<std::u8string>, std::vector<std::u8string>> divisi_vxace::findEscChars(std::u8string originalText) const
{
    static const std::vector<std::u8string> escWithValueChars = {
        u8"\\v[", u8"\\n[", u8"\\p[", u8"\\c[", u8"\\l[", u8"\\r["
    };
    static const std::vector<std::u8string> escChars = {
        u8"\\g", u8"\\{", u8"\\}", u8"\\$", u8"\\.", u8"\\|",
        u8"\\!", u8"\\>", u8"\\<", u8"\\^", u8"\\\\"
    };

    std::vector<std::u8string> result1;
    std::vector<std::u8string> result2;
    if(originalText.empty()){ return std::forward_as_tuple(result1, result2); }

    auto text = originalText;
    std::transform(text.begin(), text.end(), text.data(), ::tolower);
    for(const auto& c : escWithValueChars)
    {
        auto pos = text.find(c);
        auto offset = 0;
        for(; pos != text.npos; pos = text.find(c, offset))
        {
            auto endPos = text.find(u8']', pos);
            if(endPos == text.npos){
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
        for(; pos != text.npos; pos = text.find(c)){
            result2.emplace_back(originalText.substr(pos, c.length()));
            break;
        }
    }
    std::sort(result2.begin(), result2.end());
    result2.erase(std::unique(result2.begin(), result2.end()), result2.end());

    return std::forward_as_tuple(result1, result2);
}
