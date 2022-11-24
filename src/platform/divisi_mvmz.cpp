#include "divisi_mvmz.h"
#include "config.h"
#include <iostream>

#include <crc32.h>

using namespace langscore;
using namespace std::literals::string_literals;
namespace fs = std::filesystem;


divisi_mvmz::divisi_mvmz()
    : platform_base()
{
    config config;

    for(auto langs = config.languages(); auto lang : langs){
        this->supportLangs.emplace_back(utility::cnvStr<std::u8string>(lang.name));
    }
}

divisi_mvmz::~divisi_mvmz(){}

void divisi_mvmz::setProjectPath(std::filesystem::path path){
    this->invoker.setProjectPath(invoker::ProjectType::VXAce, std::move(path));
}


ErrorStatus divisi_mvmz::analyze()
{
    auto runResult = this->invoker.analyze();
    if(runResult.val() != 0){
        std::cerr << runResult.toStr() << std::endl;
        return runResult;
    }

    //��͂ł͌�����g�p���Ȃ��B �����o�����CSV�̓I���W�i�����݂̂�\��������B
    this->supportLangs.clear();

    config config;
    const auto baseDirecotry = config.langscoreAnalyzeDirectorty();
    //std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(baseDirecotry);

    //this->writeAnalyzedBasicData();
    //this->writeAnalyzedRvScript(baseDirecotry);

    std::cout << "AnalyzeProject Done." << std::endl;
    return Status_Success;
}

ErrorStatus langscore::divisi_mvmz::update()
{
    config config;
    //�ςɃ}�[�W���Ȃ��悤�Ɉ�U�S�폜
    const auto updateDirPath = config.langscoreUpdateDirectorty();
    const auto analyzeDirPath = config.langscoreAnalyzeDirectorty();
    fs::remove_all(updateDirPath);

    auto runResult = this->invoker.update();
    if(runResult.val() != 0){
        std::cerr << runResult.toStr() << std::endl;
        return runResult;
    }

    //�A�b�v�f�[�g�ł�������g�p���Ȃ��B �����o�����CSV�̓I���W�i�����݂̂�\��������B
    this->supportLangs.clear();

    //std::tie(this->scriptFileList, this->basicDataFileList, this->graphicFileList) = fetchFilePathList(updateDirPath);

    //update�t�H���_��CSV�̏����o��
    //this->writeAnalyzedBasicData();
    //this->writeAnalyzedRvScript(updateDirPath);

    //auto [analyzeScripts, analyzeDataList, analyzeGraphics] = fetchFilePathList(analyzeDirPath);

    //�t�@�C���̃��X�g�A�b�v
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

    //�������t�@�C���̗�
    for(auto s : analyzeCsvList){
        auto result = std::find_if(updateCsvList.begin(), updateCsvList.end(), [&s](const auto& x){
            return x.filename() == s.filename();
        });
        if(result == updateCsvList.end()){
            messageList.emplace_back(std::make_pair(s.filename(), Type::Delete));
        }
    }
    //�ǉ������t�@�C���̗�
    for(auto s : updateCsvList){
        auto result = std::find_if(analyzeCsvList.begin(), analyzeCsvList.end(), [&s](const auto& x){
            return x.filename() == s.filename();
        });
        if(result == analyzeCsvList.end()){
            messageList.emplace_back(std::make_pair(s.filename(), Type::Add));
        }
    }

    //�X�V�����t�@�C���̃`�F�b�N    
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

    //�t�@�C���̕��т������ĂȂ��̂ň�U������
    std::sort(messageList.begin(), messageList.end(), [](const auto& x, const auto& y){
        return x.first < y.first;
    });

    //���b�Z�[�W���o���A�t�@�C���̈ړ�
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

ErrorStatus langscore::divisi_mvmz::write()
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

ErrorStatus langscore::divisi_mvmz::validate()
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

    //�������`�F�b�N
    if(config.exportByLanguage())
    {
    }
    else
    {
        //this->validateTranslateFileList(std::move(csvPathList));
    }

    return Status_Success;
}

ErrorStatus langscore::divisi_mvmz::packing()
{
    std::cout << "Packing." << std::endl;
    auto runResult = this->invoker.packingVXAce();
    if(runResult.val() != 0){
        return runResult;
    }

    return Status_Success;
}
