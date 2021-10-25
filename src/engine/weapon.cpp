#include "weapon.h"
#include "game.h"
#include "projectile.h"
#include "sprite.h"
#include "utils.h"
#include "graphics.h"
#include "resources/script_resource.h"
#include "resource_loader.h"
#include "easing.h"
#include <float.h>

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
	fireTimer = FLT_MAX;
}

void Weapon::createBeamSprites()
{
	delete beamBeginSprite;
	delete beamEndSprite;
	delete beamBodySprite;

	beamBeginSprite = new Sprite();
	beamEndSprite = new Sprite();
	beamBodySprite = new Sprite();
	beamBeginSprite->initializeFromSpriteResource(weaponResource->beamBeginSprite);
	beamEndSprite->initializeFromSpriteResource(weaponResource->beamEndSprite);
	beamBodySprite->initializeFromSpriteResource(weaponResource->beamBodySprite);
}

void Weapon::copyFrom(Weapon* other)
{
	reset();
	params = other->params;
	active = other->active;
	groupIndex = other->groupIndex;
	autoFire = other->autoFire;
	weaponResource = other->weaponResource;
	parentUnit = other->parentUnit;
	attachTo = other->attachTo;
	fireSound.soundResource = other->fireSound.soundResource;
	beamFireEndSound.soundResource = other->beamFireEndSound.soundResource;

	if (other->beamBeginSprite)
		createBeamSprites();
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

	if (res->params.type == WeaponResource::Type::Beam)
		createBeamSprites();
}

void Weapon::fire()
{
	if (!active)
		return;

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

	f32 deltaTime = game->deltaTime;

	if (parentUnit)
	{
		deltaTime = parentUnit->isPlayer() ? game->realDeltaTime : deltaTime;
	}

	beamCollision.valid = false;

	if (isBeamWeapon() && attachTo && firing)
	{
		Vec2 pos = attachTo->position;

		if (attachTo != parentUnit->root && attachTo->relativeToRoot)
			pos += parentUnit->root->position;

		pos += params.offset;
		pos = Game::instance->worldToScreen(pos, parentUnit->layerIndex);

		// line to sprite check collision
		f32 beamDir = (parentUnit->unitResource->unitType == UnitType::Player) ? beamFromPlayer : beamFromEnemy;
		beamCollision = game->checkBeamIntersection(parentUnit, attachTo, pos, currentBeamScale * params.beamWidth, beamDir);

		beamBeginSprite->update(game);
		beamEndSprite->update(game);
		beamBodySprite->update(game);
	}

	if (!isBeamWeapon() && params.activeTime > 0 && params.pauseDelay > 0)
	{
		activeTimer += deltaTime;

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
			currentBeamScale = 0;
		}

		if (!firing && startedFiring)
		{
			beamWidthAnimTime = 0;
			currentBeamScale = 0;
			startedFiring = false;
			fireSound.stop();
			beamFireEndSound.play();
		}

		if (firing)
		{
			if (currentBeamScale < 1.0f)
			{
				currentBeamScale = Easing::easeValue(params.beamWidthAnimEasing, beamWidthAnimTime, 0, 1, 1);
				clampValue(currentBeamScale, 0, 1);
				beamWidthAnimTime += deltaTime * params.beamAnimSpeed;
			}

			CALL_LUA_FUNC("onFire");

			if (beamCollision.valid)
			{
				LuaIntf::LuaRef colsTbl = LuaIntf::LuaRef::createTable(getLuaState());
				LuaIntf::LuaRef col = LuaIntf::LuaRef::createTable(getLuaState());

				if (!beamCollision.friendly)
				{
					attachTo->hit(beamCollision.sprite->damage);
					beamCollision.sprite->hit(attachTo->damage);
				}

				col.set("sprite1", attachTo);
				col.set("sprite2", beamCollision.sprite);
				col.set("collisionCenter", beamCollision.point);
				colsTbl.set(1, col);
				LOG_INFO("Collide {} with target {} {}", attachTo->unit->name, beamCollision.unit->name, beamCollision.unit->health);
				CALL_LUA_FUNC2(parentUnit->scriptClass, "onCollide", beamCollision.unit, colsTbl)
				CALL_LUA_FUNC2(beamCollision.unit->scriptClass, "onCollide", parentUnit, colsTbl)
			}
		}
	}
	else
	if (firing || autoFire)
	{
		fireInterval = 1.0f / params.fireRate;
		fireTimer += deltaTime;
		fireAngleOffset += params.fireRaysRotationSpeed * deltaTime;

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

void Weapon::render(Graphics* gfx)
{
	if (params.type == WeaponResource::Type::Beam && firing)
	{
		auto sprBody = weaponResource->beamBodySprite;
		auto sprBegin = weaponResource->beamBeginSprite;
		auto sprEnd = weaponResource->beamEndSprite;
		auto frames = 2;

		gfx->setupColor(0);

		if (Game::instance->screenMode == ScreenMode::Vertical)
		{
			if (!beamCollision.valid)
				beamCollision.distance = beamCollision.beamStart.y;

			// draw the beam body
			if (beamBodySprite)
			{
				if (weaponResource->params.beamBodyType == WeaponResource::BeamBodyType::Stretch)
				{
					beamBodySprite->rect = {
						beamCollision.beamStart.x - currentBeamScale * params.beamWidth / 2,
						beamCollision.beamStart.y - beamCollision.distance,
						currentBeamScale * params.beamWidth,
						beamCollision.distance };
					beamBodySprite->render(gfx);
				}
				else if (weaponResource->params.beamBodyType == WeaponResource::BeamBodyType::Repeat)
				{
					auto repeatCount = fabsf(beamCollision.distance / (f32)beamBodySprite->spriteResource->frameHeight);
					f32 countFraction = repeatCount - (u32)repeatCount;

					for (u32 i = 0; i < (u32)repeatCount; i++)
					{
						beamBodySprite->rect = {
						beamCollision.beamStart.x - currentBeamScale * params.beamWidth / 2,
						beamCollision.beamStart.y - (f32)i * beamBodySprite->spriteResource->frameHeight - beamBodySprite->spriteResource->frameHeight,
						currentBeamScale * params.beamWidth,
						(f32)beamBodySprite->spriteResource->frameHeight };

						beamBodySprite->render(gfx);
					}

					if (countFraction > 0)
					{
						beamBodySprite->rect = {
						beamCollision.beamStart.x - currentBeamScale * params.beamWidth / 2,
						beamCollision.beamStart.y - (repeatCount - 1) * beamBodySprite->spriteResource->frameHeight - beamBodySprite->spriteResource->frameHeight,
						currentBeamScale * params.beamWidth,
						(f32)beamBodySprite->spriteResource->frameHeight * countFraction };
						// scale UV coord height left of sprite after fraction
						beamBodySprite->uvRect.y += beamBodySprite->uvRect.height * (1.0f - countFraction);
						beamBodySprite->uvRect.height *= countFraction;

						beamBodySprite->render(gfx);
					}
				}
			}

			if (attachTo->unit->unitResource->unitType == UnitType::Player)
			{
				// draw the beam begin
				if (beamBeginSprite)
				{
					beamBeginSprite->rect = {
					beamCollision.beamStart.x - sprBegin->frameWidth * currentBeamScale / 2,
					beamCollision.beamStart.y - sprBegin->frameHeight / 2,
					(f32)sprBegin->frameWidth * currentBeamScale,
					(f32)sprBegin->frameHeight };
					beamBeginSprite->render(gfx);
				}

				// draw the beam end
				if (beamEndSprite && beamCollision.valid && beamCollision.directHit)
				{
					beamEndSprite->rect = {
					beamCollision.beamStart.x - sprEnd->frameWidth * currentBeamScale / 2,
					beamCollision.point.y - sprEnd->frameHeight,
					(f32)sprEnd->frameWidth * currentBeamScale,
					(f32)sprEnd->frameHeight };
					beamEndSprite->render(gfx);
				}
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
