#pragma once
#include "types.h"
#include "resources/weapon_resource.h"
#include "game.h"
#include "sound.h"

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
	Sound fireSound;
	f32 angle = 0;
	BeamCollisionInfo dbgBeamCol;
	Vec2 dbgBeamStartPos;
	f32 fireTimer = 0.0f;
	f32 fireInterval = 0;
	f32 fireAngleOffset = 0.0f;
	f32 activeTimer = 0;
	bool paused = false;

	~Weapon();

	void reset();
	void copyFrom(Weapon* other);
	void fire();
	void initializeFrom(struct WeaponResource* res);
	void spawnProjectiles(struct Game* game);
	void update(struct Game* game);
	void render();
};

}
