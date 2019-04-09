#include "sound_resource.h"

namespace engine
{
SoundResource::~SoundResource()
{
	if (wave) Mix_FreeChunk(wave);
}

bool SoundResource::load(const std::string& filename)
{
	fileName = filename;
	wave = Mix_LoadWAV(filename.c_str());

	if (wave == NULL)
		return false;

	return true;
}

MusicResource::~MusicResource()
{
	if (music) Mix_FreeMusic(music);
}

bool MusicResource::load(const std::string& filename)
{
	fileName = filename;
	music = Mix_LoadMUS(filename.c_str());

	if (music == NULL)
		return false;

	return true;
}


}