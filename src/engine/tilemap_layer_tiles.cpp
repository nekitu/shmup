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
	Unit::update(game);
	speed = 25;
	root->position.y += game->deltaTime * speed;
}

void TilemapLayerTiles::render(struct Graphics* gfx)
{
	Unit::render(gfx);

	auto& layer = *tilemapLayer;
	for (auto& chunk : layer.chunks)
	{
		int tileIndex = 0;

		for (auto& tile : chunk.tiles)
		{
			auto tilesetInfo = layer.tilemapResource->getTilesetInfoByTileId(tile);

			gfx->atlasTextureIndex = tilesetInfo.tileset->image->atlasTexture->textureIndex;
			gfx->color = root->color.getRgba();
			gfx->colorMode = (int)root->colorMode;

			Rect rc;

			u32 col = tileIndex % (u32)chunk.size.x;
			u32 row = tileIndex / (u32)chunk.size.x;

			auto offsX = layer.tilemapResource->tileSize.x * col;
			auto offsY = layer.tilemapResource->tileSize.y * row;

			rc.x = root->position.x + chunk.position.x * layer.tilemapResource->tileSize.x + offsX - layer.start.x * layer.tilemapResource->tileSize.x;
			rc.y = root->position.y + chunk.position.y * layer.tilemapResource->tileSize.y + offsY - layer.start.y * layer.tilemapResource->tileSize.y;
			rc.width = layer.tilemapResource->tileSize.x;
			rc.height = layer.tilemapResource->tileSize.y;
			auto uv = tilesetInfo.tileset->getTileRectTexCoord(tile - tilesetInfo.firstGid);

			rc.x = roundf(rc.x);
			rc.y = roundf(rc.y);
			rc.width = roundf(rc.width);
			rc.height = roundf(rc.height);

			if (tilesetInfo.tileset->image->rotated)
				gfx->drawQuadWithTexCoordRotated90(rc, uv);
			else
				gfx->drawQuad(rc, uv);

			++tileIndex;
		}
	}
}

}
