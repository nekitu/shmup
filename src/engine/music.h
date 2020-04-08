#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct Music
{
	struct MusicResource* musicResource = nullptr;

	bool play();
};

}
