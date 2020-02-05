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

}
