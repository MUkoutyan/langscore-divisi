#include "csvwriter.h"
#include "config.h"
#include "utility.hpp"
#include "../reader/csvreader.h"
#include <filesystem>
#include <fstream>

using namespace langscore;
namespace fs = std::filesystem;

//原文はマージフラグに関わらず、全て残すようにする。但し重複はしない。
//(現状の処理では、行数判定箇所が先1行しか見ないので、複数追加されると差分が全部含まれるかも？)
//翻訳文はマージフラグに沿って、どちらで埋めるか決定する。
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

    auto source_i = sourceTranslates.begin();
    auto target_i = this->texts.begin();
    std::u8string source_origin;
    std::u8string target_origin;

    const auto AddForSource = [&](){
        if(source_i == sourceTranslates.end()){ return false; }
        result.emplace_back(*source_i);
        source_origin.clear();
        ++source_i;
        return true;
    };
    const auto AddForTarget = [&]()
    {
        if(target_i == this->texts.end()){ return false; }
        result.emplace_back(*target_i);
        target_origin.clear();
        ++target_i;
        return true;
    };

    while(source_i != sourceTranslates.end() || target_i != this->texts.end())
    {
        if(source_i != sourceTranslates.end()){
            source_origin = withoutQuote(source_i->original);
        }
        if(target_i != this->texts.end()){
            target_origin = withoutQuote(target_i->original);
        }

        //原文が競合した場合、翻訳文のマージを行う。
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

            continue;
        }

        const auto t_index = std::distance(this->texts.begin(), target_i);
        const auto s_index = std::distance(sourceTranslates.begin(), source_i);
        //行数が不一致の場合、若い方を先に挿入
        if(t_index < s_index)
        {
            if(AddForTarget() == false){
                if(AddForSource() == false){
                    break;
                }
            }
            continue;
        }
        else if(s_index < t_index)
        {
            if(AddForSource() == false){
                if(AddForTarget() == false){
                    break;
                }
            }
            continue;
        }

        //行数が競合している
        //後の行と比較してどう挿入するかを決定する。
        std::u8string next_source_origin;
        std::u8string next_target_origin;
        if(source_i != sourceTranslates.end() && (source_i + 1) != sourceTranslates.end()){
            source_origin = withoutQuote(source_i->original);
        }
        if(target_i != this->texts.end() && (target_i + 1) != this->texts.end()){
            target_origin = withoutQuote(target_i->original);
        }

        //そこまで細かく精査せず、ざっくり判定。
        //どちらかが、別の次の行と一致していなければ追加する。
        if(source_origin != next_target_origin){
            AddForSource();
        }
        else if(target_origin != next_source_origin){
            AddForTarget();
        }
        //そうでない場合は、原文なので両方とも残す。
        else{
            if((AddForSource() | AddForTarget()) == false){
                break;
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
