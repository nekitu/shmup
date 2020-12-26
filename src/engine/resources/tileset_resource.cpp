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

	auto& tilesJson = json.get("tiles", Json::ValueType::arrayValue);

	for (auto& tileJson : tilesJson)
	{
		TileData tileData;

		tileData.id = tileJson.get("id", 0).asUInt();
		tileData.probability = tileJson.get("probability", 0).asUInt();
		tileData.type = tileJson.get("type", "").asString();

		auto& animJson = tileJson.get("animation", Json::ValueType::arrayValue);

		for (auto& frameJson : animJson)
		{
			TileData::Frame frame;

			frame.duration = (f32)frameJson.get("duration", 0).asUInt() / 1000.0f;
			frame.tileId = frameJson.get("tileid", 0).asUInt();
			tileData.frames.push_back(frame);
		}

		tiles[tileData.id] = tileData;
	}

	imagePath = json.get("image", "").asString();
	imagePath = "tilesets/" + imagePath;
	imagePath = Game::instance->makeFullDataPath(imagePath);

	return loadImage();
}

bool TilesetResource::loadImage()
{
	LOG_DEBUG("Loading tileset image: {0}", imagePath);

	int width = 0;
	int height = 0;
	int comp = 0;

	stbi_uc* data = stbi_load(imagePath.c_str(), &width, &height, &comp, 4);

	if (!data)
		return false;

	image = Game::instance->graphics->atlas->addImage((Rgba32*)data, width, height);

	return image != nullptr;
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
		uvTileWidth = image->uvRect.width / (f32)(image->height / tileHeight);
		uvTileHeight = image->uvRect.height / (f32)columns;
		rc.x = image->uvRect.x + uvMargin + uvTileHeight * (f32)row + uvSpacing * (f32)row;
		rc.y = image->uvRect.y + uvMargin + uvTileWidth * (f32)(columns - col - 1) + uvSpacing * (f32)(columns - col - 1);
		rc.width = uvTileWidth;
		rc.height = uvTileHeight;
	}
	else
	{
		uvTileWidth = image->uvRect.width / (f32)columns;
		uvTileHeight = image->uvRect.height / (f32)(image->height / tileHeight);
		rc.x = image->uvRect.x + uvMargin + uvTileWidth * (f32)col + uvSpacing * (f32)col;
		rc.y = image->uvRect.y + uvMargin + uvTileHeight * (f32)row + uvSpacing * (f32)row;
		rc.width = uvTileWidth;
		rc.height = uvTileHeight;
	}

	return rc;
}

TileData* TilesetResource::findTileData(u32 index)
{
	auto iter = tiles.find(index);

	if (iter == tiles.end()) return nullptr;

	return &iter->second;
}

void TilesetResource::updateAnimations(f32 deltaTime)
{
	for (auto& tile : tiles)
	{
		if (tile.second.frames.size())
		{
			tile.second.animationTimer += deltaTime;

			if (tile.second.animationTimer >= tile.second.frames[tile.second.currentFrame].duration)
			{
				tile.second.animationTimer = 0;
				tile.second.currentFrame++;

				if (tile.second.currentFrame > tile.second.frames.size() - 1)
					tile.second.currentFrame = 0;
			}
		}
	}
}

}
