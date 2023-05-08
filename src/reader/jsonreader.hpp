#pragma once
#include "nlohmann/json.hpp"
#include "../serialize_base.h"
#include "readerbase.h"

namespace langscore {
class jsonreaderbase : public readerbase
{
public:
	jsonreaderbase(std::vector<std::u8string> useLangs, nlohmann::json json)
		: readerbase(std::move(useLangs), {}), json(std::move(json)){
	}
	virtual ~jsonreaderbase(){};

	virtual void json2tt() = 0;

protected:
	nlohmann::json json;
};
}