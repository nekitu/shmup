#pragma once
#include "types.h"
#include "unit.h"

namespace engine
{
struct TilemapLayer : Unit
{
	struct TilemapLayerData* tilemapLayerData = nullptr;

	virtual void update(struct Game* game) override;
	virtual void render(struct Graphics* gfx) override;
};
}
