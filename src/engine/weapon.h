#pragma once
#include "types.h"
#include "resources/weapon_resource.h"
#include "game.h"
#include "sound.h"

namespace engine
{
struct Weapon
{
	const f32 beamFromEnemy = 1;
	const f32 beamFromPlayer = -1;

	WeaponResource::Parameters params;
	bool active = true;
	struct WeaponResource* weaponResource = nullptr;
	struct Unit* parentUnit = nullptr;
	struct Sprite* attachTo = nullptr;
	struct ScriptClassInstanceBase* scriptClass = nullptr;
	Sound fireSound;
	Sound beamFireEndSound;
	f32 angle = 0;
	BeamCollisionInfo beamCollision;
	f32 fireTimer = 0.0f;
	f32 fireInterval = 0;
	f32 fireAngleOffset = 0.0f;
	f32 activeTimer = 0;
	f32 beamFrameAnimationTime = 0;
	bool paused = false;
	bool firing = false;
	bool autoFire = false;
	bool startedFiring = false;
	bool stoppedFiring = true;

	~Weapon();

	bool isBeamWeapon() const;
	void reset();
	void copyFrom(Weapon* other);
	void fire();
	void stopFire();
	void initializeFrom(struct WeaponResource* res);
	void spawnProjectiles(struct Game* game);
	void update(struct Game* game);
	void render();
};

}
