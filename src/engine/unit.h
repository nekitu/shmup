#pragma once
#include <vector>
#include <unordered_map>
#include "types.h"
#include "vec2.h"
#include "sprite_instance.h"
#include "color.h"
#include "transform.h"
#include "rect.h"

namespace engine
{
enum class Team
{
	Enemy,
	Neutral,
	Player
};

struct Unit
{
	enum class Type
	{
		Player,
		Enemy,
		Item,
		Projectile,

		Count
	};

	struct SpriteInstanceAnimation
	{
		SpriteInstance* spriteInstance = nullptr;
		struct Animation* animation = nullptr;
	};

	SpriteInstance* spriteInstances = nullptr;
	u32 spriteInstanceCount = 0;
	SpriteInstanceAnimation* spriteInstanceAnimations;
	u32 spriteInstanceAnimationCount = 0;
	std::string name;
	Type type = Type::Enemy;
	Team team = Team::Enemy;
	f32 speed = 10.0f;
	Transform transform;
	bool visible = true;
};

struct UnitInstance
{
	struct SpriteInstanceAnimation
	{
		SpriteInstance* spriteInstance = nullptr;
		struct AnimationInstance* animationInstance = nullptr;
	};

	typedef std::vector<SpriteInstanceAnimation> SpriteInstanceAnimations;

	std::vector<SpriteInstance*> spriteInstances;
	std::unordered_map<std::string /*animation name*/, SpriteInstanceAnimations> spriteInstanceAnimations;
	std::string currentSpriteInstanceAnimation;
	std::string name;
	Unit::Type type = Unit::Type::Enemy;
	Team team = Team::Enemy;
	Color color;
	Transform transform;
	Rect boundingBox;
	bool hasShadows = false;
	bool visible = true;
	bool collide = true;
	f32 speed = 10.0f;
	f32 shadowScale = 1.0f;
	Vec2 shadowOffset;
	bool isHit = false;
	Color hitColor;
	struct UnitController* controller = nullptr;

	void update(struct Game* game);
	void render(struct Graphics* gfx);
	void computeBoundingBox();

private:
	bool shadowToggle = true;
};

struct UnitController
{
	UnitInstance* unitInstance = nullptr;

	virtual void update(struct Game* game) = 0;
};

struct BackgroundController : UnitController
{
	void update(struct Game* game) override;
};

struct SimpleEnemyController : UnitController
{
	void update(struct Game* game) override;
};

struct PlayerController : UnitController
{
	u32 playerIndex = 0;
	struct Sound* fireSound = nullptr;
	f32 fire1Rate = 15;
	f32 fire1PlayingTime = FLT_MAX;

	PlayerController();
	void update(struct Game* game) override;
};

}