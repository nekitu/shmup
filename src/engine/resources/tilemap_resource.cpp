#include "tilemap_resource.h"
#include "game.h"
#include "resource_loader.h"

namespace engine
{
bool TilemapResource::load(Json::Value& json)
{
	tileWidth = json.get("tilewidth", 0).asInt();
	tileHeight = json.get("tileheight", 0).asInt();
	auto layersArray = json.get("layers", Json::ValueType::arrayValue);

	for (auto& layerJson : layersArray)
	{
		TilemapLayer layer;

		layer.id = layerJson.get("id", 0).asInt();
		layer.name = layerJson.get("name", "").asString();
		layer.width = layerJson.get("width", 0).asInt();
		layer.height = layerJson.get("height", 0).asInt();
		layer.startX = layerJson.get("startx", 0).asInt();
		layer.startY = layerJson.get("starty", 0).asInt();
		layer.x = layerJson.get("x", 0).asInt();
		layer.y = layerJson.get("y", 0).asInt();
		layer.opacity = layerJson.get("opacity", 0).asFloat();

		auto& chunksJson = layerJson.get("chunks", Json::ValueType::arrayValue);

		for (auto& chunkJson : chunksJson)
		{
			TilemapChunk chunk;

			chunk.width = chunkJson.get("width", 0).asInt();
			chunk.height = chunkJson.get("height", 0).asInt();
			chunk.x = chunkJson.get("x", 0).asInt();
			chunk.y = chunkJson.get("y", 0).asInt();

			auto& tiles = chunkJson.get("data", Json::ValueType::arrayValue);

			for (auto& tileIndex : tiles)
			{
				chunk.tiles.push_back(tileIndex.asInt());
			}

			layer.chunks.push_back(chunk);
		}

		layers.push_back(layer);
	}

	auto& tilesetsArray = json.get("tilesets", Json::ValueType::arrayValue);

	for (auto& tilesetJson : tilesetsArray)
	{
		TilesetInfo info;

		info.tileset = Game::instance->resourceLoader->loadTileset(tilesetJson.get("source", "").asString());
		info.firstGid = tilesetJson.get("firstgid", 0).asInt();
		tilesets.push_back(info);
	}

	return true;
}

void TilemapResource::unload()
{
	layers.clear();
}

TilesetResource* TilemapResource::getTilesetByTileId(u32 tileId)
{
	for (int i = 0; i < tilesets.size(); i++)
	{
		if (i + 1 <= tilesets.size() - 1)
		{
			if (tileId >= tilesets[i].firstGid
				&& tileId < tilesets[i + 1].firstGid)
			{
				return tilesets[i].tileset;
			}
		}
		else
		{
			return tilesets[i].tileset;
		}
	}

	return nullptr;
}

}
