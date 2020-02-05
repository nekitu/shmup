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

}
