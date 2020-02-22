#include "weapon_instance.h"
#include "game.h"
#include "projectile_instance.h"
#include "sprite_instance.h"
#include "unit_controller.h"
#include "utils.h"
#include "graphics.h"
#include "resources/script_resource.h"
#include <float.h>
#include "resource_loader.h"

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
	if (params.type == WeaponResource::Type::Beam) return;

	if (params.autoAim)
	{
		Vec2 pos = attachTo->screenRect.center();

		params.direction.x = game->players[0]->rootSpriteInstance->screenRect.center().x - pos.x;
		params.direction.y = game->players[0]->rootSpriteInstance->screenRect.center().y - pos.y;
		params.direction.normalize();
	}

    angle = dir2deg(params.direction);
	f32 angleBetweenRays = params.fireRays > 1 ? params.fireRaysAngle / (f32)(params.fireRays - 1) : 0;
	f32 angle1 = angle - params.fireRaysAngle / 2.0f;
	f32 angle2 = angle + params.fireRaysAngle / 2.0f;

	if (params.fireRays > 1)
		angle = angle1;

	fireAngleOffset += params.fireRaysRotationSpeed * game->deltaTime;

	if (fireAngleOffset > 360) fireAngleOffset = 0;

	angle += fireAngleOffset;

	for (u32 i = 0; i < params.fireRays; i++)
    {
        ProjectileInstance* newProj = new ProjectileInstance();

		newProj->initializeFrom(weaponResource->projectileUnit);
		newProj->weapon = this;
		Vec2 offRadius = Vec2(params.offsetRadius * sinf(deg2rad(angle)), params.offsetRadius * cosf(deg2rad(angle)));
		Vec2 pos = attachTo->transform.position;

		if (attachTo != parentUnitInstance->rootSpriteInstance && !attachTo->notRelativeToRoot)
			pos += parentUnitInstance->rootSpriteInstance->transform.position;
		newProj->rootSpriteInstance->transform.position = pos + params.position + params.offset + offRadius;
		auto rads = deg2rad(angle);
		newProj->velocity.x = sinf(rads);
		newProj->velocity.y = cosf(rads);
        newProj->velocity.normalize();
		newProj->controllers["main"] = new ProjectileController();
		newProj->controllers["main"]->unitInstance = newProj;
		newProj->layerIndex = parentUnitInstance->layerIndex;
		newProj->speed = params.initialProjectileSpeed;
		newProj->acceleration = params.projectileAcceleration;
		newProj->minSpeed = params.minProjectileSpeed;
		newProj->maxSpeed = params.maxProjectileSpeed;
		game->newUnitInstances.push_back(newProj);
        angle += angleBetweenRays;
    }
}

void WeaponInstance::update(struct Game* game)
{
	if (!active)
		return;

	if (attachTo && weaponResource->params.type == WeaponResource::Type::Beam)
	{
		Vec2 pos = attachTo->transform.position;

		if (attachTo != parentUnitInstance->rootSpriteInstance && !attachTo->notRelativeToRoot)
			pos += parentUnitInstance->rootSpriteInstance->transform.position;

		pos = Game::instance->worldToScreen(pos, parentUnitInstance->layerIndex);

		// line to sprite check collision
		BeamCollisionInfo bci = game->checkBeamIntersection(parentUnitInstance, attachTo, pos, params.beamWidth);

		dbgBeamStartPos = pos;
		dbgBeamCol = bci;

		return;
	}

    fireInterval = 1.0f / params.fireRate;
    fireTimer += game->deltaTime;

    if (fireTimer >= fireInterval)
    {
        fireTimer = 0;
		spawnProjectiles(game);
    }

	if (weaponResource && weaponResource->script)
	{
		auto func = weaponResource->script->getFunction("onUpdate");

		if (func.isFunction())
		{
			func.call(this);
		}
	}
}

void WeaponInstance::render()
{

	if (params.type == WeaponResource::Type::Beam)
	{
		auto spr = Game::instance->resourceLoader->loadSprite("sprites/black");
		Game::instance->graphics->currentColor = 0xffff;
		Game::instance->graphics->currentColorMode = (u32)ColorMode::Add;

		if (!dbgBeamCol.valid) dbgBeamCol.distance = dbgBeamStartPos.y;

		Game::instance->graphics->drawQuad({ dbgBeamStartPos.x - params.beamWidth / 2, dbgBeamStartPos.y, params.beamWidth, -dbgBeamCol.distance }, spr->getFrameUvRect(0));
	}
}

void WeaponInstance::debug(const std::string& info)
{
}

}
