#pragma once
#include "types.h"
#include "resources/weapon_resource.h"
#include "game.h"

namespace engine
{
struct Weapon
{
	WeaponResource::Parameters params;
	bool active = true;
	struct WeaponResource* weaponResource = nullptr;
	struct Unit* parentUnit = nullptr;
	struct Sprite* attachTo = nullptr;
	struct ScriptClassInstanceBase* scriptClass = nullptr;

	~Weapon() { delete scriptClass; }

	void copyFrom(Weapon* other);
	void fire();
	void initializeFrom(struct WeaponResource* res);
	void spawnProjectiles(struct Game* game);
	void update(struct Game* game);
	void render();

	f32 angle = 0;
private:
	BeamCollisionInfo dbgBeamCol;
	Vec2 dbgBeamStartPos;
	f32 fireTimer = 0.0f;
	f32 fireInterval = 0;
	f32 fireAngleOffset = 0.0f;
};

}
