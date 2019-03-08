#include "animation.h"

namespace engine
{
f32 AnimationTrack::animate(f32 atTime)
{
	for (u32 i = 0; i < keyCount; i++)
	{
		if (atTime > keys[i].time)
		{
			AnimationKey* key1 = &keys[i];
			AnimationKey* key2 = nullptr;

			// key1->time ----- atTime ----------------key2->time

			if (i < keyCount - 1)
				key2 = &keys[i + 1];

			if (key1 && key2)
			{
				f32 t = (atTime - key1->time) / (key2->time - key1->time);

				return key1->value + t * (key2->value - key1->value);
			}
		}
	}
}

void AnimationInstance::reset()
{
	active = true;
	currentTime = 0.0f;
	pingPongDirection = 1.0f;
	currentRepeatCount = 0;
}

void AnimationInstance::update(f32 deltaTime)
{
	if (!active)
		return;

	switch (type)
	{
	case Type::Normal:
		currentTime += deltaTime * timeScale;

		if (currentTime > animation->totalTimeSeconds)
		{
			currentTime = 0.0f;
			updateRepeatCount();
		}

		break;
	case Type::Reversed:
		currentTime -= deltaTime * timeScale;

		if (currentTime < 0)
		{
			currentTime = animation->totalTimeSeconds;
			updateRepeatCount();
		}

		break;
	case Type::PingPong:
		currentTime += deltaTime * timeScale * pingPongDirection;

		if (currentTime < 0)
		{
			currentTime = 0.0f;
			pingPongDirection = 1.0f;
			updateRepeatCount();
		}
		
		if (currentTime > animation->totalTimeSeconds)
		{
			currentTime = animation->totalTimeSeconds;
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

	return animation->tracks[trackType]->animate(currentTime / animation->totalTimeSeconds);
}

void AnimationInstance::updateRepeatCount()
{
	if (repeatCount)
	{
		currentRepeatCount++;

		if (currentRepeatCount >= repeatCount)
		{
			active = false;
		}
	}
}

}