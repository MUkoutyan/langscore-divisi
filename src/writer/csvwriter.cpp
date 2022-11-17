#include "csvwriter.h"
#include "config.h"
#include "utility.hpp"
#include "../reader/csvreader.h"
#include <filesystem>
#include <fstream>

using namespace langscore;
namespace fs = std::filesystem;

bool csvwriter::merge(std::filesystem::path sourceFilePath)
{
    if(this->overwriteMode == MergeTextMode::AcceptTarget){
        return true;
    }

    csvreader reader;
    auto sourceTranslates = reader.parse(sourceFilePath);
    if(sourceTranslates.empty()){ return true; }

    //ソース側を適用
    if(this->overwriteMode == MergeTextMode::AcceptSource){
        this->texts = sourceTranslates;
        return true;
    }

    std::vector<TranslateText> result;
    result.reserve(std::max(sourceTranslates.size(), this->texts.size())); 
    
    utility::u8stringlist languages;
    {
        auto& list = this->texts.size() < sourceTranslates.size() ? sourceTranslates : this->texts;
        if(list.empty() == false){
            for(auto& pair : list[0].translates){
                languages.emplace_back(pair.first);
            }
        }
    }

    auto AddTranslateText = [&result](TranslateText text)
    {
        auto r = std::find_if(result.begin(), result.end(), [&text](const auto& x){
            return withoutQuote(x.original) == withoutQuote(text.original);
        });
        if(r != result.end()){

        }
        else{
            result.emplace_back(std::move(text));
        }
    };

    auto source_i = sourceTranslates.begin();
    auto target_i = this->texts.begin();
    std::u8string source_origin;
    std::u8string target_origin;
    while(source_i != sourceTranslates.end() || target_i != this->texts.end())
    {
        if(source_i != sourceTranslates.end()){
            source_origin = withoutQuote(source_i->original);
        }
        if(target_i != this->texts.end()){
            target_origin = withoutQuote(target_i->original);
        }

        if(source_origin == target_origin)
        {
            if(source_origin.empty()){
                ++source_i;
                ++target_i;
                continue;
            }

            auto trans_result = *target_i;
            for(const auto& lang : languages)
            {
                //マージ元に言語列があるかをチェック
                if(source_i->translates.find(lang) == source_i->translates.end()){
                    //無ければ何もしない(競合しないし結合も出来ないので)
                    continue;
                }

                auto sourceText = withoutQuote(source_i->translates[lang]);
                auto targetText = withoutQuote(trans_result.translates[lang]);

                //内容が異なる場合はマージモードに準拠して内容を入れ替える
                if(sourceText != targetText)
                {
                    if(overwriteMode == MergeTextMode::MergeKeepSource){
                        trans_result.translates[lang] = sourceText;
                        continue;
                    }
                    else if(overwriteMode == MergeTextMode::MergeKeepTarget){
                        //MergeKeepTargetは現在の内容を残すので、何もしない。
                        continue;
                    }
                    else if(overwriteMode == MergeTextMode::Both)
                    {
                        if(sourceText != u8""){ sourceText += u8"\n===\n"; }
                        sourceText += trans_result.translates[lang];
                        trans_result.translates[lang] = sourceText;
                    }
                }
                //内容が一致する場合は何もしない
            }
            result.emplace_back(trans_result);
            source_origin.clear();
            target_origin.clear();
            ++source_i;
            ++target_i;
        }
        else if(target_origin < source_origin)
        {
            if(target_i == this->texts.end())
            {
                if(source_i != sourceTranslates.end()){
                    result.emplace_back(*source_i);
                    source_origin.clear();
                    ++source_i;
                }
            }
            else{
                result.emplace_back(*target_i);
                target_origin.clear();
                ++target_i;
            }
        }
        else if(source_origin < target_origin)
        {
            if(source_i == sourceTranslates.end()){
                if(target_i != this->texts.end()){
                    result.emplace_back(*target_i);
                    target_origin.clear();
                    ++target_i;
                }
            }
            else{
                result.emplace_back(*source_i);
                source_origin.clear();
                ++source_i;
            }
        }

    }

    this->texts = std::move(result);

    return true;
}

ErrorStatus csvwriter::write(fs::path path, MergeTextMode overwriteMode)
{
    if(this->texts.empty()){ return ErrorStatus(ErrorStatus::Module::CSVWRITER, 0); }
    path.replace_extension(csvwriter::extension);
   
    std::ofstream outputCSVFile(path);
    if(outputCSVFile.bad()){ return ErrorStatus(ErrorStatus::Module::CSVWRITER, 1); }
    
    const std::u8string delimiter(u8",");
    auto headers = this->texts[0].createHeader();

    writeU8String(outputCSVFile, utility::join(headers, delimiter));
    outputCSVFile << "\n";
    
    config config;
    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
    size_t i = 0;
    for(const auto& text : this->texts)
    {
        if(i != 0){
            outputCSVFile << "\n";
        }
        utility::u8stringlist rowtext = {text.original};
        //ヘッダーの作成方法がTranslateText依存なので、追加もそれに倣う
        for(const auto& lang : text.translates)
        {
            if(def_lang == lang.first && lang.second.empty()){
                rowtext.emplace_back(text.original);
            }
            else {
                rowtext.emplace_back(lang.second);
            }
        }

        writeU8String(outputCSVFile, utility::join(rowtext, delimiter));
        ++i;
    }
    
    return Status_Success;
}

ErrorStatus langscore::csvwriter::writePlain(std::filesystem::path path, std::vector<utility::u8stringlist> textList, MergeTextMode overwriteMode)
{
    std::ofstream outputCSVFile(path);
    if(outputCSVFile.bad()){ return ErrorStatus(ErrorStatus::Module::CSVWRITER, 1); }

    const std::u8string delimiter(u8",");
    for(const auto& text : textList)
    {
        writeU8String(outputCSVFile, utility::join(text, delimiter));
        outputCSVFile << "\n";
    }
    return Status_Success;
}
