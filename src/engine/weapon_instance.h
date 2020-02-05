#include "types.h"
#include "resources/weapon_resource.h"

namespace engine
{
struct WeaponInstance
{
	WeaponResource::Parameters params;
	bool active = true;
	struct WeaponResource* weaponResource = nullptr;
	struct UnitInstance* parentUnitInstance = nullptr;
	struct SpriteInstance* attachTo = nullptr;

	void copyFrom(WeaponInstance* other);
	void fire();
	void initializeFrom(struct WeaponResource* res);
	void spawnProjectiles(struct Game* game);
	void update(struct Game* game);
	void debug(const std::string& info);
private:
	f32 fireTimer = 0.0f;
	f32 fireInterval = 0;
	f32 fireDirectionAngle = 0.0f;
};

}
