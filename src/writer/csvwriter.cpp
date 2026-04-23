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
//source : ファイル側
//target : 新規追加側
bool csvwriter::merge(std::filesystem::path sourceFilePath)
{
	if(this->overwriteMode == MergeTextMode::AcceptTarget){
		std::cout << "\tAcceptTarget : " << std::endl;
		return true;
	}

	auto sourceTranslates = csvreader{sourceFilePath}.currentTexts();
	if(sourceTranslates.empty()){ return true; }

	//ソース側を適用
	if(this->overwriteMode == MergeTextMode::AcceptSource){
		this->texts = sourceTranslates;
		std::cout << "\tAcceptSource : " << std::endl;
		return true;
	}

	std::vector<TranslateText> result;
	result.reserve(std::max(sourceTranslates.size(), this->texts.size()));

	utility::u8stringlist enableLanguages;
	{
		//AcceptSource,AcceptTarget以外の場合、言語は常にマージする。
		for (auto& pair : sourceTranslates[0].translates) {
			enableLanguages.emplace_back(pair.first);
		}
		if (this->texts.empty() == false) {
			for (auto& pair : this->texts[0].translates)
			{
				if (std::ranges::find(enableLanguages, pair.first) == enableLanguages.end()) {
					enableLanguages.emplace_back(pair.first);
				}
			}
		}
		std::ranges::sort(enableLanguages);
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

    std::vector<TranslateText> textTobeAdded;
    std::unordered_map<MergeTextMode, std::vector<size_t>> mergeInfo;

    for(auto& text : this->texts) {
        text.isNewText = true;
    }

    if(this->overwriteMode == MergeTextMode::MergeKeepSource)
    {
        for(auto& sourceText : sourceTranslates)
        {
            //source側に翻訳文があるケースが有るため、texts側をsourceで上書きする。
            auto r = std::find_if(this->texts.begin(), this->texts.end(), [&](const auto& x) {
                return CompareText(AdjustText(x.original), AdjustText(sourceText.original));
            });
            if(r == this->texts.end())
            {
                sourceText.isNewText = true;
                textTobeAdded.emplace_back(sourceText);
                continue;
            }

            sourceText.isNewText = false;
            *r = sourceText;
        }
    }
    else {
        for(auto& sourceText : sourceTranslates)
        {
            auto r = std::find_if(this->texts.begin(), this->texts.end(), [&](const auto& x) {
                return CompareText(AdjustText(x.original), AdjustText(sourceText.original));
            });
            if(r == this->texts.end()){
                sourceText.isNewText = true;
                textTobeAdded.emplace_back(sourceText);
            }
            else {
                sourceText.isNewText = false;
            }
        }
    }


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

    if(this->isAddNewContentToEnd) {
        std::stable_partition(this->texts.begin(), this->texts.end(), [](const auto& text) { return text.isNewText == false; });
    }

	return true;
}

ErrorStatus csvwriter::write(fs::path path, std::u8string defaultLanguage, MergeTextMode overwriteMode)
{
	path.replace_extension(csvwriter::extension);

	std::ofstream outputCSVFile(path, std::ios::binary);
	if(outputCSVFile.bad()){ return ErrorStatus(ErrorStatus::Module::CSVWRITER, 1); }

    // ここでBOMを書き込む
    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    outputCSVFile.write(reinterpret_cast<const char*>(bom), sizeof(bom));

	const std::u8string delimiter(u8",");
    utility::u8stringlist headers = {u8"original"};
    std::copy(this->useLangs.begin(), this->useLangs.end(), std::back_inserter(headers));

	writeU8String(outputCSVFile, utility::join(headers, delimiter));

    if(this->texts.empty()) { return Status_Success; }

	if(this->fillDefaultLanguageColumn)
	{
		//デフォルト言語の列を埋める場合の処理
		for(const auto& text : this->texts)
		{
            if(text.original.empty()) { continue; }

			outputCSVFile << "\n";
			utility::u8stringlist rowtext = {convertCsvText(text.original)};
			//ヘッダーの作成方法がTranslateText依存なので、追加もそれに倣う
			for(const auto& lang : text.translates)
			{
                if(std::ranges::find(this->useLangs, lang.first) == this->useLangs.cend()) {
                    continue;
                }
				if(defaultLanguage == lang.first && lang.second.empty()){
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
                if(std::ranges::find(this->useLangs, lang.first) == this->useLangs.cend()) {
                    continue;
                }
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

    // ここでBOMを書き込む
    const unsigned char bom[] = {0xEF, 0xBB, 0xBF};
    outputCSVFile.write(reinterpret_cast<const char*>(bom), sizeof(bom));

	for(auto& text : textList)
	{
		for(auto& t : text) {
			t = convertCsvText(t);
		}
	}

	const std::u8string delimiter(u8",");
    if(textList.size() == 1) {
        writeU8String(outputCSVFile, utility::join(textList[0], delimiter));
    }
    else {
        for(const auto& text : textList)
        {
            writeU8String(outputCSVFile, utility::join(text, delimiter));
            outputCSVFile << "\n";
        }
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
