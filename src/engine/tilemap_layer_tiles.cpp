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
}

void TilemapLayerTiles::render(struct Graphics* gfx)
{
	Unit::render(gfx);

	if (!tilemapLayer->visible)
		return;

	switch (tilemapLayer->type)
	{
	case TilemapLayer::Type::Tiles:
		renderTiles(gfx);
		break;
	case TilemapLayer::Type::Image:
		renderImage(gfx);
		break;
	}
}

void TilemapLayerTiles::renderTiles(struct Graphics* gfx)
{
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

			if (!tile)
			{
				++tileIndex;
				continue;
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

			rc.x = boundingBox.x + layer.offset.x + chunk.position.x * tileSize.x + offsX;
			rc.y = boundingBox.y + layer.offset.y + chunk.position.y * tileSize.y + offsY;
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

void TilemapLayerTiles::renderImage(Graphics* gfx)
{
	if (!tilemapLayer->repeatCount)
	{
		Rect rc = boundingBox;

		rc.width = tilemapLayer->image->width;
		rc.height = tilemapLayer->image->height;

		if (tilemapLayer->image->rotated)
			gfx->drawQuadWithTexCoordRotated90(rc, tilemapLayer->image->uvRect);
		else
			gfx->drawQuad(rc, tilemapLayer->image->uvRect);

		return;
	}

	for (u32 i = 0; i < tilemapLayer->repeatCount; i++)
	{
		Vec2 pos;

		pos.x = boundingBox.x + tilemapLayer->offset.x;
		pos.y = boundingBox.y + tilemapLayer->offset.y;
		pos.y -= i * tilemapLayer->image->height;

		if (pos.y + tilemapLayer->image->height < 0)
			break;

		if (pos.y < gfx->videoHeight)
		{
			Rect rc;

			rc.x = pos.x;
			rc.y = pos.y;
			rc.width = tilemapLayer->image->width;
			rc.height = tilemapLayer->image->height;

			if (tilemapLayer->image->rotated)
				gfx->drawQuadWithTexCoordRotated90(rc, tilemapLayer->image->uvRect);
			else
				gfx->drawQuad(rc, tilemapLayer->image->uvRect);
		}
	}
}

}
