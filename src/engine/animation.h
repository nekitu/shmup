#pragma once
#include "types.h"
#include "resources/animation_resource.h"
#include <unordered_map>
#include "lua_scripting.h"

namespace engine
{
struct Animation
{
	struct TrackState
	{
		u32 repeatCount = 0;
		f32 playDirection = 1;
		bool timeWasHigherThanTotalTime = false;
		bool timeWasLowerThanZero = false;
		u32 previousKeyIndex = 0;
		bool active = true;
		f32 currentTime = 0.0f;
		std::vector<AnimationKey*> triggeredKeyEvents;
	};

	AnimationResource* animationResource = nullptr;
	f32 timeScale = 1.0f;
	bool active = true;
	f32 currentTime = 0.0f;
	u32 repeatCount = 0;
	f32 playDirection = 1;
	std::unordered_map<AnimationTrack*, TrackState> trackState;
	struct Unit* unit = nullptr;

	void copyFrom(Animation* other);
	void initializeFrom(AnimationResource* res);
	void rewind();
	virtual void triggerKeyEvent(TrackState& tstate, AnimationKey* key, struct Sprite* sprite);
	void update(f32 deltaTime);
	f32 animate(AnimationTrackType trackType, f32 defaultVal, struct Sprite* sprite);
	void animateSprite(struct Sprite* spr);

protected:
	f32 animateTrack(struct AnimationTrack* track, f32 atTime, struct Sprite* sprite);
	void updateRepeatCount();
	void updateTrackRepeatCount(struct AnimationTrack* track, TrackState& tstate);
};

}
