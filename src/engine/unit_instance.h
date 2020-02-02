#pragma once
#include "types.h"
#include <unordered_map>
#include "resources/unit_resource.h"

namespace engine
{
struct UnitInstance
{
	typedef std::map<struct SpriteInstance*, struct AnimationInstance*> SpriteInstanceAnimationMap;
	struct UnitResource* unit = nullptr;
	std::vector<struct SpriteInstance*> spriteInstances;
	std::map<std::string /*animation name*/, SpriteInstanceAnimationMap> spriteInstanceAnimations;
	std::vector<struct WeaponInstance*> weapons;
	SpriteInstanceAnimationMap* spriteInstanceAnimationMap = nullptr;
	std::string currentAnimationName;
	std::string name;
	UnitResource::Type type = UnitResource::Type::Enemy;
	struct SpriteInstance* rootSpriteInstance = nullptr;
	Rect boundingBox;
	bool hasShadows = false;
	bool visible = true;
	bool deleteOnOutOfScreen = false;
	bool deleteMeNow = false;
	f32 speed = 10.0f;
	f32 shadowScale = 1.0f;
	Vec2 shadowOffset;
	struct UnitController* controller = nullptr;
	struct ScriptResource* script = nullptr;

	void copyFrom(UnitInstance* other);
	void instantiateFrom(UnitResource* res);
	void update(struct Game* game);
	void render(struct Graphics* gfx);
	void computeBoundingBox();
	void setAnimation(const std::string& animName);

	static void updateShadowToggle();
private:
	static bool shadowToggle;
};

}
