#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct Layer
{
};

struct LevelResource : Resource
{

	bool load(Json::Value& json) override;
	void unload() override;
};
}
