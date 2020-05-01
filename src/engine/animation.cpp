#include "animation.h"
#include "resources/script_resource.h"
#include "sprite.h"
#include "unit.h"

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

void Animation::rewind()
{
	active = true;
	currentTime = 0.0f;
	pingPongDirection = 1.0f;
	currentRepeatCount = 0;
	triggeredKeyEvents.clear();
}

void Animation::triggerKeyEvent(AnimationKey* key, struct Sprite* sprite)
{
	triggeredKeyEvents.push_back(key);

	if (sprite && sprite->unit)
	{
		sprite->unit->onAnimationEvent(sprite, key->eventName);
	}
}

void Animation::update(f32 deltaTime)
{
	if (!active)
		return;

	timeWasHigherThanTotalTime = false;
	timeWasLowerThanZero = false;

	switch (animationResource->animationType)
	{
	case AnimationType::Normal:
		currentTime += deltaTime * timeScale * animationResource->speed;

		if (currentTime > animationResource->totalTime)
		{
			timeWasHigherThanTotalTime = true;
			currentTime = 0.0f;
			previousTrackKeys.clear();
			triggeredKeyEvents.clear();
			updateRepeatCount();
		}

		break;
	case AnimationType::Reversed:
		currentTime -= deltaTime * timeScale * animationResource->speed;

		if (currentTime < 0)
		{
			timeWasLowerThanZero = true;
			currentTime = animationResource->totalTime;
			previousTrackKeys.clear();
			triggeredKeyEvents.clear();
			updateRepeatCount();
		}

		break;
	case AnimationType::PingPong:
		currentTime += deltaTime * timeScale * pingPongDirection * animationResource->speed;

		if (currentTime < 0)
		{
			timeWasLowerThanZero = true;
			currentTime = 0.0f;
			pingPongDirection = 1.0f;
			previousTrackKeys.clear();
			triggeredKeyEvents.clear();
			updateRepeatCount();
		}
		else if (currentTime > animationResource->totalTime)
		{
			timeWasHigherThanTotalTime = true;
			currentTime = animationResource->totalTime;
			pingPongDirection = -1.0f;
			previousTrackKeys.clear();
			triggeredKeyEvents.clear();
			updateRepeatCount();
		}

		break;
	default:
		break;
	}
}

f32 Animation::animate(AnimationTrack::Type trackType, f32 defaultVal, struct Sprite* sprite)
{
	if (!animationResource->hasTrack(trackType))
		return defaultVal;

	return animationResource->tracks[trackType]->animate(currentTime, this, sprite);
}

void Animation::animateSprite(Sprite* spr)
{
	spr->position.x = animate(AnimationTrack::Type::PositionX, spr->position.x, spr);
	spr->position.y = animate(AnimationTrack::Type::PositionY, spr->position.y, spr);
	spr->rotation = animate(AnimationTrack::Type::Rotation, spr->rotation, spr);
	spr->scale = animate(AnimationTrack::Type::Scale, spr->scale, spr);
	spr->verticalFlip = animate(AnimationTrack::Type::VerticalFlip, spr->verticalFlip, spr);
	spr->horizontalFlip = animate(AnimationTrack::Type::HorizontalFlip, spr->horizontalFlip, spr);
	spr->visible = animate(AnimationTrack::Type::Visible, spr->visible, spr);
	spr->color.r = animate(AnimationTrack::Type::ColorR, spr->color.r, spr);
	spr->color.g = animate(AnimationTrack::Type::ColorG, spr->color.g, spr);
	spr->color.b = animate(AnimationTrack::Type::ColorB, spr->color.b, spr);
	spr->color.a = animate(AnimationTrack::Type::ColorA, spr->color.a, spr);
	spr->colorMode = (ColorMode)(u32)animate(AnimationTrack::Type::ColorMode, (f32)spr->colorMode, spr);
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
