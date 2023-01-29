#include "csvreader.h"
#include "csvreader.h"
#include <fstream>

using namespace langscore;

csvreader::~csvreader()
{}

std::vector<TranslateText> csvreader::parse(std::filesystem::path path)
{
	auto csv = parsePlain(std::move(path));
	if(csv.size() < 2){ return {}; }

	const auto& header = csv[0];
	if(header.size() < 2){ return {}; }
	useLangList = {header.begin() + 1, header.end()};

	std::vector<TranslateText> result;
	std::for_each(csv.cbegin() + 1, csv.cend(), [this, &header, &result](const auto& row){
		TranslateText t{row[0], useLangList};
		for(int i = 1; i < row.size(); ++i){
			t.translates[header[i]] = row[i];
		}
		result.emplace_back(std::move(t));
	});

	return result;
}

std::vector<utility::u8stringlist> csvreader::parsePlain(std::filesystem::path path)
{
	std::ifstream file(path, std::ios::binary);
	if(!file){ return {}; }

	utility::u8stringlist rows;
	std::u8string line_buffer;

	const auto GetChar = [&file]()
	{
		std::u8string result;
		if(file.eof()){ return result; }

		char c;
		file.get(c);
		auto length = utility::getUTF8ByteLength(c);
		result.resize(length);
		size_t i = 0;
		while(true)
		{
			result[i] = c;
			i++;
			if(file.eof() || length <= i){ break; }
			file.get(c);
		} 
			
		return result;
	};

	bool find_dq = false;
	std::vector<utility::u8stringlist> csv;
	std::u8string col = u8"";
	utility::u8stringlist cols;

	const auto AddCols = [&cols](std::u8string col){
		if(col.empty() == false && col[col.size() - 1] == u8'\n'){ col.erase(col.size() - 1, 1); }
		if(col.empty() == false && col[col.size() - 1] == u8'\r'){ col.erase(col.size() - 1, 1); }
		cols.emplace_back(std::move(col));
	};

	while(file.eof() == false)
	{
		auto c = GetChar();
		if(file.eof()){ break; }
		if(c == u8""){ continue; }

		if(c == u8"\""){ find_dq = !find_dq; }

		if(find_dq){
			std::copy(c.begin(), c.end(), std::back_inserter(col));
			continue;
		}

		if(c == u8","){
			find_dq = false;
			AddCols(std::move(col));
			col = u8"";
			continue;
		}
		else if(c == u8"\n"){
			find_dq = false;
			AddCols(std::move(col));
			col = u8"";
			csv.emplace_back(cols);
			cols.clear();
			continue;
		}
		std::copy(c.begin(), c.end(), std::back_inserter(col));
	}

	if(col.empty() == false){ cols.emplace_back(std::move(col)); }

	if(cols.empty() == false){
		csv.emplace_back(std::move(cols));
	}


	return csv;
}
