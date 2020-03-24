#include "projectile_instance.h"
#include "sprite_instance.h"
#include "unit_instance.h"
#include "game.h"
#include "resources/unit_resource.h"
#include "resources/script_resource.h"

namespace engine
{
void ProjectileInstance::copyFrom(struct UnitInstance* other)
{
	UnitInstance::copyFrom(other);
	scriptClass = other->unit->script ? other->unit->script->createClassInstance(this) : nullptr;
}
void ProjectileInstance::initializeFrom(struct UnitResource* res)
{
	UnitInstance::initializeFrom(res);
	scriptClass = res->script ? res->script->createClassInstance(this) : nullptr;
}

void ProjectileInstance::update(Game* game)
{
	UnitInstance::update(game);
	if (!root) return;
	root->position += velocity * speed * game->deltaTime;
	speed += speed * acceleration * game->deltaTime;
	clampValue(speed, minSpeed, maxSpeed);
}

}
