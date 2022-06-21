#pragma execution_character_set("utf-8")
#include "engine/types.h"
#include "engine/game.h"

int main(int argc, char** args)
{
	engine::Game game;

	game.loadConfig();
	game.initialize();
	//game.mainLoop();

	return 0;
}
