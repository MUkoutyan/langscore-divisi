#include "readerbase.h"
#include "scripttextparser.hpp"
#include <algorithm>

using namespace langscore;

readerbase::readerbase(std::vector<std::u8string> langs, std::vector<std::filesystem::path> scriptFileList)
	: pluginInfoList()
	, useLangList(std::move(langs)), scriptFileList(std::move(scriptFileList))
	, texts(), scriptTranslatesMap()
{
}

readerbase::~readerbase()
{}