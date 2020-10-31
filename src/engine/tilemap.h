#pragma once
#include "types.h"
#include "unit.h"

namespace engine
{
struct Tilemap : Unit
{
	struct TilemapResource* tilemapResource = nullptr;

	void load(struct ResourceLoader* loader, const Json::Value& json) override;
	virtual void update(struct Game* game);
	virtual void render(struct Graphics* gfx);
};
}
