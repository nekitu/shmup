#include "tileset_resource.h"
#include "game.h"
#include "graphics.h"
#include <stb_image.h>
#include "image_atlas.h"

namespace engine
{
bool TilesetResource::load(Json::Value& json)
{
	columns = json.get("columns", 0).asInt();
	margin = json.get("margin", 0).asInt();
	spacing = json.get("spacing", 0).asInt();
	tileCount = json.get("tilecount", 0).asInt();
	tileWidth = json.get("tilewidth", 0).asInt();
	tileHeight = json.get("tileheight", 0).asInt();
	imageFilename = json.get("image", "").asString();

	imageFilename = Game::instance->makeFullDataPath(imageFilename);

	int width = 0;
	int height = 0;
	int comp = 0;

	stbi_uc* data = stbi_load(imageFilename.c_str(), &width, &height, &comp, 4);

	if (!data)
		return false;

	image = Game::instance->graphics->atlas->addImage((Rgba32*)data, width, height);

	uvMargin = (f32)margin / (f32)width;
	uvSpacing = (f32)spacing / (f32)width;
	uvTileWidth = (f32)tileWidth / (f32)width;
	uvTileHeight = (f32)tileHeight / (f32)height;

	return true;
}

void TilesetResource::unload()
{
	image = nullptr;
}

Rect TilesetResource::getTileRectTexCoord(u32 index)
{
	Rect rc;

	u32 col = index % columns;
	u32 row = index / columns;

	if (image->rotated)
	{
		rc.x = image->uvRect.x + uvMargin + uvTileWidth * (f32)row + uvSpacing * (f32)row;
		rc.y = image->uvRect.y + uvMargin + uvTileHeight * (f32)(columns - col - 1) + uvSpacing * (f32)(columns - col - 1);
		rc.width = uvTileWidth;
		rc.height = uvTileHeight;
	}
	else
	{
		rc.x = image->uvRect.x + uvMargin + uvTileWidth * (f32)col + uvSpacing * (f32)col;
		rc.y = image->uvRect.y + uvMargin + uvTileHeight * (f32)row + uvSpacing * (f32)row;
		rc.width = uvTileWidth;
		rc.height = uvTileHeight;
	}

	return rc;
}

}
