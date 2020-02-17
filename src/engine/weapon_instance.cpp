#include "weapon_instance.h"
#include "game.h"
#include "projectile_instance.h"
#include "sprite_instance.h"
#include "unit_controller.h"
#include "utils.h"
#include "resources/script_resource.h"
#include <float.h>

namespace engine
{
void WeaponInstance::copyFrom(WeaponInstance* other)
{
	params = other->params;
	active = other->active;
	weaponResource = other->weaponResource;
	parentUnitInstance = other->parentUnitInstance;
	attachTo = other->attachTo;
}

void WeaponInstance::initializeFrom(struct WeaponResource* res)
{
	weaponResource = res;
	params = res->params;
}

void WeaponInstance::fire()
{
	if (!active)
		return;

	// set timer to highest, hence triggering the spawn of projectiles
	fireTimer = FLT_MAX;

	if (weaponResource->script)
	{
		auto f = weaponResource->script->getFunction("onFire");

		if (f.isFunction())
		{
			f(this);
		}
	}
}

void WeaponInstance::spawnProjectiles(Game* game)
{
	if (params.autoAim)
	{
		Vec2 pos = attachTo->rect.center();

		params.direction.x = game->players[0]->rootSpriteInstance->transform.position.x - pos.x;
		params.direction.y = game->players[0]->rootSpriteInstance->transform.position.y - pos.y;
		params.direction.normalize();
	}

    f32 angle = rad2deg(atan2f(params.direction.x, params.direction.y));
	f32 angleBetweenRays = params.fireRaysAngle / (f32)(params.fireRays - 1);
	f32 angle1 = angle - params.fireRaysAngle / 2.0f;
	f32 angle2 = angle + params.fireRaysAngle / 2.0f;

	if (params.fireRays > 1)
		angle = angle1;

	fireDirectionAngle += params.fireRaysRotationSpeed * game->deltaTime;

	if (fireDirectionAngle > 360) fireDirectionAngle = 0;

	angle += fireDirectionAngle;

    for (u32 i = 0; i < params.fireRays; i++)
    {
        ProjectileInstance* newProj = new ProjectileInstance();

		newProj->initializeFrom(weaponResource->projectileUnit);
		newProj->weapon = this;
		Vec2 offRadius = Vec2(params.offsetRadius * sinf(deg2rad(angle)), params.offsetRadius * cosf(deg2rad(angle)));
		Vec2 pos = attachTo->transform.position;

		if (attachTo != parentUnitInstance->rootSpriteInstance && !attachTo->noRootParent)
			pos += parentUnitInstance->rootSpriteInstance->transform.position;
		newProj->rootSpriteInstance->transform.position = pos + params.position + params.offset + offRadius;
		auto rads = deg2rad(angle);
		newProj->velocity.x = sinf(rads);
		newProj->velocity.y = cosf(rads);
        newProj->velocity.normalize();
		newProj->controllers["main"] = new ProjectileController();
		newProj->controllers["main"]->unitInstance = newProj;
		newProj->speed = params.initialProjectileSpeed;
		newProj->acceleration = params.projectileAcceleration;
		newProj->minSpeed = params.minProjectileSpeed;
		newProj->maxSpeed = params.maxProjectileSpeed;
		game->unitInstances.push_back(newProj);
        angle += angleBetweenRays;
    }
}

void WeaponInstance::update(struct Game* game)
{
	if (!active)
		return;

    fireInterval = 1.0f / params.fireRate;
    fireTimer += game->deltaTime;

    if (fireTimer >= fireInterval)
    {
        fireTimer = 0;
		spawnProjectiles(game);
    }
}

void WeaponInstance::debug(const std::string& info)
{
}

}
