#include "animation_resource.h"
#include "animation_instance.h"
#include <algorithm>

namespace engine
{
f32 AnimationTrack::animate(f32 atTime, AnimationInstance* animInstance)
{
	u32 key1Index = animInstance->previousTrackKeys[this];
	AnimationKey* key1 = &keys[key1Index];
	AnimationKey* key2 = nullptr;
	
	for (u32 i = key1Index, iCount = keys.size(); i < iCount - 1; i++)
	{
		if (atTime >= keys[i].time && atTime <= keys[i + 1].time)
		{
			key1Index = i;
			key1 = &keys[i];
			key2 = &keys[i + 1];

			if (key1->triggerEvent
				&& std::find(animInstance->triggeredKeyEvents.begin(), animInstance->triggeredKeyEvents.end(), key1) == animInstance->triggeredKeyEvents.end())
			{
				animInstance->triggerKeyEvent(key1);
			}

			break;
		}
	}

	if (key1 && key2)
	{
		// key1->time ----- atTime ----------------key2->time
		f32 t = (atTime - key1->time) / (key2->time - key1->time);
		// update first interpolation key
		animInstance->previousTrackKeys[this] = key1Index;

		return key1->value + t * (key2->value - key1->value);
	}

	return 0.0f;
}

bool AnimationResource::load(Json::Value& json)
{
	speed = json.get("speed", speed).asFloat();
	repeatCount = json.get("repeatCount", repeatCount).asUInt();
	auto typeStr = json.get("type", "Normal").asString();

	if (typeStr == "Normal") type = Type::Normal;
	if (typeStr == "Reversed") type = Type::Reversed;
	if (typeStr == "PingPong") type = Type::PingPong;

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
		if (keyType == "HasShadows") trackType = AnimationTrack::Type::HasShadows;
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

}
