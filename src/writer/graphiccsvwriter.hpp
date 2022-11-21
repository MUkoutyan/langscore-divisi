﻿#pragma once

#include "csvwriter.h"
#include "../reader/csvreader.h"
#include <string>

namespace langscore
{
    class graphiccsvwriter: public csvwriter
    {
        std::vector<std::u8string> extentions = {
            u8".jpg", u8".png", u8".bmp", u8".jpg", u8".webp", u8".tga"
        };
    public:
        graphiccsvwriter(std::vector<std::u8string> langs, const nlohmann::json& json) = delete;
        graphiccsvwriter(std::vector<std::u8string> langs, std::vector<TranslateText> texts)
            : csvwriter(std::move(langs), std::move(texts))
        {
            //拡張子の削除
            for(auto& t : this->texts)
            {
                for(const auto& ext : this->extentions){
                    auto pos = t.original.find(ext);
                    if(pos == std::u8string::npos){ continue; }
                    t.original.erase(pos, ext.length());
                    break;
                }
            }
        }

        bool merge(std::filesystem::path sourceFilePath) override
        {
            if(this->overwriteMode == MergeTextMode::AcceptTarget){
                return true;
            }

            langscore::csvreader reader;
            auto sourceTranslates = reader.parse(sourceFilePath);
            if(sourceTranslates.empty()){ return true; }

            //ソース側を適用
            if(this->overwriteMode == MergeTextMode::AcceptSource){
                this->texts = std::move(sourceTranslates);
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

            for(auto& source : sourceTranslates){
                auto find_result = std::find_if(this->texts.begin(), this->texts.end(), [&source](const auto& t){
                    return t.original == source.original;
                });
                if(find_result == this->texts.end()){ continue; }

                for(auto& l : languages){
                    if(find_result->translates.find(l) == find_result->translates.end()){
                        find_result->translates[l] = u8"";
                        continue;
                    }

                    if(find_result->translates[l] == u8""){
                        find_result->translates[l] = source.translates[l];
                    }
                }
            }

            this->texts = std::move(result);

            return true;
        }
    };
}