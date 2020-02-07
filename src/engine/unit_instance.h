#pragma once
#include "types.h"
#include <unordered_map>
#include "resources/unit_resource.h"

namespace engine
{
struct UnitInstance
{
	// members related to unit resource instantiation
	typedef std::map<struct SpriteInstance*, struct AnimationInstance*> SpriteInstanceAnimationMap;
	std::vector<struct SpriteInstance*> spriteInstances;
	std::map<std::string /*animation name*/, SpriteInstanceAnimationMap> spriteInstanceAnimations;
	std::map<std::string /*weapon instance name*/, struct WeaponInstance*> weapons;
	SpriteInstanceAnimationMap* spriteInstanceAnimationMap = nullptr;

	// unit instance general members
	static u64 lastId;
	u64 id = 0;
	std::string name;
	std::string currentAnimationName;
	Rect boundingBox;
	bool visible = true;
	f32 speed = 10.0f;
	f32 health = 100.0f;
	f32 age = 0;
	u32 stageIndex = 0;
	bool collide = true;
	struct UnitResource* unit = nullptr;
	struct UnitController* controller = nullptr;
	struct ScriptResource* script = nullptr;
	struct SpriteInstance* rootSpriteInstance = nullptr;

	bool deleteMeNow = false;

	UnitInstance();
	//void copyFrom(UnitInstance* other);
	void initializeFrom(UnitResource* res);
	void load(struct ResourceLoader* loader, const Json::Value& json);
	void update(struct Game* game);
	void render(struct Graphics* gfx);
	void computeBoundingBox();
	void computeHealth();
	void setAnimation(const std::string& animName);

	static void updateShadowToggle();
private:
	static bool shadowToggle;
};

}
