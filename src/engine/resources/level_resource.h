#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{

struct LevelResource : Resource
{
	std::vector<struct UnitInstance*> unitInstances;
	bool load(Json::Value& json);
};
}