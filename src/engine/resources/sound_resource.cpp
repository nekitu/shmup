#include "sound_resource.h"

namespace engine
{
SoundResource::~SoundResource()
{
	if (wave) Mix_FreeChunk(wave);
}

bool SoundResource::load(Json::Value& json)
{
	wave = Mix_LoadWAV(fileName.c_str());

	if (wave == NULL)
		return false;

	return true;
}

MusicResource::~MusicResource()
{
	if (music) Mix_FreeMusic(music);
}

bool MusicResource::load(Json::Value& json)
{
	music = Mix_LoadMUS(fileName.c_str());

	if (music == NULL)
		return false;

	return true;
}

}