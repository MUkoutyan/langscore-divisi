#include "divisi_vxace.h"
#include "config.h"

#include "../writer/rbscriptwriter.h"
#include "../writer/jsonwriter.h"
#include "../writer/uniquerowcsvwriter.hpp"
#include "../reader/vxacejsonreader.hpp"
#include "../reader/rubyreader.hpp"
#include "reader/speciftranstext.hpp"

#include <nlohmann/json.hpp>
#include <crc32.h>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <random>

/*
ゲーム実行中の文章中の改行コードメモ
・通常会話 : \n
  - Game_Message.all_text内で、改行が付与される。
・スクロール文章中の改行 : \n\n
・装備やスキルの説明 : \r\n

ゲーム実行中の末尾の改行コードメモ
・通常会話：\n
・スクロール文章：\n
・装備やスキルの説明：無し

rvdata2内の改行コードメモ
・通常会話：無し
・スクロール文章中：無し
・装備やスキルの説明：\r\n
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

    this->currentGameProjectPath = fs::path(config.gameProjectPath());
    this->currentGameProjectPath.make_preferred();

    this->defaultLanguage = utility::cnvStr<std::u8string>(config.defaultLanguage());
   
    for(auto langs = config.languages(); auto lang : langs){
        this->supportLangs.emplace_back(utility::cnvStr<std::u8string>(lang.name));
    }
}

divisi_vxace::~divisi_vxace(){}

void divisi_vxace::setProjectPath(std::filesystem::path path){
    this->invoker.setProjectPath(std::move(path));
}

ErrorStatus divisi_vxace::analyze()
{
    std::cout << "VXAce Analyze..." << std::endl;

    config config;
    const auto baseDirecotry = config.langscoreAnalyzeDirectorty();
    auto runResult = this->invoker.analyze(baseDirecotry);
    if(runResult.val() != 0){
        std::cerr << runResult.toStr() << std::endl;
        return runResult;
    }

    //解析では言語を使用しない。 書き出されるCSVはオリジナル文のみを表示させる。
    this->supportLangs.clear();

    std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(baseDirecotry);

    this->writeAnalyzedBasicData();
    this->writeAnalyzedRvScript(baseDirecotry);

    std::cout << "AnalyzeProject Done." << std::endl;
    return Status_Success;
}

ErrorStatus langscore::divisi_vxace::reanalysis()
{
    std::cout << "VXAce Reanalysis..." << std::endl;

    config config;
    //変にマージしないように一旦全削除
    const auto updateDirPath = config.langscoreUpdateDirectorty();
    fs::remove_all(updateDirPath);


    auto runResult = this->invoker.reanalysis(updateDirPath);
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

    const auto analyzeDirPath = config.langscoreAnalyzeDirectorty();
    auto [analyzeScripts, analyzeDataList, analyzeGraphics] = fetchFilePathList(analyzeDirPath);

    //ファイルのリストアップ
    utility::filelist analyzeCsvList;
    for(const auto& f : fs::directory_iterator{analyzeDirPath}){
        auto extension = f.path().extension();
        if(extension == ".csv" || extension == ".lsjson") {
            analyzeCsvList.emplace_back(f.path());
        }
    }
    utility::filelist updateCsvList;
    for(const auto& f : fs::directory_iterator{updateDirPath})
    {
        auto extension = f.path().extension();
        if(extension == ".lsjson"){
            updateCsvList.emplace_back(f.path());
        }
        else if(f.path().filename() == "MapInfos.csv") {
            updateCsvList.emplace_back(f.path());
        }
        else if(f.path().filename() == "Scripts.csv") {
            updateCsvList.emplace_back(f.path());
        }
    }

    enum Type{ Add, Delete, Update };
    std::vector<std::pair<fs::path, Type>> messageList;
    utility::filelist throughCopyList;  //何もせずにコピーする

    //消されるファイルの列挙
    for(const auto& s : analyzeCsvList)
    {
        if(s.extension() == ".csv") { 
            messageList.emplace_back(std::make_pair(s.filename(), Type::Delete));
            continue;
        }
        
        auto result = std::ranges::find_if(updateCsvList, [&s](const auto& x){
            return x.filename() == s.filename();
        });
        if(result == updateCsvList.end()){
            messageList.emplace_back(std::make_pair(s.filename(), Type::Delete));
        }
    }
    //追加されるファイルの列挙
    for(const auto& s : updateCsvList){
        auto result = std::ranges::find_if(analyzeCsvList, [&s](const auto& x){
            return x.filename() == s.filename();
        });
        if(result == analyzeCsvList.end()){
            messageList.emplace_back(std::make_pair(s.filename(), Type::Add));
        }
    }

    //更新されるファイルのチェック    
    const auto CompareFileHash = [&messageList](std::filesystem::path path, const utility::filelist& files)
    {
        auto result = std::ranges::find_if(files, [&path](const auto& x){
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
        if(s.extension() != ".lsjson"){ continue; }
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

    //rvdata2を解析したjsonのコピー
    for(const auto& f : fs::recursive_directory_iterator{updateDirPath}){
        auto extension = f.path().extension();
        if(extension == ".json"){
            auto filename = f.path().filename();
            fs::copy(updateDirPath / filename, analyzeDirPath / filename, fs::copy_options::overwrite_existing);
            fs::remove(f.path());
        }
    }
    //展開したスクリプトのコピー
    fs::path updateScriptPath = updateDirPath + u8"/Scripts"s;
    fs::path analyzeScriptPath = (analyzeDirPath + u8"/Scripts"s);
    for(const auto& f : fs::recursive_directory_iterator{updateScriptPath}){
        auto filename = f.path().filename();
        fs::copy(updateScriptPath / filename, analyzeScriptPath / filename, fs::copy_options::overwrite_existing);
        fs::remove(f.path());
    }

    std::cout << "Reanalysis Project Done." << std::endl;
    return Status_Success;
}

ErrorStatus langscore::divisi_vxace::updatePlugin()
{
    std::cout << "Export langscore plugin files." << std::endl;
    bool replaceScript = true;
    rewriteScriptList(replaceScript);
    std::cout << "Done." << std::endl;

    if(replaceScript) {
        std::cout << "Compress." << std::endl;
        auto runResult = this->invoker.recompressVXAce();
        if(runResult.val() != 0) {
            return runResult;
        }
        std::cout << "Done." << std::endl;
    }

    config config;
    std::cout << "Copy fonts" << std::endl;
    try {
        auto fontDestPath = this->getGameProjectFontDirectory();
        copyFonts(fontDestPath);
    }
    catch(const std::exception& e) {
        std::cout << e.what() << std::endl;
    }
    std::cout << "Done." << std::endl;

    return Status_Success;
}

ErrorStatus langscore::divisi_vxace::exportCSV()
{
    std::cout << "VXAce Write..." << std::endl;
    config config;
    std::u8string rootPath;
    auto exportFolderList = config.exportDirectory(rootPath);
    for(auto& folder : exportFolderList)
    {
        if(fs::exists(folder)) { continue; }
        fs::create_directories(folder);
    }

    std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(config.langscoreAnalyzeDirectorty());

    writeFixedBasicData();
    writeFixedRvScript();
    writeFixedGraphFileNameData();


    std::cout << "Write Translate File Done." << std::endl;
    return Status_Success;
}

ErrorStatus langscore::divisi_vxace::validate()
{
    std::cout << "Validate..." << std::endl;
    config config;
    std::u8string root;
    fs::path packingDirectory = config.packingInputDirectory();
    std::vector<ValidateFileInfo> csvPathList;

    if (fs::exists(packingDirectory) == false) {
        return ErrorStatus(ErrorStatus::Module::DIVISI_VXACE, 1);
    }

    auto basicData = config.rpgMakerBasicData();

    //検証する翻訳CSVを取得する。
    const auto FetchCsvFileList = [&basicData, &csvPathList](const auto& path)
    {
        for(const auto& f : fs::directory_iterator{path})
        {
            auto extension = f.path().extension();
            auto fileName = f.path().filename().stem();
            if(extension != ".csv") { continue; }

            auto result = std::find_if(basicData.cbegin(), basicData.cend(), [&fileName](const auto& x) {
                return fs::path(x.filename).filename().stem() == fileName;
            });
            if(result == basicData.cend()) { continue; }

            csvPathList.emplace_back(ValidateFileInfo{
                f.path(),
                result->textValidateInfos
            });
        }
    };

    if(config.enableLanguagePatch())
    {
        // 言語パッチモードが有効な場合、言語フォルダ事に探査。
        for(const auto& tsFolderName : this->supportLangs) {
            FetchCsvFileList(packingDirectory / tsFolderName);
        }
    }
    else
    {
        FetchCsvFileList(packingDirectory);
    }

    this->validateTranslateFileList(std::move(csvPathList));

    std::cout << "Done." << std::endl;
    return Status_Success;
}

ErrorStatus langscore::divisi_vxace::packing()
{
    std::cout << "Packing." << std::endl;
    config config;
    auto destPath = config.gameProjectPath() + u8"/Data/Translate";

    if(config.packingEnablePerLang() == false)
    {
        auto runResult = this->invoker.packingVXAce(destPath, config.packingInputDirectory());
        if(runResult.val() != 0) {
            return runResult;
        }
    }
    else
    {
        //言語毎に書き出す機能。
        //元のプロジェクトをコピーしてから、コピー先で書き換え作業を行う。
        //コピー処理をthis->currentGameProjectPathを使用して行うため、適宜変数の保持を行っている。
        this->currentGameProjectPath.make_preferred();
        auto srcPath = this->currentGameProjectPath;
        srcPath.make_preferred();
        auto gameProjectFolderName = srcPath.filename();

        auto defLangTemp = this->defaultLanguage;
        auto langList = this->supportLangs;

        auto inputGameProjectFolder = this->currentGameProjectPath;
        auto outputFolder = fs::path(config.packingPerLangOutputDir());
        for(const auto& lang : langList)
        {
            this->supportLangs = {lang};
            this->defaultLanguage = lang;
            this->currentGameProjectPath = (outputFolder / lang / gameProjectFolderName).make_preferred();
            if(fs::exists(this->currentGameProjectPath) == false) {
                fs::create_directories(this->currentGameProjectPath);
            }
            std::error_code ec;
            fs::copy(srcPath, this->currentGameProjectPath, std::filesystem::copy_options::recursive, ec);
            if(ec) {
                std::cout << "Packing Error: " << ec.message() << std::endl;
            }
            bool dummy = false;
            this->rewriteScriptList(dummy, true);

            auto csvOutputFolder = destPath;
            if(fs::exists(csvOutputFolder) == false) {
                fs::create_directories(csvOutputFolder);
            }
            auto translateFolder = inputGameProjectFolder / "Data" / "Translate";
            for(auto& csvPath : fs::recursive_directory_iterator{translateFolder})
            {
                auto csvReader = csvreader{csvPath};
                auto transMap = csvReader.currentTexts();
                std::vector<TranslateText> replacedTexts;
                for(auto trans : transMap)
                {
                    auto& translates = trans.translates;
                    std::erase_if(translates, [&lang](auto& pair) {
                        return pair.first != lang;
                    });

                    replacedTexts.emplace_back(std::move(trans));
                }

                auto fileName = csvPath.path().filename();
                csvwriter{speciftranstext{std::move(replacedTexts)}}.write(csvOutputFolder / fileName, this->defaultLanguage, MergeTextMode::AcceptTarget);
            }
        }

        this->currentGameProjectPath = std::move(srcPath);
        this->supportLangs = std::move(langList);
        this->defaultLanguage = std::move(defLangTemp);
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

    auto scriptCsv = plaincsvreader{deserializeOutPath + u8"/Scripts/_list.csv"s}.getPlainCsvTexts();
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
        const auto ext = path.extension();
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
    std::unordered_map<fs::path, std::unique_ptr<readerbase>> jsonreader_map;
    for(const auto& path : this->basicDataFileList)
    {
        std::ifstream loadFile(path);
        nlohmann::json json;
        loadFile >> json;

        auto jsonFilePath = path;
        jsonFilePath.make_preferred().replace_extension(".lsjson");
        auto fileName = jsonFilePath.filename();
        std::cout << "Write JSON : " << jsonFilePath << std::endl;

        std::unique_ptr<readerbase> reader = std::make_unique<vxace_jsonreader>(this->supportLangs, std::move(json));
        jsonwriter writer(reader);
        writer.writeForAnalyze(std::move(jsonFilePath), this->defaultLanguage, MergeTextMode::AcceptTarget);
        jsonreader_map[fileName] = std::move(reader);
    }

    std::cout << "Finish." << std::endl;
}

void divisi_vxace::writeAnalyzedRvScript(std::u8string baseDirectory)
{
    //解析直後の素のデータを書き出す。
    //無視リスト等は考慮せず、書き出し先はTempフォルダ以下になる。
    std::cout << "writeAnalyzedRvScript" << std::endl;

    //※scriptFileNameMapに拡張子は無し
    auto scriptFileNameMap = plaincsvreader{baseDirectory + u8"/Scripts/_list.csv"s}.getPlainCsvTexts();
    const auto GetScriptName = [&scriptFileNameMap](std::u8string scriptName)
    {
        for(const auto& row : scriptFileNameMap) {
            if(row[0] == scriptName) { return row[1]; }
        }
        return u8""s;
    };

    //Langscoreのスクリプトは除外する
    auto scriptList = this->scriptFileList;
    auto result_without_langscore_scripts = std::remove_if(scriptList.begin(), scriptList.end(), [&GetScriptName](const auto& x)
    {
        auto scriptFileName = x.stem().u8string();
        auto scriptName = GetScriptName(std::move(scriptFileName));
        return scriptName == std::u8string_view{Script_File_Name} || scriptName == std::u8string_view{Custom_Script_File_Name};
    });
    if(scriptList.end() != result_without_langscore_scripts) {
        scriptList.erase(result_without_langscore_scripts, scriptList.end());
    }

    config config;
    //Rubyスクリプトを予め解析してテキストを生成しておく。
    rubyreader reader({u8"scriptLineInfo"}, this->scriptFileList);
    auto& transTexts = reader.currentTexts();

    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
    for(auto& t : transTexts){
        if(t.translates.find(def_lang) == t.translates.end()){ continue; }
        t.translates[def_lang] = t.original;
    }

    //デフォルトスクリプトのVocab.rb内の文字列は予めScriptsの中に翻訳済みの内容を入れておく
    auto resourceFolder = this->appPath.parent_path() / "resource";
    auto vocabs = csvreader{{resourceFolder / "vocab.csv"}}.currentTexts();

    auto& scriptTrans = reader.curerntScriptTransMap();

    //Vocab.rbの内容を適用
    for(auto& pathPair : scriptTrans)
    {
        auto scriptFileName = utility::removeExtension(std::get<0>(pathPair));
        auto scriptName = GetScriptName(scriptFileName);
        if(scriptName != u8"Vocab"s){ continue; }

        const auto searchFunc = [&](const auto& x){
            return x.scriptLineInfo.find(scriptFileName) != std::u8string::npos;
        };
        auto begin = std::find_if(transTexts.begin(),  transTexts.end(),  searchFunc);
        auto end   = std::find_if(transTexts.rbegin(), transTexts.rend(), searchFunc).base();

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
        t.translates[u8"scriptLineInfo"s] = t.scriptLineInfo;
    }

    std::cout << "Write Analyze JSON : " << outputPath << std::endl;
    jsonwriter writer(reader);
    writer.writeForAnalyzeScript(outputPath, this->defaultLanguage, MergeTextMode::AcceptTarget);

    std::cout << "Finish." << std::endl;
}

void langscore::divisi_vxace::writeFixedBasicData()
{
    std::cout << "writeFixedBasicData" << std::endl;

    config config;
    std::u8string root;

    auto mergeTextMode = MergeTextMode::MergeKeepSource;
    auto mergeTextModeRaw = config.globalWriteMode();
    if(0 <= mergeTextModeRaw && mergeTextModeRaw <= 4) {
        mergeTextMode = static_cast<MergeTextMode>(mergeTextModeRaw);
    }

    auto ignoreScripts = config.rpgMakerBasicData();
    auto dataFileList = this->basicDataFileList;
    auto rm_result = std::ranges::remove_if(dataFileList, [&](auto& path) 
    {
        auto s = path.stem();
        if(s == "Tilesets" || s == "MapInfos" || s == "Animations") {
            return true;
        }
        auto result = std::ranges::find_if(ignoreScripts, [f = path.filename()](const auto& x) {
            return x.ignore && x.filename == f.u8string();
        });
        return result != ignoreScripts.cend();
    });
    dataFileList.erase(rm_result.begin(), rm_result.end());

    if(config.enableLanguagePatch())
    {
        const auto pair = config.exportDirectoryWithLang(root);
        std::unordered_map<fs::path, std::unique_ptr<readerbase>> jsonreader_map;
        for(auto& path : dataFileList)
        {
            std::ifstream loadFile(path);
            nlohmann::json json;
            loadFile >> json;

            auto csvFilePath = path.filename();
            csvFilePath.make_preferred().replace_extension(".csv");
            for(auto& [lang, translateFolder] : pair)
            {
                auto outputPath = translateFolder / csvFilePath;
                std::cout << "Write Fix Data CSV : " << outputPath << std::endl;
                std::unique_ptr<readerbase> reader = std::make_unique<vxace_jsonreader>(this->supportLangs, json);
                writeFixedTranslateText<csvwriter>(outputPath, reader, this->defaultLanguage, {lang}, mergeTextMode);
                jsonreader_map[path.filename()] = std::move(reader);

                // 言語ごとのフォルダに対して処理を行うため、u8stringlistに変換
                utility::u8stringlist folderList = {translateFolder};
                this->fetchActorTextFromMap(folderList, dataFileList, jsonreader_map);
                this->adjustCSV(folderList, dataFileList);
            }
        }
    }
    else
    {
        const auto translateFolderList = config.exportDirectory(root);
        std::unordered_map<fs::path, std::unique_ptr<readerbase>> jsonreader_map;
        for(auto& path : dataFileList)
        {
            std::ifstream loadFile(path);
            nlohmann::json json;
            loadFile >> json;

            auto csvFilePath = path.filename();
            csvFilePath.make_preferred().replace_extension(".csv");
            for(auto& translateFolder : translateFolderList) {
                auto outputPath = translateFolder / csvFilePath;
                std::cout << "Write Fix Data CSV : " << outputPath << std::endl;
                std::unique_ptr<readerbase> reader = std::make_unique<vxace_jsonreader>(this->supportLangs, json);
                writeFixedTranslateText<csvwriter>(outputPath, reader, this->defaultLanguage, this->supportLangs, mergeTextMode);
                jsonreader_map[path.filename()] = std::move(reader);
            }
        }
        this->fetchActorTextFromMap(translateFolderList, dataFileList, jsonreader_map);
        this->adjustCSV(translateFolderList, dataFileList);
    }

    std::cout << "writeFixedBasicData Finish." << std::endl;
}

void langscore::divisi_vxace::writeFixedRvScript()
{
    std::cout << "writeFixedRvScript" << std::endl;

    config config;

    auto mergeTextMode = MergeTextMode::MergeKeepSource;
    auto mergeTextModeRaw = config.globalWriteMode();
    if(0 <= mergeTextModeRaw && mergeTextModeRaw <= 4) {
        mergeTextMode = static_cast<MergeTextMode>(mergeTextModeRaw);
    }

    auto scriptInfoList = config.rpgMakerScripts();
    auto scriptList = scriptFileList;
    {
        auto rm_result = std::remove_if(scriptList.begin(), scriptList.end(), [&scriptInfoList](const auto& path) {
            auto osPath = path.stem();
            auto result = std::find_if(scriptInfoList.cbegin(), scriptInfoList.cend(), [&osPath](const auto& script) {
                return script.filename == osPath && script.ignore;
                });
            return result != scriptInfoList.cend();
            });
        scriptList.erase(rm_result, scriptList.end());
    }

    std::u8string root;

    if(config.enableLanguagePatch())
    {
        // 言語パッチモードが有効な場合、言語ごとに分けて出力
        const auto pair = config.exportDirectoryWithLang(root);

        // スクリプトの翻訳を書き込むCSVの書き出し
        for(auto& [lang, translateFolder] : pair)
        {
            // 各言語向けのreaderを作成
            rubyreader reader({lang}, scriptList);
            reader.applyIgnoreScripts(scriptInfoList);

            auto outputPath = translateFolder / fs::path{"Scripts.csv"};
            std::cout << "Write Fix Script CSV : " << outputPath << std::endl;
            writeFixedTranslateText<csvwriter>(outputPath, reader, this->defaultLanguage, {lang}, mergeTextMode, false);
        }
    }
    else
    {
        // 通常モードでは全言語を同じCSVに出力
        const auto translateFolderList = config.exportDirectory(root);

        // スクリプトの翻訳を書き込むCSVの書き出し
        rubyreader reader(this->supportLangs, scriptList);
        reader.applyIgnoreScripts(scriptInfoList);

        for(auto& translateFolder : translateFolderList) {
            std::cout << "Write Fix Script CSV : " << translateFolder / fs::path{"Scripts.csv"} << std::endl;
            writeFixedTranslateText<csvwriter>(translateFolder / fs::path{"Scripts.csv"}, reader, this->defaultLanguage, this->supportLangs, mergeTextMode, false);
        }
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
        else if(findStr(_line, u8"%{DEFAULT_LANGUAGE}%")) {
            _line = tab + u8"DEFAULT_LANGUAGE = \"" + utility::cnvStr<std::u8string>(defLanguage) + u8"\"";
        }
        else if(findStr(_line, u8"%{ENABLE_PATCH_MODE}%")){
            // _line = tab + u8"ENABLE_PATCH_MODE = "s + (config.enableLanguagePatch() ? u8"true"s : u8"false"s);
            _line = tab + u8"ENABLE_PATCH_MODE = false" + nl;
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


void divisi_vxace::rewriteScriptList(bool& replaceScript, bool forceUpdate)
{
    config config;
    const auto lsAnalyzePath = fs::path(config.langscoreAnalyzeDirectorty());

    auto scriptListCsv = plaincsvreader{lsAnalyzePath / "Scripts/_list.csv"}.getPlainCsvTexts();
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
    if(replaceLsCustom || forceUpdate)
    {
        std::cout << "Write langscore_custom : " << outputPath / scriptFileNameList[1] << std::endl;
        auto scriptInfoList = config.rpgMakerScripts();
        auto scriptList = scriptFileList;
        {
            auto rm_result = std::remove_if(scriptList.begin(), scriptList.end(), [&scriptInfoList](const auto& path) {
                auto osPath = path.stem();
                auto result = std::find_if(scriptInfoList.cbegin(), scriptInfoList.cend(), [&osPath](const auto& script) {
                    return script.filename == osPath && script.ignore;
                });
                return result != scriptInfoList.cend();
            });
            scriptList.erase(rm_result, scriptList.end());
        }

        //スクリプトの翻訳を書き込むCSVの書き出し
        rubyreader reader(this->supportLangs, scriptList);
        reader.applyIgnoreScripts(scriptInfoList);

        writeFixedTranslateText<rbscriptwriter>(reader, lsCustomScriptPath, this->defaultLanguage, this->supportLangs, langscore::MergeTextMode::AcceptTarget, true);
    }
    else {
        std::cout << "not replace langscore_custom.rb. replace flag is false" << std::endl;
    }

    //langscore.rbの出力
    auto outputScriptFilePath = outputPath / (scriptFileNameList[0] + u8".rb"s);
    bool replaceLs = fs::exists(outputScriptFilePath)==false;
    if(replaceLs == false){
        replaceLs = config.overwriteLangscore();
    }
    if(replaceLs || forceUpdate)
    {
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
    else {
        std::cout << "not replace langscore.rb. replace flag is false" << std::endl;
    }

}

void divisi_vxace::fetchActorTextFromMap(const utility::u8stringlist& rewriteCSVFolder, const utility::filelist& list, const std::unordered_map<fs::path, std::unique_ptr<readerbase>>& jsonreader_map)
{
    if(list.empty()){ return; }
    //アクター名の変更・二つ名の変更　の抽出
    std::vector<TranslateText> extend_names;
    fs::path actor_path;
    const std::array<int, 2> targetCode = {320, 324};
    for(auto path : list)
    {
        auto filename = path.filename();
        if(filename.string().find("Actors") != std::string::npos){
            actor_path = path;
            continue;
        }
        if(filename.string().find("Map") == std::string::npos){ continue; }

        if(jsonreader_map.find(filename) == jsonreader_map.end()) { continue; }
        const auto& json_reader = jsonreader_map.at(filename);
        auto texts = json_reader->currentTexts();
        for(auto& text : texts){
            if(std::find(targetCode.cbegin(), targetCode.cend(), text.code) == targetCode.cend()){
                continue;
            }
            extend_names.emplace_back(text);
        }
    }
    std::sort(extend_names.begin(), extend_names.end(), [](const auto& x, const auto& y) { return x.original < y.original; });
    extend_names.erase(std::unique(extend_names.begin(), extend_names.end()), extend_names.end());

    auto actor_filename = actor_path.filename().replace_extension(".csv");
    for(auto& folder : rewriteCSVFolder)
    {
        auto actor_csv_filepath = folder / actor_filename;
        if(fs::exists(actor_csv_filepath) == false){ continue; }

        auto plain_csv = plaincsvreader{actor_csv_filepath.replace_extension(".csv")}.getPlainCsvTexts();

        for(const auto& transText : extend_names)
        {
            auto result = std::find_if(plain_csv.cbegin(), plain_csv.cend(), [&](const auto& row){
                return row[0] == transText.original;
            });
            if(result != plain_csv.cend()){ continue; }

            utility::u8stringlist row = {
                transText.original, transText.nameType
            };
            plain_csv.emplace_back(std::move(row));

        }

        csvwriter::writePlain(actor_csv_filepath, std::move(plain_csv), MergeTextMode::AcceptTarget);
    }

}


void divisi_vxace::adjustCSV(const utility::u8stringlist& rewriteCSVFolder, const utility::filelist& list)
{
    utility::u8stringlist rnLineFiles = {
        u8"Actors.csv"s,  u8"Animations.csv"s, u8"Armors.csv"s,  u8"Classes.csv"s, 
        u8"Enemies.csv"s, u8"Items.csv"s,      u8"Scripts.csv"s, u8"Skills.csv"s, 
        u8"States.csv"s,  u8"System.csv"s,     u8"Weapons.csv"s
    };

    for(auto& folder : rewriteCSVFolder)
    {
        for(auto path : list)
        {
            auto csvPath = folder / path.filename().replace_extension(".csv");
            if(fs::exists(csvPath) == false){ continue; }
            auto plain_csv = plaincsvreader{csvPath}.getPlainCsvTexts();

            auto csvFileName = csvPath.filename();
            const bool isRNLine = std::find(rnLineFiles.cbegin(), rnLineFiles.cend(), csvFileName) != rnLineFiles.cend();
            const bool isRewrite = adjustCSVCore(plain_csv, isRNLine);

            if(isRewrite){
                csvwriter::writePlain(csvPath, std::move(plain_csv), MergeTextMode::AcceptTarget);
            }
        }
    }
}

bool divisi_vxace::adjustCSVCore(std::vector<utility::u8stringlist>& plain_csv, bool isRNLine)
{
    const std::u8string newLineEsc = isRNLine ? u8"\r\n" : u8"\n";
    bool isRewrite = false;
    for(auto& row : plain_csv)
    {
        for(auto& col : row)
        {
            if(col.find_first_of(newLineEsc) == col.npos){
                continue;
            }
            auto lines = utility::split(col, u8'\n');
            //1なら原文がそのまま入っている(=改行が無い)ので無視
            if(lines.size() < 2){ continue; }
            //for(auto& line : lines)
            std::for_each(lines.begin(), lines.end() - 1, [&](auto& line)
            {
                auto newLine = utility::right_trim(line, u8"\r\n"s);
                newLine.append(newLineEsc);
                line.swap(newLine);
            });
            auto result = utility::join(lines, u8""s);
            if(result != col){
                col.swap(result);
                isRewrite = true;
            }
        }
    }

    return isRewrite;
}

fs::path langscore::divisi_vxace::getGameProjectFontDirectory() const
{
    return this->currentGameProjectPath / u8"Fonts"s;
}
