#include "weapon.h"
#include "game.h"
#include "projectile.h"
#include "sprite.h"
#include "utils.h"
#include "graphics.h"
#include "resources/script_resource.h"
#include <float.h>
#include "resource_loader.h"

namespace engine
{
Weapon::~Weapon()
{
	LOG_INFO("Deleting weapon");
	reset();
}

void Weapon::reset()
{
	if (scriptClass)
		delete scriptClass;
	scriptClass = nullptr;
}

void Weapon::copyFrom(Weapon* other)
{
	reset();
	params = other->params;
	active = other->active;
	autoFire = other->autoFire;
	weaponResource = other->weaponResource;
	parentUnit = other->parentUnit;
	attachTo = other->attachTo;
	fireSound.soundResource = other->fireSound.soundResource;
}

void Weapon::initializeFrom(struct WeaponResource* res)
{
	reset();
	weaponResource = res;
	params = res->params;

	if (res->script)
	{
		scriptClass = res->script->createClassInstance(this);
	}

	fireSound.soundResource = res->fireSound;
}

void Weapon::fire()
{
	if (!active)
		return;

	// set timer to highest, hence triggering the spawn of projectiles
	fireTimer = FLT_MAX;
	firing = true;
}

void Weapon::spawnProjectiles(Game* game)
{
	if (params.type == WeaponResource::Type::Beam) return;

	if (params.autoAim)
	{
		Vec2 pos = attachTo->rect.center();

		params.direction = game->playerState[0].unit->root->rect.center() - pos;
		params.direction.normalize();
	}

	angle = dir2deg(params.direction);

	f32 angleBetweenRays = params.fireRays > 1 ? params.fireRaysAngle / (f32)(params.fireRays) : 0;
	f32 angle1 = angle - params.fireRaysAngle / 2.0f;
	f32 angle2 = angle + params.fireRaysAngle / 2.0f;

	if (params.fireRays > 1)
		angle = angle1;

	f32 ang = angle + fireAngleOffset;

	for (u32 i = 0; i < params.fireRays; i++)
	{
		auto newProj = Game::instance->newProjectile();

		newProj->initializeFrom(weaponResource->projectileUnit);
		newProj->weaponResource = this;
		auto rads = deg2rad(ang);
		auto sinrads = sinf(rads);
		auto cosrads = cosf(rads);
		Vec2 offRadius = Vec2(params.offsetRadius * sinrads, params.offsetRadius * cosrads);
		Vec2 pos = attachTo->position;

		if (attachTo != parentUnit->root && attachTo->relativeToRoot)
			pos += parentUnit->root->position;

		pos += params.position + params.offset + offRadius;

		newProj->root->position = newProj->spawnPosition = pos;
		newProj->velocity.x = sinrads;
		newProj->velocity.y = cosrads;
		newProj->velocity.normalize();
		newProj->layerIndex = parentUnit->layerIndex;
		newProj->speed = params.initialProjectileSpeed;
		newProj->acceleration = params.projectileAcceleration;
		newProj->minSpeed = params.minProjectileSpeed;
		newProj->maxSpeed = params.maxProjectileSpeed;
		newProj->appeared = true;
		auto rotAngle = dir2deg(newProj->velocity);
		newProj->root->setFrameAnimationFromAngle(rotAngle);
		ang += angleBetweenRays;
	}
}

void Weapon::update(struct Game* game)
{
	if (!active)
		return;

	if (attachTo && weaponResource->params.type == WeaponResource::Type::Beam)
	{
		Vec2 pos = attachTo->position;

		if (attachTo != parentUnit->root && attachTo->relativeToRoot)
			pos += parentUnit->root->position;

		pos = Game::instance->worldToScreen(pos, parentUnit->layerIndex);

		// line to sprite check collision
		BeamCollisionInfo bci = game->checkBeamIntersection(parentUnit, attachTo, pos, params.beamWidth);

		dbgBeamStartPos = pos;
		dbgBeamCol = bci;

		return;
	}

	if (params.activeTime > 0 && params.pauseDelay > 0)
	{
		activeTimer += game->deltaTime;

		if (paused)
		{
			if (activeTimer <= params.pauseDelay)
			{
				return;
			}

			paused = false;
			activeTimer = 0;
		}
		else if (activeTimer > params.activeTime)
		{
			paused = true;
			activeTimer = 0;
		}
	}

	if (firing || autoFire)
	{
		fireInterval = 1.0f / params.fireRate;
		fireTimer += game->deltaTime;
		fireAngleOffset += params.fireRaysRotationSpeed * game->deltaTime;

		if (fireTimer >= fireInterval)
		{
			fireTimer = 0;
			spawnProjectiles(game);
			fireSound.play();
			firing = false;
			CALL_LUA_FUNC("onFire");
		}
	}

	CALL_LUA_FUNC("onUpdate");
}

void Weapon::render()
{
	//TODO: remove test beam
	if (params.type == WeaponResource::Type::Beam)
	{
		static f32 f = 0;
		static f32 f2 = 0;
		static f32 f3 = 0;
		auto spr = Game::instance->resourceLoader->loadSprite("sprites/beam");
		auto sprTop = Game::instance->resourceLoader->loadSprite("sprites/beam_top");
		auto sprBtm = Game::instance->resourceLoader->loadSprite("sprites/beam_btm");
		Game::instance->graphics->color = 0;
		Game::instance->graphics->colorMode = (u32)ColorMode::Add;

		if (!dbgBeamCol.valid) dbgBeamCol.distance = dbgBeamStartPos.y;

		Game::instance->graphics->drawQuad({ dbgBeamStartPos.x - params.beamWidth / 2, dbgBeamStartPos.y, params.beamWidth, -dbgBeamCol.distance }, spr->getFrameUvRect(f));
		Game::instance->graphics->drawQuad({ dbgBeamStartPos.x - 25, dbgBeamCol.point.y - 50, 50, (f32)sprTop->frameHeight }, sprTop->getFrameUvRect(f2));
		Game::instance->graphics->drawQuad({ dbgBeamStartPos.x - 25, dbgBeamStartPos.y - 50, 50, (f32)sprBtm->frameHeight }, sprBtm->getFrameUvRect(f3));
		f += Game::instance->deltaTime * 20;
		if (f > spr->frameCount - 1) f = 0;
		f2 += Game::instance->deltaTime * 20;
		if ((u32)f2 > 1) f2 = 0;
		f3 += Game::instance->deltaTime * 20;
		if ((u32)f3 > 1) f3 = 0;
	}
}

}
