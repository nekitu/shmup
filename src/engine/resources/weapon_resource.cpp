#include "weapon_resource.h"

namespace engine
{
bool WeaponResource::load(Json::Value& json)
{
	params.fireDamage = json.get("fireDamage", params.fireDamage).asFloat();
	params.fireRate = json.get("fireRate", params.fireRate).asFloat();
	params.fireRays = json.get("fireRays", params.fireRays).asInt();
	params.fireRaysAngle = json.get("fireRaysAngle", params.fireRaysAngle).asFloat();
	params.fireRaysAngleOffset = json.get("fireRaysAngleOffset", params.fireRaysAngleOffset).asFloat();
	params.initialProjectileSpeed = json.get("initialProjectileSpeed", params.initialProjectileSpeed).asFloat();
	params.offset.x = json.get("offset.x", 0.0f).asFloat();
	params.offset.y = json.get("offset.y", 0.0f).asFloat();
	params.position.x = json.get("position.x", 0.0f).asFloat();
	params.position.y = json.get("position.y", 0.0f).asFloat();
	params.type = (Type)json.get("type", 0).asInt();

	return true;
}

}