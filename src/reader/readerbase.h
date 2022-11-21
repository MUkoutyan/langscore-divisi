#pragma once

#include "../serialize_base.h"
#include <vector>
#include <filesystem>

namespace langscore
{
	class readerbase
	{
	public:
		readerbase();
		virtual ~readerbase();

		virtual std::vector<TranslateText> parse(std::filesystem::path path) = 0;

	protected:

		std::vector<std::u8string> useLangList;
		std::vector<TranslateText> texts;
	};
}