#pragma once
#include "types.h"
#include "resource.h"
#include "rect.h"
#include <unordered_map>
#include <vector>

namespace engine
{
struct TileData
{
	struct Frame
	{
		u32 tileId = 0;
		f32 duration = 100;
	};

	u32 id;
	std::string type;
	f32 probability = 0;
	std::vector<Frame> frames;
};

struct TilesetResource : Resource
{
	struct AtlasImage* image = nullptr;

	std::string imagePath;
	u32 columns = 0;
	u32 margin = 0;
	u32 spacing = 0;
	u32 tileCount = 0;
	u32 tileWidth = 0;
	u32 tileHeight = 0;
	f32 uvMargin = 0;
	f32 uvSpacing = 0;
	f32 uvTileWidth = 0;
	f32 uvTileHeight = 0;
	u32 firstGid = 0;

	std::unordered_map<u32/*id*/, TileData> tiles;

	bool load(Json::Value& json) override;
	void unload() override;
	Rect getTileRectTexCoord(u32 index);
};

}
