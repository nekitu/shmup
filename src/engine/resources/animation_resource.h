#pragma once
#include <vector>
#include <string>
#include <unordered_map>
#include "types.h"
#include "vec2.h"
#include "color.h"
#include "rect.h"
#include "resource.h"
#include "easing.h"

namespace engine
{
enum class AnimationLoopMode
{
	None,
	Normal,
	Reversed,
	PingPong
};

enum class AnimationTrackType
{
	Unknown = 0,
	PositionX,
	PositionY,
	ScaleX,
	ScaleY,
	UniformScale,
	VerticalFlip,
	HorizontalFlip,
	Rotation,
	Visible,
	Shadow,
	ShadowOffsetX,
	ShadowOffsetY,
	ShadowScaleX,
	ShadowScaleY,
	ShadowUniformScale,
	ColorR,
	ColorG,
	ColorB,
	ColorA,
	ColorMode,

	Count
};

enum class AnimationCoordSys
{
	World,
	Local,
	Screen,

	Count
};

struct AnimationKey
{
	f32 value = 0;
	f32 time; /// time is seconds
	bool triggerEvent = false;
	std::string eventName;
	Easing::Type easeType = Easing::Type::inOutLinear;
};

struct AnimationTrack
{
	u32 repeat = 0;
	f32 totalTime = 0;
	f32 startTime = 0;
	f32 endTime = 0;
	AnimationLoopMode loopMode = AnimationLoopMode::Normal;
	std::vector<AnimationKey> keys;

	void computeTotalTime()
	{
		if (keys.empty())
			return;

		if (keys.size() == 1)
		{
			totalTime = startTime = endTime = keys.back().time;
			return;
		}

		startTime = keys.front().time;
		endTime = keys.back().time;
		totalTime = endTime - startTime;
	}
};

struct AnimationResource : Resource
{
	std::map<AnimationTrackType, AnimationTrack*> tracks;
	f32 speed = 1.0f;
	f32 totalTime = 0;
	u32 repeat = 0;
	AnimationLoopMode loopMode = AnimationLoopMode::Normal;
	AnimationCoordSys coordSys = AnimationCoordSys::World;

	virtual bool load(Json::Value& json) override;
	void unload() override;
	bool hasTrack(AnimationTrackType trackType);
};

}
