#include "font_resource.h"
#include "resource_loader.h"
#include "utils.h"

namespace engine
{
bool FontResource::load(Json::Value& json)
{
	charsSprite = loader->loadSprite(json.get("sprite", "").asString());
	startGlyphFrame = json.get("startGlyphFrame", 0).asInt();
	auto& glyphCodesJson = json.get("glyphs", Json::Value(Json::ValueType::arrayValue));
	int crtFrame = startGlyphFrame;

	for (auto& glyphCodeJson : glyphCodesJson)
	{
		auto str = glyphCodeJson.asString();
		glyphCodes.insert(std::make_pair(singleUtf8ToUtf32(str.c_str()), crtFrame++));
	}

	auto& kerningsJson = json.get("kernings", Json::Value(Json::ValueType::arrayValue));

	for (auto& kerningJson : kerningsJson)
	{
		u32 glyph1 = singleUtf8ToUtf32(kerningJson.get("leftGlyph", "").asString().c_str());
		u32 glyph2 = singleUtf8ToUtf32(kerningJson.get("rightGlyph", "").asString().c_str());
		i32 kern = kerningJson.get("value", 0).asInt();
		kernings[std::make_pair(glyph1, glyph2)] = kern;
	}

	return true;
}

void FontResource::unload()
{
	charsSprite = nullptr;
	startGlyphFrame = 0;
	glyphCodes.clear();
	kernings.clear();
}

u32 FontResource::getGlyphSpriteFrame(u32 code)
{
	return glyphCodes[code];
}

bool FontResource::hasGlyph(u32 code)
{
	return glyphCodes.find(code) != glyphCodes.end();
}

}
