#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct Layer
{
	f32 cameraParallaxScale = 1.0f;
	bool cameraParallax = true; // used by player layer to not parallax its position by camera side moves
	bool cameraScroll = true; // if true, the layer is scrolled, if false, the layer is not scrolled by camera position, used for enemies/bosses to stay in place and player to not be affected by camera scroll progression in the level
};

struct LevelResource : Resource
{
	std::vector<Layer> layers;
	std::vector<struct Unit*> units;

	bool load(Json::Value& json) override;
	void unload() override;
};
}
