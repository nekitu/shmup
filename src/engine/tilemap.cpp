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
				gfx->color = root->color.getRgba();
				gfx->colorMode = (int)root->colorMode;

				Rect rc;

				u32 col = tileIndex % (u32)chunk.size.x;
				u32 row = tileIndex / (u32)chunk.size.x;

				auto offsX = tilemapResource->tileSize.x * col;
				auto offsY = tilemapResource->tileSize.y * row;

				rc.x = boundingBox.x + chunk.size.x * tilemapResource->tileSize.x + offsX - layer.start.x * tilemapResource->tileSize.x;
				rc.y = boundingBox.y + chunk.size.y * tilemapResource->tileSize.y + offsY - layer.start.y * tilemapResource->tileSize.y;
				rc.width = tilemapResource->tileSize.x;
				rc.height = tilemapResource->tileSize.y;
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
