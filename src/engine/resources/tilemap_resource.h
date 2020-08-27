#include "types.h"
#include "resource.h"

namespace engine
{
struct TilemapChunk
{
	std::vector<u32> tiles;
	u32 width = 0, height = 0;
	u32 x = 0, y = 0;
};

struct TilemapLayer
{
	std::vector<TilemapChunk> chunks;
	std::string name;
	u32 id = 0;
	u32 width = 0, height = 0;
	u32 startX = 0, startY = 0;
	u32 x = 0, y = 0;
	f32 opacity = 1.0f;
};

struct TilesetInfo
{
	struct TilesetResource* tileset = nullptr;
	u32 firstGid = 1;
};

struct TilemapResource : Resource
{
	u32 tileWidth = 0, tileHeight = 0;
	std::vector<TilemapLayer> layers;
	std::vector<TilesetInfo> tilesets;

	bool load(Json::Value& json) override;
	void unload() override;
	struct TilesetResource* getTilesetByTileId(u32 tileId);
};

}
