#include "weapon_resource.h"

namespace engine
{
bool WeaponResource::load(Json::Value& json)
{
	params.fireDamage = json.get("fireDamage", params.fireDamage).asFloat();
	params.fireRate = json.get("fireRate", params.fireRate).asFloat();
	params.fireRays = json.get("fireRays", params.fireRays).asInt();
	params.fireRaysAngle = json.get("fireRaysAngle", params.fireRaysAngle).asFloat();
	params.direction.x = json.get("direction.x", params.direction.x).asFloat();
	params.direction.y = json.get("direction.y", params.direction.y).asFloat();
	params.fireRaysRotationSpeed = json.get("fireRaysRotationSpeed", params.fireRaysRotationSpeed).asFloat();
	params.initialProjectileSpeed = json.get("initialProjectileSpeed", params.initialProjectileSpeed).asFloat();
	params.projectileAcceleration = json.get("projectileAcceleration", params.projectileAcceleration).asFloat();
	params.minProjectileSpeed = json.get("minProjectileSpeed", params.minProjectileSpeed).asFloat();
	params.maxProjectileSpeed = json.get("maxProjectileSpeed", params.maxProjectileSpeed).asFloat();
	params.offset.x = json.get("offset.x", 0.0f).asFloat();
	params.offset.y = json.get("offset.y", 0.0f).asFloat();
	params.position.x = json.get("position.x", 0.0f).asFloat();
	params.position.y = json.get("position.y", 0.0f).asFloat();
	params.offsetRadius = json.get("offsetRadius", 0.0f).asFloat();
	params.type = (Type)json.get("type", 0).asInt();

	return true;
}

}