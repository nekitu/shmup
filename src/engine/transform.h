#pragma once
#include "types.h"
#include "vec2.h"

namespace engine
{
struct Transform
{
	Vec2 position;
	f32 scale = 1.0f;
	bool verticalFlip = false;
	bool horizontalFlip = false;
	i32 rotation = 0; /// rotation of -90 or 90 degrees
};
}