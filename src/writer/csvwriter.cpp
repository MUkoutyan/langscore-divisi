#include "csvwriter.h"
#include "config.h"
#include "utility.hpp"
#include "../reader/csvreader.h"
#include <filesystem>
#include <fstream>
#include <iostream>

using namespace langscore;
namespace fs = std::filesystem;
using namespace std::string_literals;

//ソースはマージフラグに関わらず、全て残すようにする。但し重複はしない。
//(現状の処理では、行数判定箇所が先1行しか見ないので、複数追加されると差分が全部含まれるかも？)
//翻訳文はマージフラグに沿って、どちらで埋めるか決定する。
bool csvwriter::merge(std::filesystem::path sourceFilePath)
{
	if(this->overwriteMode == MergeTextMode::AcceptTarget){
		std::cout << "\tAcceptTarget : " << std::endl;
		return true;
	}

	auto sourceTranslates = csvreader{this->useLangs, sourceFilePath}.currentTexts();
	if(sourceTranslates.empty()){ return true; }

	//ソース側を適用
	if(this->overwriteMode == MergeTextMode::AcceptSource){
		this->texts = sourceTranslates;
		std::cout << "\tAcceptSource : " << std::endl;
		return true;
	}

	std::vector<TranslateText> result;
	result.reserve(std::max(sourceTranslates.size(), this->texts.size()));

	utility::u8stringlist languages;
	{
		//AcceptSource,AcceptTarget以外の場合、言語は常にマージする。
		for (auto& pair : sourceTranslates[0].translates) {
			languages.emplace_back(pair.first);
		}
		if (this->texts.empty() == false) {
			for (auto& pair : this->texts[0].translates)
			{
				if (std::ranges::find(languages, pair.first) == languages.end()) {
					languages.emplace_back(pair.first);
				}
			}
		}
		std::ranges::sort(languages);
	}

    const auto AdjustText = [](auto text) {
        auto t = withoutQuote(std::move(text));
        auto replaced = utility::replace(t, u8"\r\n"s, u8"\n"s);
        return replaced;
    };

    const auto CompareText = [](const auto& x, const auto& y) 
    {
        auto x1 = utility::split(x, u8"\n"s);
        auto y1 = utility::split(y, u8"\n"s);
        return x1 == y1;
    };

    //追加済のテキストは弾く。
    std::vector<TranslateText> textTobeAdded;
    for(auto begin = this->texts.begin(); begin != this->texts.end(); ++begin)
    {
        auto target = AdjustText(begin->original);
        if(target.empty()) { continue; }
        auto r = std::find_if(sourceTranslates.begin(), sourceTranslates.end(), [&](const auto& t) {
            auto source_origin = AdjustText(t.original);
            return CompareText(source_origin, target);
            });
        if(r != sourceTranslates.end()) {
            *begin = *r;
        }
        else {
            textTobeAdded.emplace_back(*begin);
        }
    }

    if(textTobeAdded.empty()) {
        std::cout << "No merge required" << std::endl;
        return true;
    }

    //source : ファイル側
    //target : 新規追加側
	auto source_i = sourceTranslates.begin();
	auto target_i = textTobeAdded.begin();
	std::u8string source_origin;
	std::u8string target_origin;

	std::unordered_map<MergeTextMode, std::vector<size_t>> mergeInfo;
	const auto AddForSource = [&]()
	{
		if(source_i == sourceTranslates.end()){ return false; }
		if(source_i->translates.size() != languages.size()) {
			for (const auto& lang : languages) {
				if (source_i->translates.find(lang) == source_i->translates.end()) {
					source_i->translates[lang] = u8"";
				}
			}
		}
		result.emplace_back(*source_i);
		source_origin.clear();
		mergeInfo[MergeTextMode::MergeKeepTarget].emplace_back(result.size());
		++source_i;
		return true;
	};
	const auto AddForTarget = [&]()
	{
		if(target_i == textTobeAdded.end()){ return false; }
		if(target_i->translates.size() != languages.size()) {
			for (const auto& lang : languages) {
				if (target_i->translates.find(lang) == target_i->translates.end()) {
					target_i->translates[lang] = u8"";
				}
			}
		}
		result.emplace_back(*target_i);
		target_origin.clear();
		++target_i;
		return true;
	};

	while(target_i != textTobeAdded.end() || source_i != sourceTranslates.end())
	{
        const bool validTarget = target_i != textTobeAdded.end();
		const bool validSource = source_i != sourceTranslates.end();

        if(validTarget) {
            target_origin = AdjustText(target_i->original);
        }
		if(validSource){
			source_origin = AdjustText(source_i->original);
		}

		//原文が一致した場合した場合、翻訳文が競合しているかで適宜マージを行う。
		if(validSource && validTarget && CompareText(source_origin, target_origin))
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

				auto sourceTransText = withoutQuote(source_i->translates[lang]);
				auto targetTransText = withoutQuote(trans_result.translates[lang]);

				//内容が異なる場合はマージモードに準拠して内容を入れ替える
				if(sourceTransText != targetTransText)
				{
					if(overwriteMode == MergeTextMode::MergeKeepSource){
						trans_result.translates[lang] = sourceTransText;
						continue;
					}
					else if(overwriteMode == MergeTextMode::MergeKeepTarget){
						//MergeKeepTargetは現在の内容を残すので、何もしない。
						continue;
					}
					else if(overwriteMode == MergeTextMode::Both)
					{
						if(sourceTransText != u8""){ sourceTransText += u8"\n===\n"; }
						sourceTransText += trans_result.translates[lang];
						trans_result.translates[lang] = sourceTransText;
					}
					mergeInfo[static_cast<MergeTextMode>(overwriteMode)].emplace_back(result.size() + 1);
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

		//原文が競合している場合、現在のターゲット側の内容がソース側のどの行数と一致するかを検索。
		auto find_result = std::find_if(source_i, sourceTranslates.end(), [&AdjustText, &target_origin, &CompareText](const auto& x){
			return CompareText(AdjustText(x.original), target_origin);
		});

		//ソース側に見つからない場合(新規追加)はそのまま追加
		if(find_result == sourceTranslates.end()){
			if(AddForTarget() == false){
                break;
			}
		}
		//同じ行だった場合はソース側から追加(source_origin == target_originと等価なのであり得なそう？)
		else if(source_i == find_result){
			if(AddForSource() == false){
				if(AddForTarget() == false){
					break;
				}
			}
		}
		else
		{
			//ソース側で見つかった場合、先にソース側の検出行までの内容を挿入。
			//次のループでターゲット行とソース行が一致するはず。
			auto count = std::max(size_t(0ull), size_t(std::distance(source_i, find_result)));
			for(auto i = 0; i < count; ++i){
				AddForSource();
			}
		}
	}

    textTobeAdded = std::move(result);

#if defined(_DEBUG)
	//全ての列が検出した言語を含んでいるかをチェック(強制エラーなのでデバッグ時のみ使用)
	if (std::ranges::all_of(textTobeAdded, [this, &textTobeAdded](const auto& t) {
		return t.translates.size() == textTobeAdded[0].translates.size();
		}) == false) 
	{
		auto invalidCell = std::ranges::find_if(textTobeAdded, [this, &textTobeAdded](const auto& t) {
			return t.translates.size() != textTobeAdded[0].translates.size();
		});
		//ここに引っかかった場合、result.emplaceした要素に何らかの原因で新規追加した言語が含まれなかった。
		std::cout << "Error! : The overall number of columns does not match. (line : " << std::distance(textTobeAdded.begin(), invalidCell);
		return false;
	}
#endif

	if(mergeInfo.empty() == false)
	{
		std::cout << "Merge : " << sourceFilePath.filename() << std::endl;

		std::for_each(mergeInfo.begin(), mergeInfo.end(), [](const auto& x)
		{
			auto mode = std::get<0>(x);
			if(mode == MergeTextMode::AcceptSource){
				std::cout << "\tKeepSource line:";
			}
			else if(mode == MergeTextMode::AcceptTarget){
				std::cout << "\tKeepTarget line:";
			}
			else if(mode == MergeTextMode::MergeKeepSource){
				std::cout << "\tBoth line:";
			}
			else if(mode == MergeTextMode::MergeKeepTarget){
				std::cout << "\tInsert line:";
			}
			const auto& lines = std::get<1>(x);
			std::copy(lines.begin(), lines.end(), std::ostream_iterator<size_t>(std::cout, ", "));
		});
		std::cout << std::endl;
	}

    std::move(textTobeAdded.begin(), textTobeAdded.end(), std::back_inserter(this->texts));

	return true;
}

ErrorStatus csvwriter::write(fs::path path, MergeTextMode overwriteMode)
{
	if(this->texts.empty()){ return ErrorStatus(ErrorStatus::Module::CSVWRITER, 0); }
	path.replace_extension(csvwriter::extension);

	std::ofstream outputCSVFile(path, std::ios::binary);
	if(outputCSVFile.bad()){ return ErrorStatus(ErrorStatus::Module::CSVWRITER, 1); }

	const std::u8string delimiter(u8",");
	auto headers = this->texts[0].createHeader();


	writeU8String(outputCSVFile, utility::join(headers, delimiter));
	if(this->fillDefLangCol)
	{
		config config;
		auto def_lang = utility::cnvStr<std::u8string>(config.defaultLanguage());
		//デフォルト言語の列を埋める場合の処理
		for(const auto& text : this->texts)
		{
            if(text.original.empty()) { continue; }

			outputCSVFile << "\n";
			utility::u8stringlist rowtext = {convertCsvText(text.original)};
			//ヘッダーの作成方法がTranslateText依存なので、追加もそれに倣う
			for(const auto& lang : text.translates)
			{
				if(def_lang == lang.first && lang.second.empty()){
					rowtext.emplace_back(convertCsvText(text.original));
				}
				else {
					rowtext.emplace_back(convertCsvText(lang.second));
				}
			}

			writeU8String(outputCSVFile, utility::join(rowtext, delimiter));
		}
	}
	else{

		for(const auto& text : this->texts)
		{
            if(text.original.empty()) { continue; }

			outputCSVFile << "\n";
			utility::u8stringlist rowtext = {convertCsvText(text.original)};
			//ヘッダーの作成方法がTranslateText依存なので、追加もそれに倣う
			for(const auto& lang : text.translates)
			{
				rowtext.emplace_back(convertCsvText(lang.second));
			}

			writeU8String(outputCSVFile, utility::join(rowtext, delimiter));
		}
	}

	return Status_Success;
}

ErrorStatus csvwriter::writePlain(std::filesystem::path path, std::vector<utility::u8stringlist> textList, MergeTextMode overwriteMode)
{
	std::ofstream outputCSVFile(path, std::ios::binary);
	if(outputCSVFile.bad()){ return ErrorStatus(ErrorStatus::Module::CSVWRITER, 1); }


	for(auto& text : textList)
	{
		for(auto& t : text) {
			t = convertCsvText(t);
		}
	}

	const std::u8string delimiter(u8",");
	for(const auto& text : textList)
	{
		writeU8String(outputCSVFile, utility::join(text, delimiter));
		outputCSVFile << "\n";
	}
	return Status_Success;
}

std::u8string csvwriter::convertCsvText(std::u8string_view text)
{
	bool add_dq = false;
	//セル自体を""で括るかのチェック。先頭と末尾が"でなければ"をそれぞれに追加する。
	if(text.find(u8"\n") != std::u8string::npos || 
	   text.find(u8"\"") != std::u8string::npos ||
	   text.find(u8",") != std::u8string::npos)
	{
		add_dq = true;
	}

	std::u8string result;
	for(auto c : text)
	{
		result += c;
		if(c == u8'\"'){
			result += u8"\"";
		}
	}

	if(add_dq){
		result = u8"\"" + result + u8"\"";
	}

	return result;
}
