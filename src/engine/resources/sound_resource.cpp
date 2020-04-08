#include "sound_resource.h"

namespace engine
{
bool SoundResource::load(Json::Value& json)
{
	wave = Mix_LoadWAV(fileName.c_str());

	if (wave == NULL)
		return false;

	return true;
}

void SoundResource::unload()
{
	if (wave) Mix_FreeChunk(wave);
	wave = nullptr;
}

}
