#pragma once
#include "nlohmann/json.hpp"
#include "../serialize_base.h"

namespace langscore {
class jsonreaderbase{
public:
	jsonreaderbase(nlohmann::json json)
		: texts(), json(std::move(json)), useLangs(){}
	virtual ~jsonreaderbase(){};

	virtual void json2tt(std::vector<std::u8string> useLangs) = 0;

	std::vector<TranslateText> texts;

protected:
	nlohmann::json json;
	std::vector<std::u8string> useLangs;
};
}