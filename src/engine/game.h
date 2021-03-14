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
#include "resources/tilemap_resource.h"
#include "lua_scripting.h"
#include "projectile.h"

namespace engine
{
struct Unit;

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
	ReloadSprites,
	ReloadAnimations,
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
	struct Unit* unit = nullptr;
	struct Sprite* sprite = nullptr;
};

struct PlayerStats
{
	u32 score = 0;
	bool active = false;
	struct Unit* unit = nullptr;
};

struct ScreenFx
{
	Vec2 shakeForce;
	f32 shakeDuration = 1, shakeTimer = 0;
	u32 shakeCounter = 0;
	u32 shakeCount = 0;
	bool doingShake = false;

	f32 fadeDuration = 1, fadeTimer = 0;
	u32 fadeLayer = 0;
	Color fadeColorFrom, fadeColorTo;
	Color currentFadeColor;
	f32 fadeTimerDir = 1;
	bool fadeRevertBackAfter = false;
	bool doingFade = false;
};

struct GameScreen
{
	std::string name; // script file name part only
	std::string path;
	struct ScriptResource* script = nullptr;
	struct ScriptClassInstanceBase* scriptClass = nullptr;
	bool active = false;
};

struct Game
{
	static const int maxMouseButtons = 6;
	static const int maxPlayerCount = 1;
	static const int maxProjectileCount = 100000;
	std::string windowTitle = "Game";
	u32 windowWidth = 800, windowHeight = 600;
	ScreenMode screenMode = ScreenMode::Vertical;
	bool fullscreen = false;
	bool vSync = false;
	std::string configPath = "game.json";
	std::string dataRoot = "../data/";
	bool exitGame = false;
	bool editing = false;
	bool pauseOnAppDeactivate = true;
	SDL_Window* window = nullptr;
	SDL_GLContext glContext = 0;
	struct Graphics* graphics = nullptr;
	struct ResourceLoader* resourceLoader = nullptr;
	f32 deltaTime = 0;
	f32 lastTime = 0;
	u32 hiscore = 0;
	u32 credit = 0;
	bool pauseGame = false;
	std::vector<Unit*> units; // all units from all layers
	std::vector<Unit*> newUnits;
	std::vector<Projectile> projectilePool;
	std::vector<Projectile*> projectiles;
	PlayerStats players[maxPlayerCount];
	bool controls[(u32)InputControl::Count] = { false };
	bool mouseButtonDown[maxMouseButtons];
	std::unordered_map<u32, InputControl> mapSdlToControl;
	struct Music* music = nullptr;
	std::vector<std::pair<std::string /*map name*/, std::string /*map path*/>> maps;
	u32 currentMapIndex = 0;
	TilemapResource* map = nullptr;
	std::vector<GameScreen*> gameScreens;
	static Game* instance;
	Vec2 cameraPosition;
	Vec2 cameraPositionOffset; // used by camera fx, shake etc
	f32 cameraSpeed = 10;
	f32 cameraParallaxOffset = 0;
	f32 cameraParallaxScale = 0.2f;
	f32 cameraSpeedAnimateSpeed = 1.0f;
	bool animatingCameraSpeed = false;
	f32 cameraSpeedAnimateTime = 0;
	f32 oldCameraSpeed = 0, newCameraSpeed = 0;
	ScreenFx screenFx;
	f32 offscreenBoundaryScale = 1.5f;
	Rect offscreenBoundary;
	Vec2 mousePosition;
	Vec2 windowMousePosition;

	Game();
	~Game();
	void loadConfig();
	bool initialize();
	void shutdown();
	void createPlayers();
	bool initializeAudio();
	void handleInputEvents();
	void checkCollisions();
	BeamCollisionInfo checkBeamIntersection(struct Unit* unit, struct Sprite* sprite, const Vec2& pos, f32 beamWidth);
	Vec2 worldToScreen(const Vec2& pos, u32 layerIndex);
	Vec2 screenToWorld(const Vec2& pos, u32 layerIndex);
	Rect worldToScreen(const Rect& rc, u32 layerIndex);
	Rect screenToWorld(const Rect& rc, u32 layerIndex);
	void preloadSprites();
	void mainLoop();
	void renderUnits();
	void computeDeltaTime();
	void animateCameraSpeed(f32 towardsSpeed, f32 animSpeed);
	void shakeCamera(const Vec2& force, f32 duration, u32 count);
	void fadeScreen(const Color& color1, const Color& color2, f32 duration, bool revertBackAfter, u32 layer = 1);
	void updateScreenFx();
	void updateCamera();
	void updateTileAnimations();
	bool isControlDown(InputControl control) { return controls[(u32)control]; }
	bool isPlayerMoveLeft(u32 playerIndex);
	bool isPlayerMoveRight(u32 playerIndex);
	bool isPlayerMoveUp(u32 playerIndex);
	bool isPlayerMoveDown(u32 playerIndex);
	bool isPlayerFire1(u32 playerIndex);
	bool isPlayerFire2(u32 playerIndex);
	bool isPlayerFire3(u32 playerIndex);
	bool isMouseDown(int btn);
	void showMousePointer(bool hide);
	void deleteNonPersistentUnits();
	bool changeMap(i32 index);
	void setScreenActive(const std::string& name, bool activate = true);
	static std::string makeFullDataPath(const std::string relativeDataPath);
	struct Unit* createUnit(struct UnitResource* unitRes);
	struct Weapon* createWeapon(const std::string& weaponResPath, struct Unit* unit, struct Sprite* sprite);
	struct Projectile* newProjectile();
	std::vector<Projectile*>::iterator releaseProjectile(Projectile* proj);
};

}
