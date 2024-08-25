#pragma once

#include "readerbase.h"
#include "utility.hpp"

namespace langscore
{
	class csvreader: public readerbase
	{
	public:
		csvreader(std::vector<std::u8string> langs, utility::filelist scriptFileList);
		csvreader(std::vector<std::u8string> langs, std::filesystem::path path);
		~csvreader() override;

	private:
		//Langscore内で使用するTranslateText形式で読み込む関数
		//ヘッダーから開始されていることが必須。
		std::vector<TranslateText> parse(std::filesystem::path path);
	};

	class plaincsvreader: public readerbase
	{
	public:
		plaincsvreader(utility::filelist csvList);
		plaincsvreader(std::filesystem::path path);
		~plaincsvreader() override;

		//plainCsvTestsを取得するメソッドを定義
		std::vector<utility::u8stringlist>&  getPlainCsvTexts() &  { return plainCsvTexts; }
		std::vector<utility::u8stringlist>&& getPlainCsvTexts() && { return std::move(plainCsvTexts); }

	private:
		//通常のCSVで読み込む関数
		std::vector<utility::u8stringlist> parse(std::filesystem::path path);
		std::vector<utility::u8stringlist> plainCsvTexts;
	};
}