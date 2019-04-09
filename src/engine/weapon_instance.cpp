#include "weapon_instance.h"
#include "game.h"
#include "projectile_instance.h"
#include "sprite_instance.h"

namespace engine
{
void WeaponInstance::fire(Game* game)
{
    f32 angle = params.fireRaysAngleOffset;

    for (u32 i = 0; i < params.fireRays; i++)
    {
        ProjectileInstance* newProj = new ProjectileInstance();

        newProj->weapon = this;
        newProj->transform.position = params.position;
        newProj->velocity.x = sinf(angle);
        newProj->velocity.y = cosf(angle);
        newProj->velocity.normalize();
        newProj->speed = params.initialProjectileSpeed;

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
        fire(game);
    }
}

}