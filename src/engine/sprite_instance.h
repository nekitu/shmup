#pragma once
#include <string>
#include "types.h"
#include "vec2.h"
#include "transform.h"
#include "sprite.h"

namespace engine
{
struct SpriteInstance
{
	std::string name;
	struct Sprite* sprite = nullptr;
	Transform transform;
	SpriteAnimationInstance spriteAnimationInstance;

	void update(struct Game* game);
	void setAnimation(const std::string& name);
};

}
