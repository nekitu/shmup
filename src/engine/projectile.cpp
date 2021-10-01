#include "projectile.h"
#include "sprite.h"
#include "unit.h"
#include "game.h"
#include "resources/unit_resource.h"
#include "resources/script_resource.h"

namespace engine
{
void Projectile::copyFrom(struct Unit* other)
{
	Unit::copyFrom(other);
	scriptClass = other->unitResource->script ? other->unitResource->script->createClassInstance(this) : nullptr;
}

void Projectile::initializeFrom(struct UnitResource* res)
{
	Unit::initializeFrom(res);
	scriptClass = res->script ? res->script->createClassInstance(this) : nullptr;
}

void Projectile::update(Game* game)
{
	Unit::update(game);

	f32 deltaTime = isPlayer() ? game->realDeltaTime : game->deltaTime;

	if (!root) return;
	root->position += velocity * speed * deltaTime;
	speed += speed * acceleration * deltaTime;
	clampValue(speed, minSpeed, maxSpeed);
}

}
