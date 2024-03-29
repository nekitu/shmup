#pragma once
#include "types.h"
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
#include "input.h"

namespace engine
{
struct Unit;

const u32 maxPlayerCount = 2;

enum class ScreenMode
{
	Vertical,
	Horizontal
};

struct BeamCollisionInfo
{
	bool directHit = false;
	bool valid = false;
	bool friendly = false;
	Vec2 beamStart;
	Vec2 point;
	f32 distance = 0;
	struct Unit* unit = nullptr;
	struct Sprite* sprite = nullptr;
};

struct PlayerState
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
	u32 id = 0;
	std::string name; // script file name part only
	std::string path;
	struct ScriptResource* script = nullptr;
	struct ScriptClassInstanceBase* scriptClass = nullptr;
	bool active = false;
};

struct GameState
{
	u32 score = 0;
	u32 credit = 0;
	bool playerActive[maxPlayerCount] = { false };
};

struct CameraState
{
	Vec2 position;
	Vec2 positionOffset; // used by camera fx, shake etc
	f32 speed = 10;
	f32 parallaxOffset = 0;
	f32 parallaxScale = 0.2f;
	f32 speedAnimateSpeed = 1.0f;
	bool animatingSpeed = false;
	f32 speedAnimateTime = 0;
	f32 oldSpeed = 0, newSpeed = 0;
	f32 offscreenBoundaryScale = 1.5f;
	Rect offscreenBoundary;
};

struct TimeScaleState
{
	enum class StateType
	{
		Off,
		AnimateIn,
		Holding,
		AnimateOut,

		Count
	};

	StateType stateType = StateType::Off;
	f32 previousScale = 1.0f;
	f32 targetScale = 1.0f;
	f32 holdSeconds = 1.0f;
	f32 inSpeed = 1.0f;
	f32 outSpeed = 1.0f;
	f32 timer = 0;
};

enum CollideFlags : u32
{
	Collide = BIT(1),
	Friends = BIT(2)
};

struct CollisionMatrix
{
	CollideFlags matrix[(int)UnitType::Count][(int)UnitType::Count];

	CollisionMatrix();
	void set(UnitType a, UnitType b, u32 flags);
	u32 get(UnitType a, UnitType b);
};

struct Game
{
	static const u32 maxMouseButtons = 6;
	static const u32 maxProjectileCount = 100000;
	static Game* instance;

	std::string windowTitle = "Game";
	u32 windowWidth = 800, windowHeight = 600;
	u32 videoWidth = 240, videoHeight = 320;
	ScreenMode screenMode = ScreenMode::Vertical;
	bool fullscreen = false;
	bool vSync = false;
	bool prebakedAtlas = false;
	std::string configPath = "game.json";
	std::string dataRoot = "../data/";
	std::string atlasFolder = "atlas";
	bool exitGame = false;
	bool editing = false;
	bool pauseOnAppDeactivate = true;
	SDL_Window* window = nullptr;
	SDL_GLContext glContext = 0;
	struct Graphics* graphics = nullptr;
	struct ResourceLoader* resourceLoader = nullptr;
	Input input;
	f32 deltaTime = 0; // can be affected by time slowdown
	f32 realDeltaTime = 0; // always real time
	f32 lastTime = 0;
	f32 timeScale = 1.0f;
	bool pauseGame = false;
	std::vector<Unit*> units; // all units from all layers
	std::vector<Unit*> newUnits;
	std::vector<Projectile> projectilePool;
	std::vector<Projectile*> projectiles;
	CollisionMatrix collisionMatrix;
	struct Music* music = nullptr;
	std::vector<std::pair<std::string /*map name*/, std::string /*map path*/>> maps;
	u32 currentMapIndex = 0;
	TilemapResource* map = nullptr;
	std::vector<GameScreen*> gameScreens;
	GameState gameState;
	CameraState cameraState;
	TimeScaleState timeScaleState;
	ScreenFx screenFx;
	PlayerState playerState[maxPlayerCount];

	Game();
	~Game();
	void loadConfig();
	bool initialize();
	void shutdown();
	void createPlayers();
	bool initializeAudio();
	void checkCollisions();
	BeamCollisionInfo checkBeamIntersection(struct Unit* unit, struct Sprite* sprite, const Vec2& pos, f32 beamWidth, f32 beamDir);
	Vec2 worldToScreen(const Vec2& pos, u32 layerIndex);
	Vec2 screenToWorld(const Vec2& pos, u32 layerIndex);
	Rect worldToScreen(const Rect& rc, u32 layerIndex);
	Rect screenToWorld(const Rect& rc, u32 layerIndex);
	void preloadSprites();
	void preloadTilesetAndTilemapImages();
	void mainLoop();
	void renderUnits();
	void computeDeltaTime();
	void animateTimeScale(f32 targetTimeScale, f32 holdSeconds, f32 inSpeed, f32 outSpeed);
	void animateCameraSpeed(f32 towardsSpeed, f32 animSpeed);
	void shakeCamera(const Vec2& force, f32 duration, u32 count);
	void fadeScreen(const Color& color1, const Color& color2, f32 duration, bool revertBackAfter, u32 layer = 1);
	void updateScreenFx();
	void updateCamera();
	void updateTileAnimations();
	void updateTimeScaleAnimation();
	void showMousePointer(bool hide);
	void deleteNonPersistentUnits();
	bool changeMapByIndex(i32 index);
	bool changeMapByName(const std::string& name);
	void setScreenActive(const std::string& name, bool activate = true);
	static std::string makeFullDataPath(const std::string relativeDataPath);
	struct Unit* createUnit(struct UnitResource* unitRes);
	struct Weapon* createWeapon(const std::string& weaponResPath, struct Unit* unit, struct Sprite* sprite);
	struct Projectile* newProjectile();
	std::vector<Projectile*>::iterator releaseProjectile(Projectile* proj);
	Unit* findUnitById(u32 id);
	Unit* findUnitByName(const std::string& name);
	GameScreen* findGameScreenById(u32 id);
};

}
