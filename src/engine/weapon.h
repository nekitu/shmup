#pragma once
#include "types.h"

namespace engine
{
struct Weapon
{
    enum class Type
    {
        ProjectileBased,
        Laser
    };

    f32 fireRays = 1;
    f32 fireRate = 1;
    f32 fireRaysAngle = 30;
    f32 fireRaysAngleOffset = 0;
    Vec2 position;
    Vec2 offset;
    f32 initialProjectileSpeed = 1;
    bool active = true;
    struct SpriteInstance* attachTo = nullptr;
    struct Projectile* projectile = nullptr;

    void fire(struct Game* game);
    void update(struct Game* game);

private:
    f32 fireTimer = 0.0f;
    f32 fireInterval = 0;
};

}
