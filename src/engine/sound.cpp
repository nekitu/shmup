#include "sound.h"
#include <SDL_mixer.h>
#include "resources/sound_resource.h"

namespace engine
{
bool Sound::play()
{
	if (!soundResource)
		return false;

	if (Mix_PlayChannel((int)channel, soundResource->wave, 0) == -1)
		return false;

	return true;
}

bool Sound::isPlaying()
{
	return Mix_Playing((int)channel);
}

}
