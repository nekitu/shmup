#pragma once
#include "types.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_version.h>
#include <SDL_audio.h>
#include <string>
#include <unordered_map>
#include "vec2.h"
#include "rect.h"
#include "utils.h"
#include "color.h"
#include "resources/level_resource.h"
#include "lua_scripting.h"
#include "projectile_instance.h"

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
	ReloadScripts,
	Count
};

enum class ScreenMode
{
	Vertical,
	Horizontal
};

struct BeamCollisionInfo
{
	bool valid = false;
	Vec2 point;
	f32 distance = 0;
	struct UnitInstance* unitInst = nullptr;
	struct SpriteInstance* spriteInst = nullptr;
};

struct PlayerStats
{
	u32 score = 0;
	bool active = false;
	struct UnitInstance* unitInstance = nullptr;
};

struct ScreenFx
{
	Vec2 shakeForce;
	f32 shakeDuration, shakeTimer = 0;
	u32 shakeCounter = 0;
	u32 shakeCount = 0;
	bool doingShake = false;

	f32 fadeDuration, fadeTimer = 0;
	Color fadeColor;
	Color currentFadeColor;
	ColorMode fadeColorMode = ColorMode::Add;
	f32 fadeTimerDir = 1;
	bool fadeRevertBackAfter = false;
	bool doingFade = false;
};

struct Game
{
	static const int maxPlayerCount = 1;
	static const int maxProjectileCount = 100000;
	std::string windowTitle = "Game";
	u32 windowWidth = 800, windowHeight = 600;
	ScreenMode screenMode = ScreenMode::Vertical;
	bool fullscreen = false;
	bool vSync = true;
	std::string dataRoot = "../data/";
	bool exitGame = false;
	bool editing = false;
	SDL_Window* window = nullptr;
	SDL_GLContext glContext = 0;
	struct Graphics* graphics = nullptr;
	struct ResourceLoader* resourceLoader = nullptr;
	f32 deltaTime = 0;
	f32 lastTime = 0;
	u32 hiscore = 0;
	u32 credit = 0;
	std::vector<UnitInstance*> unitInstances;
	std::vector<UnitInstance*> newUnitInstances;
	std::vector<ProjectileInstance> projectiles;
	PlayerStats players[maxPlayerCount];
	bool controls[(u32)InputControl::Count] = { false };
	std::unordered_map<u32, InputControl> mapSdlToControl;
	struct MusicInstance* music = nullptr;
	std::vector<std::pair<std::string /*level name*/, std::string /*level file*/>> levels;
	u32 currentLevelIndex = 0;
	struct ScriptResource* currentMainScript = nullptr;
	struct ScriptClassInstanceBase* scriptClass;
	static Game* instance;
	Vec2 cameraPosition;
	Vec2 cameraPositionOffset;
	f32 cameraSpeed = 11;
	f32 cameraParallaxOffset = 0;
	f32 cameraParallaxScale = 0.2f;
	std::vector<Layer> layers;
	f32 cameraSpeedAnimateSpeed = 1.0f;
	bool animatingCameraSpeed = false;
	f32 cameraSpeedAnimateTime = 0;
	f32 oldCameraSpeed = 0, newCameraSpeed = 0;
	ScreenFx screenFx;

	u32 lastProjectileIndex = 0;

	Game();
	~Game();
	bool initialize();
	void shutdown();
	void createPlayers();
	bool initializeAudio();
	void handleInputEvents();
	void checkCollisions();
	BeamCollisionInfo checkBeamIntersection(UnitInstance* inst, SpriteInstance* sprInst, const Vec2& pos, f32 beamWidth);
	Vec2 worldToScreen(const Vec2& pos, u32 layerIndex);
	Vec2 screenToWorld(const Vec2& pos, u32 layerIndex);
	Rect worldToScreen(const Rect& rc, u32 layerIndex);
	Rect screenToWorld(const Rect& rc, u32 layerIndex);
	void preloadSprites();
	void mainLoop();
	void computeDeltaTime();
	void animateCameraSpeed(f32 towardsSpeed, f32 animSpeed);
	void shakeCamera(const Vec2& force, f32 duration, u32 count);
	void fadeScreen(const Color& color, ColorMode colorMode, f32 duration, bool revertBackAfter);
	void updateScreenFx();
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
	struct ProjectileInstance* newProjectileInstance();
	void releaseProjectileInstance(ProjectileInstance* inst);
};

}
