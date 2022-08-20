#include "csvwriter.h"
#include "config.h"
#include "utility.hpp"
#include "../reader/csvreader.h"
#include <filesystem>
#include <fstream>

using namespace langscore;
namespace fs = std::filesystem;

bool csvwriter::merge(std::filesystem::path filePath)
{
    //旧CSVとのマージ
    csvreader reader;
    auto oldTexts = reader.parse(filePath);
    if(oldTexts.empty()){ return true; }

    for(auto& newT : this->texts)
    {
        auto oldText = std::find_if(oldTexts.begin(), oldTexts.end(), [&newT](const auto& x){
            return x.original == newT.original;
        });
        if(oldText == oldTexts.end()){ continue; }
        for(auto& pair : newT.translates)
        {
            //言語チェック
            if(oldText->translates.find(pair.first) == oldText->translates.end()){
                continue;
            }

            auto text = oldText->translates[pair.first];

            if(overwriteMode == OverwriteTextMode::LeaveOldNonBlank){
                if(text != u8""){  //既に文字が入っていたら残す
                    pair.second = text;
                    continue;
                }
            }

            if(text == pair.second){ continue; }

            if(overwriteMode == OverwriteTextMode::OverwriteNew){
                continue;
            }

            if(overwriteMode == OverwriteTextMode::Both)
            {
                if(text != u8""){ text += u8"\n===\n"; }
                text += pair.second;
            }
            pair.second = text;
        }
    }

    return true;
}

bool csvwriter::write(fs::path path, OverwriteTextMode overwriteMode)
{
    if(this->texts.empty()){ return false; }
    path.replace_extension(csvwriter::extension);
   
    std::ofstream outputCSVFile(path);
    if(outputCSVFile.bad()){ return false; }
    
    const std::u8string delimiter(u8",");
    auto headers = this->texts[0].createHeader();

    writeU8String(outputCSVFile, utility::join(headers, delimiter));
    outputCSVFile << "\n";
    
    config config;
    auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
    for(const auto& text : this->texts)
    {
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
        outputCSVFile << "\n";
    }
    
    return true;
}

bool langscore::csvwriter::writePlain(std::filesystem::path path, std::vector<utility::u8stringlist> textList, OverwriteTextMode overwriteMode)
{
    std::ofstream outputCSVFile(path);
    if(outputCSVFile.bad()){ return false; }

    const std::u8string delimiter(u8",");
    for(const auto& text : textList)
    {
        writeU8String(outputCSVFile, utility::join(text, delimiter));
        outputCSVFile << "\n";
    }
    return false;
}
