#include "sound_instance.h"
#include <SDL_mixer.h>
#include "resources/sound_resource.h"

namespace engine
{
bool SoundInstance::play()
{
	if (Mix_PlayChannel(1, soundResource->wave, 0) == -1)
		return false;

	return true;
}
	
bool MusicInstance::play()
{
	if (Mix_PlayingMusic())
		return false;

	if (Mix_PlayMusic(musicResource->music, -1) == -1)
		return false;

	return true;
}

}