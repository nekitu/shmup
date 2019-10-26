#include "unit_resource.h"
#include "graphics.h"
#include "game.h"
#include "image_atlas.h"
#include "resource_loader.h"
#include "utils.h"
#include "sprite_instance.h"
#include "resources/sprite_resource.h"
#include "resources/sound_resource.h"
#include "animation.h"

namespace engine
{
bool UnitResource::load(Json::Value& json)
{
	name = json["name"].asString();
	auto typeStr = json["type"].asString();
	
	if (typeStr == "enemy")
		type = Type::Enemy;

	if (typeStr == "item")
		type = Type::Item;

	if (typeStr == "player")
		type = Type::Player;

	if (typeStr == "projectile")
		type = Type::Projectile;

	auto teamStr = json["team"].asString();

	if (teamStr == "enemies")
		team = Team::Enemies;

	if (teamStr == "players")
		team = Team::Players;

	if (teamStr == "neutrals")
		team = Team::Neutrals;

	speed = json.get("speed", speed).asFloat();
	visible = json.get("visible", visible).asBool();

	// sprite instances
	auto spriteInstancesJson = json.get("spriteInstances", Json::Value());

	for (u32 i = 0; i < spriteInstancesJson.size(); i++)
	{
		SpriteInstanceResource* sprInst = new SpriteInstanceResource();
		auto& sprJson = spriteInstancesJson[i];
		auto sprRes = loader->loadSprite(sprJson.get("sprite", "").asString() + ".json");

		sprInst->name = sprJson["name"].asString();
		sprInst->animationName = sprJson["animationName"].asString();
		sprInst->sprite = sprRes;
		sprInst->transform.position.x = sprJson.get("positionX", 0.0f).asFloat();
		sprInst->transform.position.y = sprJson.get("positionY", 0.0f).asFloat();
		sprInst->transform.rotation = sprJson.get("rotation", 0).asInt();
		sprInst->transform.scale = sprJson.get("scale", 1.0f).asFloat();
		sprInst->transform.horizontalFlip = sprJson.get("horizontalFlip", false).asBool();
		sprInst->transform.verticalFlip = sprJson.get("verticalFlip", false).asBool();
		spriteInstances.push_back(sprInst);
	}

	// sprite instance animations
	auto spriteInstanceAnimsJson = json.get("animations", Json::Value());

	for (u32 i = 0; i < spriteInstanceAnimsJson.size(); i++)
	{
		Animation* anim = new Animation();
		auto& sprAnimJson = spriteInstanceAnimsJson[i];

		print("Anim name: %s\n", sprAnimJson->);
	}

	// resolve default animations

	return true;
}
}
