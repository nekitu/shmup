#pragma once
#include "types.h"
#include "resources/animation_resource.h"
#include <unordered_map>
#include "lua_scripting.h"

namespace engine
{
struct Animation
{
	AnimationResource* animationResource = nullptr;
	f32 timeScale = 1.0f;
	f32 currentTime = 0.0f;
	u32 currentRepeatCount = 0;
	f32 pingPongDirection = 1.0f;
	bool active = true;
	bool timeWasHigherThanTotalTime = false;
	bool timeWasLowerThanZero = false;
	struct Unit* unit = nullptr;
	std::vector<AnimationKey*> triggeredKeyEvents;
	std::unordered_map<AnimationTrack*, u32> previousTrackKeys;

	void copyFrom(Animation* other);
	void initializeFrom(AnimationResource* res);
	void rewind();
	virtual void triggerKeyEvent(AnimationKey* key, struct Sprite* sprite);
	void update(f32 deltaTime);
	f32 animate(AnimationTrack::Type trackType, f32 defaultVal, struct Sprite* sprite);
	void animateSprite(struct Sprite* spr);

protected:
	void updateRepeatCount();
};

}
