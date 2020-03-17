#include "unit_resource.h"
#include "graphics.h"
#include "game.h"
#include "image_atlas.h"
#include "resource_loader.h"
#include "utils.h"
#include "sprite_instance.h"
#include "resources/sprite_resource.h"
#include "resources/sound_resource.h"
#include "animation_instance.h"
#include "resources/weapon_resource.h"

namespace engine
{
bool UnitResource::load(Json::Value& json)
{
	name = json["name"].asString();
	auto typeStr = json["type"].asString();

	if (typeStr == "Enemy")
		type = Type::Enemy;

	if (typeStr == "Item")
		type = Type::Item;

	if (typeStr == "Player")
		type = Type::Player;

	if (typeStr == "EnemyProjectile")
		type = Type::EnemyProjectile;

	if (typeStr == "PlayerProjectile")
		type = Type::PlayerProjectile;

	speed = json.get("speed", speed).asFloat();
	parallaxScale = json.get("parallaxScale", parallaxScale).asFloat();
	visible = json.get("visible", visible).asBool();
	collide = json.get("collide", collide).asBool();
	rootName = json.get("root", "").asString();
	script = loader->loadScript(json.get("script", "").asString());
	shadowScale = json.get("shadowScale", shadowScale).asFloat();
	shadow = json.get("shadow", shadow).asBool();
	shadowOffset.parse(json.get("shadowOffset", "0 0").asString());
	auto autoDeleteTypeStr = json.get("autoDeleteType", "EndOfScreen").asString();

	if (autoDeleteTypeStr == "None") autoDeleteType = AutoDeleteType::None;
	if (autoDeleteTypeStr == "OutOfScreen") autoDeleteType = AutoDeleteType::OutOfScreen;
	if (autoDeleteTypeStr == "EndOfScreen") autoDeleteType = AutoDeleteType::EndOfScreen;

	// load stages
	auto stagesJson = json.get("stages", Json::Value(Json::ValueType::arrayValue));

	for (u32 i = 0; i < stagesJson.size(); i++)
	{
		UnitLifeStage* stage = new UnitLifeStage();
		auto& stageJson = stagesJson[i];

		stage->name = stageJson.get("name", "").asString();
		stage->triggerOnHealth = stageJson.get("triggerOnHealth", 100.0f).asFloat();
		stages.push_back(stage);
	}

	// load sprite instances
	auto spriteInstancesJson = json.get("spriteInstances", Json::Value());

	for (u32 i = 0; i < spriteInstancesJson.size(); i++)
	{
		SpriteInstanceResource* sprInst = new SpriteInstanceResource();
		auto& sprJson = spriteInstancesJson[i];
		auto sprRes = loader->loadSprite(sprJson.get("sprite", "").asString());

		sprInst->sprite = sprRes;
		sprInst->name = sprJson["name"].asString();
		sprInst->animationName = sprJson.get("animationName", "default").asString();
		sprInst->position.parse(sprJson.get("position", "0 0").asString());
		sprInst->rotation = sprJson.get("rotation", 0).asInt();
		sprInst->scale = sprJson.get("scale", 1.0f).asFloat();
		sprInst->horizontalFlip = sprJson.get("horizontalFlip", false).asBool();
		sprInst->verticalFlip = sprJson.get("verticalFlip", false).asBool();
		sprInst->orderIndex = i;
		sprInst->collide = sprJson.get("collide", true).asBool();
		sprInst->shadow = sprJson.get("shadow", sprInst->shadow).asBool();
		sprInst->visible = sprJson.get("visible", true).asBool();
		sprInst->health = sprJson.get("health", 100.0f).asFloat();
		sprInst->maxHealth = sprJson.get("maxHealth", 100.0f).asFloat();
		sprInst->color.parse(sprJson.get("color", sprInst->color.toString()).asString());
		auto colMode = json.get("colorMode", "Add").asString();

		if (colMode == "Add") sprInst->colorMode = ColorMode::Add;
		if (colMode == "Sub") sprInst->colorMode = ColorMode::Sub;
		if (colMode == "Mul") sprInst->colorMode = ColorMode::Mul;

		sprInst->hitColor.parse(sprJson.get("hitColor", sprInst->hitColor.toString()).asString());

		// load weapons for this sprite instance
		auto weaponsJson = sprJson.get("weapons", Json::Value());
		for (int j = 0; j < weaponsJson.getMemberNames().size(); j++)
		{
			auto weaponName = weaponsJson.getMemberNames()[j];
			auto weaponJson = weaponsJson[weaponName];

			WeaponInstanceResource* weaponInstRes = new WeaponInstanceResource();
			weaponInstRes->attachTo = sprInst;
			weaponInstRes->localPosition.parse(weaponJson.get("position", "0 0").asString());
			weaponInstRes->weapon = loader->loadWeapon(weaponJson.get("weapon", "").asString());
			weaponInstRes->ammo = weaponJson.get("ammo", weaponInstRes->weapon->params.ammo).asFloat();
			weaponInstRes->active = weaponJson.get("active", weaponInstRes->active).asBool();
			weapons[weaponName] = weaponInstRes;
		}

		spriteInstances[sprInst->name] = sprInst;
	}

	// load sprite instance animations
	auto animsJson = json.get("animations", Json::Value());

	for (int j = 0; j < animsJson.getMemberNames().size(); j++)
	{
		auto animName = animsJson.getMemberNames()[j];
		auto animJson = animsJson[animName];

		// load each sprite instance animation
		for (int k = 0; k < animJson.getMemberNames().size(); k++)
		{
			auto sprInstName = animJson.getMemberNames()[k];
			auto filename = animJson[sprInstName].asString();
			spriteInstances[sprInstName]->animations[animName] = loader->loadAnimation(filename);
		}
	}

	return true;
}
}
