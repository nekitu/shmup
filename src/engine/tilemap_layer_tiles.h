#pragma once
#include "types.h"
#include "unit.h"

namespace engine
{
struct TilemapLayerTiles : Unit
{
	struct TilemapLayer* tilemapLayer = nullptr;

	virtual void update(struct Game* game) override;
	virtual void render(struct Graphics* gfx) override;
	void renderTiles(struct Graphics* gfx, const Vec2& location);
};
}
