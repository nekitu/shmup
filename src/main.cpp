#pragma execution_character_set("utf-8")
#include "engine/types.h"
#include "engine/game.h"

int main(int argc, char** args)
{
	engine::Game game;

	//TODO: load from cfg file
	game.windowWidth = 0;
	game.windowHeight = 0;
	game.windowTitle = "Stellar Eagle";
	game.fullscreen = true;
	game.initialize();
	game.mainLoop();

	return 0;
}
