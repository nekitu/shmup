#include "animation_instance.h"
#include "resources/script_resource.h"
#include "sprite_instance.h"

namespace engine
{
void AnimationInstance::copyFrom(AnimationInstance* other)
{
	animation = other->animation;
	timeScale = other->timeScale;
	currentTime = other->currentTime;
	currentRepeatCount = other->currentRepeatCount;
	pingPongDirection = other->pingPongDirection;
	active = other->active;
	triggeredKeyEvents = other->triggeredKeyEvents;
	previousTrackKeys = other->previousTrackKeys;
	scriptResource = other->scriptResource;
	keyEventScriptCallback = other->keyEventScriptCallback;
}

void AnimationInstance::initializeFrom(AnimationResource* res)
{
	animation = res;
	currentTime = 0;
	currentRepeatCount = 0;
	pingPongDirection = 1;
	triggeredKeyEvents.clear();
	previousTrackKeys.clear();
}

void AnimationInstance::reset()
{
	active = true;
	currentTime = 0.0f;
	pingPongDirection = 1.0f;
	currentRepeatCount = 0;
	triggeredKeyEvents.clear();
}

void AnimationInstance::triggerKeyEvent(AnimationKey* key)
{
	triggeredKeyEvents.push_back(key);

	if (scriptResource)
	{
		auto func = scriptResource->getFunction(keyEventScriptCallback);
		if (func.isFunction()) func.call(key->eventName, key->time, key->value);
	}
}

void AnimationInstance::update(f32 deltaTime)
{
	if (!active)
		return;

	switch (animation->type)
	{
	case AnimationResource::Type::Normal:
		currentTime += deltaTime * timeScale * animation->speed;

		if (currentTime > animation->totalTime)
		{
			currentTime = 0.0f;
			updateRepeatCount();
		}

		break;
	case AnimationResource::Type::Reversed:
		currentTime -= deltaTime * timeScale * animation->speed;

		if (currentTime < 0)
		{
			currentTime = animation->totalTime;
			updateRepeatCount();
		}

		break;
	case AnimationResource::Type::PingPong:
		currentTime += deltaTime * timeScale * pingPongDirection * animation->speed;

		if (currentTime < 0)
		{
			currentTime = 0.0f;
			pingPongDirection = 1.0f;
			updateRepeatCount();
		}
		
		if (currentTime > animation->totalTime)
		{
			currentTime = animation->totalTime;
			pingPongDirection = -1.0f;
			updateRepeatCount();
		}

		break;
	default:
		break;
	}
}

f32 AnimationInstance::animate(AnimationTrack::Type trackType)
{
	if (!animation->tracks[trackType])
		return 0;

	return animation->tracks[trackType]->animate(currentTime, this);
}

void AnimationInstance::animateSpriteInstance(SpriteInstance* sprInst)
{
	sprInst->transform.position.x = animate(AnimationTrack::Type::PositionX);
	sprInst->transform.position.y = animate(AnimationTrack::Type::PositionY);
	sprInst->transform.rotation = animate(AnimationTrack::Type::Rotation);
	sprInst->transform.scale = animate(AnimationTrack::Type::Scale);
	sprInst->transform.verticalFlip = animate(AnimationTrack::Type::VerticalFlip);
	sprInst->transform.horizontalFlip = animate(AnimationTrack::Type::HorizontalFlip);
	sprInst->visible = animate(AnimationTrack::Type::Visible);
	sprInst->color.r = animate(AnimationTrack::Type::ColorR);
	sprInst->color.g = animate(AnimationTrack::Type::ColorG);
	sprInst->color.b = animate(AnimationTrack::Type::ColorB);
	sprInst->color.a = animate(AnimationTrack::Type::ColorA);
	sprInst->colorMode = (ColorMode)(u32)animate(AnimationTrack::Type::ColorMode);
}

void AnimationInstance::updateRepeatCount()
{
	if (animation->repeatCount)
	{
		currentRepeatCount++;

		if (currentRepeatCount >= animation->repeatCount)
		{
			active = false;
		}
	}
}

}
