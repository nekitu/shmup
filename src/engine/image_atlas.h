#pragma once
#include "types.h"
#include "color.h"
#include "rect.h"
#include <unordered_map>
#include <string>
#include <vector>
#include <libs/binpack/Rect.h>
#include <libs/binpack/GuillotineBinPack.h>
#include <libs/binpack/ShelfBinPack.h>
#include <libs/binpack/SkylineBinPack.h>
#include <libs/binpack/MaxRectsBinPack.h>

namespace engine
{
class ImageAtlas;
typedef u32 AtlasImageId;

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
	AtlasImageId id = 0;
	ImageAtlas* atlas = nullptr;
	AtlasTexture* atlasTexture = nullptr;
	bool rotated = false;
	Rect uvRect;
	Rect rect;
	Rgba32* imageData = nullptr;
	u32 width = 0, height = 0;
	bool bleedOut = false;
};

struct ImageAtlas
{
	ImageAtlas() {}
	ImageAtlas(u32 width, u32 height);
	~ImageAtlas();

	void create(u32 width, u32 height);
	AtlasImage* getImageById(AtlasImageId id) const;
	AtlasImage* addImage(const Rgba32* imageData, u32 width, u32 height, bool addBleedOut = false);
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

	AtlasImage* whiteImage = nullptr;
	TextureArray* textureArray = nullptr;

protected:
	struct PackImageData
	{
		AtlasImageId id = 0;
		ImageAtlas* atlas = nullptr;
		AtlasTexture* atlasTexture = nullptr;
		Rgba32* imageData = nullptr;
		u32 width = 0;
		u32 height = 0;
		Rect packedRect;
		bool bleedOut = false;
	};

	void deletePackerImages();
	AtlasImage* addImageInternal(AtlasImageId imgId, const Rgba32* imageData, u32 imageWidth, u32 imageHeight, bool addBleedOut);

	u32 id = 0;
	u32 lastImageId = 1;
	u32 width;
	u32 height;
	u32 lastUsedSpacing = 0;
	Color lastUsedBgColor = Color::black;
	AtlasPackPolicy lastUsedPolicy = AtlasPackPolicy::Skyline;
	bool useWasteMap = true;
	std::vector<AtlasTexture*> atlasTextures;
	std::unordered_map<AtlasImageId, AtlasImage*> images;
	std::vector<PackImageData> pendingPackImages;
};

}