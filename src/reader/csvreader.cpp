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
	if(header.size() < 3){ return {}; }
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
	std::ifstream file(path);
	if(file.bad()){ return {}; }

	utility::u8stringlist rows;
	std::string _line;
	std::u8string line_buffer;
	while(std::getline(file, _line))
	{
		std::u8string line(_line.begin(), _line.end());
		auto num_dq = std::count(line.begin(), line.end(), u8'\"');

		if(line_buffer.empty() == false){
			num_dq += 1;
		}

		if(num_dq % 2 == 0){
			line_buffer += line;
			rows.emplace_back(std::move(line_buffer));
			line_buffer = u8"";
		}
		else{
			line_buffer += line + u8'\n';	//getlineに改行は含まれないのでここで足す
		}
	}

	bool find_dq = false;
	std::vector<utility::u8stringlist> csv;
	for(auto& row : rows)
	{
		std::u8string tmp = u8"";
		utility::u8stringlist cols;
		for(auto c : row)
		{
			if(c == u8'\"'){ find_dq = !find_dq; }

			if(find_dq){
				tmp.append(1, c);
				continue;
			}

			if(c == u8','){
				find_dq = false;
				cols.emplace_back(std::move(tmp));
				tmp = u8"";
				continue;
			}
			else if(c == u8'\n'){
				find_dq = false;
				cols.emplace_back(std::move(tmp));
				tmp = u8"";
				break;
			}
			tmp.append(1, c);
		}
		if(tmp.empty() == false){ cols.emplace_back(std::move(tmp)); }

		csv.emplace_back(std::move(cols));
	}

	return csv;
}
