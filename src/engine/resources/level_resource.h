#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct Layer
{
	f32 parallaxScale = 1.0f;
	bool cameraScroll = true; // if true, the layer is scrolled, if false, the layer is not scrolled by camera position, used for enemies/bosses
};

struct LevelResource : Resource
{
	std::vector<Layer> layers;
	std::vector<struct UnitInstance*> unitInstances;
	bool load(Json::Value& json);
};
}
