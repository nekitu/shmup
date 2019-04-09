#include "types.h"

namespace engine
{
struct UnitInstance
{
	struct SpriteInstanceAnimation
	{
		SpriteInstance* spriteInstance = nullptr;
		struct AnimationInstance* animationInstance = nullptr;
	};

	typedef std::vector<SpriteInstanceAnimation> SpriteInstanceAnimations;

	struct UnitResource* unit = nullptr;
	std::vector<SpriteInstance*> spriteInstances;
	std::unordered_map<std::string /*animation name*/, SpriteInstanceAnimations> spriteInstanceAnimations;
	std::string currentSpriteInstanceAnimation;
	std::string name;
	UnitResource::Type type = UnitResource::Type::Enemy;
	Team team = Team::Enemies;
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

}