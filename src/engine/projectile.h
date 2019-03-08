#pragma once
#include "types.h"

namespace engine
{
struct Projectile : Unit
{
    f32 damage = 1;

    Projectile()
    {
        type = Type::Projectile;
    }
};

struct ProjectileInstance : UnitInstance
{
    Projectile* projectile = nullptr;
    struct Weapon* weapon = nullptr;
    Vec2 velocity;

};

}