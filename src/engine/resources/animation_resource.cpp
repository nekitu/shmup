#include "animation_resource.h"
#include "animation.h"
#include <algorithm>

namespace engine
{
bool AnimationResource::load(Json::Value& json)
{
	speed = json.get("speed", speed).asFloat();
	auto tracksJson = json.get("tracks", Json::Value(Json::ValueType::arrayValue));
	auto modeStr = json.get("loopMode", "None");
	repeat = json.get("repeat", repeat).asInt();

	if (modeStr == "None") loopMode = AnimationLoopMode::None;
	if (modeStr == "Normal") loopMode = AnimationLoopMode::Normal;
	if (modeStr == "Reversed") loopMode = AnimationLoopMode::Reversed;
	if (modeStr == "PingPong") loopMode = AnimationLoopMode::PingPong;

	for (u32 i = 0, iCount = tracksJson.getMemberNames().size(); i < iCount; i++)
	{
		auto track = new AnimationTrack();
		auto keyType = tracksJson.getMemberNames()[i];
		auto& trackJson = tracksJson.get(keyType, Json::Value(Json::ValueType::objectValue));
		auto trackType = AnimationTrackType::Unknown;

		if (keyType == "PositionX") trackType = AnimationTrackType::PositionX;
		if (keyType == "PositionY") trackType = AnimationTrackType::PositionY;
		if (keyType == "ScaleX") trackType = AnimationTrackType::ScaleX;
		if (keyType == "ScaleY") trackType = AnimationTrackType::ScaleY;
		if (keyType == "UniformScale") trackType = AnimationTrackType::UniformScale;
		if (keyType == "VerticalFlip") trackType = AnimationTrackType::VerticalFlip;
		if (keyType == "HorizontalFlip") trackType = AnimationTrackType::HorizontalFlip;
		if (keyType == "Rotation") trackType = AnimationTrackType::Rotation;
		if (keyType == "Visible") trackType = AnimationTrackType::Visible;
		if (keyType == "Shadow") trackType = AnimationTrackType::Shadow;
		if (keyType == "ShadowOffsetX") trackType = AnimationTrackType::ShadowOffsetX;
		if (keyType == "ShadowOffsetY") trackType = AnimationTrackType::ShadowOffsetY;
		if (keyType == "ShadowScaleX") trackType = AnimationTrackType::ShadowScaleX;
		if (keyType == "ShadowScaleY") trackType = AnimationTrackType::ShadowScaleY;
		if (keyType == "ShadowUniformScale") trackType = AnimationTrackType::ShadowUniformScale;
		if (keyType == "ColorR") trackType = AnimationTrackType::ColorR;
		if (keyType == "ColorG") trackType = AnimationTrackType::ColorG;
		if (keyType == "ColorB") trackType = AnimationTrackType::ColorB;
		if (keyType == "ColorA") trackType = AnimationTrackType::ColorA;
		if (keyType == "ColorMode") trackType = AnimationTrackType::ColorMode;

		modeStr = trackJson.get("loopMode", "None");
		if (modeStr == "None") track->loopMode = AnimationLoopMode::None;
		if (modeStr == "Normal") track->loopMode = AnimationLoopMode::Normal;
		if (modeStr == "Reversed") track->loopMode = AnimationLoopMode::Reversed;
		if (modeStr == "PingPong") track->loopMode = AnimationLoopMode::PingPong;

		track->repeat = trackJson.get("repeat", 0).asInt();

		auto& keysJson = trackJson.get("keys", Json::Value(Json::ValueType::arrayValue));

		for (auto& keyJson : keysJson)
		{
			AnimationKey key;

			key.time = keyJson.get("time", 0.0f).asFloat();
			key.value = keyJson.get("value", 0.0f).asFloat();
			key.easeType = Easing::getTypeFromString(keyJson.get("easeType", "inOutLinear").asString());
			key.triggerEvent = keyJson.get("triggerEvent", false).asBool();
			key.eventName = keyJson.get("eventName", "").asString();
			track->keys.push_back(key);
		}

		track->computeTotalTime();

		if (totalTime < track->endTime)
		{
			totalTime = track->endTime;
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

	totalTime = 0;
	speed = 1.0f;
	repeat = 0;
	loopMode = AnimationLoopMode::Normal;
	tracks.clear();
}

bool AnimationResource::hasTrack(AnimationTrackType trackType)
{
	auto iter = tracks.find(trackType);

	if (iter == tracks.end()) return false;
	if (iter->second->keys.size() == 0) return false;

	return true;
}

}
