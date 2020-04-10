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
#include "resources/level_resource.h"
#include "resources/script_resource.h"
#include "resources/font_resource.h"
#include "unit.h"
#include "sprite.h"
#include "music.h"
#include "sound.h"
#include "weapon.h"
#include "projectile.h"
#include <filesystem>

namespace engine
{
Game* Game::instance = nullptr;

Game::Game()
{
	instance = this;
}

Game::~Game()
{
}

std::string Game::makeFullDataPath(const std::string relativeDataFilename)
{
	return instance->resourceLoader->root + relativeDataFilename;
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
	initializeLua();
	loadLevels();
	changeLevel(0);
	createPlayers();

	mapSdlToControl[SDLK_ESCAPE] = InputControl::Exit;
	mapSdlToControl[SDLK_PAUSE] = InputControl::Pause;
	mapSdlToControl[SDLK_1] = InputControl::Coin;
	mapSdlToControl[SDLK_LEFT] = InputControl::Player1_MoveLeft;
	mapSdlToControl[SDLK_RIGHT] = InputControl::Player1_MoveRight;
	mapSdlToControl[SDLK_UP] = InputControl::Player1_MoveUp;
	mapSdlToControl[SDLK_DOWN] = InputControl::Player1_MoveDown;
	mapSdlToControl[SDLK_LCTRL] = InputControl::Player1_Fire1;
	mapSdlToControl[SDLK_RSHIFT] = InputControl::Player1_Fire2;
	mapSdlToControl[SDLK_RALT] = InputControl::Player1_Fire3;
	mapSdlToControl[SDLK_a] = InputControl::Player2_MoveLeft;
	mapSdlToControl[SDLK_d] = InputControl::Player2_MoveRight;
	mapSdlToControl[SDLK_w] = InputControl::Player2_MoveUp;
	mapSdlToControl[SDLK_s] = InputControl::Player2_MoveDown;
	mapSdlToControl[SDLK_b] = InputControl::Player2_Fire1;
	mapSdlToControl[SDLK_g] = InputControl::Player2_Fire2;
	mapSdlToControl[SDLK_t] = InputControl::Player2_Fire3;
	mapSdlToControl[SDLK_F5] = InputControl::ReloadScripts;

	//TODO: remove
	music = new Music();
	music->musicResource = resourceLoader->loadMusic("music/Retribution.ogg");
	music->play();
	Mix_VolumeMusic(1);

	currentMainScript = resourceLoader->loadScript("scripts/ingame_screen");
	scriptClass = currentMainScript->createClassInstance(this);
	preloadSprites();
	lastTime = SDL_GetTicks();
}

void Game::shutdown()
{
	Mix_CloseAudio();
}

void Game::createPlayers()
{
	for (u32 i = 0; i < maxPlayerCount; i++)
	{
		players[i].unit = new Unit();
		players[i].unit->initializeFrom(resourceLoader->loadUnit("units/player"));
		units.push_back(players[i].unit);
		players[i].unit->name = "Player" + std::to_string(i + 1);
		players[i].unit->root->position.x = graphics->videoWidth / 2;
		players[i].unit->root->position.y = graphics->videoHeight / 2;
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
				break;
			}
			case SDL_WINDOWEVENT_ENTER:
			{
				break;
			}
			case SDL_WINDOWEVENT_FOCUS_LOST:
				break;
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
		default:
			break;
		}
	}
}

void Game::updateCamera()
{
	if (animatingCameraSpeed)
	{
		cameraSpeedAnimateTime += deltaTime * cameraSpeedAnimateSpeed;

		if (cameraSpeedAnimateTime >= 1.0f)
		{
			animatingCameraSpeed = false;
			cameraSpeed = newCameraSpeed;
		}
		else
		{
			cameraSpeed = lerp(oldCameraSpeed, newCameraSpeed, cameraSpeedAnimateTime);
		}
	}

	cameraPosition.y += cameraSpeed * deltaTime;
	cameraPosition.x = cameraParallaxOffset;
}

void Game::mainLoop()
{
	while (!exitGame)
	{
		computeDeltaTime();
		handleInputEvents();

		if (isControlDown(InputControl::Exit))
			exitGame = true;

		static bool reloadKeyDown = false;
		static bool pauseKeyDown = false;

		bool reload = isControlDown(InputControl::ReloadScripts);
		bool pause = isControlDown(InputControl::Pause);

		if (!reloadKeyDown && reload)
		{
			reloadKeyDown = true;
			resourceLoader->reloadScripts();
		}

		if (!reload)
			reloadKeyDown = false;

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

		CALL_LUA_FUNC("onUpdate", deltaTime);

		updateScreenFx();
		updateCamera();

		for (u32 i = 0; i < units.size(); i++)
		{
			units[i]->update(this);
		}

		for (auto proj : projectiles)
		{
			proj->update(this);
		}

		// add the new created units
		units.insert(units.end(), newUnits.begin(), newUnits.end());
		newUnits.clear();

		Unit::updateShadowToggle();

		checkCollisions();

		auto iter = units.begin();

		while (iter != units.end())
		{
			if ((*iter)->deleteMeNow)
			{
				delete *iter;
				iter = units.erase(iter);
				continue;
			}

			++iter;
		}

		auto iterProj = projectiles.begin();
		while (iterProj != projectiles.end())
		{
			if ((*iterProj)->deleteMeNow)
			{
				iterProj = releaseProjectile(*iterProj);
				continue;
			}

			++iterProj;
		}

		// update and render the game graphics render target
		graphics->setupRenderTargetRendering();
		graphics->beginFrame();

		for (auto unit : units)
		{
			unit->render(graphics);
		}

		for (auto proj : projectiles)
		{
			proj->render(graphics);
		}

		CALL_LUA_FUNC("onRender", 0)

		if (screenFx.doingFade)
		{
			graphics->color = screenFx.currentFadeColor.getRgba();
			graphics->colorMode = (u32)screenFx.fadeColorMode;
			graphics->drawQuad({ 0, 0, graphics->videoWidth, graphics->videoHeight }, graphics->atlas->whiteImage->uvRect);
		}

		graphics->endFrame();

		// display the render target contents in the main backbuffer
		graphics->blitRenderTarget();
		SDL_GL_SwapWindow(window);
	}
}

void Game::checkCollisions()
{
	std::unordered_map<Unit*, Unit*> collisionPairs;

	// check normal units
	for (auto unit1 : units)
	{
		if (!unit1->collide) continue;

		for (auto unit2 : units)
		{
			if (!unit2->collide) continue;

			if (unit1 == unit2
				|| unit1->unitResource->type == unit2->unitResource->type) continue;

			if ((unit1->unitResource->unitType == UnitType::Enemy &&
				unit2->unitResource->unitType == UnitType::EnemyProjectile)
				|| (unit2->unitResource->unitType == UnitType::Enemy &&
					unit1->unitResource->unitType == UnitType::EnemyProjectile))
			{
				continue;
			}

			if ((unit1->unitResource->unitType == UnitType::Player &&
				unit2->unitResource->unitType == UnitType::PlayerProjectile)
				|| (unit2->unitResource->unitType == UnitType::Player &&
					unit1->unitResource->unitType == UnitType::PlayerProjectile))
			{
				continue;
			}

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
	for (auto unitProj : projectiles)
	{
		if (!unitProj->collide) continue;

		for (auto unit2 : units)
		{
			if (!unit2->collide) continue;

			if (unitProj == unit2
				|| unitProj->unitResource->unitType == unit2->unitResource->unitType) continue;

			if (unit2->unitResource->unitType == UnitType::Enemy &&
				unitProj->unitResource->unitType == UnitType::EnemyProjectile)
			{
				continue;
			}

			if (unit2->unitResource->unitType == UnitType::Player &&
				unitProj->unitResource->unitType == UnitType::PlayerProjectile)
			{
				continue;
			}

			if (unitProj->boundingBox.overlaps(unit2->boundingBox))
			{
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

		if (cp.first->checkPixelCollision(cp.second, pixelCols))
		{
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
				}

				CALL_LUA_FUNC2(cp.first->scriptClass, "onCollide", cp.second, colsTbl)
				CALL_LUA_FUNC2(cp.second->scriptClass, "onCollide", cp.first, colsTbl)
			}
		}
	}
}

BeamCollisionInfo Game::checkBeamIntersection(Unit* unit, Sprite* sprite, const Vec2& pos, f32 beamWidth)
{
	BeamCollisionInfo closest;
	Rect rc;

	closest.distance = FLT_MAX;

	if (screenMode == ScreenMode::Vertical)
	{
		rc.set(pos.x - beamWidth / 2, 0, beamWidth, pos.y);
	}
	else if (screenMode == ScreenMode::Horizontal)
	{
		rc.set(pos.x, pos.y - beamWidth / 2, graphics->videoWidth - pos.x, beamWidth);
	}

	for (auto unit1 : units)
	{
		if (unit1 == unit || !unit1->collide) continue;

		for (auto sprite2 : unit1->sprites)
		{
			if (sprite == sprite2) continue;
			if (!sprite2->collide) continue;

			if (screenMode == ScreenMode::Vertical)
			{
				if (sprite2->screenRect.y > pos.y) continue;

				if (sprite2->screenRect.bottom() > pos.y && sprite2->screenRect.y <= pos.y)
				{
					closest.valid = true;
					closest.distance = 0;
					closest.point = pos;
					closest.unit = unit;
					closest.sprite = sprite2;

					return closest;
				}
			}
			else if (screenMode == ScreenMode::Horizontal)
			{
				if (sprite2->screenRect.right() < pos.x) continue;

				if (sprite2->screenRect.x < pos.x && sprite2->screenRect.right() >= pos.x)
				{
					closest.valid = true;
					closest.distance = 0;
					closest.point = pos;
					closest.unit = unit;
					closest.sprite = sprite2;

					return closest;
				}
			}

			if (sprite2->screenRect.overlaps(rc))
			if (screenMode == ScreenMode::Vertical)
			{
				Vec2 col;
				col.x = pos.x;
				col.y = sprite2->screenRect.bottom();

				// for small sprites
				bool isInsideBeam = rc.contains(sprite2->screenRect);
				bool isTouchingHalfBeam = false;
				bool pixelCollided = false;

				f32 relativeX = round(col.x - sprite2->screenRect.x);

				// if the middle of the beam is outside the sprite rect
				// one of the beam haves are touching the sprite, so just set hit to middle of sprite
				if (relativeX < 0 || relativeX >= sprite2->screenRect.width)
				{
					if (fabs(relativeX) <= beamWidth / 2 || (sprite2->screenRect.width - relativeX) <= beamWidth / 2)
					{
						isTouchingHalfBeam = true;
						col.y = sprite2->screenRect.center().y;
					}

					continue;
				}
				else
				{
					Rect frmRc = sprite2->spriteResource->getSheetFramePixelRect(sprite2->animationFrame);

					for (int y = sprite2->spriteResource->frameHeight - 1; y >= 0; y--)
					{
						u8* p = (u8*)&sprite2->spriteResource->image->imageData[
							(u32)(frmRc.y + y) * sprite2->spriteResource->image->width
								+ (u32)(frmRc.x + relativeX)];

						if (p[3] == 0xff)
						{
							pixelCollided = true;
							col.y -= sprite2->screenRect.height - y;
							break;
						}
					}
				}

				if (pixelCollided || isInsideBeam || isTouchingHalfBeam)
				{
					f32 dist = pos.y - col.y;

					if (dist < closest.distance)
					{
						closest.valid = true;
						closest.distance = dist;
						closest.point = col;
						closest.unit = unit;
						closest.sprite = sprite2;
					}
				}
			}
			else if (screenMode == ScreenMode::Horizontal)
			{
				Vec2 col;
				col.x = sprite2->screenRect.x;
				col.y = pos.y;

				// for small sprites
				bool isInsideBeam = rc.contains(sprite2->screenRect);
				bool isTouchingHalfBeam = false;
				bool pixelCollided = false;

				f32 relativeY = round(col.y - sprite2->screenRect.y);

				// if the middle of the beam is outside the sprite rect
				// one of the beam haves are touching the sprite, so just set hit to middle of sprite
				if (relativeY < 0 || relativeY >= sprite2->screenRect.height)
				{
					if (fabs(relativeY) <= beamWidth / 2 || (sprite2->screenRect.height - relativeY) <= beamWidth / 2)
					{
						isTouchingHalfBeam = true;
						col.x = sprite2->screenRect.center().x;
					}

					continue;
				}
				else
				{
					Rect frmRc = sprite2->spriteResource->getSheetFramePixelRect(sprite2->animationFrame);

					for (int x = sprite2->spriteResource->frameWidth - 1; x >= 0; x--)
					{
						u8* p = (u8*)&sprite2->spriteResource->image->imageData[
							(u32)(frmRc.y + relativeY) * sprite2->spriteResource->image->width
								+ (u32)(frmRc.x + x)];

						if (p[3] == 0xff)
						{
							pixelCollided = true;
							col.x -= sprite2->screenRect.width - x;
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
						closest.unit = unit;
						closest.sprite = sprite2;
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

	newPos.x += (cameraPosition.x + cameraPositionOffset.x) * Game::instance->layers[layerIndex].parallaxScale;
	newPos.y += (cameraPosition.y + cameraPositionOffset.y) * Game::instance->layers[layerIndex].parallaxScale;

	return newPos;
}

Vec2 Game::screenToWorld(const Vec2& pos, u32 layerIndex)
{
	Vec2 newPos = pos;

	newPos.x -= (cameraPosition.x + cameraPositionOffset.x) * Game::instance->layers[layerIndex].parallaxScale;
	newPos.y -= (cameraPosition.y + cameraPositionOffset.y) * Game::instance->layers[layerIndex].parallaxScale;

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

	for (auto spriteResource : resourceLoader->sprites)
	{
		spriteResource->computeParamsAfterAtlasGeneration();
	}
}

void Game::computeDeltaTime()
{
	f32 ticks = SDL_GetTicks();
	deltaTime = (ticks - lastTime) / 1000.0f;
	lastTime = ticks;
}

void Game::animateCameraSpeed(f32 towardsSpeed, f32 animSpeed)
{
	cameraSpeedAnimateSpeed = animSpeed;
	cameraSpeedAnimateTime = 0;
	animatingCameraSpeed = true;
	oldCameraSpeed = cameraSpeed;
	newCameraSpeed = towardsSpeed;
}

void Game::shakeCamera(const Vec2& force, f32 duration, u32 count)
{
	screenFx.doingShake = true;
	screenFx.shakeTimer = 0;
	screenFx.shakeForce = force;
	screenFx.shakeCount = screenFx.shakeCounter = count;
	screenFx.shakeDuration = duration;
}

void Game::fadeScreen(const Color& color, ColorMode colorMode, f32 duration, bool revertBackAfter)
{
	screenFx.doingFade = true;
	screenFx.fadeTimer = 0;
	screenFx.fadeTimerDir = 1;
	screenFx.fadeColor = color;
	screenFx.fadeDuration = duration;
	screenFx.fadeColorMode = colorMode;
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
			cameraPositionOffset.clear();
		}
		else
			if (screenFx.shakeTimer >= slice && screenFx.shakeCounter)
			{
				screenFx.shakeTimer = 0;
				f32 x = screenFx.shakeForce.x * (f32)screenFx.shakeCounter / (f32)screenFx.shakeCount;
				f32 y = screenFx.shakeForce.y * (f32)screenFx.shakeCounter / (f32)screenFx.shakeCount;
				cameraPositionOffset.x = randomFloat(-x, x);
				cameraPositionOffset.y = randomFloat(-y, y);
				screenFx.shakeCounter--;
			}
	}

	if (screenFx.doingFade)
	{
		screenFx.fadeTimer += deltaTime * screenFx.fadeTimerDir * 1.0 / screenFx.fadeDuration;

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
				screenFx.currentFadeColor = screenFx.fadeColor;
			}
		}
		else if (screenFx.fadeTimerDir < 0 && screenFx.fadeTimer < 0)
		{
			screenFx.doingFade = false;
		}
		else
		{
			screenFx.currentFadeColor = screenFx.fadeColor;

			if (screenFx.fadeColorMode == ColorMode::Add
				|| screenFx.fadeColorMode == ColorMode::Sub)
			{
				screenFx.currentFadeColor.r *= screenFx.fadeTimer;
				screenFx.currentFadeColor.g *= screenFx.fadeTimer;
				screenFx.currentFadeColor.b *= screenFx.fadeTimer;
				screenFx.currentFadeColor.a = 1;
			}
			else if (screenFx.fadeColorMode == ColorMode::Mul)
			{
				screenFx.currentFadeColor.r = 1.0 + screenFx.fadeTimer * (screenFx.currentFadeColor.r - 1.0f);
				screenFx.currentFadeColor.g = 1.0 + screenFx.fadeTimer * (screenFx.currentFadeColor.g - 1.0f);
				screenFx.currentFadeColor.b = 1.0 + screenFx.fadeTimer * (screenFx.currentFadeColor.b - 1.0f);
				screenFx.currentFadeColor.a = 1;
			}
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

bool Game::loadLevels()
{
	Json::Value listJson;

	if (!loadJson(makeFullDataPath("levels/list.json"), listJson))
	{
		return false;
	}

	for (u32 i = 0; i < listJson.size(); i++)
	{
		Json::Value& lvlInfoJson = listJson[i];
		LOG_INFO("Level: {0} in {1}", lvlInfoJson["title"].asCString(), lvlInfoJson["file"].asCString());
		levels.push_back(std::make_pair(lvlInfoJson["title"].asCString(), lvlInfoJson["file"].asString()));
	}

	return true;
}

void Game::deleteNonPersistentUnits()
{
	auto iter = units.begin();

	while (iter != units.end())
	{
		if ((*iter)->unitResource->unitType != UnitType::Player)
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

bool Game::changeLevel(i32 index)
{
	deleteNonPersistentUnits();
	projectilePool.clear();
	projectilePool.resize(maxProjectileCount);
	projectiles.clear();

	if (index == -1)
	{
		currentLevelIndex++;
	}

	if (index >= levels.size())
	{
		LOG_ERROR("Level index out of range {0}", index);
		return false;
	}

	currentLevelIndex = index;
	auto level = resourceLoader->loadLevel(levels[currentLevelIndex].second);

	for (auto& layer : level->layers)
	{
		layers.push_back(layer);
	}

	for (auto& unit : level->units)
	{
		units.push_back(unit);
	}

	resourceLoader->unload(level);

	return true;
}

Unit* Game::createUnit(UnitResource* unitResource)
{
	auto unit = new Unit();

	unit->initializeFrom(unitResource);
	newUnits.push_back(unit);

	return unit;
}

Weapon* Game::createWeapon(const std::string& weaponResFilename, struct Unit* unit, struct Sprite* sprite)
{
	Weapon* weapon = new Weapon();
	auto weaponRes = resourceLoader->loadWeapon(weaponResFilename);

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
		auto newIter = projectiles.erase(iter);
		return newIter;
	}

	return projectiles.end();
}

}
