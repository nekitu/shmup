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
	f32 damageDamping = 1.0f;
	Color color = Color::black;
	ColorMode colorMode = ColorMode::Add;
	Color hitColor = Color::red;
	std::map<std::string /*anim name*/, struct AnimationResource*> animations;
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
		Player,
		Enemy,
		Item,
		PlayerProjectile,
		EnemyProjectile,

		Count
	};

	std::string name;
	Type type = Type::Enemy;
	f32 speed = 10.0f;
	bool visible = true;
	std::string rootSpriteInstanceName;
	struct ScriptResource* scriptResource = nullptr;
	std::map<std::string /*sprite instance name*/, SpriteInstanceResource*> spriteInstances;
	std::vector<UnitLifeStage> stages;

	virtual bool load(Json::Value& json) override;
};

}
