#include "music.h"
#include <SDL_mixer.h>
#include "game.h"
#include "resources/music_resource.h"
#include "resource_loader.h"

namespace engine
{
bool Music::play()
{
	if (Mix_PlayingMusic())
		return false;

	if (Mix_PlayMusic(musicResource->music, -1) == -1)
		return false;

	return true;
}

void Music::fadeOutMusic(i32 msec)
{
	Mix_FadeOutMusic(msec);
}

void Music::changeMusic(const std::string& path, i32 newFadeOutMsec, i32 newFadeInMsec)
{
	newMusicResource = Game::instance->resourceLoader->loadMusic(path);
	
	if (!newMusicResource)
	{
		LOG_ERROR("Cannot load music: {}", path);
	}

	fadeOutMsec = newFadeOutMsec;
	fadeInMsec = newFadeInMsec;
	fadeOutMusic(fadeOutMsec);
}

void Music::update()
{
	if (Mix_FadingMusic() != MIX_NO_FADING || !newMusicResource)
		return;

	Mix_FadeInMusic(newMusicResource->music, 0, fadeInMsec);
	musicResource = newMusicResource;
	newMusicResource = nullptr;
}

}
