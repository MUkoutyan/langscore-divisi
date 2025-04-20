#include "divisi_mvmz.h"
#include "config.h"

#include "writer/jsscriptwriter.h"
#include "writer/jsonwriter.h"
#include "writer/csvwriter.h"
#include "reader/csvreader.h"
#include "reader/mvmzjsonreader.hpp"
#include "reader/javascriptreader.hpp"
#include "reader/speciftranstext.hpp"
#include <iostream>

#include <crc32.h>

using namespace langscore;
using namespace utility;
using namespace std::literals::string_literals;
namespace fs = std::filesystem;

const static std::u8string nl = u8"\n"s;
const static std::u8string tab = u8"\t"s;

const static std::u8string pluginDescription = u8"翻訳アプリLangscoreのRPGツクールMV/MZ用プラグインです。";

// JSONオブジェクトの中のテキストを再帰的に置換する関数
static void replaceJsonValues(nlohmann::json& j, const std::vector<TranslateText>& replacementMap) 
{
    if(j.is_object()) {
        for(auto& [key, value] : j.items()) {
            replaceJsonValues(value, replacementMap); // 再帰的にオブジェクト内を置換
        }
    }
    else if(j.is_array()) {
        for(auto& item : j) {
            replaceJsonValues(item, replacementMap); // 配列要素も再帰的に置換
        }
    }
    else if(j.is_string()) {
        auto strValue = utility::cnvStr<std::u8string>(j.get<std::string>());
        auto result = std::ranges::find_if(replacementMap, [&](auto v) { return strValue == v.original; });
        if(result != replacementMap.end() && result->translates.empty() == false) {
            j = utility::cnvStr<std::string>((result->translates.begin())->second); // 置換マッピングに従って値を変更
        }
    }
}

divisi_mvmz::divisi_mvmz()
    : platform_base()
{
    config config;

    this->currentGameProjectPath = fs::path(config.gameProjectPath());
    this->currentGameProjectPath.make_preferred();

    this->defaultLanguage = utility::cnvStr<std::u8string>(config.defaultLanguage());

    for(auto langs = config.languages(); auto lang : langs){
        this->supportLangs.emplace_back(cnvStr<std::u8string>(lang.name));
    }
}

divisi_mvmz::~divisi_mvmz(){}

void divisi_mvmz::setProjectPath(std::filesystem::path path){
    this->invoker.setProjectPath(std::move(path));
}

ErrorStatus divisi_mvmz::analyze()
{
    std::cout << "Analyze Project..." << std::endl;
    config config;
    auto gameProjPath = this->currentGameProjectPath / u8"data";
    const auto baseDirectory = config.langscoreAnalyzeDirectorty();
    if(fs::exists(baseDirectory) == false) {
        std::cout << "Create Langscore Project Folder... : " << utility::cnvStr<std::string>(baseDirectory) << std::endl;
        fs::create_directories(baseDirectory);
    }

    fs::copy(gameProjPath, baseDirectory, fs::copy_options::overwrite_existing);

    auto scriptProjPath = this->currentGameProjectPath / u8"js" / u8"plugins";
    auto destScriptPath = baseDirectory+u8"\\Scripts";
    if(std::filesystem::exists(destScriptPath) == false){
        std::filesystem::create_directories(destScriptPath);
    }
    fs::copy(scriptProjPath, destScriptPath, fs::copy_options::overwrite_existing | fs::copy_options::recursive);


    //解析では言語を使用しない。 書き出されるCSVはオリジナル文のみを表示させる。
    this->supportLangs.clear();

    std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(baseDirectory);

    this->writeAnalyzedBasicData();
    this->writeAnalyzedScript(baseDirectory);

    std::cout << "AnalyzeProject Done." << std::endl;
    return Status_Success;
}

ErrorStatus divisi_mvmz::reanalysis()
{
    std::cout << "Update project..." << std::endl;
    config config;

    const auto analyzeDirPath = config.langscoreAnalyzeDirectorty();
    if(fs::exists(analyzeDirPath) == false) {
        return {ErrorStatus::Module::DIVISI_MVMZ, 1, utility::cnvStr<std::string>(analyzeDirPath)};
    }

    //変にマージしないように一旦全削除
    const auto updateDirPath = config.langscoreUpdateDirectorty();
    fs::remove_all(updateDirPath);

    //ベースデータのコピー
    auto gameProjPath = this->currentGameProjectPath / u8"data";
    fs::copy(gameProjPath, updateDirPath, fs::copy_options::overwrite_existing);
    //スクリプトのコピー
    auto scriptProjPath = this->currentGameProjectPath / u8"js\\plugins";
    auto destScriptPath = updateDirPath + u8"\\Scripts";
    if(std::filesystem::exists(destScriptPath) == false){
        std::filesystem::create_directories(destScriptPath);
    }
    fs::copy(scriptProjPath, destScriptPath, fs::copy_options::overwrite_existing | fs::copy_options::recursive);


    //アップデートでも言語を使用しない。 書き出されるCSVはオリジナル文のみを表示させる。
    this->supportLangs.clear();

    std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(updateDirPath);

    //updateフォルダへCSVの書き出し
    this->writeAnalyzedBasicData();
    this->writeAnalyzedScript(updateDirPath);

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

    enum Type { Add, Delete, Update };
    std::vector<std::pair<fs::path, Type>> messageList;
    utility::filelist throughCopyList;  //何もせずにコピーする

    //消されるファイルの列挙
    for(const auto& s : analyzeCsvList){
        auto result = std::find_if(updateCsvList.begin(), updateCsvList.end(), [&s](const auto& x){
            return x.filename() == s.filename();
        });
        if(result == updateCsvList.end()){
            messageList.emplace_back(std::make_pair(s.filename(), Type::Delete));
        }
    }
    //追加されるファイルの列挙
    for(const auto& s : updateCsvList){
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

    //展開したスクリプトのコピー
    fs::path updateScriptPath = updateDirPath + u8"/Scripts"s;
    fs::path analyzeScriptPath = (analyzeDirPath + u8"/Scripts"s);
    for(const auto& f : fs::recursive_directory_iterator{updateScriptPath}){
        auto filename = f.path().filename();
        fs::copy(updateScriptPath / filename, analyzeScriptPath / filename, fs::copy_options::overwrite_existing);
        fs::remove(f.path());
    }

    std::cout << "UpdateProject Done." << std::endl;
    return Status_Success;
}

ErrorStatus langscore::divisi_mvmz::updatePlugin()
{
    std::cout << "Update plugin..." << std::endl;
    config config;
    auto outputScriptFilePath = this->currentGameProjectPath / u8"js/plugins/Langscore.js"s;
    bool replaceLs = fs::exists(outputScriptFilePath) == false;
    if(replaceLs == false) {
        replaceLs = config.overwriteLangscore();
    }
    writeLangscorePlugin(replaceLs);
    std::cout << "Update plugin done." << std::endl;

    std::cout << "Copy font file." << std::endl;
    auto fontDestPath = this->getGameProjectFontDirectory();
    copyFonts(fontDestPath);
    std::cout << "Copy font file done." << std::endl;
    return Status_Success;
}

ErrorStatus divisi_mvmz::exportCSV()
{
    std::cout << "Export CSV..." << std::endl;
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
    writeFixedScript();
    writeFixedGraphFileNameData();

    std::cout << "Export Translate CSV File Done." << std::endl;
    return Status_Success;
}

void langscore::divisi_mvmz::fetchActorTextFromMap(const std::u8string& csvWriteFolder, const utility::filelist& list, const std::unordered_map<std::filesystem::path, std::unique_ptr<readerbase>>& jsonreader_map)
{
    if(list.empty()){ return; }
    //アクター名の変更・二つ名の変更・ニックネームの変更　の抽出
    utility::u8stringlist extend_names;
    fs::path actor_path;
    const std::array<int, 3> targetCode = {320, 324, 325};
    for(const auto& path : list)
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
            extend_names.emplace_back(text.original);
        }
    }
    std::sort(extend_names.begin(), extend_names.end());
    extend_names.erase(std::unique(extend_names.begin(), extend_names.end()), extend_names.end());

    auto actor_filename = actor_path.filename().replace_extension(".csv");

    {
        auto actor_csv_filepath = csvWriteFolder / actor_filename;
        if(fs::exists(actor_csv_filepath) == false){ return; }

        auto plain_csv = plaincsvreader{actor_csv_filepath.replace_extension(".csv")}.getPlainCsvTexts();

        for(const auto& name : extend_names)
        {
            auto result = std::find_if(plain_csv.cbegin(), plain_csv.cend(), [&](const auto& row){
                return row[0] == name;
            });
            if(result != plain_csv.cend()){ return; }

            utility::u8stringlist row;
            row.reserve(this->supportLangs.size() + 1);
            row.emplace_back(name);
            for(const auto& l : this->supportLangs){
                row.emplace_back((l == this->defaultLanguage) ? name : u8""s);
            }
            plain_csv.emplace_back(std::move(row));

        }

        csvwriter::writePlain(actor_csv_filepath, std::move(plain_csv), MergeTextMode::AcceptTarget);
    }
}

void divisi_mvmz::adjustCSV(const std::u8string& writeFolder, const utility::filelist& list)
{
    //MV/MZの改行は\nで統一
    for(const auto& path : list)
    {
        auto csvPath = writeFolder / path.filename().replace_extension(".csv");
        if(fs::exists(csvPath) == false){ continue; }
        auto plain_csv = plaincsvreader{csvPath}.getPlainCsvTexts();

        auto csvFileName = csvPath.filename();
        const bool isRewrite = adjustCSVCore(plain_csv);

        if(isRewrite){
            csvwriter::writePlain(csvPath, std::move(plain_csv), MergeTextMode::AcceptTarget);
        }
    }
}

bool divisi_mvmz::adjustCSVCore(std::vector<utility::u8stringlist>& plain_csv)
{
    const std::u8string continueLineEsc = u8"\r\n";
    bool isRewrite = false;
    for(auto& row : plain_csv)
    {
        for(auto& col : row)
        {
            if(col.find_first_of(continueLineEsc) == col.npos){
                continue;
            }
            auto lines = utility::split(col, continueLineEsc);
            //1なら原文がそのまま入っている(=改行が無い)ので無視
            if(lines.size() < 2){ continue; }
            std::for_each(lines.begin(), lines.end() - 1, [&](auto& line)
            {
                auto newLine = utility::right_trim(line, u8"\n"s);
                newLine.append(continueLineEsc);
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

ErrorStatus divisi_mvmz::validate()
{
    config config;
    std::u8string root;
    const auto exportDirectory = config.packingInputDirectory();
    std::vector<ValidateFileInfo> csvPathList;

    auto basicData = config.rpgMakerBasicData();
    for(const auto& f : fs::recursive_directory_iterator{exportDirectory})
    {
        auto extension = f.path().extension();
        auto fileName = f.path().filename();
        if(extension != ".csv") { continue; }

        auto result = std::find_if(basicData.cbegin(), basicData.cend(), [&fileName](const auto& x) {
            return fs::path(x.filename).filename() == fileName;
            });
        if(result == basicData.cend()) { continue; }

        csvPathList.emplace_back(ValidateFileInfo{
            f.path(),
            result->textValidateInfos
        });
    }

    //整合性チェック
    this->validateTranslateFileList(std::move(csvPathList));

    std::cout << "Done." << std::endl;
    return Status_Success;
}

ErrorStatus divisi_mvmz::packing()
{
    std::cout << "Packing." << std::endl;

    config config;
    if(config.packingEnablePerLang())
    {
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

            auto translateFolder = inputGameProjectFolder / "data" / "translate";
            for(auto& csvPath : fs::recursive_directory_iterator{translateFolder})
            {
                auto csvReader = csvreader{{lang}, csvPath};
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
                {
                    fileName.replace_extension(".json");
                    auto jsonPath = this->currentGameProjectPath / "data" / fileName;
                    if(fs::exists(jsonPath))
                    {
                        nlohmann::json jsonData;
                        {
                            std::ifstream jsonFile(jsonPath);
                            jsonFile >> jsonData;
                            jsonFile.close();
                        }
                        replaceJsonValues(jsonData, replacedTexts);

                        auto output = jsonData.dump(-1);
                        size_t pos = 0;
                        while((pos = output.find("\r\n", pos)) != std::string::npos) {
                            output.replace(pos, 2, "\n");
                            pos += 1;
                        }
                        std::ofstream outputFile(jsonPath, std::ios::trunc);
                        outputFile << output;
                    }
                }
            }
        }

        this->currentGameProjectPath = std::move(srcPath);
        this->supportLangs = std::move(langList);
        this->defaultLanguage = std::move(defLangTemp);
    }

    return Status_Success;
}

std::tuple<filelist, filelist, filelist> divisi_mvmz::fetchFilePathList(std::u8string deserializeOutPath)
{
    size_t numScripts = 0;
    const auto& scriptFolderPath = deserializeOutPath + u8"/Scripts";
    for(const auto& f : fs::recursive_directory_iterator{scriptFolderPath}){
        auto extension = f.path().extension();
        if(extension == ".json"){ numScripts++; }
    }
    filelist scripts;
    scripts.resize(numScripts);
    filelist basicDataList;

    auto scriptCsv = plaincsvreader{scriptFolderPath + u8"/_list.csv"s}.getPlainCsvTexts();
    for(auto& f : fs::directory_iterator{deserializeOutPath})
    {
        //Basic
        auto extension = f.path().extension();
        if(extension == ".json"){
            basicDataList.emplace_back(f.path());
        }
    }

    for(auto& f : fs::directory_iterator{scriptFolderPath})
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

    filelist graphics;
    config config;
    auto graphicsPath = this->currentGameProjectPath / "img";
    fs::recursive_directory_iterator graphItr(graphicsPath);
    for(auto& f : graphItr)
    {
        if(f.is_directory()){ continue; }
        const auto& path = f.path();
        const auto ext = path.extension();
        const auto stem = path.stem();
        if(ext == ".jpg" || ext == ".png" || ext == ".bmp"){
            auto relative_path = "img" / path.lexically_relative(graphicsPath);
            graphics.emplace_back(relative_path.parent_path() / stem);
        }
    }

    return std::forward_as_tuple(scripts, basicDataList, graphics);

}

void divisi_mvmz::writeAnalyzedBasicData()
{
    std::cout << "writeAnalyzedBasicData" << std::endl;
    std::unordered_map<fs::path, std::unique_ptr<readerbase>> jsonreader_map;
    for(auto& path : this->basicDataFileList)
    {
        std::ifstream loadFile(path);
        nlohmann::json json;
        loadFile >> json;

        auto jsonFilePath = path;
        jsonFilePath.make_preferred().replace_extension(".lsjson");
        std::cout << "Write JSON : " << jsonFilePath << std::endl;

        std::unique_ptr<readerbase> reader = std::make_unique<mvmz_jsonreader>(path, this->supportLangs, std::move(json));
        jsonwriter writer(reader);
        writer.writeForAnalyze(jsonFilePath, this->defaultLanguage, MergeTextMode::AcceptTarget);
        jsonreader_map[jsonFilePath.filename()] = std::move(reader);
    }

    if(this->basicDataFileList.empty() == false) {
        auto jsonFilePath = this->basicDataFileList[0].make_preferred().replace_extension(".lsjson");
        auto writeJsonFolder = jsonFilePath.parent_path().u8string();

        // CSVとJSONの両方を出力（互換性のため）
        this->fetchActorTextFromMap(writeJsonFolder, this->basicDataFileList, jsonreader_map);
        this->adjustCSV(writeJsonFolder, this->basicDataFileList);
    }

    std::cout << "Finish." << std::endl;
}

void langscore::divisi_mvmz::writeFixedBasicData()
{
    std::cout << "writeFixedBasicData" << std::endl;

    config config;
    std::u8string root;

    auto mergeTextMode = MergeTextMode::MergeKeepSource;
    auto mergeTextModeRaw = config.globalWriteMode();
    if(0 <= mergeTextModeRaw && mergeTextModeRaw <= 4){
        mergeTextMode = static_cast<MergeTextMode>(mergeTextModeRaw);
    }

    auto ignoreScripts = config.rpgMakerBasicData();
    auto dataFileList = this->basicDataFileList;
    auto rm_result = std::ranges::remove_if(dataFileList, [&](auto& path) {
        auto result = std::ranges::find_if(ignoreScripts, [f = path.filename()](const auto& x) {
            return x.ignore && x.filename == f.u8string();
        });
        return result != ignoreScripts.cend();
    });
    dataFileList.erase(rm_result.begin(), rm_result.end());

    if(config.exportByLanguage()) 
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
            std::unique_ptr<readerbase> reader = std::make_unique<mvmz_jsonreader>(path, this->supportLangs, json);
            for(auto& [lang, translateFolder] : pair)
            {
                auto outputPath = translateFolder / csvFilePath;
                std::cout << "Write Fix Data CSV : " << outputPath << std::endl;
                writeFixedTranslateText<csvwriter>(outputPath, reader, this->defaultLanguage, {lang}, mergeTextMode);

                this->fetchActorTextFromMap(translateFolder, dataFileList, jsonreader_map);
                this->adjustCSV(translateFolder, dataFileList);
            }                

            jsonreader_map[path.filename()] = std::move(reader);
        }
    }
    else 
    {
        const auto translateFolder = config.exportDirectory(root)[0];
        std::unordered_map<fs::path, std::unique_ptr<readerbase>> jsonreader_map;
        for(auto& path : dataFileList)
        {
            std::ifstream loadFile(path);
            nlohmann::json json;
            loadFile >> json;

            auto csvFilePath = path.filename();
            csvFilePath.make_preferred().replace_extension(".csv");
            std::unique_ptr<readerbase> reader = std::make_unique<mvmz_jsonreader>(path, this->supportLangs, json);
            
            {
                auto outputPath = translateFolder / csvFilePath;
                std::cout << "Write Fix Data CSV : " << outputPath << std::endl;
                writeFixedTranslateText<csvwriter>(outputPath, reader, this->defaultLanguage, this->supportLangs, mergeTextMode);
                jsonreader_map[path.filename()] = std::move(reader);
            }
        }
        this->fetchActorTextFromMap(translateFolder, dataFileList, jsonreader_map);
        this->adjustCSV(translateFolder, dataFileList);
    }
    

    std::cout << "writeFixedBasicData Finish." << std::endl;
}

void langscore::divisi_mvmz::writeFixedScript()
{
    std::cout << "writeFixedScript" << std::endl;
    config config;

    auto mergeTextMode = MergeTextMode::MergeKeepSource;
    auto mergeTextModeRaw = config.globalWriteMode();
    if(0 <= mergeTextModeRaw && mergeTextModeRaw <= 4){
        mergeTextMode = static_cast<MergeTextMode>(mergeTextModeRaw);
    }

    auto scriptInfoList = config.rpgMakerScripts();
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
    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
    auto pluginsPath = this->currentGameProjectPath / u8"js/plugins.js"s;
    javascriptreader scriptReader(def_lang, this->supportLangs, pluginsPath, scriptList);
    auto transTexts = scriptReader.currentTexts();
    scriptReader.applyIgnoreScripts(scriptInfoList);

    std::u8string root;
    const auto translateFolderList = config.exportDirectory(root);
    for(auto& translateFolder : translateFolderList){
        std::cout << "Write Fix Script CSV : " << translateFolder / fs::path{"Scripts.csv"} << std::endl;
        writeFixedTranslateText<csvwriter>(translateFolder / fs::path{"Scripts.csv"}, scriptReader, this->defaultLanguage, this->supportLangs, mergeTextMode, false);
    }

    std::cout << "writeFixedScript Finish." << std::endl;
}


void langscore::divisi_mvmz::writeLangscorePlugin(bool replaceLs)
{
    config config;
    //Langscore.jsの出力
    auto outputScriptFilePath = this->currentGameProjectPath / u8"js/plugins/Langscore.js"s;

    if(replaceLs) {
        std::cout << "output langscore.js" << std::endl;
        auto resourceFolder = this->appPath.parent_path() / "resource";
        const auto langscoreScriptFilePath = resourceFolder / u8"Langscore.js"s;
        std::cout << "Copy langscore : From " << langscoreScriptFilePath << " To : " << outputScriptFilePath << std::endl;
        fs::copy(langscoreScriptFilePath, outputScriptFilePath, fs::copy_options::overwrite_existing);
        std::cout << "Done." << std::endl;

        //現在の設定を元にlangscore.jsのカスタマイズ
        auto fileLines = this->formatSystemVariable(langscoreScriptFilePath);

        if(fs::exists(outputScriptFilePath))
        {
            std::cout << "Replace langscore : " << outputScriptFilePath << std::endl;
            std::ofstream outScriptFile(outputScriptFilePath, std::ios_base::trunc);
            for(const auto& l : fileLines) {
                outScriptFile << utility::toString(l) << "\n";
            }
        }
    }
    else {
        std::cout << "not output langscore.js. replace flag is false" << std::endl;
    }

    //plugin.jsの更新
    updatePluginInfo();
}

void divisi_mvmz::updatePluginInfo()
{
    std::cout << "update plugin info" << std::endl;
    
    //plugin.jsに対してLangscoreを追加
    auto pluginsPath = this->currentGameProjectPath / u8"js"s / u8"plugins.js"s;
    std::ifstream input_file(pluginsPath.generic_string());
    std::string content((std::istreambuf_iterator<char>(input_file)), std::istreambuf_iterator<char>());
    input_file.close();

    if(content.empty()){
        std::cout << "plugins.js was empty" << std::endl;
        return;
    }

    std::size_t startPos = content.find('[');
    std::size_t endPos = content.rfind(']');

    auto jsonStr = utility::cnvStr<std::u8string>(content.substr(startPos, endPos - startPos + 1));

    // JSONオブジェクトに変換
    auto jsonObject = nlohmann::ordered_json::parse(jsonStr);
    std::stringstream formattedJson;


    //整形
    formattedJson << R"(// Generated by RPG Maker.
// Do not edit this file directly.
var $plugins =
[
)";


    //SupportLanguageに埋めるパラメータの作成
    auto list = this->supportLangs;
    for(auto& t : list) { t = u8"\"" + t + u8"\""; }
    auto langs = cnvStr<std::string>(u8"["s + utility::join(list, u8","s) + u8"]");

    //MustBeIncludedImageに埋めるパラメータの作成
    auto picturesPath = this->currentGameProjectPath / u8"img";
    utility::u8stringlist pictureFiles;
    for(const auto& f : fs::recursive_directory_iterator(picturesPath)) {

        if(f.is_directory()) { continue; }
        auto fileName = f.path().filename().u8string();
        auto relativePath = f.path().lexically_relative(picturesPath);
        for(const auto& t : this->supportLangs) {
            if(utility::includes(fileName, u8"_" + t)) {
                pictureFiles.emplace_back(u8"\""s + (relativePath.parent_path() / relativePath.stem()).generic_u8string() + u8"\""s);
            }
        }
    }

    constexpr auto SupportLanguage = "Support Language";
    constexpr auto DefaultLanguage = "Default Language";
    constexpr auto MustBeIncludedImage = "Must Be Included Image";
    constexpr auto EnableLanguagePatch = "Enable Language Patch Mode";

    int numPlugins = 0;
    const auto size = jsonObject.size();
    bool findPluginInfo = false;
    config config;
    const bool enableLangPatch = config.enableLanguagePatch();

    for(auto begin = jsonObject.begin(); begin != jsonObject.end(); ++begin)
    {
        if((*begin)["name"] == "Langscore") {

            findPluginInfo = true;
            auto& params = (*begin)["parameters"];

            params[SupportLanguage] = langs;

            if(params[DefaultLanguage].empty() == false) {
                params[DefaultLanguage] = utility::cnvStr<std::string>(this->defaultLanguage);
            }
            params[EnableLanguagePatch] = (enableLangPatch ? "true"s : "false"s);
            params[MustBeIncludedImage] = "["s + utility::cnvStr<std::string>(utility::join(pictureFiles, u8","s)) + "]"s;
        }
    }

    if(findPluginInfo == false)
    {
        //他のプラグインがLangscoreのオブザーバーに追加できるようにするために、
        //必ず先頭に埋め込む。
        nlohmann::ordered_json newPlugin = {
            {"name", "Langscore"},
            {"status", true},
            {"description", cnvStr<std::string>(pluginDescription)},
            {"parameters", {
                {SupportLanguage,   langs},
                {DefaultLanguage,   utility::cnvStr<std::string>(this->defaultLanguage)},
                {EnableLanguagePatch, (enableLangPatch ? "true"s : "false"s)},
                {MustBeIncludedImage, "["s + utility::cnvStr<std::string>(utility::join(pictureFiles, u8","s)) + "]"s}
            }}
        };

        if(size == 0) {
            formattedJson << newPlugin.dump() << "\n";
        }
        else {
            formattedJson << newPlugin.dump() << ",\n";
        }
    }

    for(auto begin = jsonObject.begin(); begin != jsonObject.end(); ++begin)
    {
        // プラグインを整形して追加
        numPlugins++;
        formattedJson << (*begin).dump();
        if(numPlugins < size) {
            formattedJson << ",\n";
        }
    }
    formattedJson << "\n];\n";


    // ファイルに書き戻す
    std::ofstream outFile(this->currentGameProjectPath / u8"js/plugins.js"s);
    outFile << formattedJson.str();
    outFile.close();
}

utility::u8stringlist divisi_mvmz::formatSystemVariable(std::filesystem::path path)
{
    std::ifstream inScriptFile(path);

    std::string _linTmp;
    utility::u8stringlist result;
    config config;

    const auto findStr = [](const auto& _line, auto str) {
        return _line.find(str) != std::u8string::npos;
    };

    while(std::getline(inScriptFile, _linTmp))
    {
        std::u8string _line(_linTmp.begin(), _linTmp.end());
        if(findStr(_line, u8"%{DESCRIPTION}%")) {
            auto desc = pluginDescription;
            desc = utility::replace(desc, u8"\r\n"s, u8"\r\n * "s);
            _line = pluginDescription;
        }
        else if(findStr(_line, u8"%{DEFAULT_LANGUAGE}%"))
        {
            _line = utility::replace(_line, u8"%{DEFAULT_LANGUAGE}%"s, this->defaultLanguage);
        }
        else if(findStr(_line, u8"%{COMBO_LANGUAGE}%"))
        {
            auto list = this->supportLangs;
            std::u8string text;
            for(auto& t : list) { 
                text += u8" * @option " + t + u8" \n";
                text += u8" * @value " + t + u8" \n";
            }
            text += u8" * ";

            _line = utility::replace(_line, u8"%{COMBO_LANGUAGE}%"s, std::move(text));
        }
        else if(findStr(_line, u8"%{SUPPORT_LANGUAGE}%"))
        {
            auto list = this->supportLangs;
            for(auto& t : list) { t = u8"\"" + t + u8"\""; }
            auto langs = utility::join(list, u8","s);
            _line = u8"Langscore.Support_Language = [" + langs + u8"]";
        }
        else if(findStr(_line, u8"%{SUPPORT_LANGUAGE_STR}%"))
        {
            auto list = this->supportLangs;
            for(auto& t : list) { t = u8"\"" + t + u8"\""; }
            auto langs = utility::join(list, u8","s);
            
            _line = utility::replace(_line, u8"%{SUPPORT_LANGUAGE_STR}%"s, std::move(langs));
        }
        else if(findStr(_line, u8"%{SUPPORT_FONTS}%"))
        {
            auto fonts = config.languages();
            auto langList = this->supportLangs;
            _line = u8"Langscore.FontList = {" + nl;
            for(auto& pair : fonts)
            {
                auto lang = utility::cnvStr<std::u8string>(pair.name);
                if(std::find(langList.begin(), langList.end(), lang) == langList.end()) {
                    continue;
                }
                auto sizeStr = utility::cnvStr<std::u8string>(std::to_string(pair.font.size));
                _line += tab;
                _line += u8"\"" + lang + u8"\": {name:\"" + pair.font.name + u8"\", size:" + sizeStr + u8", fileName: \"" + pair.font.file.filename().u8string() + u8"\", isLoaded : false }, " + nl;
            }
            _line += u8"}\n";
        }
        else if(findStr(_line, u8"%{UNISON_LSCSV}%"))
        {
            auto resourceFolder = this->appPath.parent_path() / "resource";
            std::stringstream ss;
            std::cout << "Include LSCSV : " << resourceFolder / "lscsv.js" << std::endl;
            std::ifstream lscsv(resourceFolder / "lscsv.js");
            assert(lscsv.good());
            std::vector<std::string> lscsvTexts;
            std::string lscsvTmp;
            while(std::getline(lscsv, lscsvTmp)) 
            {
                //module.exports~はLSCSVのテスト用の記述なので削除する。
                //あるとツクールの結合テスト時に動作しない。
                if(findStr(lscsvTmp, "module.exports = LSCSV;") == false) {
                    lscsvTexts.emplace_back(lscsvTmp);
                }
            }
            ss << utility::join(lscsvTexts, "\n"s);
            lscsv.close();
            _line = utility::cnvStr<std::u8string>(ss.str());
        }
        else if(findStr(_line, u8"%{REQUIRED_ASSETS}%"))
        {
            std::stringstream ss;
            for(const auto& file : this->basicDataFileList) {
                ss << " * @requiredAssets data/translates/" << file.filename().stem().string()+".csv" << utility::cnvStr<std::string>(nl);
            }
            ss << " * @requiredAssets data/translates/Graphics.csv" << utility::cnvStr<std::string>(nl);
            ss << " * @requiredAssets data/translates/Scripts.csv";
            _line = utility::cnvStr<std::u8string>(ss.str());
        }

        result.emplace_back(std::move(_line));
    }
    inScriptFile.close();

    return result;
}

fs::path langscore::divisi_mvmz::getGameProjectFontDirectory() const
{
    return this->currentGameProjectPath / u8"fonts"s;
}


void divisi_mvmz::writeAnalyzedScript(std::u8string baseDirectory)
{
    //解析直後の素のデータを書き出す。
    //無視リスト等は考慮せず、書き出し先はTempフォルダ以下になる。
    std::cout << "writeAnalyzedScript" << std::endl;

    config config;
    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
    auto pluginsPath = this->currentGameProjectPath / u8"js/plugins.js"s;
    //Javascriptを予め解析してテキストを生成しておく。
    javascriptreader scriptWriter(def_lang, this->supportLangs, pluginsPath, this->scriptFileList);
    auto& transTexts = scriptWriter.currentTexts();

    for(auto& t : transTexts){
        if(t.translates.find(def_lang) == t.translates.end()){ continue; }
        t.translates[def_lang] = t.original;
        //t.scriptLineInfo.swap(t.original);
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
    csvwriter writer(scriptWriter);
    writer.write(outputPath, this->defaultLanguage, MergeTextMode::AcceptTarget);

    std::cout << "Finish." << std::endl;
}
