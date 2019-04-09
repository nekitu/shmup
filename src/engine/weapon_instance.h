#include "types.h"
#include "resources/weapon_resource.h"

namespace engine
{
struct WeaponInstance
{
	WeaponResource::Parameters params;
	bool active = true;
    struct SpriteInstance* attachTo = nullptr;
    struct ProjectileResource* projectile = nullptr;

    void fire(struct Game* game);
    void update(struct Game* game);

private:
    f32 fireTimer = 0.0f;
    f32 fireInterval = 0;
};

}