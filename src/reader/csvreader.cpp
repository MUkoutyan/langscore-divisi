﻿#include "csvreader.h"
#include <fstream>
#include <ranges>
#include <algorithm>

using namespace langscore;

csvreader::csvreader(utility::filelist scriptFileList)
	: readerbase(std::move(scriptFileList))
{
	for(auto& path : this->scriptFileList){
		auto texts = parse(std::move(path));
		this->texts.insert(this->texts.end(), texts.begin(), texts.end());
	}
}

csvreader::csvreader(std::filesystem::path path)
	: csvreader(utility::filelist{std::move(path)})
{}

csvreader::~csvreader()
{}


std::vector<TranslateText> csvreader::parse(std::filesystem::path path)
{
	auto csv = plaincsvreader{path}.getPlainCsvTexts();
	if(csv.size() < 2){ return {}; }

	const auto& header = csv[0];
	if(header.size() < 2){ return {}; }
	this->useLangList = {header.begin() + 1, header.end()};

	std::vector<TranslateText> result;
	std::for_each(csv.cbegin() + 1, csv.cend(), [this, &header, &result](const auto& row){
		TranslateText t{row[0], this->useLangList};
		for(int i = 1; i < row.size(); ++i)
		{
			if (i < header.size())
			{
				t.translates[header[i]] = row[i];
				continue;
			}
			//何らかの原因でヘッダーと内容が噛み合わない場合、空欄の箇所に埋める。(クラッシュ防止)
			for (auto& pair : t.translates) {
				if (pair.second.empty()) {
					pair.second = row[i];
					break;
				}
			}
		}
		result.emplace_back(std::move(t));
	});

	return result;
}

plaincsvreader::plaincsvreader(utility::filelist path)
	: readerbase({std::move(path)})
{
	for(auto& path : scriptFileList){
		this->plainCsvTexts = parse(path);
	}
}

plaincsvreader::plaincsvreader(std::filesystem::path path)
	: plaincsvreader(utility::filelist{std::move(path)})
{}

plaincsvreader::~plaincsvreader()
{}

std::vector<utility::u8stringlist> plaincsvreader::parse(std::filesystem::path path)
{
	std::ifstream file(path, std::ios::binary);
	if(!file){ return {}; 
    }
    // BOMチェックとスキップ
    if(file.peek() == 0xEF) {
        unsigned char bom[3];
        file.read(reinterpret_cast<char*>(bom), 3);
        if(!(bom[0] == 0xEF && bom[1] == 0xBB && bom[2] == 0xBF)) {
            // BOMでなければファイルポインタを先頭に戻す
            file.seekg(0);
        }
    }


	utility::u8stringlist rows;
	std::u8string line_buffer;

	const auto GetChar = [&file]()
	{
		std::u8string result = u8"";
		if(file.eof()){ return result; }

		char c; 
		file.get(c);
		if(file.eof()){ return result; }
		auto length = utility::getUTF8ByteLength(c);
		result.resize(length);
		size_t i = 0;
		while(file.eof() == false)
		{
			result[i] = c;
			i++;
			if(length <= i){ break; }
			file.get(c);
			if(file.fail()){ break; }
		} 
			
		return result;
	};

	const auto ReadAndPeekNextChar = [&file, &GetChar]()
	{
		auto current_pos = file.tellg(); // 現在のファイル位置を保存
		auto result = GetChar();
		if(file.eof()){ return result; } //EOFを検出した場合は変に戻さない。
		file.seekg(current_pos); // 元の位置に戻す
		return result;
	};

	bool bracketed_dq = false;
	std::vector<utility::u8stringlist> csv;
	std::u8string col = u8"";
	utility::u8stringlist cols;

	const auto AddCols = [&cols](std::u8string col)
	{
		auto rpos = col.find(u8'\r');
		if(rpos != std::u8string::npos){
			col.erase(std::remove(col.begin(), col.end(), u8'\r'), col.end());
		}
		cols.emplace_back(std::move(col));
	};

	while(file.eof() == false)
	{
		auto c = GetChar();
		if(file.eof()){ break; }
		if(c == u8""){ continue; }

		//クオートを最優先に読み取る。
		if(c == u8"\"")
		{
			auto next = ReadAndPeekNextChar();
			if(file.eof()) { break; }
			if(bracketed_dq == false && col.empty())
			{
				//文字が何も入っていない == セルの先頭。
				//ダブルクオーテーションを含むセルも""で括られているため、
				//""の検出よりも先に行う。
				//セルの先頭が"で開始されていたら""で括られていると判定する。
				bracketed_dq = true;
				//素通りさせると下記ifのbracketed_dq判定に引っかかるので、ここでcontinueさせる。
				continue;
			}
			else if(next == u8"\"") {
				GetChar();
				//""を"として解釈するため、先読みした箇所まで読み込む。
				std::ranges::copy(c, std::back_inserter(col));
			}

			if(bracketed_dq && (next == u8"," || next == u8"\r" || next == u8"\n"))
			{
				//bracketed_dqの時点でここに来たということは、クオートが閉じられた。
				//,が続く場合はセルの末尾なので、括りフラグも無効にする。
				bracketed_dq = false;
			}
			//\nや,を含む、クオートで括られているセルはクオートを除外して格納するため、ここでcontinue
			continue;
		}

		if(bracketed_dq)
		{
			//クオートが閉じられるまで無条件で追加
			std::ranges::copy(c, std::back_inserter(col));
			continue;
		}

		//制御文字の検出。
		//クオートの処理より先に行うと、文字としての\rも制御文字として認識されてしまう。
		//ツクールのプラグインによっては\rを制御文字としているプラグインもあるため、
		//ASCIIとしての制御文字として解釈されないようにしなければいけない。
		if(c == u8"\\")
		{
			auto next = ReadAndPeekNextChar();

			if(file.eof()){
				std::ranges::copy(c, std::back_inserter(col));
				break; 
			}
			bool find = true;
			if(next == u8"a"){ next = u8"\a"; }
			else if(next == u8"b"){ next = u8"\b"; }
			else if(next == u8"t"){ next = u8"\t"; }
			else if(next == u8"n"){ next = u8"\n"; }
			else if(next == u8"r"){ next = u8"\r"; }
			else if(next == u8"\""){ next = u8"\"\""; }
            // \\\\を意図している場合、nextだけを埋め込むと\\と
            // なってしまうため、ここでは無視する。
			//else if(next == u8"\\" ){ next = u8"\\"; }
			else if(next == u8"\'"){ next = u8"\''"; }
			else{
				find = false;
			}

			if(find){
				GetChar();	//先読みしたため先読み後の位置にする。
				std::ranges::copy(next, std::back_inserter(col));
				continue;
			}
			//制御文字が見つからない場合、ツクール系の制御文字の可能性があるので\\は追加する。
		}

		if(c == u8","){
			bracketed_dq = false;
			AddCols(std::move(col));
			col = u8"";
			continue;
		}
		else if(c == u8"\n"){
			bracketed_dq = false;
			AddCols(std::move(col));
			col = u8"";
			csv.emplace_back(cols);
			cols.clear();
			continue;
		}
		std::ranges::copy(c, std::back_inserter(col));
	}

	if(col.empty() == false){ AddCols(std::move(col)); }

    // 行の最後がカンマで終わっている場合、空の列を追加
    else if(!cols.empty()) {
        AddCols(u8"");
    }

	if(cols.empty() == false){
		csv.emplace_back(std::move(cols));
	}


	return csv;
}
