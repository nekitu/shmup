#include "music_instance.h"
#include <SDL_mixer.h>
#include "resources/music_resource.h"

namespace engine
{
bool MusicInstance::play()
{
	if (Mix_PlayingMusic())
		return false;

	if (Mix_PlayMusic(musicResource->music, -1) == -1)
		return false;

	return true;
}

}
