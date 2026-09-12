#pragma once

#include "csvwriter.h"
#include "../reader/csvreader.h"
#include <string>

namespace langscore
{
    class uniquerowcsvwriter: public csvwriter
    {
    public:
        using csvwriter::csvwriter;

        bool merge(std::filesystem::path sourceFilePath) override
        {
            if(this->overwriteMode == MergeTextMode::AcceptTarget){
                return true;
            }

            langscore::csvreader reader{sourceFilePath};
            auto sourceTranslates = reader.currentTexts();
            if(sourceTranslates.empty()){ return true; }

            //ソース側を適用
            if(this->overwriteMode == MergeTextMode::AcceptSource){
                this->texts = std::move(sourceTranslates);
                return true;
            }


            utility::u8stringlist enableLanguages;
            {
                auto& list = this->texts.size() < sourceTranslates.size() ? sourceTranslates : this->texts;
                if(list.empty() == false){
                    for(auto& pair : list[0].translates){
                        enableLanguages.emplace_back(pair.first);
                    }
                }
            }

            //Graphics.csv等の行 (原文) は解析側で一意に決まるため、行の増減は行わない。
            //マージ先の行を維持したまま、空欄の翻訳だけをソースから補完する。
            for(auto& source : sourceTranslates){
                auto find_result = std::find_if(this->texts.begin(), this->texts.end(), [&source](const auto& t){
                    return t.original == source.original;
                });
                if(find_result == this->texts.end()){ continue; }

                for(auto& l : enableLanguages){
                    if(find_result->translates.find(l) == find_result->translates.end()){
                        find_result->translates[l] = u8"";
                        continue;
                    }

                    if(find_result->translates[l] == u8""){
                        find_result->translates[l] = source.translates[l];
                    }
                }
            }

            return true;
        }
    };
}