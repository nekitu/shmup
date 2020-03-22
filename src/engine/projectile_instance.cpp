#include "projectile_instance.h"
#include "unit_instance.h"
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
}

}
