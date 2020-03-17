#pragma once
#include "types.h"
#include "resources/animation_resource.h"
#include <unordered_map>
#include "lua_scripting.h"

namespace engine
{
struct AnimationInstance
{
	AnimationResource* animation = nullptr;
	f32 timeScale = 1.0f;
	f32 currentTime = 0.0f;
	u32 currentRepeatCount = 0;
	f32 pingPongDirection = 1.0f;
	bool active = true;
	std::vector<AnimationKey*> triggeredKeyEvents;
	std::unordered_map<AnimationTrack*, u32> previousTrackKeys;
	struct ScriptClassInstance* scriptClass = nullptr;
	std::string keyEventScriptCallback;

	void copyFrom(AnimationInstance* other);
	void initializeFrom(AnimationResource* res);
	void reset();
	void triggerKeyEvent(AnimationKey* key);
	void update(f32 deltaTime);
	f32 animate(AnimationTrack::Type trackType);
	void animateSpriteInstance(struct SpriteInstance* sprInst);

protected:
	void updateRepeatCount();
};

}
