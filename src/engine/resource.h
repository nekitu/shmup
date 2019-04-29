#pragma once
#include "types.h"
#include <json/value.h>
#include <string>

namespace engine
{
struct Resource
{
	struct ResourceLoader* loader = nullptr;
	std::string fileName;
	u32 usageCount = 1;

	virtual ~Resource() {}
	virtual bool load(Json::Value& json) { return false; }
};
}