#pragma once
#include "types.h"
#include "color.h"
#include "rect.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <binpack/Rect.h>
#include <binpack/GuillotineBinPack.h>
#include <binpack/ShelfBinPack.h>
#include <binpack/SkylineBinPack.h>
#include <binpack/MaxRectsBinPack.h>

namespace engine
{
class ImageAtlas;
typedef u32 AtlasImageId;

struct PaletteInfo
{
	bool isPaletted = false;
	u32 bitsPerPixel = 0;
	u32 paletteSlot = 0;
	u32 transparentColorIndex = 0;
	std::vector<u32> colors;
};

enum class AtlasPackPolicy
{
	Guillotine,
	MaxRects,
	ShelfBin,
	Skyline
};

struct AtlasTexture
{
	struct TextureArray* textureArray = nullptr;
	u32 textureIndex = 0;
	Rgba32* textureImage = nullptr;
	bool dirty = false;
	AtlasPackPolicy packPolicy = AtlasPackPolicy::Skyline;
	GuillotineBinPack guillotineBinPack;
	MaxRectsBinPack maxRectsBinPack;
	ShelfBinPack shelfBinPack;
	SkylineBinPack skylineBinPack;
};

struct AtlasImage
{
	std::string path;
	AtlasImageId id = 0;
	ImageAtlas* atlas = nullptr;
	AtlasTexture* atlasTexture = nullptr;
	bool rotated = false; // if true, its rotated clockwise 90deg in the atlas
	Rect uvRect;
	Rect rect;
	u32 width = 0, height = 0;
	bool bleedOut = false;
	Rgba32* cachedImageData = nullptr; // valid only when atlas not prebaked

	Rgba32 getPixel(u32 x, u32 y);
	u8* getPixelAddr(u32 x, u32 y);
	void setPixel(u32 x, u32 y, Rgba32 color);
	u32 getPixelOffsetInsideAtlas(u32 x, u32 y);
};

struct ImageAtlas
{
	ImageAtlas() {}
	ImageAtlas(u32 width, u32 height);
	~ImageAtlas();

	void create(u32 width, u32 height);
	AtlasImage* getImageById(AtlasImageId id) const;
	AtlasImage* addImage(const std::string& path, const Rgba32* imageData, u32 width, u32 height, bool addBleedOut = false);
	void updateImageData(AtlasImageId imgId, const Rgba32* imageData, u32 width, u32 height);
	void deleteImage(AtlasImage* image);
	AtlasImage* addWhiteImage(u32 width = 8);
	bool pack(
		u32 spacing = 5,
		const Color& bgColor = Color::black,
		AtlasPackPolicy packing = AtlasPackPolicy::Skyline);
	void repackImages();
	void packWithLastUsedParams() { pack(lastUsedSpacing, lastUsedBgColor, lastUsedPolicy); }
	void clearImages();
	struct AtlasImage* loadImageToAtlas(const std::string& path, PaletteInfo* paletteInfo);
	bool save(const std::string& path);
	bool load(const std::string& path);

	AtlasImage* whiteImage = nullptr;
	TextureArray* textureArray = nullptr;

protected:
	struct PackImageData
	{
		AtlasImageId id = 0;
		std::string path;
		ImageAtlas* atlas = nullptr;
		AtlasTexture* atlasTexture = nullptr;
		Rgba32* imageData = nullptr;
		u32 width = 0;
		u32 height = 0;
		Rect packedRect;
		bool bleedOut = false;
	};

	void deletePackerImages();
	AtlasImage* addImageInternal(const std::string& path, AtlasImageId imgId, const Rgba32* imageData, u32 imageWidth, u32 imageHeight, bool addBleedOut);

	bool prebaked = false;
	u32 id = 0;
	u32 lastImageId = 1;
	u32 width = 0;
	u32 height = 0;
	u32 lastUsedSpacing = 0;
	Color lastUsedBgColor = Color::black;
	AtlasPackPolicy lastUsedPolicy = AtlasPackPolicy::Skyline;
	bool useWasteMap = true;
	std::vector<AtlasTexture*> atlasTextures;
	std::unordered_map<AtlasImageId, AtlasImage*> images;
	std::unordered_map<std::string, AtlasImage*> pathImageLUT;
	std::unordered_map<std::string, PaletteInfo> tgaPalettes;
	std::vector<PackImageData> pendingPackImages;
};

}
