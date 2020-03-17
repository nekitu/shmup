#pragma once
#include "types.h"
#include <unordered_map>
#include "resources/unit_resource.h"
#include "lua_scripting.h"

namespace engine
{
struct SpriteInstanceCollision
{
	struct SpriteInstance* a = nullptr;
	struct SpriteInstance* b = nullptr;
	Vec2 collisionCenter;
};

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
	u32 layerIndex = 0;
	std::string name;
	std::string currentAnimationName;
	Rect boundingBox;
	bool visible = true;
	bool appeared = false;
	f32 speed = 10.0f;
	f32 health = 100.0f; // auto computed, always a percentage
	f32 maxHealth = 100.0f; // auto computed from sprite instances health
	f32 age = 0;
	u32 stageIndex = 0;
	bool collide = true;
	bool shadow = false;
	struct UnitResource* unit = nullptr;
	struct ScriptClassInstance* scriptClass = nullptr;
	struct SpriteInstance* root = nullptr;
	struct UnitLifeStage* currentStage = nullptr;
	std::vector<UnitLifeStage*> triggeredStages;
	bool deleteMeNow = false;

	UnitInstance();
	void copyFrom(UnitInstance* other);
	void initializeFrom(UnitResource* res);
	virtual void load(struct ResourceLoader* loader, const Json::Value& json);
	virtual void update(struct Game* game);
	virtual void render(struct Graphics* gfx);
	virtual void computeBoundingBox();
	virtual void computeHealth();
	void setAnimation(const std::string& animName);
	struct SpriteInstance* findSpriteInstance(const std::string& sname);

	bool checkPixelCollision(struct UnitInstance* other, std::vector<SpriteInstanceCollision>& collisions);

	static void updateShadowToggle();
private:
	static bool shadowToggle;
};

}
