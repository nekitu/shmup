#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct Music
{
	struct MusicResource* musicResource = nullptr;
	struct MusicResource* newMusicResource = nullptr;
	i32 fadeOutMsec = 3000;
	i32 fadeInMsec = 3000;

	bool play();
	void fadeOutMusic(i32 msec);
	void changeMusic(const std::string& path, i32 fadeOutMsec = 3000, i32 fadeInMsec = 3000);
	void update();
};

}
