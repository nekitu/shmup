#pragma once
#include "types.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_version.h>
#include <SDL_audio.h>
#include <string>
#include <unordered_map>

namespace engine
{
struct UnitInstance;

enum class InputControl
{
	Exit = 0,
	Pause,
	Coin,
	Player1_MoveLeft,
	Player1_MoveRight,
	Player1_MoveUp,
	Player1_MoveDown,
	Player1_Fire1,
	Player1_Fire2,
	Player1_Fire3,
	Player2_MoveLeft,
	Player2_MoveRight,
	Player2_MoveUp,
	Player2_MoveDown,
	Player2_Fire1,
	Player2_Fire2,
	Player2_Fire3,

	Count
};

struct Game
{
	static const int maxPlayerCount = 2;
	std::string windowTitle = "Game";
	u32 windowWidth = 800, windowHeight = 600;
	bool vSync = true;
	bool exitGame = false;
	SDL_Window* window = nullptr;
	SDL_GLContext glContext = 0;
	struct Graphics* graphics = nullptr;
	struct ResourceLoader* resourceLoader = nullptr;
	f32 deltaTime = 0;
	f32 lastTime = 0;
	std::vector<UnitInstance*> unitInstances;
	UnitInstance* players[maxPlayerCount];
	bool controls[(u32)InputControl::Count] = { false };
	std::unordered_map<u32, InputControl> mapSdlToControl;
	struct MusicInstance* music = nullptr;
	std::vector<std::pair<std::string /*level name*/, std::string /*level file*/>> levels;
	u32 currentLevelIndex = 0;
	static Game* instance;

	Game();
	~Game();
	bool initialize();
	void shutdown();
	void createPlayers();
	bool initializeAudio();
	void handleInputEvents();
	void checkCollisions();
	void mainLoop();
	void computeDeltaTime();
	bool isControlDown(InputControl control) { return controls[(u32)control]; }
	bool isPlayerMoveLeft(u32 playerIndex);
	bool isPlayerMoveRight(u32 playerIndex);
	bool isPlayerMoveUp(u32 playerIndex);
	bool isPlayerMoveDown(u32 playerIndex);
	bool isPlayerFire1(u32 playerIndex);
	bool isPlayerFire2(u32 playerIndex);
	bool isPlayerFire3(u32 playerIndex);
	void deleteNonPersistentUnitInstances();
	bool loadLevels();
	bool changeLevel(i32 index);
	static std::string makeFullDataPath(const std::string relativeDataFilename);
	struct SpriteInstance* createSpriteInstance(struct SpriteResource* spriteRes);
	struct UnitInstance* createUnitInstance(struct UnitResource* unitRes);
	struct WeaponInstance* createWeaponInstance(const std::string& weaponResFilename, struct UnitInstance* unitInst, struct SpriteInstance* spriteInst);
	struct UnitController* createUnitController(const std::string& name);
};

}
