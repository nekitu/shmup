#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct Layer
{
	f32 parallaxScale = 1.0f;
};

struct LevelResource : Resource
{
	std::vector<Layer> layers;
	std::vector<struct UnitInstance*> unitInstances;
	bool load(Json::Value& json);
};
}
