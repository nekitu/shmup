#pragma once
#include "types.h"
#include "SDL_mixer.h"
#include "resource.h"

namespace engine
{
struct SoundResource : Resource
{
	Mix_Chunk* wave = nullptr;

	~SoundResource();
	bool load(Json::Value& json) override;
};

struct MusicResource : Resource
{
	Mix_Music* music = nullptr;

	~MusicResource();
	bool load(Json::Value& json) override;
};

}