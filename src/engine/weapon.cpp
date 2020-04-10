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
	delete scriptClass;
}

void Weapon::reset()
{
	delete scriptClass;
	scriptClass = nullptr;
}

void Weapon::copyFrom(Weapon* other)
{
	reset();
	params = other->params;
	active = other->active;
	weaponResource = other->weaponResource;
	parentUnit = other->parentUnit;
	attachTo = other->attachTo;
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
}

void Weapon::fire()
{
	if (!active)
		return;

	// set timer to highest, hence triggering the spawn of projectiles
	fireTimer = FLT_MAX;
	CALL_LUA_FUNC("onFire");
}

void Weapon::spawnProjectiles(Game* game)
{
	if (params.type == WeaponResource::Type::Beam) return;

	if (params.autoAim)
	{
		Vec2 pos = attachTo->screenRect.center();

		params.direction.x = game->players[0].unit->root->screenRect.center().x - pos.x;
		params.direction.y = game->players[0].unit->root->screenRect.center().y - pos.y;
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

	f32 ang = angle;

	for (u32 i = 0; i < params.fireRays; i++)
	{
		auto newProj = Game::instance->newProjectile();

		newProj->initializeFrom(weaponResource->projectileUnit);
		newProj->weaponResource = this;

		Vec2 offRadius = Vec2(params.offsetRadius * sinf(deg2rad(ang)), params.offsetRadius * cosf(deg2rad(ang)));
		Vec2 pos = attachTo->position;

		if (attachTo != parentUnit->root && !attachTo->notRelativeToRoot)
			pos += parentUnit->root->position;

		newProj->root->position = pos + params.position + params.offset + offRadius;
		auto rads = deg2rad(ang);
		newProj->velocity.x = sinf(rads);
		newProj->velocity.y = cosf(rads);
		newProj->velocity.normalize();
		newProj->layerIndex = parentUnit->layerIndex;
		newProj->speed = params.initialProjectileSpeed;
		newProj->acceleration = params.projectileAcceleration;
		newProj->minSpeed = params.minProjectileSpeed;
		newProj->maxSpeed = params.maxProjectileSpeed;
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

		if (attachTo != parentUnit->root && !attachTo->notRelativeToRoot)
			pos += parentUnit->root->position;

		pos = Game::instance->worldToScreen(pos, parentUnit->layerIndex);

		// line to sprite check collision
		BeamCollisionInfo bci = game->checkBeamIntersection(parentUnit, attachTo, pos, params.beamWidth);

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

	Game::instance->graphics->color = 0;
	Game::instance->graphics->colorMode = (u32)ColorMode::Add;
	auto sprB = Game::instance->resourceLoader->loadSprite("sprites/black");
	static f32 r1 = 30;
	static f32 r2 = 60;
	static f32 rotAng1 = 0;
	static f32 angDelta = 0;

	for (f32 a = angDelta; a < angDelta + M_PI * 2.0f; a += deg2rad(30))
	{
		Vec2 p;

		p.x = sinf(a) * r1;
		p.y = cosf(a) * r2;
		p.rotateAround({ 0.f,0.f }, rotAng1);
		p.x += 120;
		p.y += 120;

		Game::instance->graphics->drawQuad({ p.x, p.y, 5, 5 }, sprB->getFrameUvRect(0));
	}

	r1 += sinf(rotAng1) * 10.0 * Game::instance->deltaTime * 5.0f;
	r2 += sinf(rotAng1) * 10.0 * Game::instance->deltaTime * 5.0f;
	rotAng1 += Game::instance->deltaTime * 0.1;
	angDelta += Game::instance->deltaTime * 0.2;
}

}
