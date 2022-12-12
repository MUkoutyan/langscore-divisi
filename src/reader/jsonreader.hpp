#pragma once
#include "nlohmann/json.hpp"
#include "../serialize_base.h"

namespace langscore {
class jsonreaderbase{
public:
	jsonreaderbase(const nlohmann::json& json)
		: texts(), json(json), useLangs(){}
	virtual ~jsonreaderbase(){};

	virtual void json2tt(std::vector<std::u8string> useLangs) = 0;

	std::vector<TranslateText> texts;

protected:
	const nlohmann::json& json;
	std::vector<std::u8string> useLangs;
};
}