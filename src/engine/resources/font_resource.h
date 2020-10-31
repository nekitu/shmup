#pragma once
#include "types.h"
#include "resource.h"
#include "json/value.h"

namespace engine
{
struct FontResource : Resource
{
	typedef std::pair<u32, u32> GlyphCodePair;

	struct SpriteResource* charsSprite = nullptr;
	u32 startGlyphFrame = 0;
	std::map<u32, u32 /*frame*/> glyphCodes;
	std::map<GlyphCodePair, i32 /*kern value*/> kernings;

	bool load(Json::Value& json) override;
	void unload() override;
	u32 getGlyphSpriteFrame(u32 code);
};

}
