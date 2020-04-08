#include "resources/music_resource.h"

namespace engine
{
MusicResource::~MusicResource()
{
}

bool MusicResource::load(Json::Value& json)
{
	music = Mix_LoadMUS(fileName.c_str());

	if (music == NULL)
		return false;

	return true;
}

void MusicResource::unload()
{
	if (music) Mix_FreeMusic(music);
	music = nullptr;
}

}
