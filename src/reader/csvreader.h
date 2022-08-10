#pragma once

#include "readerbase.h"
#include "utility.hpp"

namespace langscore
{
	class csvreader: public readerbase
	{
	public:
		~csvreader() override;

		std::vector<TranslateText> parse(std::filesystem::path path) override;

		std::vector<utility::u8stringlist> parsePlain(std::filesystem::path path);
	};
}