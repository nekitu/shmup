#include "unit_resource.h"
#include "graphics.h"
#include "game.h"
#include "image_atlas.h"
#include "resource_loader.h"
#include "utils.h"
#include "sprite.h"
#include "resources/sprite_resource.h"
#include "resources/sound_resource.h"
#include "animation.h"
#include "resources/weapon_resource.h"

namespace engine
{
bool UnitResource::load(Json::Value& json)
{
	name = json["name"].asString();
	auto typeStr = json["type"].asString();

	if (typeStr == "Enemy")
		unitType = UnitType::Enemy;

	if (typeStr == "Item")
		unitType = UnitType::Item;

	if (typeStr == "Player")
		unitType = UnitType::Player;

	if (typeStr == "EnemyProjectile")
		unitType = UnitType::EnemyProjectile;

	if (typeStr == "PlayerProjectile")
		unitType = UnitType::PlayerProjectile;

	speed = json.get("speed", speed).asFloat();
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
	if (autoDeleteTypeStr == "OffscreenBoundary") autoDeleteType = AutoDeleteType::OffscreenBoundary;

	// load controllers
	auto controllersJson = json.get("controllers", Json::Value(Json::ValueType::arrayValue));
	for (u32 i = 0; i < controllersJson.size(); i++)
	{
		ControllerInstanceResource ctrl;

		ctrl.json = controllersJson[i];
		ctrl.script = Game::instance->resourceLoader->loadScript(ctrl.json.get("script", "").asString());
		controllers[ctrl.json.get("name", "noname").asString()] = ctrl;
	}

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

	// load sprites
	auto spritesJson = json.get("sprites", Json::Value());

	for (u32 i = 0; i < spritesJson.size(); i++)
	{
		SpriteInstanceResource* spr = new SpriteInstanceResource();
		auto& sprJson = spritesJson[i];
		auto sprRes = loader->loadSprite(sprJson.get("sprite", "").asString());

		spr->spriteResource = sprRes;
		spr->name = sprJson["name"].asString();
		spr->animationName = sprJson.get("animationName", "default").asString();
		spr->position.parse(sprJson.get("position", "0 0").asString());
		spr->rotation = sprJson.get("rotation", 0).asInt();
		spr->scale.parse(sprJson.get("scale", "1 1").asString());
		spr->horizontalFlip = sprJson.get("horizontalFlip", false).asBool();
		spr->verticalFlip = sprJson.get("verticalFlip", false).asBool();
		spr->orderIndex = i;
		spr->collide = sprJson.get("collide", true).asBool();
		spr->shadow = sprJson.get("shadow", spr->shadow).asBool();
		spr->visible = sprJson.get("visible", true).asBool();
		spr->health = sprJson.get("health", 100.0f).asFloat();
		spr->color.parse(sprJson.get("color", spr->color.toString()).asString());
		spr->relativeToRoot = sprJson.get("relativeToRoot", spr->relativeToRoot).asBool();
		auto colMode = json.get("colorMode", "Add").asString();

		if (colMode == "Add") spr->colorMode = ColorMode::Add;
		if (colMode == "Sub") spr->colorMode = ColorMode::Sub;
		if (colMode == "Mul") spr->colorMode = ColorMode::Mul;

		spr->hitColor.parse(sprJson.get("hitColor", spr->hitColor.toString()).asString());

		// load weapons for this sprite
		auto weaponsJson = sprJson.get("weapons", Json::Value());
		for (int j = 0; j < weaponsJson.getMemberNames().size(); j++)
		{
			auto weaponName = weaponsJson.getMemberNames()[j];
			auto weaponJson = weaponsJson[weaponName];

			WeaponInstanceResource* weaponInstRes = new WeaponInstanceResource();

			weaponInstRes->attachTo = spr;
			weaponInstRes->localPosition.parse(weaponJson.get("position", "0 0").asString());
			weaponInstRes->weaponResource = loader->loadWeapon(weaponJson.get("weapon", "").asString());
			weaponInstRes->ammo = weaponJson.get("ammo", weaponInstRes->weaponResource->params.ammo).asFloat();
			weaponInstRes->active = weaponJson.get("active", weaponInstRes->active).asBool();
			weapons[weaponName] = weaponInstRes;
		}

		sprites[spr->name] = spr;
	}

	// load sprite animations
	auto animsJson = json.get("animations", Json::Value());

	for (int j = 0; j < animsJson.getMemberNames().size(); j++)
	{
		auto animName = animsJson.getMemberNames()[j];
		auto animJson = animsJson[animName];

		// load each sprite animation
		for (int k = 0; k < animJson.getMemberNames().size(); k++)
		{
			auto sprName = animJson.getMemberNames()[k];
			auto filename = animJson[sprName].asString();

			if (sprites.find(sprName) != sprites.end())
			{
				sprites[sprName]->animations[animName] = loader->loadAnimation(filename);
			}
		}
	}

	return true;
}

void UnitResource::unload()
{
	//TODO
}

}
