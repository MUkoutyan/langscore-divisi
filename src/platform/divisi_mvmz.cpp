#include "divisi_mvmz.h"
#include "config.h"
#include "writer/csvwriter.h"
#include "reader/csvreader.h"
#include "reader/mvmzjsonreader.hpp"
#include <iostream>

#include <crc32.h>

using namespace langscore;
using namespace utility;
using namespace std::literals::string_literals;
namespace fs = std::filesystem;


divisi_mvmz::divisi_mvmz()
    : platform_base()
{
    config config;

    for(auto langs = config.languages(); auto lang : langs){
        this->supportLangs.emplace_back(cnvStr<std::u8string>(lang.name));
    }
}

divisi_mvmz::~divisi_mvmz(){}

void divisi_mvmz::setProjectPath(std::filesystem::path path){
    this->invoker.setProjectPath(invoker::ProjectType::VXAce, std::move(path));
}

std::filesystem::path divisi_mvmz::exportFolderPath(std::filesystem::path fileName)
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


ErrorStatus divisi_mvmz::analyze()
{
    config config;
    auto gameProjPath = config.gameProjectPath()+u8"\\data";
    const auto baseDirecotry = config.langscoreAnalyzeDirectorty();
    fs::copy(gameProjPath, baseDirecotry, fs::copy_options::overwrite_existing);

    auto scriptProjPath = config.gameProjectPath()+u8"\\js\\plugins";
    auto destScriptPath = baseDirecotry+u8"\\Scripts";
    if(std::filesystem::exists(destScriptPath) == false){
        std::filesystem::create_directories(destScriptPath);
    }
    fs::copy(scriptProjPath, destScriptPath, fs::copy_options::overwrite_existing | fs::copy_options::recursive);


    //解析では言語を使用しない。 書き出されるCSVはオリジナル文のみを表示させる。
    this->supportLangs.clear();

    std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(baseDirecotry);

    this->writeAnalyzedBasicData();
    //this->writeAnalyzedRvScript(baseDirecotry);

    std::cout << "AnalyzeProject Done." << std::endl;
    return Status_Success;
}

ErrorStatus divisi_mvmz::update()
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

    //std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(updateDirPath);

    //updateフォルダへCSVの書き出し
    //this->writeAnalyzedBasicData();
    //this->writeAnalyzedRvScript(updateDirPath);

    //auto [analyzeScripts, analyzeDataList, analyzeGraphics] = fetchFilePathList(analyzeDirPath);

    //ファイルのリストアップ
    filelist analyzeCsvList;
    for(const auto& f : fs::directory_iterator{analyzeDirPath}){
        auto extension = f.path().extension();
        if(extension == ".csv"){
            analyzeCsvList.emplace_back(f.path());
        }
    }
    filelist updateCsvList;
    for(const auto& f : fs::directory_iterator{updateDirPath}){
        auto extension = f.path().extension();
        if(extension == ".csv"){
            updateCsvList.emplace_back(f.path());
        }
    }

    enum Type{ Add, Delete, Update };
    std::vector<std::pair<fs::path, Type>> messageList;

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
    const auto CompareFileHash = [&messageList](std::filesystem::path path, const filelist& files)
    {
        auto result = std::find_if(files.begin(), files.end(), [&path](const auto& x){
            return x.filename() == path.filename();
        });
        if(result == files.end()){
            return;
        }

        auto x_data = getFileData(path);
        CRC32 x;
        auto x_hash = x(x_data.data(), x_data.size());

        auto y_data = getFileData(*result);
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

    std::cout << "UpdateProject Done." << std::endl;
    return Status_Success;
}

ErrorStatus divisi_mvmz::write()
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

    //std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(config.langscoreAnalyzeDirectorty());

    //writeFixedBasicData();
    //writeFixedRvScript();
    //writeFixedGraphFileNameData();

    copyFonts();

    std::cout << "Export script files." << std::endl;
    //rewriteScriptList();
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

ErrorStatus divisi_mvmz::validate()
{
    config config;
    std::u8string root;
    const auto exportDirectory = config.exportDirectory(root);
    filelist csvPathList;

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
        //this->validateTranslateFileList(std::move(csvPathList));
    }

    return Status_Success;
}

ErrorStatus divisi_mvmz::packing()
{
    std::cout << "Packing." << std::endl;
    auto runResult = this->invoker.packingVXAce();
    if(runResult.val() != 0){
        return runResult;
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

    csvreader scriptCsvReader;
    auto scriptCsv = scriptCsvReader.parsePlain(scriptFolderPath + u8"/_list.csv"s);
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
    auto gameProjectPath = fs::path(config.gameProjectPath());
    auto graphicsPath = gameProjectPath / "img";
    fs::recursive_directory_iterator graphItr(graphicsPath);
    for(auto& f : graphItr)
    {
        if(f.is_directory()){ continue; }
        //パス区切り文字は\\ではなく/に統一(\\はRubyで読み取れない)
        const auto& path = f.path();
        const auto ext = path.stem();
        if(ext == ".png"){
            auto relative_path = "img" / path.lexically_relative(graphicsPath);
            graphics.emplace_back(relative_path.parent_path() / ext);
        }
    }

    return std::forward_as_tuple(scripts, basicDataList, graphics);

}

void divisi_mvmz::writeAnalyzedBasicData()
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

        csvwriter writer(this->supportLangs, std::make_unique<mvmz_jsonreader>(json));
        writer.write(csvFilePath, MergeTextMode::AcceptTarget);
    }
    std::cout << "Finish." << std::endl;
}
