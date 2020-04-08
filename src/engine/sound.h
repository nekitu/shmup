#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct Sound
{
	struct SoundResource* soundResource = nullptr;

	bool play();
};

}
