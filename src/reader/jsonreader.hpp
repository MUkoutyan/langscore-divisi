#pragma once
#include "nlohmann/json.hpp"
#include "../serialize_base.h"
#include "readerbase.h"

namespace langscore {
class jsonreaderbase : public readerbase
{
public:
	jsonreaderbase(std::vector<std::u8string> useLangs, nlohmann::json json)
		: readerbase(std::move(useLangs), {}), json(std::move(json)){
	}
	virtual ~jsonreaderbase(){};

	virtual void json2tt() = 0;

protected:
	nlohmann::json json;
};

class analyzejsonreader : public readerbase
{
public:
    // 単一のJSONファイルを読み込む
    analyzejsonreader(std::filesystem::path jsonPath)
        : readerbase({}, {}) // 言語リストとスクリプトリストは空で初期化
    {
        if(std::filesystem::exists(jsonPath)) {
            jsonFiles.push_back(jsonPath);
            plainJsonTexts = parse(jsonPath);
        }
    }

    // 複数のJSONファイルを読み込む
    analyzejsonreader(std::vector<std::filesystem::path> jsonPaths)
        : readerbase({}, {}) // 言語リストとスクリプトリストは空で初期化
    {
        for(const auto& path : jsonPaths) {
            if(std::filesystem::exists(path)) {
                jsonFiles.push_back(path);
                auto fileTexts = parse(path);
                std::move(fileTexts.begin(), fileTexts.end(), std::back_inserter(plainJsonTexts));
            }
        }
    }

    ~analyzejsonreader() override = default;

    // 生のJSONデータを取得するメソッド
    std::vector<utility::u8stringlist>& getPlainJsonTexts()& { return plainJsonTexts; }
    std::vector<utility::u8stringlist>&& getPlainJsonTexts()&& { return std::move(plainJsonTexts); }

private:
    // JSONファイルをパースする関数
    std::vector<utility::u8stringlist> parse(std::filesystem::path path)
    {
        std::vector<utility::u8stringlist> result;

        // ファイルを開く
        std::ifstream file(path);
        if(!file.is_open()) {
            return result;
        }

        // JSONを読み込む
        nlohmann::json jsonData;
        try {
            file >> jsonData;
        }
        catch(const std::exception& e) {
            std::cerr << "JSON parse error: " << e.what() << " in file: " << path.string() << std::endl;
            return result;
        }

        // JSONが配列の場合
        if(jsonData.is_array() == false) {
            return result;
        }
        // 各要素をTranslateTextに変換
        for(const auto& item : jsonData) 
        {
            if(item.contains("original")) {
                std::u8string original = utility::cnvStr<std::u8string>(item["original"].get<std::string>());
                TranslateText text(original, useLangList);

                // 翻訳テキストを取得
                if(item.contains("translates") && item["translates"].is_object()) {
                    for(const auto& [lang, value] : item["translates"].items()) {
                        std::u8string langCode = utility::cnvStr<std::u8string>(lang);
                        if(!langCode.empty()) {
                            text.translates[langCode] = utility::cnvStr<std::u8string>(value.get<std::string>());

                            // 言語リストに追加（まだなければ）
                            if(std::find(useLangList.begin(), useLangList.end(), langCode) == useLangList.end()) {
                                useLangList.push_back(langCode);
                            }
                        }
                    }
                }

                // テキストタイプを取得
                if(item.contains("type")) {
                    if(item["type"].is_array()) {
                        for(const auto& type : item["type"]) {
                            text.textType.push_back(utility::cnvStr<std::u8string>(type.get<std::string>()));
                        }
                    }
                    else if(item["type"].is_string()) {
                        text.textType.push_back(utility::cnvStr<std::u8string>(item["type"].get<std::string>()));
                    }
                }

                // コード値を取得
                if(item.contains("code")) {
                    text.code = item["code"].get<int>();
                }

                // スクリプト情報を取得
                if(item.contains("scriptLineInfo")) {
                    text.scriptLineInfo = utility::cnvStr<std::u8string>(item["scriptLineInfo"].get<std::string>());
                }

                if(item.contains("scriptParamType")) {
                    text.scriptParamType = utility::cnvStr<std::u8string>(item["scriptParamType"].get<std::string>());
                }

                texts.push_back(std::move(text));
            }

            // 生のデータもu8stringlistとして保存
            if(item.is_object()) {
                utility::u8stringlist row;
                for(auto it = item.begin(); it != item.end(); ++it) {
                    row.push_back(utility::cnvStr<std::u8string>(it.key()));
                    if(it.value().is_string()) {
                        row.push_back(utility::cnvStr<std::u8string>(it.value().get<std::string>()));
                    }
                    else {
                        row.push_back(utility::cnvStr<std::u8string>(it.value().dump()));
                    }
                }
                result.push_back(std::move(row));
            }
        }

        return result;
    }

    std::vector<std::filesystem::path> jsonFiles;
    std::vector<utility::u8stringlist> plainJsonTexts;
};
}