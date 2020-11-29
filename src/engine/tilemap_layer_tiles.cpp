#include "tilemap_layer_tiles.h"
#include "game.h"
#include "resources/tilemap_resource.h"
#include "resources/tileset_resource.h"
#include "graphics.h"
#include "image_atlas.h"
#include "resource_loader.h"
#include "sprite.h"
#include "utils.h"

namespace engine
{
void TilemapLayerTiles::update(struct Game* game)
{
	speed = 22;
	root->position.y += game->deltaTime * speed;
	root->position.x = game->cameraParallaxOffset;
	Unit::update(game);
}

void TilemapLayerTiles::render(struct Graphics* gfx)
{
	Unit::render(gfx);

	auto& layer = *tilemapLayer;
	for (auto& chunk : layer.chunks)
	{
		int tileIndex = 0;

		for (auto tile : chunk.tiles)
		{
			auto tilesetInfo = layer.tilemapResource->getTilesetInfoByTileId(tile);
			auto tileData = tilesetInfo.tileset->findTileData(tile - tilesetInfo.firstGid);

			if (tileData)
			{
				if (tileData->frames.size())
				{
					tile = tilesetInfo.firstGid + tileData->frames[tileData->currentFrame].tileId;
				}
			}

			gfx->atlasTextureIndex = tilesetInfo.tileset->image->atlasTexture->textureIndex;
			gfx->color = root->color.getRgba();
			gfx->colorMode = (int)root->colorMode;

			Rect rc;

			f32 col = tileIndex % (u32)chunk.size.x;
			f32 row = tileIndex / (u32)chunk.size.x;

			auto& tileSize = layer.tilemapResource->tileSize;

			auto offsX = tileSize.x * col;
			auto offsY = tileSize.y * row;

			rc.x = root->position.x + layer.offset.x * tileSize.x + chunk.position.x * tileSize.x + offsX;// +layer.start.x * tileSize.x;
			rc.y = root->position.y + layer.offset.y * tileSize.y + chunk.position.y * tileSize.y + offsY;// +layer.start.y * tileSize.y;
			rc.width = layer.tilemapResource->tileSize.x;
			rc.height = layer.tilemapResource->tileSize.y;
			auto uv = tilesetInfo.tileset->getTileRectTexCoord(tile - tilesetInfo.firstGid);

			rc.x = round(rc.x);
			rc.y = round(rc.y);

			if (tilesetInfo.tileset->image->rotated)
				gfx->drawQuadWithTexCoordRotated90(rc, uv);
			else
				gfx->drawQuad(rc, uv);

			++tileIndex;
		}
	}
}

}
