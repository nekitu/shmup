#include "game.h"
#include "graphics.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include <iostream>
#include "color.h"
#include "texture_array.h"
#include "utils.h"
#include "resource_loader.h"
#include "image_atlas.h"
#include <SDL_mixer.h>
#include "animation.h"
#include "resources/sound_resource.h"
#include "resources/music_resource.h"
#include "resources/unit_resource.h"
#include "resources/sprite_resource.h"
#include "resources/weapon_resource.h"
#include "resources/tilemap_resource.h"
#include "resources/tileset_resource.h"
#include "resources/script_resource.h"
#include "resources/font_resource.h"
#include "unit.h"
#include "sprite.h"
#include "music.h"
#include "sound.h"
#include "weapon.h"
#include "projectile.h"
#include "tilemap_layer_tiles.h"
#include <filesystem>

namespace engine
{
Game* Game::instance = nullptr;

CollisionMatrix::CollisionMatrix()
{
	memset(matrix, 0, sizeof(u32) * (int)UnitType::Count * (int)UnitType::Count);
	set(UnitType::Enemy, UnitType::Enemy, CollideFlags::Friends);
	set(UnitType::Enemy, UnitType::EnemyProjectile, CollideFlags::Friends);
	set(UnitType::Enemy, UnitType::Player, CollideFlags::Collide);
	set(UnitType::Enemy, UnitType::PlayerProjectile, CollideFlags::Collide);
	set(UnitType::Enemy, UnitType::Item, CollideFlags::Friends);
	set(UnitType::Player, UnitType::Item, CollideFlags::Collide |CollideFlags::Friends);
	set(UnitType::Player, UnitType::Player, CollideFlags::Friends);
	set(UnitType::Player, UnitType::PlayerProjectile, CollideFlags::Friends);
	set(UnitType::Player, UnitType::EnemyProjectile, CollideFlags::Collide);
}

void CollisionMatrix::set(UnitType a, UnitType b, u32 flags)
{
	matrix[(int)a][(int)b] = (CollideFlags)flags;
	matrix[(int)b][(int)a] = (CollideFlags)flags;
}

u32 CollisionMatrix::get(UnitType a, UnitType b)
{
	return matrix[(int)a][(int)b];
}

Game::Game()
{
	instance = this;
}

Game::~Game()
{
}

std::string Game::makeFullDataPath(const std::string relativeDataPath)
{
	return instance->dataRoot + relativeDataPath;
}

void Game::loadConfig()
{
	Json::Value json;

	if (!loadJson(makeFullDataPath(configPath), json)) return;

	windowWidth = json.get("windowWidth", windowWidth).asInt();
	windowHeight = json.get("windowHeight", windowHeight).asInt();
	windowTitle = json.get("windowTitle", windowTitle).asString();
	fullscreen = json.get("fullscreen", fullscreen).asBool();
	vSync = json.get("vSync", vSync).asBool();
	pauseOnAppDeactivate = json.get("pauseOnAppDeactivate", pauseOnAppDeactivate).asBool();
	cameraState.speed = json.get("cameraSpeed", cameraState.speed).asFloat();

	auto screensJson = json.get("screens", Json::ValueType::arrayValue);

	for (u32 i = 0; i < screensJson.size(); i++)
	{
		Json::Value& screenInfoJson = screensJson[i];
		GameScreen* gs = new GameScreen();

		auto path = screenInfoJson["path"].asString();
		gs->name = path.substr(path.find_last_of("/") + 1);
		gs->path = path;
		gs->active = screenInfoJson["active"].asBool();
		gameScreens.push_back(gs);

		LOG_INFO("Adding game screen: {0} path: {1}", gs->name, screenInfoJson["path"].asString());
	}

	auto mapsJson = json.get("tilemaps", Json::ValueType::arrayValue);

	for (u32 i = 0; i < mapsJson.size(); i++)
	{
		Json::Value& mapInfoJson = mapsJson[i];
		LOG_INFO("Map: {0} in {1}", mapInfoJson["title"].asCString(), mapInfoJson["path"].asCString());
		maps.push_back(std::make_pair(mapInfoJson["title"].asCString(), mapInfoJson["path"].asString()));
	}
}

bool Game::initialize()
{
#ifdef _DEBUG
	spdlog::set_level(spdlog::level::debug); // Set global log level to debug
	LOG_DEBUG("Debug mode enabled");
#endif

	// change log pattern
	spdlog::set_pattern("[%^%L%$] %v");

	SDL_SetMainReady();

	int err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	if (err != 0)
	{
		LOG_ERROR("SDL initialize error: {0}", SDL_GetError());
		return false;
	}

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

	window = SDL_CreateWindow(
		windowTitle.c_str(),
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | (fullscreen ? SDL_WINDOW_FULLSCREEN : 0));

	if (fullscreen)
	{
		int w, h;
		SDL_GetWindowSize(window, &w, &h);
		windowWidth = w;
		windowHeight = h;
	}

	LOG_INFO("Creating GL context...");
	glContext = SDL_GL_CreateContext(window);

	if (!glContext)
	{
		LOG_ERROR("Cannot create GL context for SDL: {0}", SDL_GetError());
		return false;
	}

	SDL_GL_MakeCurrent(window, glContext);
	SDL_GL_SetSwapInterval(vSync ? 1 : 0);
	SDL_ShowWindow(window);
	SDL_RaiseWindow(window);

	GLenum errGlew = 0;
	glewExperimental = GL_TRUE;
	LOG_INFO("Initializing GLEW...");

	errGlew = glewInit();

	LOG_INFO("Initializing game...");

	if (errGlew != GLEW_OK)
	{
		LOG_ERROR("Cannot initialize GLEW. Error: {0}", glewGetErrorString(errGlew));
		return false;
	}

	int result = 0;
	int flags = MIX_INIT_MP3;

	if (flags != (result = Mix_Init(flags))) {
		LOG_ERROR("Could not initialize mixer (result: {0})", result);
		LOG_ERROR("Mix_Init: {0}", Mix_GetError());
		exit(1);
	}

	initializeAudio();
	graphics = new Graphics(this);
	resourceLoader = new ResourceLoader();
	resourceLoader->atlas = graphics->atlas;

	cameraState.offscreenBoundary = Rect(0, 0, graphics->videoWidth, graphics->videoHeight);
	cameraState.offscreenBoundary = cameraState.offscreenBoundary.getCenterScaled(cameraState.offscreenBoundaryScale);

	initializeLua();
	mapSdlToControl[SDLK_ESCAPE] = InputControl::Exit;
	mapSdlToControl[SDLK_PAUSE] = InputControl::Pause;
	mapSdlToControl[SDLK_1] = InputControl::Coin;
	mapSdlToControl[SDLK_LEFT] = InputControl::Player1_MoveLeft;
	mapSdlToControl[SDLK_RIGHT] = InputControl::Player1_MoveRight;
	mapSdlToControl[SDLK_UP] = InputControl::Player1_MoveUp;
	mapSdlToControl[SDLK_DOWN] = InputControl::Player1_MoveDown;
	mapSdlToControl[SDLK_LCTRL] = InputControl::Player1_Fire1;
	mapSdlToControl[SDLK_LSHIFT] = InputControl::Player1_Fire2;
	mapSdlToControl[SDLK_SPACE] = InputControl::Player1_Fire3;
	mapSdlToControl[SDLK_a] = InputControl::Player2_MoveLeft;
	mapSdlToControl[SDLK_d] = InputControl::Player2_MoveRight;
	mapSdlToControl[SDLK_w] = InputControl::Player2_MoveUp;
	mapSdlToControl[SDLK_s] = InputControl::Player2_MoveDown;
	mapSdlToControl[SDLK_b] = InputControl::Player2_Fire1;
	mapSdlToControl[SDLK_g] = InputControl::Player2_Fire2;
	mapSdlToControl[SDLK_t] = InputControl::Player2_Fire3;
	mapSdlToControl[SDLK_F5] = InputControl::ReloadScripts;
	mapSdlToControl[SDLK_F6] = InputControl::ReloadSprites;
	mapSdlToControl[SDLK_F7] = InputControl::ReloadAnimations;

	music = new Music();
	projectilePool.resize(maxProjectileCount);
	preloadSprites();
	lastTime = SDL_GetTicks();

	for (auto& gs : gameScreens)
	{
		gs->script = resourceLoader->loadScript(gs->path);
		gs->scriptClass = gs->script->createClassInstance(gs);

		if (gs->active)
		{
			CALL_LUA_FUNC2(gs->scriptClass, "onActivate");
		}
	}
}

void Game::shutdown()
{
	Mix_CloseAudio();
}

void Game::createPlayers()
{
	for (u32 i = 0; i < maxPlayerCount; i++)
	{
		playerState[i].unit = new Unit();
		playerState[i].unit->initializeFrom(resourceLoader->loadUnit("units/player"));
		playerState[i].unit->name = "Player" + std::to_string(i + 1);
		playerState[i].unit->root->position.x = graphics->videoWidth / 2;
		playerState[i].unit->root->position.y = graphics->videoHeight / 2;
		playerState[i].unit->layerIndex = (u32)~0 - 1;
		newUnits.push_back(playerState[i].unit);
	}
}

bool Game::initializeAudio()
{
	// initialize SDL_mixer
	if (Mix_OpenAudio(22050, MIX_DEFAULT_FORMAT, 2, 4096) == -1)
	{
		return -1;
	}
}

void Game::handleInputEvents()
{
	SDL_PumpEvents();

	SDL_Event ev;

	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
		case SDL_KEYDOWN:
		{
			for (auto& item : mapSdlToControl)
			{
				if (item.first == (u32)ev.key.keysym.sym)
				{
					controls[(u32)item.second] = true;
				}
			}

			break;
		}

		case SDL_KEYUP:
		{
			for (auto& item : mapSdlToControl)
			{
				if (item.first == (u32)ev.key.keysym.sym)
				{
					controls[(u32)item.second] = false;
				}
			}

			break;
		}
		case SDL_WINDOWEVENT:
		{
			switch (ev.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			case SDL_WINDOWEVENT_MOVED:
			case SDL_WINDOWEVENT_EXPOSED:
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			{
				if (pauseOnAppDeactivate)
					pauseGame = false;
				break;
			}
			case SDL_WINDOWEVENT_ENTER:
			{
				break;
			}
			case SDL_WINDOWEVENT_FOCUS_LOST:
			{
				if (pauseOnAppDeactivate)
					pauseGame = true;
				break;
			}
			case SDL_WINDOWEVENT_CLOSE:
			{
				exitGame = true;
				break;
			}
			default:
				break;
			}

			break;
		}

		case SDL_MOUSEBUTTONDOWN:
			mouseButtonDown[ev.button.button] = true;
			break;

		case SDL_MOUSEBUTTONUP:
			mouseButtonDown[ev.button.button] = false;
			break;

		case SDL_MOUSEMOTION:
		{
			windowMousePosition = { (f32)ev.motion.x, (f32)ev.motion.y };
			f32 scale = graphics->videoWidth / graphics->blittedRect.width;
			mousePosition.x = (windowMousePosition.x - graphics->blittedRect.x) * scale;
			mousePosition.y = (windowMousePosition.y - graphics->blittedRect.y) * scale;
			break;
		}
		default:
			break;
		}
	}
}

void Game::updateCamera()
{
	if (cameraState.animatingSpeed)
	{
		cameraState.speedAnimateTime += deltaTime * cameraState.speedAnimateSpeed;

		if (cameraState.speedAnimateTime >= 1.0f)
		{
			cameraState.animatingSpeed = false;
			cameraState.speed = cameraState.newSpeed;
		}
		else
		{
			cameraState.speed = lerp(cameraState.oldSpeed, cameraState.newSpeed, cameraState.speedAnimateTime);
		}
	}

	if (screenMode == ScreenMode::Vertical)
	{
		cameraState.position.y += cameraState.speed * deltaTime;
		cameraState.position.x = cameraState.parallaxOffset;
	}
	else
	{
		cameraState.position.x += cameraState.speed * deltaTime;
		cameraState.position.x = cameraState.parallaxOffset;
	}
}

void Game::mainLoop()
{
	while (!exitGame)
	{
		computeDeltaTime();
		updateTimeScaleAnimation();
		handleInputEvents();
		music->update();

		if (isControlDown(InputControl::Exit))
			exitGame = true;

		//TODO: make a proper state object for the key presses
		static bool reloadScriptsKeyDown = false;
		static bool reloadSpritesKeyDown = false;
		static bool reloadAnimationsKeyDown = false;
		static bool pauseKeyDown = false;

		bool reloadScripts = isControlDown(InputControl::ReloadScripts);
		bool reloadSprites = isControlDown(InputControl::ReloadSprites);
		bool reloadAnimations = isControlDown(InputControl::ReloadAnimations);
		bool pause = isControlDown(InputControl::Pause);

		if (!reloadScriptsKeyDown && reloadScripts)
		{
			reloadScriptsKeyDown = true;
			resourceLoader->reloadScripts();
		}

		if (!reloadScripts)
			reloadScriptsKeyDown = false;

		//--

		if (!reloadSpritesKeyDown && reloadSprites)
		{
			reloadSpritesKeyDown = true;
			resourceLoader->reloadSprites();
		}

		if (!reloadSprites)
			reloadSpritesKeyDown = false;

		//--

		if (!reloadAnimationsKeyDown && reloadAnimations)
		{
			reloadAnimationsKeyDown = true;
			resourceLoader->reloadAnimations();
		}

		if (!reloadAnimations)
			reloadAnimationsKeyDown = false;

		//--

		if (!pauseKeyDown && pause)
		{
			pauseKeyDown = true;
			pauseGame = !pauseGame;
		}

		if (!pause)
			pauseKeyDown = false;

		if (pauseGame)
		{
			deltaTime = 0;
		}

		for (auto& scr : gameScreens)
		{
			if (scr->active)
			{
				CALL_LUA_FUNC2(scr->scriptClass, "onUpdate", deltaTime);
			}
		}

		updateScreenFx();
		updateCamera();
		updateTileAnimations();

		for (auto& unit : units)
		{
			unit->update(this);
		}

		for (auto& proj : projectiles)
		{
			if (proj->used)
				proj->update(this);
		}

		// add the new created units
		if (newUnits.size())
		{
			units.insert(units.end(), newUnits.begin(), newUnits.end());
			newUnits.clear();
			std::stable_sort(units.begin(), units.end(), [](Unit* a, Unit* b) { return a->layerIndex < b->layerIndex; });
		}

		checkCollisions();

		auto iter = units.begin();

		while (iter != units.end())
		{
			if ((*iter)->deleteMeNow)
			{
				CALL_LUA_FUNC2((*iter)->scriptClass, "onDelete");
				delete *iter;
				iter = units.erase(iter);
				continue;
			}

			++iter;
		}

		auto iterProj = projectiles.begin();
		while (iterProj != projectiles.end())
		{
			if ((*iterProj)->used && (*iterProj)->deleteMeNow)
			{
				iterProj = releaseProjectile(*iterProj);
				continue;
			}

			++iterProj;
		}

		// update and render the game graphics render target
		graphics->setupRenderTargetRendering();
		graphics->beginFrame();

		for (auto& gs : gameScreens)
		{
			if (gs->active)
			{
				CALL_LUA_FUNC2(gs->scriptClass, "onRender");
			}
		}

		if (screenFx.doingFade)
		{
			graphics->pushColor(screenFx.currentFadeColor);
			graphics->pushColorMode(ColorMode::Mul);
			graphics->pushAlphaMode(AlphaMode::Blend);
			graphics->drawQuad({ 0, 0, graphics->videoWidth, graphics->videoHeight }, graphics->atlas->whiteImage->uvRect);
			graphics->popColor();
			graphics->popColorMode();
			graphics->popAlphaMode();
		}

		graphics->endFrame();

		// display the render target contents in the main backbuffer
		graphics->blitRenderTarget();
		SDL_GL_SwapWindow(window);
	}
}

void Game::renderUnits()
{
	for (auto& unit : units)
	{
		for (auto& gs : gameScreens)
		{
			if (gs->active)
			{
				CALL_LUA_FUNC2(gs->scriptClass, "onBeforeRenderUnit", unit);
			}
		}

		unit->render(graphics);

		for (auto& gs : gameScreens)
		{
			if (gs->active)
			{
				CALL_LUA_FUNC2(gs->scriptClass, "onAfterRenderUnit", unit);
			}
		}
	}

	for (auto& unit : projectiles)
	{
		if (unit && unit->used)
			unit->render(graphics);
	}

	for (auto& gs : gameScreens)
	{
		if (gs->active)
		{
			CALL_LUA_FUNC2(gs->scriptClass, "onAfterRenderProjectiles");
		}
	}
}

void Game::checkCollisions()
{
	static std::unordered_map<Unit*, Unit*> collisionPairs;

	collisionPairs.clear();

	// check normal units
	for (auto& unit1 : units)
	{
		if (!unit1->collide) continue;

		for (auto& unit2 : units)
		{
			if (unit1 == unit2)
				continue;

			if (!unit1->unitResource) continue;
			if (!unit2->unitResource) continue;

			if (unit1->unitResource->unitType == unit2->unitResource->unitType) continue;

			if (!unit2->collide) continue;

			auto colFlags = collisionMatrix.get(unit1->unitResource->unitType, unit2->unitResource->unitType);

			if (!(colFlags & CollideFlags::Collide))
				continue;

			if (unit1->boundingBox.overlaps(unit2->boundingBox))
			{
				auto iter1 = collisionPairs.find(unit1);
				auto iter2 = collisionPairs.find(unit2);
				bool exists1 = false;
				bool exists2 = false;

				if (iter1 != collisionPairs.end() && iter1->second == unit2)
				{
					exists1 = true;
				}

				if (iter2 != collisionPairs.end() && iter2->second == unit1)
				{
					exists2 = true;
				}

				if (!exists1 && !exists2)
				{
					collisionPairs[unit1] = unit2;
				}
			}
		}
	}

	// check projectiles vs normal units
	for (auto& unitProj : projectiles)
	{
		if (!unitProj->used) continue;

		for (auto& unit2 : units)
		{
			if (unitProj == unit2) continue;

			if (!unit2->collide) continue;
			if (!unitProj->unitResource) continue;
			if (!unit2->unitResource) continue;
			if (unitProj->unitResource->unitType == unit2->unitResource->unitType) continue;

			auto colFlags = collisionMatrix.get(unit2->unitResource->unitType, unitProj->unitResource->unitType);

			if (!(colFlags & CollideFlags::Collide))
				continue;

			if (unitProj->boundingBox.overlaps(unit2->boundingBox))
			{
				LOG_INFO("{} {},{},{},{} --- {} {},{},{},{}", unitProj->name,
					unitProj->boundingBox.x, unitProj->boundingBox.y, unitProj->boundingBox.right(), unitProj->boundingBox.bottom(),
					unit2->name, unit2->boundingBox.x, unit2->boundingBox.y, unit2->boundingBox.right(), unit2->boundingBox.bottom());
				auto iter1 = collisionPairs.find(unitProj);
				auto iter2 = collisionPairs.find(unit2);
				bool exists1 = false;
				bool exists2 = false;

				if (iter1 != collisionPairs.end() && iter1->second == unit2)
				{
					exists1 = true;
				}

				if (iter2 != collisionPairs.end() && iter2->second == unitProj)
				{
					exists2 = true;
				}

				if (!exists1 && !exists2)
				{
					collisionPairs[unitProj] = unit2;
				}
			}
		}
	}

	std::vector<SpriteCollision> pixelCols;

	for (auto& cp : collisionPairs)
	{
		pixelCols.clear();

		if (!cp.first->unitResource) continue;
		if (!cp.second->unitResource) continue;

		if (cp.first->checkPixelCollision(cp.second, pixelCols))
		{
			// delete projectiles colliding with other normal units
			if (cp.first->unitResource->unitType == UnitType::EnemyProjectile
				|| cp.first->unitResource->unitType == UnitType::PlayerProjectile)
			{
				cp.first->deleteMeNow = true;
			}

			if (cp.second->unitResource->unitType == UnitType::EnemyProjectile
				|| cp.second->unitResource->unitType == UnitType::PlayerProjectile)
			{
				cp.second->deleteMeNow = true;
			}

			if (cp.first->scriptClass || cp.second->scriptClass)
			{
				LuaIntf::LuaRef colsTbl = LuaIntf::LuaRef::createTable(getLuaState());

				for (int i = 0; i < pixelCols.size(); i++)
				{
					LuaIntf::LuaRef col = LuaIntf::LuaRef::createTable(getLuaState());

					col.set("sprite1", pixelCols[i].sprite1);
					col.set("sprite2", pixelCols[i].sprite2);
					col.set("collisionCenter", pixelCols[i].collisionCenter);
					colsTbl.set(i + 1, col);

					// inflict damage
					pixelCols[i].sprite1->hit(pixelCols[i].sprite2->damage);
					pixelCols[i].sprite2->hit(pixelCols[i].sprite1->damage);
				}

				CALL_LUA_FUNC2(cp.first->scriptClass, "onCollide", cp.second, colsTbl)
				CALL_LUA_FUNC2(cp.second->scriptClass, "onCollide", cp.first, colsTbl)
			}
		}
	}
}

BeamCollisionInfo Game::checkBeamIntersection(Unit* unit, Sprite* sprite, const Vec2& pos, f32 beamWidth, f32 beamDir)
{
	BeamCollisionInfo closest;
	Rect rc;

	closest.beamStart = pos;
	closest.distance = FLT_MAX;

	if (screenMode == ScreenMode::Vertical)
	{
		rc.set(pos.x - beamWidth / 2, 0, beamWidth, pos.y);
	}
	else if (screenMode == ScreenMode::Horizontal)
	{
		rc.set(pos.x, pos.y - beamWidth / 2, graphics->videoWidth - pos.x, beamWidth);
	}

	for (auto& otherUnit : units)
	{
		if (otherUnit == unit || !otherUnit->collide) continue;

		for (auto& otherSprite : otherUnit->sprites)
		{
			if (sprite == otherSprite) continue;
			if (!otherSprite->collide) continue;
			if (otherSprite->health <= 0) continue;

			if (otherSprite->rect.overlaps(rc))
			{
				if (screenMode == ScreenMode::Vertical)
				{
					Vec2 col;
					col.x = pos.x;

					// for small sprites
					bool isInsideBeam = rc.contains(otherSprite->rect);
					bool isTouchingHalfBeam = false;
					bool pixelCollided = false;

					f32 relativeX = round(col.x - otherSprite->rect.x);

					// if the middle of the beam is outside the sprite rect
					// one of the beam haves are touching the sprite, so just set hit to middle of sprite
					if (relativeX < 0 || relativeX >= otherSprite->rect.width)
					{
						if (fabs(relativeX) <= beamWidth / 2 || (otherSprite->rect.width - relativeX) <= beamWidth / 2)
						{
							isTouchingHalfBeam = true;
						}
						else continue;
					}
					else
					{
						col.y = otherSprite->rect.bottom();
						Rect frmRc = otherSprite->spriteResource->getSheetFramePixelRect(otherSprite->animationFrame);
						f32 stepY = 1.0f / otherSprite->scale.y;

						// consider flipping
						f32 xFinal = otherSprite->horizontalFlip ? otherSprite->spriteResource->frameWidth - relativeX : relativeX;

						for (f32 y = otherSprite->spriteResource->frameHeight - 1; y >= 0; y -= stepY)
						{
							f32 yFinal = otherSprite->verticalFlip ? otherSprite->spriteResource->frameHeight - y : y;
							// get the scaled pixel from the sprite image
							f32 scaledY = frmRc.y + yFinal;
							f32 scaledX = frmRc.x + xFinal / otherSprite->scale.x;

							u8* p = (u8*)&otherSprite->spriteResource->image->imageData[
								(u32)scaledY * otherSprite->spriteResource->image->width
									+ (u32)scaledX];
							// if alpha is 0xFF, then we hit an opaque pixel
							if (p[3] == 0xff)
							{
								pixelCollided = true;
								col.y -= otherSprite->rect.height - yFinal * otherSprite->scale.y;
								break;
							}
						}
					}

					if (pixelCollided || isInsideBeam || isTouchingHalfBeam)
					{
						f32 dist = pos.y - col.y;

						if (dist >= 0 && dist < closest.distance)
						{
							closest.directHit = !isTouchingHalfBeam;
							closest.valid = true;
							closest.distance = dist;
							closest.point = col;
							closest.unit = otherUnit;
							closest.sprite = otherSprite;
						}
					}
				}
				else if (screenMode == ScreenMode::Horizontal)
				{
					Vec2 col;
					col.x = otherSprite->rect.x;
					col.y = pos.y;

					// for small sprites
					bool isInsideBeam = rc.contains(otherSprite->rect);
					bool isTouchingHalfBeam = false;
					bool pixelCollided = false;

					f32 relativeY = round(col.y - otherSprite->rect.y);

					// if the middle of the beam is outside the sprite rect
					// one of the beam haves are touching the sprite, so just set hit to middle of sprite
					if (relativeY < 0 || relativeY >= otherSprite->rect.height)
					{
						if (fabs(relativeY) <= beamWidth / 2 || (otherSprite->rect.height - relativeY) <= beamWidth / 2)
						{
							isTouchingHalfBeam = true;
							col.x = otherSprite->rect.center().x;
						}

						continue;
					}
					else
					{
						Rect frmRc = otherSprite->spriteResource->getSheetFramePixelRect(otherSprite->animationFrame);

						for (int x = otherSprite->spriteResource->frameWidth - 1; x >= 0; x--)
						{
							u8* p = (u8*)&otherSprite->spriteResource->image->imageData[
								(u32)(frmRc.y + relativeY) * otherSprite->spriteResource->image->width
									+ (u32)(frmRc.x + x)];

							if (p[3] == 0xff)
							{
								pixelCollided = true;
								col.x -= otherSprite->rect.width - x;
								break;
							}
						}
					}

					if (pixelCollided || isInsideBeam || isTouchingHalfBeam)
					{
						f32 dist = pos.x - col.x;

						if (dist < closest.distance)
						{
							closest.valid = true;
							closest.distance = dist;
							closest.point = col;
							closest.unit = otherUnit;
							closest.sprite = otherSprite;
						}
					}
				}
			}
		}
	}

	return closest;
}

Vec2 Game::worldToScreen(const Vec2& pos, u32 layerIndex)
{
	Vec2 newPos = pos;

	newPos.x = roundf(newPos.x);
	newPos.y = roundf(newPos.y);

	if (!Game::instance->map || layerIndex >= Game::instance->map->layers.size())
	{
		return newPos;
	}

	if (!Game::instance->map->layers[layerIndex].cameraScroll
		&& !Game::instance->map->layers[layerIndex].cameraParallax)
	{
		return pos;
	}

	if (!Game::instance->map->layers[layerIndex].cameraScroll
		&& Game::instance->map->layers[layerIndex].cameraParallax)
	{
		newPos.x += cameraState.positionOffset.x * Game::instance->map->layers[layerIndex].cameraParallaxScale;
		newPos.y += cameraState.positionOffset.y * Game::instance->map->layers[layerIndex].cameraParallaxScale;

		return newPos;
	}

	newPos.x += cameraState.position.x + cameraState.positionOffset.x * Game::instance->map->layers[layerIndex].cameraParallaxScale;
	newPos.y += cameraState.position.y * Game::instance->map->layers[layerIndex].cameraParallaxScale + cameraState.positionOffset.y * Game::instance->map->layers[layerIndex].cameraParallaxScale;

	return newPos;
}

Vec2 Game::screenToWorld(const Vec2& pos, u32 layerIndex)
{
	Vec2 newPos = pos;

	if (layerIndex >= Game::instance->map->layers.size())
	{
		return newPos;
	}

	if (!Game::instance->map->layers[layerIndex].cameraScroll
		&& !Game::instance->map->layers[layerIndex].cameraParallax)
	{
		return pos;
	}

	if (!Game::instance->map->layers[layerIndex].cameraScroll
		&& Game::instance->map->layers[layerIndex].cameraParallax)
	{
		newPos.x -= cameraState.positionOffset.x * Game::instance->map->layers[layerIndex].cameraParallaxScale;
		newPos.y -= cameraState.positionOffset.y * Game::instance->map->layers[layerIndex].cameraParallaxScale;

		return newPos;
	}

	newPos.x -= cameraState.position.x + cameraState.positionOffset.x * Game::instance->map->layers[layerIndex].cameraParallaxScale;
	newPos.y -= cameraState.position.y + cameraState.positionOffset.y * Game::instance->map->layers[layerIndex].cameraParallaxScale;

	return newPos;
}

Rect Game::worldToScreen(const Rect& rc, u32 layerIndex)
{
	Vec2 pos = worldToScreen({ rc.x, rc.y }, layerIndex);
	return { pos.x, pos.y, rc.width, rc.height };
}

Rect Game::screenToWorld(const Rect& rc, u32 layerIndex)
{
	Vec2 pos = screenToWorld({ rc.x, rc.y }, layerIndex);
	return { pos.x, pos.y, rc.width, rc.height };
}

void Game::preloadSprites()
{
	LOG_INFO("Preloading sprites...");

	auto recursiveSearch = [](const std::string& currentPath)
	{
		static void (*recursiveSearchIn)(const std::string&) = [](const std::string& currentPath)
		{
			for (auto& p : std::filesystem::directory_iterator(currentPath))
			{
				auto path = p.path();

				if (p.is_regular_file())
				{
					if (path.extension() == ".json")
					{
						path.replace_extension("");
						auto str = path.relative_path().u8string();
						std::replace(str.begin(), str.end(), '\\', '/');
						replaceAll(str, Game::instance->dataRoot, "");
					Game:instance->resourceLoader->loadSprite(str);
					}
				}
				else if (p.is_directory())
				{
					recursiveSearchIn(path.relative_path().u8string());
				}
			}
		};

		recursiveSearchIn(currentPath);
	};

	recursiveSearch(dataRoot + "sprites");

	LOG_INFO("Packing atlas sprites...");
	graphics->atlas->pack();

	LOG_INFO("Computing sprite params after packing...");

	for (auto& spriteResource : resourceLoader->sprites)
	{
		spriteResource->computeParamsAfterAtlasGeneration();
	}
}

void Game::computeDeltaTime()
{
	f32 ticks = SDL_GetTicks();
	realDeltaTime = deltaTime = (ticks - lastTime) / 1000.0f;
	deltaTime *= timeScale;
	lastTime = ticks;
}

void Game::animateTimeScale(f32 targetTimeScale, f32 holdSeconds, f32 inSpeed, f32 outSpeed)
{
	timeScaleState.targetScale = targetTimeScale;
	timeScaleState.holdSeconds = holdSeconds;
	timeScaleState.inSpeed = inSpeed;
	timeScaleState.outSpeed = outSpeed;
	timeScaleState.timer = 0;
	timeScaleState.previousScale = timeScale;
	timeScaleState.stateType = TimeScaleState::StateType::AnimateIn;
}

void Game::animateCameraSpeed(f32 towardsSpeed, f32 animSpeed)
{
	cameraState.speedAnimateSpeed = animSpeed;
	cameraState.speedAnimateTime = 0;
	cameraState.animatingSpeed = true;
	cameraState.oldSpeed = cameraState.speed;
	cameraState.newSpeed = towardsSpeed;
}

void Game::shakeCamera(const Vec2& force, f32 duration, u32 count)
{
	screenFx.doingShake = true;
	screenFx.shakeTimer = 0;
	screenFx.shakeForce = force;
	screenFx.shakeCount = screenFx.shakeCounter = count;
	screenFx.shakeDuration = duration;
}

void Game::fadeScreen(const Color& color1, const Color& color2, f32 duration, bool revertBackAfter, u32 layer)
{
	screenFx.doingFade = true;
	screenFx.fadeTimer = 0;
	screenFx.fadeLayer = layer;
	screenFx.fadeTimerDir = 1;
	screenFx.fadeColorFrom = color1;
	screenFx.fadeColorTo = color2;
	screenFx.fadeDuration = duration;
	screenFx.fadeRevertBackAfter = revertBackAfter;
}

void Game::updateScreenFx()
{
	if (screenFx.doingShake)
	{
		f32 slice = screenFx.shakeDuration / (f32)screenFx.shakeCount;
		screenFx.shakeTimer += deltaTime;

		if (screenFx.shakeTimer >= slice && !screenFx.shakeCounter)
		{
			screenFx.doingShake = false;
			cameraState.positionOffset.clear();
		}
		else
			if (screenFx.shakeTimer >= slice && screenFx.shakeCounter)
			{
				screenFx.shakeTimer = 0;
				f32 x = screenFx.shakeForce.x * (f32)screenFx.shakeCounter / (f32)screenFx.shakeCount;
				f32 y = screenFx.shakeForce.y * (f32)screenFx.shakeCounter / (f32)screenFx.shakeCount;
				cameraState.positionOffset.x = randomFloat(-x, x);
				cameraState.positionOffset.y = randomFloat(-y, y);
				screenFx.shakeCounter--;
			}
	}

	if (screenFx.doingFade)
	{
		screenFx.fadeTimer += deltaTime * screenFx.fadeTimerDir * 1.0f / screenFx.fadeDuration;

		if (screenFx.fadeTimer >= 1 && screenFx.fadeTimerDir > 0)
		{
			if (screenFx.fadeRevertBackAfter)
			{
				screenFx.fadeTimer = 1;
				screenFx.fadeTimerDir = -1;
				screenFx.fadeRevertBackAfter = false;
			}
			else
			{
				screenFx.doingFade = false;
				screenFx.fadeTimer = 1;
				screenFx.currentFadeColor = screenFx.fadeColorFrom;
				graphics->alphaMode = (u32)AlphaMode::Mask;
			}
		}
		else if (screenFx.fadeTimerDir < 0 && screenFx.fadeTimer < 0)
		{
			screenFx.doingFade = false;
			graphics->alphaMode = (u32)AlphaMode::Mask;
		}
		else
		{
			screenFx.currentFadeColor = screenFx.fadeColorFrom + (screenFx.fadeColorTo - screenFx.fadeColorFrom) * screenFx.fadeTimer;
		}
	}
}

void Game::updateTileAnimations()
{
	for (auto& res : resourceLoader->resources)
	{
		if (res.second->type == ResourceType::Tileset)
		{
			TilesetResource* tset = (TilesetResource*)res.second;
			tset->updateAnimations(deltaTime);
		}
	}
}

void Game::updateTimeScaleAnimation()
{
	switch (timeScaleState.stateType)
	{
	case TimeScaleState::StateType::AnimateIn:
	{
		timeScaleState.timer += timeScaleState.inSpeed * realDeltaTime;

		if (timeScaleState.timer > 1.0f)
		{
			timeScaleState.timer = 0.0f;
			timeScaleState.stateType = TimeScaleState::StateType::Holding;
			timeScale = timeScaleState.targetScale;
		}
		else
		{
			timeScale = lerp(timeScaleState.previousScale, timeScaleState.targetScale, timeScaleState.timer);
		}
		break;
	}
	case TimeScaleState::StateType::Holding:
	{
		timeScaleState.timer += realDeltaTime;
		if (timeScaleState.timer >= timeScaleState.holdSeconds)
		{
			timeScaleState.timer = 0;
			timeScaleState.stateType = TimeScaleState::StateType::AnimateOut;
		}

		break;
	}
	case TimeScaleState::StateType::AnimateOut:
	{
		timeScaleState.timer += timeScaleState.outSpeed * realDeltaTime;

		if (timeScaleState.timer > 1.0f)
		{
			timeScaleState.timer = 0.0f;
			timeScaleState.stateType = TimeScaleState::StateType::Off;
			timeScale = timeScaleState.previousScale;
		}
		else
		{
			timeScale = lerp(timeScaleState.targetScale, timeScaleState.previousScale, timeScaleState.timer);
		}
		break;
	}
	}
}

bool Game::isPlayerMoveLeft(u32 playerIndex)
{
	return controls[(u32)(playerIndex ? InputControl::Player2_MoveLeft : InputControl::Player1_MoveLeft)];
}

bool Game::isPlayerMoveRight(u32 playerIndex)
{
	return controls[(u32)(playerIndex ? InputControl::Player2_MoveRight : InputControl::Player1_MoveRight)];
}

bool Game::isPlayerMoveUp(u32 playerIndex)
{
	return controls[(u32)(playerIndex ? InputControl::Player2_MoveUp : InputControl::Player1_MoveUp)];
}

bool Game::isPlayerMoveDown(u32 playerIndex)
{
	return controls[(u32)(playerIndex ? InputControl::Player2_MoveDown : InputControl::Player1_MoveDown)];
}

bool Game::isPlayerFire1(u32 playerIndex)
{
	return controls[(u32)(playerIndex ? InputControl::Player2_Fire1 : InputControl::Player1_Fire1)];
}

bool Game::isPlayerFire2(u32 playerIndex)
{
	return controls[(u32)(playerIndex ? InputControl::Player2_Fire2 : InputControl::Player1_Fire2)];
}

bool Game::isPlayerFire3(u32 playerIndex)
{
	return controls[(u32)(playerIndex ? InputControl::Player2_Fire3 : InputControl::Player1_Fire3)];
}

bool Game::isMouseDown(int btn)
{
	return mouseButtonDown[btn];
}

void Game::showMousePointer(bool hide)
{
	SDL_ShowCursor(hide);
}

void Game::deleteNonPersistentUnits()
{
	auto iter = units.begin();

	while (iter != units.end())
	{
		bool isPlayer = (*iter)->unitResource && (*iter)->unitResource->unitType == UnitType::Player;
		if (!isPlayer && !(*iter)->persistentOnChangeMap)
		{
			delete *iter;
			iter = units.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

bool Game::changeMap(i32 index)
{
	deleteNonPersistentUnits();
	projectilePool.clear();
	projectilePool.resize(maxProjectileCount);
	projectiles.clear();

	if (index == -1)
	{
		currentMapIndex++;
	}

	if (index >= maps.size())
	{
		LOG_ERROR("Map index out of range {0}", index);
		return false;
	}

	currentMapIndex = index;
	map = resourceLoader->loadTilemap(maps[currentMapIndex].second);
	int layerIndex = 0;

	for (auto& layer : map->layers)
	{
		TilemapLayerTiles* layerTiles = new TilemapLayerTiles();

		layerTiles->name = layer.name;
		layerTiles->tilemapLayer = &layer;
		layerTiles->layerIndex = layerIndex;
		layerTiles->root = new Sprite();
		layerTiles->speed = atof(layer.properties["speed"].c_str());
		newUnits.push_back(layerTiles);

		for (auto& obj : layer.objects)
		{
			switch (obj.type)
			{
			case TilemapObject::Type::Point:
			{
				auto& unitClass = obj.typeString;

				if (unitClass == "")
				{
					auto unitPath = obj.properties["unit"];
					replaceAll(unitPath, "\/", "/");
					replaceAll(unitPath, "\"", "");
					Unit* unit = createUnit(resourceLoader->loadUnit(unitPath));

					if (unit)
					{
						unit->load(obj);
						unit->layerIndex = layerIndex;
					}
				}

				break;
			}
			}
		}

		++layerIndex;
	}

	LOG_INFO("Packing atlas images...");
	graphics->atlas->pack();

	LOG_INFO("Computing sprite params after packing...");

	for (auto& spriteResource : resourceLoader->sprites)
	{
		spriteResource->computeParamsAfterAtlasGeneration();
	}

	cameraState.position.clear();

	return true;
}

void Game::setScreenActive(const std::string& name, bool activate)
{
	for (auto& scr : gameScreens)
	{
		if (scr->name == name)
		{
			scr->active = activate;
			if (activate)
			{
				CALL_LUA_FUNC2(scr->scriptClass, "onActivate");
			}
			else
			{
				CALL_LUA_FUNC2(scr->scriptClass, "onDeactivate");
			}

			break;
		}
	}
}

Unit* Game::createUnit(UnitResource* unitResource)
{
	Unit* unit = nullptr;

	if (!unitResource)
		return nullptr;

	if (unitResource->className == "")
	{
		unit = new Unit();
	}

	unit->initializeFrom(unitResource);
	newUnits.push_back(unit);

	return unit;
}

Weapon* Game::createWeapon(const std::string& weaponResPath, struct Unit* unit, struct Sprite* sprite)
{
	Weapon* weapon = new Weapon();
	auto weaponRes = resourceLoader->loadWeapon(weaponResPath);

	weapon->parentUnit = unit;
	weapon->attachTo = sprite;
	weapon->initializeFrom(weaponRes);

	return weapon;
}

Projectile* Game::newProjectile()
{
	for (u32 i = 0; i < maxProjectileCount; i++)
	{
		if (!projectilePool[i].used)
		{
			Projectile* proj = &projectilePool[i];

			proj->used = true;
			projectiles.push_back(proj);

			return proj;
		}
	}

	return nullptr;
}

std::vector<Projectile*>::iterator Game::releaseProjectile(Projectile* proj)
{
	proj->used = false;

	auto iter = std::find(projectiles.begin(), projectiles.end(), proj);

	if (iter != projectiles.end())
	{
		CALL_LUA_FUNC2((*iter)->scriptClass, "onDelete");
		auto newIter = projectiles.erase(iter);
		return newIter;
	}

	return projectiles.end();
}

}
