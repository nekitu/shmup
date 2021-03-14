#include "sound_resource.h"
#include "game.h"

namespace engine
{
bool SoundResource::load(Json::Value& json)
{
	auto absPath = Game::instance->dataRoot + path;
	wave = Mix_LoadWAV(absPath.c_str());

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
