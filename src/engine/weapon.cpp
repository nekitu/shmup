#include "weapon.h"

namespace engine
{
void Weapon::fire(Game* game)
{
    f32 angle = fireRaysAngleOffset;

    for (u32 i = 0; i < fireRays; i++)
    {
        ProjectileInstance* newProj = new ProjectileInstance();

        newProj->weapon = this;
        newProj->transform.position = position;
        newProj->velocity.x = sinf(angle);
        newProj->velocity.y = cosf(angle);
        newProj->velocity.normalize();
        newProj->projectile = projectile;
        newProj->speed = initialProjectileSpeed;

        game->unitInstances.push_back(newProj);
        angle += fireRaysAngle;
    }
}

void Weapon::update(struct Game* game)
{
    if (attachTo)
    {
        position = attachTo->transform.position;
        position += offset;
    }

    fireInterval = 1.0f / fireRate;
    fireTimer += game->deltaTime;

    if (fireTimer >= fireInterval)
    {
        fireTimer = 0;
        fire(game);
    }
}
}