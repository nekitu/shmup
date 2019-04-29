#include "weapon_instance.h"
#include "game.h"
#include "projectile_instance.h"
#include "sprite_instance.h"
#include "unit_controller.h"

namespace engine
{
void WeaponInstance::setWeaponResource(struct WeaponResource* res)
{
	weaponResource = res;
	params = res->params;
}

void WeaponInstance::fire()
{
	// set timer to highest, hence triggering the spawn of projectiles
	fireTimer = FLT_MAX;
}

void WeaponInstance::spawnProjectiles(Game* game)
{
    f32 angle = params.fireRaysAngleOffset * M_PI / 180.f;

    for (u32 i = 0; i < params.fireRays; i++)
    {
        ProjectileInstance* newProj = new ProjectileInstance();

        newProj->weapon = this;
        newProj->transform.position = parentUnitInstance->transform.position + params.position + params.offset;
        newProj->velocity.x = sinf(angle);
        newProj->velocity.y = cosf(angle);
        newProj->velocity.normalize();
        newProj->speed = params.initialProjectileSpeed;
		newProj->controller = new ProjectileController();
		newProj->controller->unitInstance = newProj;
		game->copyUnitToUnitInstance(weaponResource->projectileUnit, newProj);
		printf("Adding projectile %p\n", newProj);
		game->unitInstances.push_back(newProj);
        angle += params.fireRaysAngle;
    }
}

void WeaponInstance::update(struct Game* game)
{
    if (attachTo)
    {
		params.position = attachTo->transform.position;
		params.position += params.offset;
    }

    fireInterval = 1.0f / params.fireRate;
    fireTimer += game->deltaTime;

    if (fireTimer >= fireInterval)
    {
        fireTimer = 0;
        spawnProjectiles(game);
    }
}

}