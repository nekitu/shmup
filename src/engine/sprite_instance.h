#pragma once
#include <string>
#include "types.h"
#include "vec2.h"
#include "transform.h"
#include "resources/sprite_resource.h"

namespace engine
{
struct SpriteInstance
{
	std::string name;
	struct SpriteResource* sprite = nullptr;
	Transform transform;
	SpriteAnimationInstance spriteAnimationInstance;

	void update(struct Game* game);
	void setAnimation(const std::string& name);
};

}
