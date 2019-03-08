#include "font.h"
#include "utils.h"
#include <ft2build.h>
#include <freetype/freetype.h>
#include <freetype/ftglyph.h>
#include <freetype/ftoutln.h>
#include <freetype/fttrigon.h>
#include <assert.h>

#include FT_FREETYPE_H
#include FT_STROKER_H
#include FT_LCD_FILTER_H

namespace engine
{
static FT_Library freetypeLibHandle;
static u32 freetypeUsageCount = 0;

#define PIXEL(x) ((((x)+63) & -64)>>6)
#define PIXEL2(x) ((x) >> 6)

static bool startFreeType()
{
	if (!freetypeUsageCount)
	{
		freetypeUsageCount++;
		return !(FT_Init_FreeType(&freetypeLibHandle));
	}

	freetypeUsageCount++;

	return true;
}

static void stopFreeType()
{
	freetypeUsageCount--;

	if (freetypeUsageCount <= 0)
	{
		FT_Done_FreeType(freetypeLibHandle);
	}
}

Font::Font(const std::string& fontFilename, u32 faceSize, ImageAtlas* atlas)
{
	load(fontFilename, faceSize, atlas);
}

void Font::load(const std::string& fontFilename, u32 facePointSize, ImageAtlas* atlas)
{
	filename = fontFilename;
	faceSize = facePointSize;
	usedAtlas = atlas;

	startFreeType();

	if (face)
	{
		FT_Done_Face((FT_Face)face);
	}

	face = new FT_Face();

	// load the font from the file
	if (FT_New_Face(freetypeLibHandle, fontFilename.c_str(), 0, (FT_Face*)&face))
	{
		FT_Done_Face((FT_Face)face);
		return;
	}

	/* Select charmap */
	int error = FT_Select_Charmap((FT_Face)face, FT_ENCODING_UNICODE);

	if (error)
	{
		FT_Done_Face((FT_Face)face);
		return;
	}

	// freetype measures fonts in 64ths of pixels
	//FT_Set_Char_Size((FT_Face)face, faceSize << 6, faceSize << 6, 96, 96);
	FT_Set_Pixel_Sizes((FT_Face)face, 0, faceSize);

	metrics.ascender = PIXEL2(((FT_Face)face)->size->metrics.ascender);
	metrics.descender = PIXEL2(((FT_Face)face)->size->metrics.descender);
	metrics.height = PIXEL2(((FT_Face)face)->size->metrics.height);
	metrics.underlinePosition = PIXEL2(((FT_Face)face)->underline_position);
	metrics.underlineThickness = PIXEL2(((FT_Face)face)->underline_thickness);

	// if its too big, clamp it
	if (metrics.underlinePosition < -2)
		metrics.underlinePosition = -2;

	metrics.underlinePosition = round(metrics.underlinePosition);
}

void Font::resetFaceSize(u32 fontFaceSize)
{
	faceSize = fontFaceSize;
	load(filename, faceSize, usedAtlas);
	resizeFaceMode = true;

	for (auto glyph : glyphs)
	{
		cacheGlyph(glyph.first);
	}

	resizeFaceMode = false;
}

Font::~Font()
{
	if (face)
	{
		FT_Done_Face((FT_Face)face);
	}

	stopFreeType();
	deleteGlyphs();
}

FontGlyph* Font::getGlyph(u32 glyphCode)
{
	auto iter = glyphs.find(glyphCode);

	// glyph not cached, do it
	if (iter == glyphs.end())
	{
		return cacheGlyph(glyphCode, true);
	}

	return iter->second;
}

f32 Font::getKerning(u32 glyphCodeLeft, u32 glyphCodeRight)
{
	u64 hash = ((u64)glyphCodeLeft) << 32 + glyphCodeRight;
	auto iter = kerningPairs.find(hash);

	if (iter != kerningPairs.end())
	{
		return iter->second;
	}
	else
	{
		FT_Vector kerning;

		FT_Get_Kerning(
			(FT_Face)face,
			FT_Get_Char_Index((FT_Face)face, glyphCodeLeft),
			FT_Get_Char_Index((FT_Face)face, glyphCodeRight),
			FT_KERNING_DEFAULT,
			&kerning);

		f32 kern = kerning.x >> 6;
		kerningPairs[hash] = kern;

		return kern;
	}

	return 0;
}

void Font::precacheGlyphs(const UnicodeString& glyphCodes)
{
	for (auto glyphCode : glyphCodes)
	{
		cacheGlyph(glyphCode);
	}
}

void Font::precacheGlyphs(u32* glyphs, u32 glyphCount)
{
	for (size_t i = 0; i < glyphCount; i++)
	{
		cacheGlyph(glyphs[i]);
	}
}

void Font::precacheLatinAlphabetGlyphs()
{
	std::string alphabet = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()_+-=~`[]{};':\",./<>?®© ";

	for (auto code : alphabet)
	{
		cacheGlyph((u32)code);
	}
}

FontGlyph* Font::cacheGlyph(u32 glyphCode, bool packAtlasNow)
{
	if (!face)
	{
		return nullptr;
	}

	auto iter = glyphs.find(glyphCode);

	if (iter != glyphs.end() && !resizeFaceMode)
		return iter->second;

	FontGlyph* fontGlyph = resizeFaceMode ? iter->second : new FontGlyph();
	FT_GlyphSlot slot = ((FT_Face)face)->glyph;

	// FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
	// FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
	u8 lcd_weights[10];

	lcd_weights[0] = 0x10;
	lcd_weights[1] = 0x40;
	lcd_weights[2] = 0x70;
	lcd_weights[3] = 0x40;
	lcd_weights[4] = 0x10;

	int flags = FT_LOAD_FORCE_AUTOHINT;
	FT_Library_SetLcdFilter(freetypeLibHandle, FT_LCD_FILTER_LIGHT);
	flags |= FT_LOAD_TARGET_LCD;
	FT_Library_SetLcdFilterWeights(freetypeLibHandle, lcd_weights);

	if (FT_Load_Glyph(
		(FT_Face)face,
		FT_Get_Char_Index((FT_Face)face, glyphCode),
		flags))
	{
		return nullptr;
	}

	if (FT_Render_Glyph(slot, FT_RENDER_MODE_NORMAL))
	{
		return nullptr;
	}

	FT_Bitmap bitmap = slot->bitmap;
	u32 width = bitmap.width;
	u32 height = bitmap.rows;
	Rgba32* rgbaBuffer = new Rgba32[width * height];

	for (int j = 0; j < height; ++j)
	{
		for (int i = 0; i < width; ++i)
		{
			u8 lum = bitmap.buffer[i + width * j];
			u32 index = (i + j * width);
			rgbaBuffer[index] = ~0;
			*((u8*)&(rgbaBuffer[index]) + 3) = lum;
		}
	}

	fontGlyph->pixelWidth = width;
	fontGlyph->pixelHeight = height;

	if (resizeFaceMode)
		delete[] fontGlyph->rgbaBuffer;

	fontGlyph->rgbaBuffer = rgbaBuffer;
	fontGlyph->code = glyphCode;
	fontGlyph->advanceX = ((FT_Face)face)->glyph->advance.x >> 6;
	fontGlyph->advanceY = ((FT_Face)face)->glyph->advance.y >> 6;
	fontGlyph->bearingX = ((FT_Face)face)->glyph->metrics.horiBearingX >> 6;
	fontGlyph->bearingY = ((FT_Face)face)->glyph->metrics.horiBearingY >> 6;
	fontGlyph->bitmapLeft = ((FT_Face)face)->glyph->bitmap_left;
	fontGlyph->bitmapTop = ((FT_Face)face)->glyph->bitmap_top;

	// if we do not currently resizing the font glyphs, then create and insert the image into the atlas
	if (!resizeFaceMode)
	{
		glyphs.insert(std::make_pair(glyphCode, fontGlyph));
		assert(rgbaBuffer);

		auto image = usedAtlas->addImage(
			(Rgba32*)rgbaBuffer,
			fontGlyph->pixelWidth,
			fontGlyph->pixelHeight);

		fontGlyph->image = image;

		if (packAtlasNow)
		{
			usedAtlas->packWithLastUsedParams();
		}
	}
	else
	{
		// if we are in resize mode, then just update the image buffer for the glyph and its size
		delete[] fontGlyph->image->imageData;
		auto imgSize = fontGlyph->pixelWidth * fontGlyph->pixelHeight * sizeof(Rgba32);
		fontGlyph->image->imageData = new Rgba32[imgSize];
		fontGlyph->image->width = width;
		fontGlyph->image->height = height;
		memcpy(fontGlyph->image->imageData, rgbaBuffer, imgSize);
	}

	return fontGlyph;
}

AtlasImage* Font::getGlyphImage(u32 glyphCode)
{
	auto iter = glyphs.find(glyphCode);

	if (iter == glyphs.end())
		return nullptr;

	return iter->second->image;
}

FontTextSize Font::computeTextSize(const UnicodeString& text)
{
	return computeTextSize(text.data(), text.size());
}

FontTextSize Font::computeTextSize(const char* text)
{
	static UnicodeString str;

	utf8ToUtf32(text, str);

	return computeTextSize(str.data(), str.size());
}

FontTextSize Font::computeTextSize(const u32* const text, u32 size)
{
	FontTextSize fsize;
	u32 lastChr = 0;
	f32 lineCount = 0;
	f32 crtLineWidth = 0;

	for (size_t i = 0; i < size; i++)
	{
		auto chr = text[i];
		auto glyph = getGlyph(chr);
		auto glyphImage = glyph->image;

		if (chr == '\n')
		{
			if (fsize.width < crtLineWidth)
			{
				fsize.width = crtLineWidth;
			}

			crtLineWidth = 0;
			lineCount++;
			continue;
		}

		if (glyphImage && glyph)
		{
			f32 top = glyph->bearingY;
			f32 bottom = -(glyph->pixelHeight - glyph->bearingY);
			auto kern = getKerning(lastChr, chr);
			crtLineWidth += glyph->advanceX + kern;
			lastChr = chr;

			if (fsize.maxGlyphHeight < fabs(top - bottom))
			{
				fsize.maxGlyphHeight = fabs(top - bottom);
			}

			if (fsize.maxBearingY < glyph->bearingY)
			{
				fsize.maxBearingY = glyph->bearingY;
			}
		}
	}

	if (fsize.width < crtLineWidth)
	{
		fsize.width = crtLineWidth;
	}

	lineCount++;
	fsize.height = lineCount * metrics.height;

	return fsize;
}

void Font::deleteGlyphs()
{
	for (auto glyph : glyphs)
	{
		usedAtlas->deleteImage(glyph.second->image);
		delete[] glyph.second->rgbaBuffer;
		delete glyph.second;
	}

	kerningPairs.clear();
	glyphs.clear();
}

}