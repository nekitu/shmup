#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct SoundInstance
{
	struct SoundResource* soundResource = nullptr;

	bool play();
};

struct MusicInstance
{
	struct MusicResource* musicResource = nullptr;

	bool play();
};

}