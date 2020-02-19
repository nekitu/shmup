#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "types.h"
#include "vec2.h"
#include "color.h"
#include "transform.h"
#include "rect.h"
#include "resource.h"
#include "resources/sprite_resource.h"

namespace engine
{
struct SpriteInstanceResource
{
	struct SpriteResource* sprite = nullptr;
	std::string name;
	std::string animationName = "default";
	Transform transform;
	u32 orderIndex = 0;
	bool collide = true;
	bool visible = true;
	bool shadow = true;
	f32 damageScale = 1.0f;
	Color color = Color::black;
	ColorMode colorMode = ColorMode::Add;
	Color hitColor = Color::red;
	std::map<std::string /*anim name*/, struct AnimationResource*> animations;
};

struct WeaponInstanceResource
{
	SpriteInstanceResource* attachTo = nullptr;
	struct WeaponResource* weapon = nullptr;
	Vec2 localPosition;
	f32 ammo = 100;
	bool active = true;
};

struct UnitLifeStage
{
	f32 healthPercent = 100;
	std::string introAnimationName;
	std::string outroAnimationName;
	std::string introFunction; // called once
	std::string outroFunction; // called once
	std::string updateFunction; // called every frame
};

struct UnitResource : Resource
{
	enum class Type
	{
		Unknown,
		Player,
		Enemy,
		Item,
		PlayerProjectile,
		EnemyProjectile,

		Count
	};

	std::string name;
	Type type = Type::Unknown;
	f32 speed = 10.0f;
	f32 shadowScale = 1.0f;
	f32 parallaxScale = 1.0f;
	bool shadow = false;
	Vec2 shadowOffset;
	bool visible = true;
	bool deleteOnOutOfScreen = false;
	bool collide = true;
	std::string rootSpriteInstanceName;
	Json::Value controllersJson;
	struct ScriptResource* script = nullptr;
	std::map<std::string /*sprite instance name*/, SpriteInstanceResource*> spriteInstances;
	std::vector<UnitLifeStage> stages;
	std::map<std::string /*weapon name*/, WeaponInstanceResource*> weapons;

	virtual bool load(Json::Value& json) override;
};

}
