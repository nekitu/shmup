#include "tilemap.h"
#include "game.h"
#include "resources/tilemap_resource.h"
#include "resources/tileset_resource.h"
#include "graphics.h"
#include "image_atlas.h"

namespace engine
{
void Tilemap::update(struct Game* game)
{
	Unit::update(game);
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
				auto tileset = tilemapResource->getTilesetByTileId(tile);

				gfx->atlasTextureIndex = tileset->image->atlasTexture->textureIndex;
				gfx->color = 0;
				gfx->colorMode = (u32)ColorMode::Add;

				Rect rc;

				u32 col = tileIndex % chunk.width;
				u32 row = tileIndex / chunk.width;

				auto offsX = tilemapResource->tileWidth * col;
				auto offsY = tilemapResource->tileHeight * row;

				rc.x = boundingBox.x + chunk.x + offsX;
				rc.y = boundingBox.y + chunk.y + offsY;
				rc.width = tilemapResource->tileWidth;
				rc.height = tilemapResource->tileHeight;

				gfx->drawQuad(rc, tileset->getTileRectTexCoord(tile));
				++tileIndex;
			}
		}
	}

}

}
