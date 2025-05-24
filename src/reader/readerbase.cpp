#include "readerbase.h"
#include "scripttextparser.hpp"
#include <algorithm>

using namespace langscore;

readerbase::readerbase(std::vector<std::filesystem::path> scriptFileList)
	: pluginInfoList()
	, scriptFileList(std::move(scriptFileList))
	, texts(), scriptTranslatesMap()
{
}

readerbase::~readerbase()
{}