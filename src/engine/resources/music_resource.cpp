#include "resources/music_resource.h"
#include "game.h"

namespace engine
{
MusicResource::~MusicResource()
{
}

bool MusicResource::load(Json::Value& json)
{
	auto absPath = Game::instance->dataRoot + path;
	music = Mix_LoadMUS(absPath.c_str());

	if (music == NULL)
	{
		LOG_ERROR("Cannot load music: {} abspath: {}", path, absPath);
		return false;
	}

	return true;
}

void MusicResource::unload()
{
	if (music) Mix_FreeMusic(music);
	music = nullptr;
}

}
