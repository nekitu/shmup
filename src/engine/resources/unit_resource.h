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

namespace engine
{
enum class Team
{
	Enemies,
	Neutrals,
	Players
};

struct UnitResource : Resource
{
	enum class Type
	{
		Player,
		Enemy,
		Item,
		Projectile,

		Count
	};

	struct SpriteInstanceResource
	{
		struct SpriteResource* sprite = nullptr;
		Transform transform;
	};

	struct SpriteInstanceAnimation
	{
		SpriteInstanceResource* spriteInstance = nullptr;
		struct Animation* animation = nullptr;
	};

	std::string name;
	Type type = Type::Enemy;
	Team team = Team::Enemies;
	f32 speed = 10.0f;
	bool visible = true;
	struct ScriptResource* scriptResource = nullptr;
	SpriteInstanceResource* spriteInstances = nullptr;
	u32 spriteInstanceCount = 0;
	SpriteInstanceAnimation* spriteInstanceAnimations;
	u32 spriteInstanceAnimationCount = 0;

	virtual bool load(const std::string& filename) override;
};

}