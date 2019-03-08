#pragma once
#include "types.h"
#include <unordered_map>

namespace engine
{
struct AnimationKey
{
	f32 value = 0;
	f32 time; /// time is from 0 to 1.0f
};

struct AnimationTrack
{
	enum class Type
	{
		PositionX,
		PositionY,
		Scale,
		VerticalFlip,
		HorizontalFlip,
		Rotation,
		Visible,
		HasShadows,
		ShadowScale,
		ShadowOffset,

		Count
	};

	AnimationKey* keys = nullptr;
	u32 keyCount = 0;

	f32 animate(f32 atTime);
};

struct Animation
{
	std::unordered_map<AnimationTrack::Type, AnimationTrack*> tracks;
	f32 totalTimeSeconds = 0;
};

struct AnimationInstance
{
	enum class Type
	{
		Normal,
		Reversed,
		PingPong
	};

	Animation* animation = nullptr;
	f32 timeScale = 1.0f;
	Type type = Type::Normal;
	u32 repeatCount = 0; /// zero means infinite
	f32 currentTime = 0.0f;
	u32 currentRepeatCount = 0;
	f32 pingPongDirection = 1.0f;
	bool active = true;

	void reset();
	void update(f32 deltaTime);
	f32 animate(AnimationTrack::Type trackType);

protected:
	void updateRepeatCount();
};

}