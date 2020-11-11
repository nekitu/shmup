#include "tilemap_resource.h"
#include "game.h"
#include "resource_loader.h"
#include <stb_image.h>
#include "image_atlas.h"
#include "graphics.h"

namespace engine
{
void TilemapObject::load(Json::Value& json)
{
	gid = objJson.get("gid", obj.gid).asInt();
	size.x = objJson.get("width", obj.size.x).asFloat();
	size.y = objJson.get("height", obj.size.y).asFloat();
	id = objJson.get("id", obj.id).asInt();
	name = objJson.get("name", obj.id).asInt();
}

void TilemapLayer::load(Json::Value& json)
{
	id = json.get("id", 0).asInt();
	name = json.get("name", "").asString();
	size.x = json.get("width", 0).asInt();
	size.y = json.get("height", 0).asInt();
	start.x = json.get("startx", 0).asInt();
	start.y = json.get("starty", 0).asInt();
	visible = json.get("visible", 0).asBool();
	position.x = json.get("x", 0).asInt();
	position.y = json.get("y", 0).asInt();
	opacity = json.get("opacity", 0).asFloat();

	auto typeName = json.get("type", 0).asString();

	if (typeName == "tilelayer") type = TilemapLayer::Type::Tiles;

	if (typeName == "group")
	{
		type = TilemapLayer::Type::Group;

		auto& layersJson = json.get("layers", Json::ValueType::arrayValue);

		for (auto& layerJson : layersJson)
		{
			TilemapLayer layer;

			layer.load(layerJson);
			layers.push_back(layer);
		}
	}

	if (typeName == "imagelayer")
	{
		type = TilemapLayer::Type::Image;
		imagePath = json.get("image", 0).asString();
		imagePath = getParentPath(tilemapResource->fileName) + imagePath;
		replaceAll(imagePath, "\\/", "/");

		int imgWidth = 0;
		int imgHeight = 0;
		int comp = 0;

		stbi_uc* imgData = stbi_load(imagePath.c_str(), &imgWidth, &imgHeight, &comp, 4);
		LOG_INFO("Loaded layer image: {0} {1}x{2}", imagePath, imgWidth, imgHeight);

		if (imgData)
		{
			image = Game::instance->graphics->atlas->addImage((Rgba32*)imgData, imgWidth, imgHeight);
		}
	}

	if (typeName == "objectgroup")
	{
		type = TilemapLayer::Type::Objects;
		auto& objectsJson = json.get("objects", Json::ValueType::arrayValue);

		for (auto& objJson : objectsJson)
		{
			TilemapObject obj;

			obj.tilemapResource = this;
			obj.load(objJson);


		}
	}

	auto& chunksJson = json.get("chunks", Json::ValueType::arrayValue);
	auto& dataJson = json.get("data", Json::ValueType::arrayValue);

	bool infinite = dataJson.isNull();

	if (infinite)
	{
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

			chunks.push_back(chunk);
		}
	}
	else
	{
		TilemapChunk chunk;

		chunk.width = width;
		chunk.height = height;
		chunk.x = x;
		chunk.y = y;

		for (auto& tileIndex : dataJson)
		{
			chunk.tiles.push_back(tileIndex.asInt());
		}

		chunks.push_back(chunk);
	}
}

bool TilemapResource::load(Json::Value& json)
{
	// loading Tiled json map
	tileSize.x = json.get("tilewidth", 0).asInt();
	tileSize.y = json.get("tileheight", 0).asInt();
	infinite = json.get("infinite", 0).asBool();

	auto& layersArray = json.get("layers", Json::ValueType::arrayValue);

	for (auto& layerJson : layersArray)
	{
		TilemapLayer layer;

		layer.tilemapResource = this;
		layer.load(layerJson);
		layers.push_back(layer);
	}

	auto& tilesetsArray = json.get("tilesets", Json::ValueType::arrayValue);

	for (auto& tilesetJson : tilesetsArray)
	{
		TilesetInfo info;
		auto filename = tilesetJson.get("source", "").asString();

		replaceAll(filename, "..", "");
		replaceAll(filename, "\/", "/");
		replaceAll(filename, ".json", "");

		info.tileset = Game::instance->resourceLoader->loadTileset(filename);
		info.firstGid = tilesetJson.get("firstgid", 0).asInt();
		tilesets.push_back(info);
	}

	return true;
}

void TilemapResource::unload()
{
	layers.clear();
}

TilesetInfo TilemapResource::getTilesetInfoByTileId(u32 tileId)
{
	for (int i = 0; i < tilesets.size(); i++)
	{
		if (i + 1 <= tilesets.size() - 1)
		{
			if (tileId >= tilesets[i].firstGid
				&& tileId < tilesets[i + 1].firstGid)
			{
				return tilesets[i];
			}
		}
		else
		{
			return tilesets[i];
		}
	}

	return TilesetInfo();
}

}
