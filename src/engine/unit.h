#pragma once
#include "types.h"
#include <unordered_map>
#include "resources/unit_resource.h"
#include "lua_scripting.h"

namespace engine
{
struct SpriteCollision
{
	struct Sprite* sprite1 = nullptr;
	struct Sprite* sprite2 = nullptr;
	Vec2 collisionCenter;
};

struct Unit
{
	typedef std::map<struct Sprite*, struct Animation*> SpriteAnimationMap;

	std::vector<struct Sprite*> sprites;
	std::map<std::string /*animation name*/, SpriteAnimationMap> spriteAnimations;
	std::map<std::string /*weapon name*/, struct Weapon*> weapons;
	SpriteAnimationMap* spriteAnimationMap = nullptr;

	static u64 lastId;
	u64 id = 0;
	u32 layerIndex = 0;
	std::string name;
	std::string currentAnimationName;
	Rect boundingBox;
	bool visible = true;
	bool appeared = false;
	f32 speed = 0.0f;
	f32 health = 100.0f; // auto computed, always a percentage
	f32 maxHealth = 100.0f; // auto computed from sprites health
	f32 age = 0;
	f32 maxOutOfScreenAge = 10;
	u32 stageIndex = 0;
	bool collide = true;
	bool shadow = false;
	struct UnitResource* unitResource = nullptr;
	struct ScriptClassInstanceBase* scriptClass = nullptr;
	struct Sprite* root = nullptr;
	struct UnitLifeStage* currentStage = nullptr;
	std::vector<UnitLifeStage*> triggeredStages;
	bool deleteMeNow = false;
	std::map<std::string, ScriptClassInstanceBase*> controllers;

	Unit();
	virtual ~Unit();
	void reset();
	virtual void copyFrom(Unit* other);
	virtual void initializeFrom(UnitResource* res);
	virtual void load(struct TilemapObject& object);
	virtual void update(struct Game* game);
	virtual void render(struct Graphics* gfx);
	virtual void computeBoundingBox();
	virtual void computeHealth();
	void setAnimation(const std::string& animName);
	struct Sprite* findSprite(const std::string& sname);
	void deleteSprite(struct Sprite* spr);
	void deleteSprite(const std::string& sname);
	void replaceSprite(const std::string& sname, const std::string& path);
	bool checkPixelCollision(struct Unit* other, std::vector<SpriteCollision>& collisions);
	static void updateShadowToggle();
	void onAnimationEvent(struct Sprite* sprite, const std::string& eventName);
	void hideAllSprites();
	void disableAllWeapons();
private:
	static bool shadowToggle;

	void removeSpriteAnimations(struct Sprite* spr);
};

}
