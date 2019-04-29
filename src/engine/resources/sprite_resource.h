#pragma once
#include "types.h"
#include <string>
#include "rect.h"
#include "resource.h"
#include <unordered_map>

namespace engine
{
struct SpriteAnimation
{
	enum class Type
	{
		Normal,
		Reversed,
		PingPong
	};

	std::string name;
	u32 startFrame = 0;
	u32 frameCount = 0;
	Type type = Type::Normal;
	u32 framesPerSecond = 1;
	u32 repeatCount = 0; /// zero means infinite
};

struct SpriteAnimationInstance
{
	SpriteAnimation* spriteAnimation = nullptr;
	f32 currentFrame = 0;
	u32 currentRepeatCount = 0;
	f32 direction = 1;
	bool active = true;

	void play()
	{
		direction = 1;
		if (!spriteAnimation) return;
		active = true;
		currentFrame = spriteAnimation->startFrame;
	}
};

struct SpriteResource : Resource
{
	struct AtlasImage* image = nullptr;
	struct ImageAtlas* atlas = nullptr;
	u32 frameCount = 0;
	u32 frameWidth = 0;
	u32 frameHeight = 0;
	f32 uvFrameWidth = 0;
	f32 uvFrameHeight = 0;
	std::unordered_map<std::string, SpriteAnimation*> animations;

	struct AtlasImage* loadImage(const std::string& filename);
	Rect getFrameUvRect(u32 frame);
	bool load(Json::Value& json) override;
};

}