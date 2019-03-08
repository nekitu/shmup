#pragma once
#include "types.h"
#include "SDL_mixer.h"

namespace engine
{
struct Sound
{
	Mix_Chunk* wave = nullptr;

	~Sound();
	bool load(const char* filename);
	bool play();
};

struct Music
{
	Mix_Music* music = nullptr;

	~Music();
	bool load(const char* filename);
	bool play();
};
}