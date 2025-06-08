// src/writer/jsonwriter.cpp
#include "jsonwriter.h"
#include "config.h"
#include "utility.hpp"
#include "../reader/csvreader.h"
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace langscore;
namespace fs = std::filesystem;
using namespace std::string_literals;

bool jsonwriter::merge(std::filesystem::path sourceFilePath)
{
    if(this->overwriteMode == MergeTextMode::AcceptTarget) {
        std::cout << "\tAcceptTarget : " << std::endl;
        return true;
    }

    // �\�[�X�t�@�C������JSON��ǂݍ���
    std::ifstream sourceFile(sourceFilePath);
    if(!sourceFile.good()) {
        std::cout << "Failed to open source file: " << sourceFilePath << std::endl;
        return false;
    }

    nlohmann::json sourceJson;
    try {
        sourceFile >> sourceJson;
    }
    catch(const std::exception& e) {
        std::cout << "Failed to parse JSON: " << e.what() << std::endl;
        return false;
    }

    // JSON�z��łȂ��ꍇ�̓G���[
    if(!sourceJson.is_array()) {
        std::cout << "Source JSON is not an array" << std::endl;
        return false;
    }

    // �\�[�XJSON����TranslateText�I�u�W�F�N�g�ɕϊ�
    std::vector<TranslateText> sourceTranslates;
    for(const auto& item : sourceJson) {
        std::u8string original;
        if(item.contains("original")) {
            original = utility::cnvStr<std::u8string>(item["original"].get<std::string>());
        }
        else {
            continue; // original���Ȃ����ڂ̓X�L�b�v
        }

        TranslateText text(original, this->useLangs);

        if(item.contains("translates") && item["translates"].is_object()) {
            for(const auto& [lang, value] : item["translates"].items()) {
                text.translates[utility::cnvStr<std::u8string>(lang)] =
                    utility::cnvStr<std::u8string>(value.get<std::string>());
            }
        }

        if(item.contains("type")) {
            for(const auto& type : item["type"]) {
                text.textType.push_back(utility::cnvStr<std::u8string>(type.get<std::string>()));
            }
        }

        if(item.contains("code")) {
            text.code = item["code"].get<int>();
        }

        sourceTranslates.emplace_back(std::move(text));
    }

    // �\�[�X����K�p����ꍇ�͒P���ɏ㏑��
    if(this->overwriteMode == MergeTextMode::AcceptSource) {
        this->texts = sourceTranslates;
        std::cout << "\tAcceptSource : " << std::endl;
        return true;
    }

    // ���ꃊ�X�g�̃}�[�W
    utility::u8stringlist enableLanguages;
    {
        // AcceptSource,AcceptTarget�ȊO�̏ꍇ�A����͏�Ƀ}�[�W����
        for(auto& pair : sourceTranslates.empty() ? this->texts[0].translates : sourceTranslates[0].translates) {
            enableLanguages.emplace_back(pair.first);
        }

        if(!this->texts.empty() && !sourceTranslates.empty()) {
            for(auto& pair : this->texts[0].translates) {
                if(std::ranges::find(enableLanguages, pair.first) == enableLanguages.end()) {
                    enableLanguages.emplace_back(pair.first);
                }
            }
        }
        std::ranges::sort(enableLanguages);
    }

    // MergeKeepSource�̏ꍇ�A�\�[�X�̃e�L�X�g��D��
    if(this->overwriteMode == MergeTextMode::MergeKeepSource) {
        for(auto& sourceText : sourceTranslates) {
            auto r = std::find_if(this->texts.begin(), this->texts.end(), [&](const auto& x) {
                return x.original == sourceText.original;
                });

            if(r == this->texts.end()) {
                // ���݂̃e�L�X�g�ɑ��݂��Ȃ��ꍇ�͒ǉ�
                this->texts.emplace_back(sourceText);
                continue;
            }

            // �����̃e�L�X�g���㏑��
            *r = sourceText;
        }
    }
    // ����ȊO�̏ꍇ�iMergeKeepTarget, Both�j
    else {
        std::vector<TranslateText> textTobeAdded;
        for(auto& sourceText : sourceTranslates) {
            auto r = std::find_if(this->texts.begin(), this->texts.end(), [&](const auto& x) {
                return x.original == sourceText.original;
                });

            if(r == this->texts.end()) {
                // ���݂̃e�L�X�g�ɑ��݂��Ȃ��ꍇ�͒ǉ����X�g�ɓ����
                textTobeAdded.emplace_back(sourceText);
            }
            // MergeKeepTarget�̏ꍇ�͊����e�L�X�g��D�悷��̂ŉ������Ȃ�
            // Both�̏ꍇ�͗����̃e�L�X�g���}�[�W���鏈���������ɒǉ�
            else if(this->overwriteMode == MergeTextMode::Both) {
                // ���ꂲ�ƂɃ}�[�W
                for(const auto& lang : enableLanguages) {
                    if(sourceText.translates.find(lang) != sourceText.translates.end() &&
                        !sourceText.translates.at(lang).empty()) {

                        if(r->translates.find(lang) != r->translates.end() &&
                            !r->translates.at(lang).empty()) {
                            // �����ɖ|�󂪂���ꍇ�́A�����ێ�
                            r->translates[lang] = r->translates.at(lang) + u8"\n===\n" + sourceText.translates.at(lang);
                        }
                        else {
                            // �^�[�Q�b�g�ɖ|�󂪂Ȃ��ꍇ�̓\�[�X�̖|����g�p
                            r->translates[lang] = sourceText.translates.at(lang);
                        }
                    }
                }

                // textType����̏ꍇ�̂݁A�\�[�X����⊮
                if(r->textType.empty() == false && sourceText.textType.empty() == false){
                    r->textType = sourceText.textType;
                }

                // code��0�̏ꍇ�̂݁A�\�[�X����⊮
                if(r->code == 0 && sourceText.code != 0) {
                    r->code = sourceText.code;
                }
            }
        }

        // �ǉ��Ώۂ̃e�L�X�g���}�[�W
        std::move(textTobeAdded.begin(), textTobeAdded.end(), std::back_inserter(this->texts));
    }

    std::cout << "Merge completed: " << sourceFilePath.filename() << std::endl;
    return true;
}


ErrorStatus jsonwriter::write(fs::path path, std::u8string defaultLanguage, MergeTextMode overwriteMode)
{
    if(this->texts.empty()){ return ErrorStatus(ErrorStatus::Module::CSVWRITER, 0); }
    path.replace_extension(jsonwriter::extension);

    std::ofstream outputFile(path);
    if(outputFile.bad()){ return ErrorStatus(ErrorStatus::Module::CSVWRITER, 1); }

    nlohmann::json outputJson = nlohmann::json::array();

    for(const auto& text : this->texts)
    {
        if(text.original.empty()) { continue; }

        nlohmann::json item;
        item["original"] = utility::cnvStr<std::string>(text.original);
        
        if(text.textType.empty() == false) 
        {
            nlohmann::json typeArray = nlohmann::json::array();
            for(const auto& type : text.textType) {
                typeArray.push_back(utility::cnvStr<std::string>(type));
            }
            item["type"] = typeArray;
        }
        if(text.code != 0) {
            item["code"] = text.code;
        }

        outputJson.push_back(item);
    }

    outputFile << outputJson.dump(2);
    return Status_Success;
}

ErrorStatus jsonwriter::writeForAnalyze(fs::path path, std::u8string defaultLanguage, MergeTextMode overwriteMode)
{
    if(this->texts.empty()) { return ErrorStatus(ErrorStatus::Module::CSVWRITER, 0); }
    path.replace_extension(jsonwriter::extension);

    std::ofstream outputFile(path);
    if(outputFile.bad()) { return ErrorStatus(ErrorStatus::Module::CSVWRITER, 1); }

    nlohmann::json outputJson = nlohmann::json::array();

    for(const auto& text : this->texts)
    {
        if(text.original.empty()) { continue; }

        nlohmann::json item;
        item["original"] = utility::cnvStr<std::string>(text.original);
        
        item["type"] = utility::cnvStr<std::string>(text.textType);
        if(text.code != 0) {
            item["code"] = text.code;
        }

        outputJson.push_back(item);
    }

    outputFile << outputJson.dump(2);
    return Status_Success;
}

ErrorStatus jsonwriter::writeForAnalyzeScript(std::filesystem::path path, std::u8string defaultLanguage, MergeTextMode overwriteMode)
{
    if(this->texts.empty()) { return ErrorStatus(ErrorStatus::Module::CSVWRITER, 0); }
    path.replace_extension(jsonwriter::extension);

    std::ofstream outputFile(path);
    if(outputFile.bad()) { return ErrorStatus(ErrorStatus::Module::CSVWRITER, 1); }

    nlohmann::json outputJson = nlohmann::json::array();

    for(const auto& text : this->texts)
    {
        if(text.original.empty()) { continue; }

        nlohmann::json item;
        item["original"] = utility::cnvStr<std::string>(text.original);

        item["type"] = utility::cnvStr<std::string>(text.textType);

        auto lineInfo = utility::split(text.scriptLineInfo, u8":"s);
        item["file"] = utility::cnvStr<std::string>(lineInfo[0]);
        item["row"] = atoll(utility::cnvStr<std::string>(lineInfo[1]).c_str());
        item["col"] = atoll(utility::cnvStr<std::string>(lineInfo[2]).c_str());

        outputJson.push_back(item);
    }

    outputFile << outputJson.dump(2);
    return Status_Success;
}

ErrorStatus jsonwriter::writePlain(std::filesystem::path path, std::vector<utility::u8stringlist> textList, MergeTextMode overwriteMode)
{
    std::ofstream outputFile(path.replace_extension(jsonwriter::extension));
    if(outputFile.bad()){ return ErrorStatus(ErrorStatus::Module::CSVWRITER, 1); }

    nlohmann::json outputJson = nlohmann::json::array();
    
    if(textList.size() > 1) {
        // �w�b�_�[�s���擾
        auto headers = textList[0];
        
        // �f�[�^�s������
        for(size_t i = 1; i < textList.size(); i++) {
            nlohmann::json row = nlohmann::json::object();
            for(size_t j = 0; j < headers.size() && j < textList[i].size(); j++) {
                row[utility::cnvStr<std::string>(headers[j])] = utility::cnvStr<std::string>(textList[i][j]);
            }
            outputJson.push_back(row);
        }
    } else if(textList.size() == 1) {
        // 1�s�݂̂̏ꍇ�̏���
        nlohmann::json row = nlohmann::json::object();
        for(const auto& item : textList[0]) {
            row.push_back(utility::cnvStr<std::string>(item));
        }
        outputJson.push_back(row);
    }

    outputFile << outputJson.dump(2);
    return Status_Success;
}
