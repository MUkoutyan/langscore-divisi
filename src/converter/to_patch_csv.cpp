#include "to_patch_csv.h"
#include "../utility.hpp"
#include "../reader/csvreader.h"
#include "../writer/csvwriter.h"
#include "../reader/speciftranstext.hpp"

using namespace langscore;


convert_patch_csv::convert_patch_csv(std::filesystem::path directory_path)
{
    std::vector<std::filesystem::path> csv_files;
    for(const auto& entry : std::filesystem::directory_iterator{directory_path})
    {
        if(entry.is_regular_file() && entry.path().extension() == ".csv")
        {
            csv_files.push_back(entry.path());
        }
    }

    utility::u8stringlist useLangs = {};
    for(const auto& csv_file : csv_files)
    {
        std::cout << "Reading CSV file: " << csv_file << std::endl;
        auto&& reader = langscore::csvreader{csv_file};
        const auto& useLangs = reader.curerntUseLangList();
        const auto& readTexts = reader.currentTexts();

        for(const auto& lang : useLangs)
        {
            auto output_path = csv_file.parent_path() / lang;
            if(false == std::filesystem::exists(output_path))
            {
                std::filesystem::create_directories(output_path);
            }

            auto transTexts = readTexts;
            for(auto& t : transTexts)
            {
                //該当の言語以外の翻訳を削除
                for(auto itr = t.translates.begin(); itr != t.translates.end();) {
                    auto& pair = *itr;
                    if(pair.first == lang) { 
                        itr = std::next(itr);
                        continue; 
                    }
                    itr = t.translates.erase(itr);
                }

                if(t.translates.find(lang) == t.translates.end()){
                    t.translates[lang] = u8"";
                }
            }


            auto output_file = output_path / csv_file.filename();
            std::cout << "Writing to: " << output_file << std::endl;
            csvwriter{speciftranstext{{lang}, std::move(transTexts)}}.write(output_file, u8"", MergeTextMode::AcceptTarget);
        }
    }
}


convert_bound_csv::convert_bound_csv(std::filesystem::path directory_path)
{
    // 言語ディレクトリを探す
    std::vector<std::filesystem::path> lang_dirs;
    utility::u8stringlist enableLanguages;

    for(const auto& entry : std::filesystem::directory_iterator{directory_path})
    {
        if(entry.is_directory())
        {
            lang_dirs.push_back(entry.path());
            enableLanguages.push_back(entry.path().filename().u8string());
        }
    }

    if(lang_dirs.empty())
    {
        std::cout << "No language directories found in: " << directory_path << std::endl;
        return;
    }

    // 各言語ディレクトリ内のCSVファイルをマッピング
    std::map<std::u8string, std::vector<std::pair<std::u8string, std::filesystem::path>>> file_lang_map;

    for(const auto& lang_dir : lang_dirs)
    {
        auto lang = lang_dir.filename().u8string(); // 言語名

        for(const auto& entry : std::filesystem::directory_iterator{lang_dir})
        {
            if(entry.is_regular_file() && entry.path().extension() == ".csv")
            {
                auto filename = entry.path().filename().u8string();
                file_lang_map[filename].emplace_back(lang, entry.path());
            }
        }
    }

    // 各ファイルセットを処理
    for(const auto& [filename, lang_paths] : file_lang_map)
    {
        std::cout << "Processing file set: " << utility::cnvStr<std::string>(filename) << std::endl;

        // 結合用のテキストコレクション
        std::vector<langscore::TranslateText> combined_texts;

        // 各言語のファイルからテキストを読み込んで結合
        for(const auto& [lang, file_path] : lang_paths)
        {
            std::cout << "Reading language file: " << file_path << " (Language: " << utility::cnvStr<std::string>(lang) << ")" << std::endl;

            auto&& reader = langscore::csvreader{file_path};
            const auto& texts = reader.currentTexts();

            // 各テキストエントリを処理
            for(const auto& text : texts)
            {

                auto result = std::ranges::find(combined_texts, text.original, &TranslateText::original);
                if(result != combined_texts.end()){
                    result->translates.emplace(lang, text.translates.at(lang));
                }
                else
                {
                    auto t = TranslateText{text.original, {lang}};
                    t.translates[lang] = text.translates.at(lang);
                    combined_texts.emplace_back(std::move(t));
                }
            }
        }

        // 結合したデータを出力
        auto output_file = directory_path / filename;
        std::cout << "Writing combined file to: " << output_file << std::endl;
        csvwriter{speciftranstext{enableLanguages, std::move(combined_texts)}}.write(output_file, u8"", MergeTextMode::AcceptTarget);
    }
}
