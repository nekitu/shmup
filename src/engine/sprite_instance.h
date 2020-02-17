#pragma once
#include <string>
#include "types.h"
#include "vec2.h"
#include "transform.h"
#include "resources/sprite_resource.h"
#include "color.h"

namespace engine
{
struct SpriteInstance
{
	std::string name;
	struct SpriteResource* sprite = nullptr;
	Transform transform;
	Vec2 cameraSpacePosition;
	u32 orderIndex = 0;
	bool visible = true;
	bool collide = true;
	f32 health = 100;
	Color defaultColor = Color::black;
	Color color = Color::black;
	ColorMode colorMode = ColorMode::Add;
	Rect rect;
	bool noRootParent = false;

	Color hitColor = Color::red;
	ColorMode hitOldColorMode = ColorMode::Add;
	f32 hitColorFlashSpeed = 20.0f;
	f32 hitFlashCount = 5;
	bool hitFlashActive = false;
	f32 hitColorTimer = 0.0f;
	f32 currentHitFlashCount = 0;

	SpriteFrameAnimation* frameAnimation = nullptr;
	f32 animationFrame = 0;
	u32 animationRepeatCount = 0;
	f32 animationDirection = 1;
	bool animationIsActive = true;

	void copyFrom(SpriteInstance* other);
	void initializeFrom(struct SpriteInstanceResource* res);
	void update(struct Game* game);
	void setFrameAnimation(const std::string& name);
	void play();
	void hit(f32 hitDamage);
	bool checkPixelCollision(SpriteInstance* other, Vec2& outCollisionCenter);
};

}
