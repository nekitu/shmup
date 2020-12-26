#pragma once
#include "types.h"
#include "resource.h"
#include "vec2.h"
#include "color.h"

namespace engine
{
struct TilemapChunk
{
	std::vector<u32> tiles;
	Vec2 size;
	Vec2 position;
};

struct TilemapObject
{
	enum class Type
	{
		Polygon,
		Polyline,
		Rect,
		Point,
		Ellipse,
		Tile,
		Text,

		Count
	};

	Type type = Type::Point;
	Vec2 size;
	Vec2 position;
	u32 gid = 0;
	u32 id = 0;
	std::string name;
	f32 rotation = 0;
	std::string templatePath;
	std::string text;
	bool wrap = true;
	bool visible = true;
	std::string typeString;
	std::vector<Vec2> points;
	std::unordered_map<std::string, std::string> properties;

	void load(Json::Value& json);
};

struct TilemapLayer
{
	enum class Type
	{
		Tiles,
		Objects,
		Group,
		Image,

		Count
	};

	struct TilemapResource* tilemapResource = nullptr;
	Type type = Type::Tiles;
	std::vector<TilemapChunk> chunks;
	std::vector<TilemapObject> objects;
	std::vector<TilemapLayer> layers;
	std::unordered_map<std::string, std::string> properties;
	std::string imagePath;
	std::string name;
	u32 id = 0;
	Vec2 size;
	Vec2 start;
	Vec2 offset;
	Vec2 position;
	f32 opacity = 1.0f;
	bool visible = true;
	struct AtlasImage* image = nullptr;
	Color tintColor;

	// custom properties
	f32 cameraParallaxScale = 1.0f;
	bool cameraParallax = true; // used by player layer to not parallax its position by camera side moves
	bool cameraScroll = true; // if true, the layer is scrolled, if false, the layer is not scrolled by camera position, used for enemies/bosses to stay in place and player to not be affected by camera scroll progression in the map
	u32 repeatCount = 0; // how many times this layer is repeated, 0 no repeat, ~0 infinite, or a number of times. This is valid for image layers only

	void load(Json::Value& json);
	void unload();
	void loadImage();
};

struct TilesetInfo
{
	struct TilesetResource* tileset = nullptr;
	u32 firstGid = 1;
};

struct TilemapResource : Resource
{
	Vec2 tileSize;
	Vec2 size;
	std::vector<TilemapLayer> layers;
	std::vector<TilesetInfo> tilesets;
	Color backgroundColor;
	bool infinite = false;

	bool load(Json::Value& json) override;
	void unload() override;
	struct TilesetInfo getTilesetInfoByTileId(u32 tileId);
};

}
