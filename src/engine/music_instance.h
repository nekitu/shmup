#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct MusicInstance
{
	struct MusicResource* musicResource = nullptr;

	bool play();
};

}
