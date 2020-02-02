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
	visible = json.get("visible", visible).asBool();
	rootSpriteInstanceName = json.get("rootSpriteInstance", "").asString();

	// load stages
	auto stagesJson = json.get("stages", Json::Value(Json::ValueType::arrayValue));
	for (u32 i = 0; i < stagesJson.size(); i++)
	{
		UnitLifeStage stage;
		auto& stageJson = stagesJson[i];

		stage.healthPercent = stageJson.get("healthPercent", 100.0f).asFloat();
		stage.introAnimationName = stageJson.get("introAnimation", "").asString();
		stage.outroAnimationName = stageJson.get("outroAnimation", "").asString();
		stage.introFunction = stageJson.get("introFunction", "").asString();
		stage.outroFunction = stageJson.get("outroFunction", "").asString();
		stage.updateFunction = stageJson.get("updateFunction", "").asString();
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
		sprInst->transform.position.parse(sprJson.get("position", "0 0").asString());
		sprInst->transform.rotation = sprJson.get("rotation", 0).asInt();
		sprInst->transform.scale = sprJson.get("scale", 1.0f).asFloat();
		sprInst->transform.horizontalFlip = sprJson.get("horizontalFlip", false).asBool();
		sprInst->transform.verticalFlip = sprJson.get("verticalFlip", false).asBool();
		sprInst->orderIndex = i;
		sprInst->collide = sprJson.get("collide", true).asBool();
		sprInst->damageDamping = sprJson.get("damageDamping", 1.0f).asFloat();
		sprInst->color.parse(sprJson.get("color", sprInst->color.toString()).asString());
		auto colMode = json.get("colorMode", "Add").asString();

		if (colMode == "Add") sprInst->colorMode = ColorMode::Add;
		if (colMode == "Sub") sprInst->colorMode = ColorMode::Sub;
		if (colMode == "Mul") sprInst->colorMode = ColorMode::Mul;

		sprInst->hitColor.parse(sprJson.get("hitColor", sprInst->hitColor.toString()).asString());
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
