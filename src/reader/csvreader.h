#pragma once

#include "readerbase.h"

namespace langscore
{
	class csvreader: public readerbase
	{
	public:
		~csvreader() override;

		std::vector<TranslateText> parse(std::filesystem::path path) override;
	};
}