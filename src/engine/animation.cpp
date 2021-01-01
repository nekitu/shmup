#include "animation.h"
#include "resources/script_resource.h"
#include "sprite.h"
#include "unit.h"
#include "easing.h"

namespace engine
{
void Animation::copyFrom(Animation* other)
{
	animationResource = other->animationResource;
	timeScale = other->timeScale;
	trackState = other->trackState;
	active = other->active;
	rewind();
}

void Animation::initializeFrom(AnimationResource* res)
{
	animationResource = res;
	trackState.clear();
	rewind();
}

void Animation::rewind()
{
	active = true;
	trackState.clear();

	for (auto& trackIter : animationResource->tracks)
	{
		auto& track = trackIter.second;
		TrackState ts;

		ts.playDirection = track->loopMode == AnimationLoopMode::Reversed ? -1 : 1;
		ts.previousKeyIndex = track->loopMode == AnimationLoopMode::Reversed ? track->keys.size() - 1 : 0;

		trackState[track] = ts;
	}
}

void Animation::triggerKeyEvent(TrackState& tstate, AnimationKey* key, struct Sprite* sprite)
{
	tstate.triggeredKeyEvents.push_back(key);

	if (sprite && sprite->unit)
	{
		sprite->unit->onAnimationEvent(sprite, key->eventName);
	}
}

void Animation::updateTrackRepeatCount(AnimationTrack* track, TrackState& tstate)
{
	if (track->repeat == 0)
		return;

	if (!tstate.active)
		return;

	tstate.repeatCount++;

	if (tstate.repeatCount > track->repeat)
	{
		tstate.active = false;
	}
}

void Animation::updateRepeatCount()
{
	if (animationResource->repeat == 0)
		return;

	if (!active)
		return;

	repeatCount++;

	if (repeatCount > animationResource->repeat)
	{
		active = false;
	}
}

void Animation::update(f32 deltaTime)
{
	if (!active)
		return;

	currentTime += deltaTime * timeScale * animationResource->speed;
	switch (animationResource->loopMode)
	{
	case AnimationLoopMode::None:
		if (currentTime > animationResource->totalTime)
		{
			active = false;
			currentTime = animationResource->totalTime;
		}
		break;
	case AnimationLoopMode::Normal:
		if (currentTime > animationResource->totalTime)
		{
			currentTime = 0;
			for (auto trackIter : animationResource->tracks)
			{
				auto track = trackIter.second;
				auto& state = trackState[track];

				state.active = true;
				state.playDirection = 1;
				state.previousKeyIndex = 0;
				state.repeatCount = 0;
				state.triggeredKeyEvents.clear();
			}
			updateRepeatCount();
		}
		break;
	case AnimationLoopMode::Reversed:
		if (currentTime < 0)
		{
			currentTime = animationResource->totalTime;
			updateRepeatCount();
		}
		break;
	case AnimationLoopMode::PingPong:
		if (currentTime < 0)
		{
			currentTime = 0;
			playDirection = 1;
			updateRepeatCount();
		}
		else if (currentTime > animationResource->totalTime)
		{
			currentTime = animationResource->totalTime;
			playDirection = -1;
		}
		break;
	}

	for (auto& trackIter : animationResource->tracks)
	{
		auto& track = trackIter.second;
		auto& state = trackState[track];

		if (!state.active)
			continue;

		state.timeWasHigherThanTotalTime = false;
		state.timeWasLowerThanZero = false;

		switch (track->loopMode)
		{
		case AnimationLoopMode::None:
			state.currentTime = currentTime;

			if (state.currentTime > track->endTime)
			{
				state.timeWasHigherThanTotalTime = true;
				state.currentTime = track->endTime;
				state.triggeredKeyEvents.clear();
				state.active = false;
			}

			break;
		case AnimationLoopMode::Normal:
		{
			auto oldTime = state.currentTime;
			state.currentTime = fmod(currentTime, track->endTime);

			// we have a change in the time, we got a rewind
			if (oldTime > state.currentTime)
			{
				state.timeWasHigherThanTotalTime = true;
				state.previousKeyIndex = 0;
				state.triggeredKeyEvents.clear();
				updateTrackRepeatCount(track, state);
			}

			break;
		}
		case AnimationLoopMode::Reversed:
		{
			auto oldTime = state.currentTime;
			state.currentTime = track->endTime - fmod(currentTime, track->endTime);

			// we got a rewind
			if (state.currentTime > oldTime)
			{
				state.timeWasLowerThanZero = true;
				state.previousKeyIndex = track->keys.size() - 1;
				state.triggeredKeyEvents.clear();
				updateTrackRepeatCount(track, state);
			}

			break;
		}
		case AnimationLoopMode::PingPong:
		{
			auto oldTime = state.currentTime;

			if (state.playDirection == 1)
				state.currentTime = fmod(currentTime, track->endTime);
			else
				state.currentTime = track->endTime - fmod(currentTime, track->endTime);

			// we got rewind on forward play direction
			if (state.playDirection == 1 && state.currentTime < oldTime)
			{
				state.timeWasHigherThanTotalTime = true;
				state.playDirection = -1.0f;
				state.previousKeyIndex = track->keys.size() - 1;
				state.triggeredKeyEvents.clear();
				state.currentTime = track->endTime - fmod(currentTime, track->endTime);
				updateTrackRepeatCount(track, state);
			}
			else if (state.playDirection == -1 && state.currentTime > oldTime)
			{
				state.timeWasLowerThanZero = true;
				state.playDirection = 1.0f;
				state.previousKeyIndex = 0;
				state.triggeredKeyEvents.clear();
				state.currentTime = fmod(currentTime, track->endTime);
				updateTrackRepeatCount(track, state);
			}

			break;
		}
		default:
			break;
		}
	}
}

f32 Animation::animate(AnimationTrackType trackType, f32 defaultVal, struct Sprite* sprite)
{
	if (!animationResource->hasTrack(trackType))
		return defaultVal;

	auto track = animationResource->tracks[trackType];

	return animateTrack(animationResource->tracks[trackType], trackState[track].currentTime, sprite);
}

f32 Animation::animateTrack(AnimationTrack* track, f32 atTime, struct Sprite* sprite)
{
	auto& keys = track->keys;
	auto& tstate = trackState[track];

	if (keys.empty())
		return 0;

	if (keys.size() == 1)
	{
		//TODO event trigger
		return keys[0].value;
	}

	// wrap if time is greater
	auto oldTime = atTime;
	atTime = fmod(atTime, keys[keys.size() - 1].time); // keys always sorted by time

	if (oldTime != atTime)
	{
		if (track->loopMode == AnimationLoopMode::PingPong
			&& tstate.playDirection == -1)
		{
			atTime = keys[keys.size() - 1].time - atTime;
		}
	}

	if (tstate.timeWasLowerThanZero && track->loopMode == AnimationLoopMode::Reversed)
	{
		tstate.previousKeyIndex = keys.size() - 1;
	}

	u32 key1Index = tstate.previousKeyIndex;
	AnimationKey* key1 = &keys[key1Index];
	AnimationKey* key2 = nullptr;

	auto isTimeOverLastKeyAndHasTrigger = tstate.timeWasHigherThanTotalTime && keys[keys.size() - 1].triggerEvent;
	auto isTimeBelowFirstKeyAndHasTrigger = tstate.timeWasLowerThanZero && keys[0].triggerEvent;
	auto isForwardDir = ((tstate.playDirection == 1 && track->loopMode == AnimationLoopMode::PingPong) || track->loopMode == AnimationLoopMode::Normal || track->loopMode == AnimationLoopMode::None);

	if (isTimeBelowFirstKeyAndHasTrigger)
	{
		if (std::find(
			tstate.triggeredKeyEvents.begin(),
			tstate.triggeredKeyEvents.end(),
			&keys[0]) == tstate.triggeredKeyEvents.end())
		{
			if (track->loopMode == AnimationLoopMode::Reversed)
			{
				tstate.previousKeyIndex = keys.size() - 1;
			}
			else
				tstate.previousKeyIndex = 0;

			triggerKeyEvent(tstate, &keys[0], sprite);

			return keys[0].value;
		}
	}

	if (isTimeOverLastKeyAndHasTrigger)
	{
		if (std::find(
			tstate.triggeredKeyEvents.begin(),
			tstate.triggeredKeyEvents.end(),
			&keys[keys.size() - 1]) == tstate.triggeredKeyEvents.end())
		{
			triggerKeyEvent(tstate, &keys[keys.size() - 1], sprite);

			if (track->loopMode == AnimationLoopMode::PingPong)
				tstate.previousKeyIndex = keys.size() - 1;

			return keys[keys.size() - 1].value;
		}
	}

	if (isForwardDir)
	{
		for (i32 i = key1Index, iCount = keys.size(); i < iCount - 1; i++)
		{
			if (atTime >= keys[i].time && atTime <= keys[i + 1].time)
			{
				key1Index = i;
				key1 = &keys[i];
				key2 = &keys[i + 1];

				if (key1->triggerEvent
					&& std::find(
						tstate.triggeredKeyEvents.begin(),
						tstate.triggeredKeyEvents.end(), key1) == tstate.triggeredKeyEvents.end())
				{
					triggerKeyEvent(tstate, key1, sprite);
				}
				break;
			}
		}
	}
	else
	{
		for (i32 i = key1Index; i > 0; i--)
		{
			if (atTime >= keys[i - 1].time && atTime <= keys[i].time)
			{
				key1Index = i;
				key1 = &keys[i];
				key2 = &keys[i - 1];

				if (key1->triggerEvent
					&& std::find(
						tstate.triggeredKeyEvents.begin(),
						tstate.triggeredKeyEvents.end(), key1) == tstate.triggeredKeyEvents.end())
				{
					triggerKeyEvent(tstate, key1, sprite);
				}

				break;
			}
		}
	}

	if (key1 && key2)
	{
		// key1->time ----- atTime ----------------key2->time
		f32 t = (atTime - key1->time) / (key2->time - key1->time);

		// update first interpolation key
		tstate.previousKeyIndex = key1Index;

		return Easing::easeValue(key1->easeType, t, key1->value, key2->value - key1->value, 1.0f);

		//return key1->value + fabs(t) * (key2->value - key1->value);
	}

	return 0.0f;
}

void Animation::animateSprite(Sprite* spr)
{
	spr->position.x = animate(AnimationTrackType::PositionX, spr->position.x, spr);
	spr->position.y = animate(AnimationTrackType::PositionY, spr->position.y, spr);
	spr->rotation = animate(AnimationTrackType::Rotation, spr->rotation, spr);

	if (animationResource->hasTrack(AnimationTrackType::UniformScale))
	{
		spr->scale.x = spr->scale.y = animate(AnimationTrackType::UniformScale, spr->scale.x, spr);
	}
	else
	{
		spr->scale.x = animate(AnimationTrackType::ScaleX, spr->scale.x, spr);
		spr->scale.y = animate(AnimationTrackType::ScaleY, spr->scale.y, spr);
	}

	spr->verticalFlip = animate(AnimationTrackType::VerticalFlip, spr->verticalFlip, spr);
	spr->horizontalFlip = animate(AnimationTrackType::HorizontalFlip, spr->horizontalFlip, spr);
	spr->visible = animate(AnimationTrackType::Visible, spr->visible, spr);
	spr->color.r = animate(AnimationTrackType::ColorR, spr->color.r, spr);
	spr->color.g = animate(AnimationTrackType::ColorG, spr->color.g, spr);
	spr->color.b = animate(AnimationTrackType::ColorB, spr->color.b, spr);
	spr->color.a = animate(AnimationTrackType::ColorA, spr->color.a, spr);
	spr->colorMode = (ColorMode)(u32)animate(AnimationTrackType::ColorMode, (f32)spr->colorMode, spr);
}

}
