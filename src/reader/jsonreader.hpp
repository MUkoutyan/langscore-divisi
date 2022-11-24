#pragma once
#include "nlohmann/json.hpp"
#include "../serialize_base.h"

namespace langscore {
class jsonreaderbase{
public:
	jsonreaderbase(const nlohmann::json& json): json(json){}
	virtual ~jsonreaderbase(){};

	virtual void json2tt() = 0;

	const nlohmann::json& json;
	std::vector<std::u8string> useLangs;
	std::vector<TranslateText> texts;
};
}