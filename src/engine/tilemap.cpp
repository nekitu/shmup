#include "tilemap.h"
#include "game.h"
#include "resources/tilemap_resource.h"
#include "resources/tileset_resource.h"
#include "graphics.h"
#include "image_atlas.h"
#include "resource_loader.h"
#include "sprite.h"

namespace engine
{
void Tilemap::load(struct ResourceLoader* loader, const Json::Value& json)
{
	Unit::load(loader, json);
	tilemapResource = loader->loadTilemap(json["tilemap"].asString());
}

void Tilemap::update(struct Game* game)
{
	Unit::update(game);
	root->position.y += game->deltaTime * 10.0f;
}

void Tilemap::render(struct Graphics* gfx)
{
	Unit::render(gfx);

	for (auto& layer : tilemapResource->layers)
	{
		for (auto& chunk : layer.chunks)
		{
			int tileIndex = 0;

			for (auto& tile : chunk.tiles)
			{
				auto tilesetInfo = tilemapResource->getTilesetInfoByTileId(tile);

				gfx->atlasTextureIndex = tilesetInfo.tileset->image->atlasTexture->textureIndex;
				gfx->color = root->color;
				gfx->colorMode = root->colorMode;

				Rect rc;

				u32 col = tileIndex % (u32)chunk.width;
				u32 row = tileIndex / (u32)chunk.width;

				auto offsX = tilemapResource->tileWidth * col;
				auto offsY = tilemapResource->tileHeight * row;

				rc.x = boundingBox.x + chunk.x * tilemapResource->tileWidth + offsX - layer.startX * tilemapResource->tileWidth;
				rc.y = boundingBox.y + chunk.y * tilemapResource->tileHeight + offsY - layer.startY * tilemapResource->tileHeight;
				rc.width = tilemapResource->tileWidth;
				rc.height = tilemapResource->tileHeight;
				auto uv = tilesetInfo.tileset->getTileRectTexCoord(tile - tilesetInfo.firstGid);

				if (tilesetInfo.tileset->image->rotated)
					gfx->drawQuadWithTexCoordRotated90(rc, uv);
				else
					gfx->drawQuad(rc, uv);

				++tileIndex;
			}
		}
	}

}

}
