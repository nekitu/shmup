#pragma once
#include "types.h"
#include "SDL_mixer.h"
#include "resource.h"

namespace engine
{
struct SoundResource : Resource
{
	Mix_Chunk* wave = nullptr;

	bool load(Json::Value& json) override;
	void unload() override;
};

}
