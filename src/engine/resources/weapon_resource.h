#pragma once
#include "types.h"
#include "resource.h"
#include "vec2.h"

namespace engine
{
struct WeaponResource : Resource
{
    enum class Type
    {
        Projectile,
        Beam
    };

	struct Parameters
	{
		f32 fireRays = 1;
		f32 fireRate = 1;
		f32 fireRaysAngle = 30;
		f32 fireRaysAngleOffset = 0;
		f32 initialProjectileSpeed = 1;
		f32 fireDamage = 1; /// if this is projectile, damage is per projectile hit, if beam then its per second
		Vec2 position;
		Vec2 offset;
	};

	Parameters params;

	bool load(const std::string& filename) override;
};

}
