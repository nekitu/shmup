#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "types.h"
#include "vec2.h"
#include "color.h"
#include "transform.h"
#include "rect.h"
#include "resource.h"

namespace engine
{
struct AnimationKey
{
	f32 value = 0;
	f32 time; /// time is seconds
	bool triggerEvent = false;
	std::string eventName;
};

struct AnimationTrack
{
	enum class Type
	{
		Unknown,
		PositionX,
		PositionY,
		Scale,
		VerticalFlip,
		HorizontalFlip,
		Rotation,
		Visible,
		HasShadows,
		ShadowOffsetX,
		ShadowOffsetY,
		ShadowScaleX,
		ShadowScaleY,
		ColorR,
		ColorG,
		ColorB,
		ColorA,
		ColorMode,

		Count
	};

	std::vector<AnimationKey> keys;

	f32 animate(f32 atTime, struct AnimationInstance* animInstance);
};

struct AnimationResource : Resource
{
	enum class Type
	{
		Normal,
		Reversed,
		PingPong
	};

	std::map<AnimationTrack::Type, AnimationTrack*> tracks;
	f32 speed = 1.0f;
	f32 totalTime = 0.0f;
	Type type = Type::Normal;
	u32 repeatCount = 0; /// zero means infinite

	virtual bool load(Json::Value& json) override;
};

}
