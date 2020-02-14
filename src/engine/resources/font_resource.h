#pragma once
#include "types.h"
#include "resource.h"
#include "json/value.h"

namespace engine
{
struct FontResource : Resource
{
	struct SpriteResource* charsSprite = nullptr;
	std::vector<u32> glyphCodes;
	std::map<std::pair<u32, u32>, i32> kernings;
	bool load(Json::Value& json) override;
};

}
