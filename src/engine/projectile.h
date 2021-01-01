#pragma once
#include "unit.h"

namespace engine
{
struct Projectile : Unit
{
	struct Weapon* weaponResource = nullptr;
	Vec2 velocity;
	f32 minSpeed = 1, maxSpeed = 100000;
	f32 acceleration = 0;
	bool used = false;
	Vec2 spawnPosition; // used for CCD

	void copyFrom(struct Unit* other) override;
	void initializeFrom(struct UnitResource* res) override;
	void update(struct Game* game) override;
};

}
