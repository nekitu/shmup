#include "weapon_resource.h"
#include "resource_loader.h"

namespace engine
{
bool WeaponResource::load(Json::Value& json)
{
	script = loader->loadScript(json.get("script", "").asString());
	projectileUnit = loader->loadUnit(json.get("projectileUnit", "").asString());
	params.fireDamage = json.get("fireDamage", params.fireDamage).asFloat();
	params.beamWidth = json.get("beamWidth", params.beamWidth).asFloat();
	params.fireRate = json.get("fireRate", params.fireRate).asFloat();
	params.fireRays = json.get("fireRays", params.fireRays).asInt();
	params.fireRaysAngle = json.get("fireRaysAngle", params.fireRaysAngle).asFloat();
	params.direction.parse(json.get("direction", params.direction.toString()).asString());
	params.fireRaysRotationSpeed = json.get("fireRaysRotationSpeed", params.fireRaysRotationSpeed).asFloat();
	params.initialProjectileSpeed = json.get("initialProjectileSpeed", params.initialProjectileSpeed).asFloat();
	params.projectileAcceleration = json.get("projectileAcceleration", params.projectileAcceleration).asFloat();
	params.minProjectileSpeed = json.get("minProjectileSpeed", params.minProjectileSpeed).asFloat();
	params.maxProjectileSpeed = json.get("maxProjectileSpeed", params.maxProjectileSpeed).asFloat();
	params.offset.parse(json.get("offset", "0 0").asString());
	params.position.parse(json.get("position", "0 0").asString());
	params.offsetRadius = json.get("offsetRadius", 0.0f).asFloat();
	auto typeStr = json.get("type", "Projectile").asString();

	if (typeStr == "Projectile") params.type = Type::Projectile;
	else if (typeStr == "Beam") params.type = Type::Beam;

	params.autoAim = json.get("autoAim", params.autoAim).asBool();

	return true;
}

}
