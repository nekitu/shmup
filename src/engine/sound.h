#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
enum class SoundChannel
{
	Other,
	Music,
	Player,
	Enemy,
	PlayerExplosion,
	EnemyExplosion,
	Item,

	Count
};

struct Sound
{
	struct SoundResource* soundResource = nullptr;
	SoundChannel channel = SoundChannel::Other;

	bool play();
	bool isPlaying();
};

}
