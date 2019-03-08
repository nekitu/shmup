#pragma once
#include "types.h"
#include "image_atlas.h"
#include <string>
#include <vector>
#include <unordered_map>

namespace engine
{
struct FontGlyph
{
	AtlasImage* image = nullptr;
	u32 code = 0;
	f32 bearingX = 0.0f;
	f32 bearingY = 0.0f;
	f32 advanceX = 0.0f;
	f32 advanceY = 0.0f;
	i32 bitmapLeft = 0;
	i32 bitmapTop = 0;
	u32 pixelWidth = 0;
	u32 pixelHeight = 0;
	i32 pixelX = 0;
	i32 pixelY = 0;
	Rgba32* rgbaBuffer = nullptr;
};

struct FontKerningPair
{
	u32 glyphLeft = 0;
	u32 glyphRight = 0;
	f32 kerning = 0.0f;
};

struct FontMetrics
{
	f32 height;
	f32 ascender;
	f32 descender;
	f32 underlinePosition;
	f32 underlineThickness;
};

struct FontTextSize
{
	f32 width = 0;
	f32 height = 0;
	f32 maxBearingY = 0;
	f32 maxGlyphHeight = 0;
	u32 lastFontIndex = 0;
	std::vector<f32> lineHeights;
};

class Font
{
public:
	Font() {}
	Font(const std::string& fontFilename, u32 fontFaceSize, ImageAtlas* atlas);
	~Font();

	void load(const std::string& fontFilename, u32 fontFaceSize, ImageAtlas* atlas);
	void resetFaceSize(u32 fontFaceSize);
	FontGlyph* getGlyph(u32 glyphCode);
	AtlasImage* getGlyphImage(u32 glyphCode);
	f32 getKerning(u32 glyphCodeLeft, u32 glyphCodeRight);
	const FontMetrics& getMetrics() const { return metrics; }
	void precacheGlyphs(const UnicodeString& glyphCodes);
	void precacheGlyphs(u32* glyphs, u32 glyphCount);
	void precacheLatinAlphabetGlyphs();
	FontTextSize computeTextSize(const u32* const text, u32 size);
	FontTextSize computeTextSize(const UnicodeString& text);
	FontTextSize computeTextSize(const char* text);
	void deleteGlyphs();

	ImageAtlas* usedAtlas = nullptr;

protected:
	FontGlyph* cacheGlyph(u32 glyphCode, bool packAtlasNow = false);

	bool resizeFaceMode = false;
	std::string filename;
	u32 faceSize = 12;
	f32 ascender = 0;
	FontMetrics metrics;
	void* face = 0;
	std::unordered_map<u32, FontGlyph*> glyphs;
	std::unordered_map<u64, f32> kerningPairs;
};

}