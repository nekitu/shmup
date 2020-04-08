#include "animation.h"
#include "resources/script_resource.h"
#include "sprite.h"

namespace engine
{
void Animation::copyFrom(Animation* other)
{
	animationResource = other->animationResource;
	timeScale = other->timeScale;
	currentTime = other->currentTime;
	currentRepeatCount = other->currentRepeatCount;
	pingPongDirection = other->pingPongDirection;
	active = other->active;
	triggeredKeyEvents = other->triggeredKeyEvents;
	previousTrackKeys = other->previousTrackKeys;
	scriptClass = other->scriptClass;
	keyEventScriptCallback = other->keyEventScriptCallback;
}

void Animation::initializeFrom(AnimationResource* res)
{
	animationResource = res;
	currentTime = 0;
	currentRepeatCount = 0;
	pingPongDirection = 1;
	triggeredKeyEvents.clear();
	previousTrackKeys.clear();
}

void Animation::reset()
{
	active = true;
	currentTime = 0.0f;
	pingPongDirection = 1.0f;
	currentRepeatCount = 0;
	triggeredKeyEvents.clear();
}

void Animation::triggerKeyEvent(AnimationKey* key)
{
	triggeredKeyEvents.push_back(key);
	CALL_LUA_FUNC(keyEventScriptCallback, key->eventName, key->time, key->value);
}

void Animation::update(f32 deltaTime)
{
	if (!active)
		return;

	switch (animationResource->animationType)
	{
	case AnimationType::Normal:
		currentTime += deltaTime * timeScale * animationResource->speed;

		if (currentTime > animationResource->totalTime)
		{
			currentTime = 0.0f;
			updateRepeatCount();
		}

		break;
	case AnimationType::Reversed:
		currentTime -= deltaTime * timeScale * animationResource->speed;

		if (currentTime < 0)
		{
			currentTime = animationResource->totalTime;
			updateRepeatCount();
		}

		break;
	case AnimationType::PingPong:
		currentTime += deltaTime * timeScale * pingPongDirection * animationResource->speed;

		if (currentTime < 0)
		{
			currentTime = 0.0f;
			pingPongDirection = 1.0f;
			updateRepeatCount();
		}

		if (currentTime > animationResource->totalTime)
		{
			currentTime = animationResource->totalTime;
			pingPongDirection = -1.0f;
			updateRepeatCount();
		}

		break;
	default:
		break;
	}
}

f32 Animation::animate(AnimationTrack::Type trackType)
{
	if (!animationResource->tracks[trackType])
		return 0;

	return animationResource->tracks[trackType]->animate(currentTime, this);
}

void Animation::animateSprite(Sprite* spr)
{
	spr->position.x = animate(AnimationTrack::Type::PositionX);
	spr->position.y = animate(AnimationTrack::Type::PositionY);
	spr->rotation = animate(AnimationTrack::Type::Rotation);
	spr->scale = animate(AnimationTrack::Type::Scale);
	spr->verticalFlip = animate(AnimationTrack::Type::VerticalFlip);
	spr->horizontalFlip = animate(AnimationTrack::Type::HorizontalFlip);
	spr->visible = animate(AnimationTrack::Type::Visible);
	spr->color.r = animate(AnimationTrack::Type::ColorR);
	spr->color.g = animate(AnimationTrack::Type::ColorG);
	spr->color.b = animate(AnimationTrack::Type::ColorB);
	spr->color.a = animate(AnimationTrack::Type::ColorA);
	spr->colorMode = (ColorMode)(u32)animate(AnimationTrack::Type::ColorMode);
}

void Animation::updateRepeatCount()
{
	if (animationResource->repeatCount)
	{
		currentRepeatCount++;

		if (currentRepeatCount >= animationResource->repeatCount)
		{
			active = false;
		}
	}
}

}
