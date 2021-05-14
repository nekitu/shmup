#include "sound.h"
#include <SDL_mixer.h>
#include "resources/sound_resource.h"

namespace engine
{
bool Sound::play(int loops)
{
	if (!soundResource)
		return false;

	if (Mix_PlayChannel((int)channel, soundResource->wave, loops) == -1)
		return false;

	return true;
}

void Sound::stop()
{
	Mix_HaltChannel((int)channel);
}

bool Sound::isPlaying()
{
	return Mix_Playing((int)channel);
}

}
