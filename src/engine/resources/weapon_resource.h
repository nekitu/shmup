#pragma once
#include "types.h"
#include "resource.h"
#include "vec2.h"
#include "json/value.h"

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
		u32 fireRays = 1;
		f32 fireRate = 1;
		f32 fireRaysAngle = 20;
		Vec2 direction = Vec2(0, 1);
		f32 fireRaysRotationSpeed = 0;
		f32 initialProjectileSpeed = 1;
		f32 projectileAcceleration = 0;
		f32 minProjectileSpeed = 10;
		f32 maxProjectileSpeed = 10000000;
		f32 fireDamage = 1; /// if this is projectile, damage is per projectile hit, if beam then its per second
		Vec2 position;
		Vec2 offset;
		f32 offsetRadius = 0;
		Type type = Type::Projectile;
	};

	Parameters params;
	struct UnitResource* projectileUnit = nullptr;
	struct ScriptResource* fireScript = nullptr;

	bool load(Json::Value& json) override;
};

}
