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

}
