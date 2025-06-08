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

    // ソースファイルからJSONを読み込む
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

    // JSON配列でない場合はエラー
    if(!sourceJson.is_array()) {
        std::cout << "Source JSON is not an array" << std::endl;
        return false;
    }

    // ソースJSONからTranslateTextオブジェクトに変換
    std::vector<TranslateText> sourceTranslates;
    for(const auto& item : sourceJson) {
        std::u8string original;
        if(item.contains("original")) {
            original = utility::cnvStr<std::u8string>(item["original"].get<std::string>());
        }
        else {
            continue; // originalがない項目はスキップ
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

    // ソース側を適用する場合は単純に上書き
    if(this->overwriteMode == MergeTextMode::AcceptSource) {
        this->texts = sourceTranslates;
        std::cout << "\tAcceptSource : " << std::endl;
        return true;
    }

    // 言語リストのマージ
    utility::u8stringlist enableLanguages;
    {
        // AcceptSource,AcceptTarget以外の場合、言語は常にマージする
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

    // MergeKeepSourceの場合、ソースのテキストを優先
    if(this->overwriteMode == MergeTextMode::MergeKeepSource) {
        for(auto& sourceText : sourceTranslates) {
            auto r = std::find_if(this->texts.begin(), this->texts.end(), [&](const auto& x) {
                return x.original == sourceText.original;
                });

            if(r == this->texts.end()) {
                // 現在のテキストに存在しない場合は追加
                this->texts.emplace_back(sourceText);
                continue;
            }

            // 既存のテキストを上書き
            *r = sourceText;
        }
    }
    // それ以外の場合（MergeKeepTarget, Both）
    else {
        std::vector<TranslateText> textTobeAdded;
        for(auto& sourceText : sourceTranslates) {
            auto r = std::find_if(this->texts.begin(), this->texts.end(), [&](const auto& x) {
                return x.original == sourceText.original;
                });

            if(r == this->texts.end()) {
                // 現在のテキストに存在しない場合は追加リストに入れる
                textTobeAdded.emplace_back(sourceText);
            }
            // MergeKeepTargetの場合は既存テキストを優先するので何もしない
            // Bothの場合は両方のテキストをマージする処理をここに追加
            else if(this->overwriteMode == MergeTextMode::Both) {
                // 言語ごとにマージ
                for(const auto& lang : enableLanguages) {
                    if(sourceText.translates.find(lang) != sourceText.translates.end() &&
                        !sourceText.translates.at(lang).empty()) {

                        if(r->translates.find(lang) != r->translates.end() &&
                            !r->translates.at(lang).empty()) {
                            // 両方に翻訳がある場合は、両方保持
                            r->translates[lang] = r->translates.at(lang) + u8"\n===\n" + sourceText.translates.at(lang);
                        }
                        else {
                            // ターゲットに翻訳がない場合はソースの翻訳を使用
                            r->translates[lang] = sourceText.translates.at(lang);
                        }
                    }
                }

                // textTypeが空の場合のみ、ソースから補完
                if(r->textType.empty() == false && sourceText.textType.empty() == false){
                    r->textType = sourceText.textType;
                }

                // codeが0の場合のみ、ソースから補完
                if(r->code == 0 && sourceText.code != 0) {
                    r->code = sourceText.code;
                }
            }
        }

        // 追加対象のテキストをマージ
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
        // ヘッダー行を取得
        auto headers = textList[0];
        
        // データ行を処理
        for(size_t i = 1; i < textList.size(); i++) {
            nlohmann::json row = nlohmann::json::object();
            for(size_t j = 0; j < headers.size() && j < textList[i].size(); j++) {
                row[utility::cnvStr<std::string>(headers[j])] = utility::cnvStr<std::string>(textList[i][j]);
            }
            outputJson.push_back(row);
        }
    } else if(textList.size() == 1) {
        // 1行のみの場合の処理
        nlohmann::json row = nlohmann::json::object();
        for(const auto& item : textList[0]) {
            row.push_back(utility::cnvStr<std::string>(item));
        }
        outputJson.push_back(row);
    }

    outputFile << outputJson.dump(2);
    return Status_Success;
}
