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
	beamFireEndSound.soundResource = other->beamFireEndSound.soundResource;
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
	beamFireEndSound.soundResource = res->beamFireEndSound;
}

void Weapon::fire()
{
	if (!active)
		return;

	// set timer to highest, hence triggering the spawn of projectiles
	fireTimer = FLT_MAX;
	firing = true;
	startedFiring = false;
}

void Weapon::stopFire()
{
	firing = false;
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

bool Weapon::isBeamWeapon() const
{
	return weaponResource->params.type == WeaponResource::Type::Beam;
}

void Weapon::update(struct Game* game)
{
	if (!active)
		return;

	beamCollision.valid = false;

	if (isBeamWeapon() && attachTo && firing)
	{
		Vec2 pos = attachTo->position;

		if (attachTo != parentUnit->root && attachTo->relativeToRoot)
			pos += parentUnit->root->position;

		pos = Game::instance->worldToScreen(pos, parentUnit->layerIndex);

		// line to sprite check collision
		f32 beamDir = (parentUnit->unitResource->unitType == UnitType::Player) ? beamFromPlayer : beamFromEnemy;
		beamCollision = game->checkBeamIntersection(parentUnit, attachTo, pos, params.beamWidth, beamDir);
	}

	if (!isBeamWeapon() && params.activeTime > 0 && params.pauseDelay > 0)
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

	if (isBeamWeapon())
	{
		if (!startedFiring && firing)
		{
			startedFiring = true;
			fireSound.play(-1); // infinite loop
		}

		if (!firing && startedFiring)
		{
			startedFiring = false;
			fireSound.stop();
			beamFireEndSound.play();
		}

		if (firing)
		{
			beamFrameAnimationTime += game->deltaTime * 10;

			if (beamFrameAnimationTime > 1)
				beamFrameAnimationTime = 0;

			CALL_LUA_FUNC("onFire");

			if (beamCollision.valid)
			{
				LuaIntf::LuaRef colsTbl = LuaIntf::LuaRef::createTable(getLuaState());
				LuaIntf::LuaRef col = LuaIntf::LuaRef::createTable(getLuaState());

				col.set("sprite1", attachTo);
				col.set("sprite2", beamCollision.sprite);
				col.set("collisionCenter", beamCollision.point);
				colsTbl.set(1, col);
				LOG_INFO("Collide {0} with target {1}", attachTo->unit->name, beamCollision.unit->name);
				CALL_LUA_FUNC2(parentUnit->scriptClass, "onCollide", parentUnit, colsTbl)
				CALL_LUA_FUNC2(beamCollision.unit->scriptClass, "onCollide", beamCollision.unit, colsTbl)
			}
		}
	}
	else
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
	if (params.type == WeaponResource::Type::Beam && firing)
	{
		LOG_INFO("firing {0}", Game::instance->deltaTime);
		auto sprBody = weaponResource->beamBodySprite;
		auto sprBegin = weaponResource->beamBeginSprite;
		auto sprEnd = weaponResource->beamEndSprite;
		auto frames = 2;

		Game::instance->graphics->setupColor(0);

		if (Game::instance->screenMode == ScreenMode::Vertical)
		{
			if (!beamCollision.valid)
				beamCollision.distance = beamCollision.beamStart.y;

			// draw the beam body
			Game::instance->graphics->drawQuad({
					beamCollision.beamStart.x - params.beamWidth / 2,
					beamCollision.beamStart.y - beamCollision.distance,
					params.beamWidth,
					beamCollision.distance },
					sprBody->getFrameUvRect(beamFrameAnimationTime * frames));

			if (attachTo->unit->unitResource->unitType == UnitType::Player)
			{
				// begin of beam
				Game::instance->graphics->drawQuad({
					beamCollision.beamStart.x - sprBegin->frameWidth / 2,
					beamCollision.beamStart.y - sprBegin->frameHeight / 2,
					(f32)sprBegin->frameWidth,
					(f32)sprBegin->frameHeight },
					sprBegin->getFrameUvRect(beamFrameAnimationTime * frames));

				// end of beam
				Game::instance->graphics->drawQuad({
					beamCollision.beamStart.x - sprEnd->frameWidth / 2,
					beamCollision.point.y - sprEnd->frameHeight / 2,
					(f32)sprEnd->frameWidth,
					(f32)sprEnd->frameHeight },
					sprEnd->getFrameUvRect(beamFrameAnimationTime * frames));
			}
			//else if (attachTo->unit->unitResource->unitType == UnitType::Enemy)
			//{
			//	// begin of beam
			//	Game::instance->graphics->drawQuad({
			//		beamBeginPos.x - sprBegin->frameWidth / 2,
			//		beamBeginPos.y,
			//		(f32)sprBegin->frameWidth,
			//		(f32)sprBegin->frameHeight },
			//		sprBegin->getFrameUvRect(beamFrameAnimationTime * (sprBegin->frameCount - 1)));

			//	// end of beam
			//	Game::instance->graphics->drawQuad({
			//		beamBeginPos.x - sprEnd->frameWidth / 2,
			//		beamCollision.point.y,
			//		(f32)sprEnd->frameWidth,
			//		(f32)sprEnd->frameHeight },
			//		sprEnd->getFrameUvRect(beamFrameAnimationTime * (sprEnd->frameCount - 1)));
			//}
		}
		else
		{
			//TODO: horizontal screen
		}
	}
}

}
