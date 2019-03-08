#include "sound.h"

namespace engine
{
Sound::~Sound()
{
	if (wave) Mix_FreeChunk(wave);
}

bool Sound::load(const char* filename)
{
	wave = Mix_LoadWAV(filename);

	if (wave == NULL)
		return false;

	return true;
}

bool Sound::play()
{
	if (Mix_PlayChannel(1, wave, 0) == -1)
		return false;

	return true;
}


Music::~Music()
{
	if (music) Mix_FreeMusic(music);
}

bool Music::load(const char* filename)
{
	music = Mix_LoadMUS(filename);

	if (music == NULL)
		return false;

	return true;
}

bool Music::play()
{
	if (Mix_PlayingMusic())
		return false;

	if (Mix_PlayMusic(music, -1) == -1)
		return false;

	return true;
}

}