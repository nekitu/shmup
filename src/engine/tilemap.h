#pragma once
#include "types.h"
#include "unit.h"

namespace engine
{
struct Tilemap : Unit
{
	struct TilemapResource* tilemapResource = nullptr;

	virtual void update(struct Game* game);
	virtual void render(struct Graphics* gfx);
};
}
