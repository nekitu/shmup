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
	params.spawnLayerOffset = json.get("spawnLayerOffset", -1).asInt();
	params.activeTime = json.get("activeTime", 0.0f).asFloat();
	params.pauseDelay = json.get("pauseDelay", 0.0f).asFloat();
	params.autoAim = json.get("autoAim", params.autoAim).asBool();

	auto fireSoundPath = json.get("fireSound", "").asString();

	if (!fireSoundPath.empty())
	{
		fireSound = loader->loadSound(fireSoundPath);
	}

	auto typeStr = json.get("type", "Projectile").asString();
	auto rotTypeStr = json.get("rotationType", "custom").asString();

	if (typeStr == "Projectile") params.type = Type::Projectile;
	else if (typeStr == "Beam") params.type = Type::Beam;

	if (rotTypeStr == "Custom") params.rotationType = RotationType::Custom;
	if (rotTypeStr == "PingPong") params.rotationType = RotationType::PingPong;
	if (rotTypeStr == "EasedPingPong") params.rotationType = RotationType::EasedPingPong;
	if (rotTypeStr == "Rewind") params.rotationType = RotationType::Rewind;

	return true;
}

void WeaponResource::unload()
{
}

}
