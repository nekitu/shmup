#pragma once
#include "types.h"
#include "resource.h"
#include "json/value.h"

namespace engine
{
struct FontResource : Resource
{
	struct SpriteResource* charsSprite = nullptr;
	u32 startGlyphFrame = 0;
	std::map<u32, u32 /*frame*/> glyphCodes;
	std::map<std::pair<u32, u32>, i32> kernings;

	bool load(Json::Value& json) override;
	u32 getGlyphSpriteFrame(u32 code);
};

}
