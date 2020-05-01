#include "animation_resource.h"
#include "animation.h"
#include <algorithm>

namespace engine
{
f32 AnimationTrack::animate(f32 atTime, Animation* anim, struct Sprite* sprite)
{
	if (keys.empty())
		return 0;

	if (keys.size() == 1)
	{
		//TODO event trigger
		return keys[0].value;
	}

	u32 key1Index = anim->previousTrackKeys[this];
	AnimationKey* key1 = &keys[key1Index];
	AnimationKey* key2 = nullptr;

	auto isTimeOverLastKeyAndHasTrigger = anim->timeWasHigherThanTotalTime && keys[keys.size() - 1].triggerEvent;
	auto isTimeBelowFirstKeyAndHasTrigger = anim->timeWasLowerThanZero && keys[0].triggerEvent;
	auto isForwardDir = (anim->pingPongDirection == 1 || anim->animationResource->animationType == AnimationType::Normal);

	if (isTimeBelowFirstKeyAndHasTrigger && !isForwardDir)
	{
		if (std::find(
			anim->triggeredKeyEvents.begin(),
			anim->triggeredKeyEvents.end(),
			&keys[0]) == anim->triggeredKeyEvents.end())
		{
			anim->previousTrackKeys[this] = 0;
			anim->triggerKeyEvent(&keys[0], sprite);
			return keys[0].value;
		}
	}

	if (isTimeOverLastKeyAndHasTrigger && isForwardDir)
	{
		if (std::find(
			anim->triggeredKeyEvents.begin(),
			anim->triggeredKeyEvents.end(),
			&keys[keys.size() - 1]) == anim->triggeredKeyEvents.end())
		{
			anim->triggerKeyEvent(&keys[keys.size() - 1], sprite);
			anim->previousTrackKeys[this] = keys.size() - 1;
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
						anim->triggeredKeyEvents.begin(),
						anim->triggeredKeyEvents.end(), key1) == anim->triggeredKeyEvents.end())
				{
					anim->triggerKeyEvent(key1, sprite);
				}
				break;
			}
		}
	}
	else
	{
		for (i32 i = key1Index; i > 0; i--)
		{
			if (atTime <= keys[i - 1].time && atTime >= keys[i].time)
			{
				key1Index = i;
				key1 = &keys[i];
				key2 = &keys[i - 1];

				if (key1->triggerEvent
					&& std::find(
						anim->triggeredKeyEvents.begin(),
						anim->triggeredKeyEvents.end(), key1) == anim->triggeredKeyEvents.end())
				{
					anim->triggerKeyEvent(key1, sprite);
				}
			}
		}
	}

	if (key1 && key2)
	{
		// key1->time ----- atTime ----------------key2->time
		f32 t = (atTime - key1->time) / (key2->time - key1->time);

		// update first interpolation key
		anim->previousTrackKeys[this] = key1Index;

		return key1->value + t * (key2->value - key1->value);
	}

	return 0.0f;
}

bool AnimationResource::load(Json::Value& json)
{
	speed = json.get("speed", speed).asFloat();
	repeatCount = json.get("repeatCount", repeatCount).asUInt();
	auto typeStr = json.get("type", "Normal").asString();

	if (typeStr == "Normal") animationType = AnimationType::Normal;
	if (typeStr == "Reversed") animationType = AnimationType::Reversed;
	if (typeStr == "PingPong") animationType = AnimationType::PingPong;

	auto tracksJson = json.get("tracks", Json::Value(Json::ValueType::arrayValue));

	for (u32 i = 0, iCount = tracksJson.getMemberNames().size(); i < iCount; i++)
	{
		auto track = new AnimationTrack();

		auto keyType = tracksJson.getMemberNames()[i];
		auto trackJson = tracksJson.get(keyType, Json::Value(Json::ValueType::arrayValue));
		auto trackType = AnimationTrack::Type::Unknown;

		if (keyType == "PositionX") trackType = AnimationTrack::Type::PositionX;
		if (keyType == "PositionY") trackType = AnimationTrack::Type::PositionY;
		if (keyType == "Scale") trackType = AnimationTrack::Type::Scale;
		if (keyType == "VerticalFlip") trackType = AnimationTrack::Type::VerticalFlip;
		if (keyType == "HorizontalFlip") trackType = AnimationTrack::Type::HorizontalFlip;
		if (keyType == "Rotation") trackType = AnimationTrack::Type::Rotation;
		if (keyType == "Visible") trackType = AnimationTrack::Type::Visible;
		if (keyType == "Shadow") trackType = AnimationTrack::Type::Shadow;
		if (keyType == "ShadowOffsetX") trackType = AnimationTrack::Type::ShadowOffsetX;
		if (keyType == "ShadowOffsetY") trackType = AnimationTrack::Type::ShadowOffsetY;
		if (keyType == "ShadowScaleX") trackType = AnimationTrack::Type::ShadowScaleX;
		if (keyType == "ShadowScaleY") trackType = AnimationTrack::Type::ShadowScaleY;
		if (keyType == "ColorR") trackType = AnimationTrack::Type::ColorR;
		if (keyType == "ColorG") trackType = AnimationTrack::Type::ColorG;
		if (keyType == "ColorB") trackType = AnimationTrack::Type::ColorB;
		if (keyType == "ColorA") trackType = AnimationTrack::Type::ColorA;
		if (keyType == "ColorMode") trackType = AnimationTrack::Type::ColorMode;

		for (auto& keyJson : trackJson)
		{
			AnimationKey key;

			key.time = keyJson.get("time", 0.0f).asFloat();
			key.value = keyJson.get("value", 0.0f).asFloat();
			key.triggerEvent = keyJson.get("triggerEvent", false).asBool();
			key.eventName = keyJson.get("eventName", "").asString();
			track->keys.push_back(key);
		}

		if (track->keys.size() && totalTime < track->keys[track->keys.size() - 1].time)
		{
			totalTime = track->keys[track->keys.size() - 1].time;
		}

		tracks[trackType] = track;
	}

	return true;
}

void AnimationResource::unload()
{
	for (auto& track : tracks)
	{
		delete track.second;
	}

	tracks.clear();
}

bool AnimationResource::hasTrack(AnimationTrack::Type trackType)
{
	auto iter = tracks.find(trackType);

	if (iter == tracks.end()) return false;
	if (iter->second->keys.size() == 0) return false;

	return true;
}


}
