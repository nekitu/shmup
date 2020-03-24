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
#include "animation_instance.h"
#include "resources/sound_resource.h"
#include "resources/music_resource.h"
#include "resources/unit_resource.h"
#include "resources/sprite_resource.h"
#include "resources/weapon_resource.h"
#include "resources/level_resource.h"
#include "resources/script_resource.h"
#include "resources/font_resource.h"
#include "unit_instance.h"
#include "sprite_instance.h"
#include "music_instance.h"
#include "sound_instance.h"
#include "weapon_instance.h"
#include "projectile_instance.h"
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
	SDL_SetMainReady();

	int err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS);

	if (err != 0)
	{
		printf("SDL initialize error: %s\n", SDL_GetError());
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

	printf("Creating GL context...");
	glContext = SDL_GL_CreateContext(window);

	if (!glContext)
	{
		printf("Cannot create GL context for SDL: %s\n", SDL_GetError());
		return false;
	}

	SDL_GL_MakeCurrent(window, glContext);
	SDL_GL_SetSwapInterval(vSync ? 1 : 0);
	SDL_ShowWindow(window);
	SDL_RaiseWindow(window);

	GLenum errGlew = 0;
	glewExperimental = GL_TRUE;
	printf("Initializing GLEW...");

	errGlew = glewInit();

	printf("Initializing game...");

	if (errGlew != GLEW_OK)
	{
		printf("FATAL ERROR: Cannot initialize GLEW. Error: %s\n", glewGetErrorString(errGlew));
		return false;
	}

	int result = 0;
	int flags = MIX_INIT_MP3;

	if (flags != (result = Mix_Init(flags))) {
		printf("Could not initialize mixer (result: %d).\n", result);
		printf("Mix_Init: %s\n", Mix_GetError());
		exit(1);
	}

	initializeAudio();

	lastTime = SDL_GetTicks();
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
	music = new MusicInstance();
	music->musicResource = resourceLoader->loadMusic("music/Retribution.ogg");
	music->play();
	Mix_VolumeMusic(1);

	currentMainScript = resourceLoader->loadScript("scripts/ingame_screen");
	scriptClass = currentMainScript->createClassInstance(this);
	preloadSprites();
}

void Game::shutdown()
{
	Mix_CloseAudio();
}

void Game::createPlayers()
{
	for (u32 i = 0; i < maxPlayerCount; i++)
	{
		players[i].unitInstance = new UnitInstance();
		players[i].unitInstance->initializeFrom(resourceLoader->loadUnit("units/player"));
		unitInstances.push_back(players[i].unitInstance);
		players[i].unitInstance->name = "Player" + std::to_string(i + 1);
		players[i].unitInstance->root->position.x = graphics->videoWidth / 2;
		players[i].unitInstance->root->position.y = graphics->videoHeight / 2;
		//static_cast<PlayerController*>(players[i].unitInstance->findController("main"))->playerIndex = i;
	}

	//TODO: fix later to have a single atlas pack call after loading all sprites
	//graphics->atlas->pack();
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

void Game::mainLoop()
{
	while (!exitGame)
	{
		computeDeltaTime();
		handleInputEvents();

		if (scriptClass)
		{
			if (scriptClass->getFunction("onUpdate").isFunction())
			{
				scriptClass->getFunction("onUpdate").call(scriptClass->classInstance, deltaTime);
			}
		}

		updateScreenFx();

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

		if (isControlDown(InputControl::Exit))
			exitGame = true;

		if (isControlDown(InputControl::ReloadScripts))
		{
			resourceLoader->reloadScripts();
		}

		for (u32 i = 0; i < unitInstances.size(); i++)
		{
			unitInstances[i]->update(this);
		}

		for (u32 i = 0; i < lastProjectileIndex; i++)
		{
			if (projectiles[i].used)
			{
				projectiles[i].update(this);
			}
		}

		// add the new created instances
		unitInstances.insert(unitInstances.end(), newUnitInstances.begin(), newUnitInstances.end());
		newUnitInstances.clear();

		UnitInstance::updateShadowToggle();

		checkCollisions();

		auto iter = unitInstances.begin();

		while (iter != unitInstances.end())
		{
			if ((*iter)->deleteMeNow)
			{
				delete* iter;
				iter = unitInstances.erase(iter);
				continue;
			}

			++iter;
		}

		for (u32 i = 0; i < lastProjectileIndex; i++)
		{
			if (projectiles[i].deleteMeNow && projectiles[i].used)
			{
				releaseProjectileInstance(&projectiles[i]);
			}
		}

		// update and render the game graphics render target
		graphics->setupRenderTargetRendering();
		graphics->beginFrame();

		for (auto inst : unitInstances)
		{
			inst->render(graphics);
		}

		for (u32 i = 0; i < lastProjectileIndex; i++)
		{
			auto& projInst = projectiles[i];

			if (projInst.used)
			{
				projInst.render(graphics);
			}
		}

		if (scriptClass)
		{
			auto func = scriptClass->getFunction("onRender");

			if (func.isFunction())
			{
				func.call(scriptClass->classInstance, 0);
			}
		}

		if (screenFx.doingFade)
		{
			graphics->currentColor = screenFx.currentFadeColor.getRgba();
			graphics->currentColorMode = (u32)screenFx.fadeColorMode;
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
	std::unordered_map<UnitInstance*, UnitInstance*> collisionPairs;

	// check normal units
	for (auto unitInst : unitInstances)
	{
		if (!unitInst->collide) continue;

		for (auto unitInst2 : unitInstances)
		{
			if (!unitInst2->collide) continue;

			if (unitInst == unitInst2
				|| unitInst->unit->type == unitInst2->unit->type) continue;

			if ((unitInst->unit->type == UnitResource::Type::Enemy &&
				unitInst2->unit->type == UnitResource::Type::EnemyProjectile)
				|| (unitInst2->unit->type == UnitResource::Type::Enemy &&
					unitInst->unit->type == UnitResource::Type::EnemyProjectile))
			{
				continue;
			}

			if ((unitInst->unit->type == UnitResource::Type::Player &&
				unitInst2->unit->type == UnitResource::Type::PlayerProjectile)
				|| (unitInst2->unit->type == UnitResource::Type::Player &&
					unitInst->unit->type == UnitResource::Type::PlayerProjectile))
			{
				continue;
			}

			if (unitInst->boundingBox.overlaps(unitInst2->boundingBox))
			{
				auto iter1 = collisionPairs.find(unitInst);
				auto iter2 = collisionPairs.find(unitInst2);
				bool exists1 = false;
				bool exists2 = false;

				if (iter1 != collisionPairs.end() && iter1->second == unitInst2)
				{
					exists1 = true;
				}

				if (iter2 != collisionPairs.end() && iter2->second == unitInst)
				{
					exists2 = true;
				}

				if (!exists1 && !exists2)
				{
					collisionPairs[unitInst] = unitInst2;
				}
			}
		}
	}

	// check projectiles vs normal units
	for (u32 i = 0; i < lastProjectileIndex; i++)
	{
		ProjectileInstance& unitInst = projectiles[i];

		if (!unitInst.collide || !unitInst.used) continue;

		for (auto unitInst2 : unitInstances)
		{
			if (!unitInst2->collide) continue;

			if (&unitInst == unitInst2
				|| unitInst.unit->type == unitInst2->unit->type) continue;

			if (unitInst2->unit->type == UnitResource::Type::Enemy &&
				unitInst.unit->type == UnitResource::Type::EnemyProjectile)
			{
				continue;
			}

			if (unitInst2->unit->type == UnitResource::Type::Player &&
				unitInst.unit->type == UnitResource::Type::PlayerProjectile)
			{
				continue;
			}

			if (unitInst.boundingBox.overlaps(unitInst2->boundingBox))
			{
				auto iter1 = collisionPairs.find(&unitInst);
				auto iter2 = collisionPairs.find(unitInst2);
				bool exists1 = false;
				bool exists2 = false;

				if (iter1 != collisionPairs.end() && iter1->second == unitInst2)
				{
					exists1 = true;
				}

				if (iter2 != collisionPairs.end() && iter2->second == &unitInst)
				{
					exists2 = true;
				}

				if (!exists1 && !exists2)
				{
					collisionPairs[&unitInst] = unitInst2;
				}
			}
		}
	}

	std::vector<SpriteInstanceCollision> pixelCols;

	for (auto& cp : collisionPairs)
	{
		pixelCols.clear();

		if (cp.first->checkPixelCollision(cp.second, pixelCols))
		{
			if (cp.first->unit->type == UnitResource::Type::EnemyProjectile
				|| cp.first->unit->type == UnitResource::Type::PlayerProjectile)
			{
				cp.first->deleteMeNow = true;
			}

			if (cp.second->unit->type == UnitResource::Type::EnemyProjectile
				|| cp.second->unit->type == UnitResource::Type::PlayerProjectile)
			{
				cp.second->deleteMeNow = true;
			}
		}

		if (cp.first->scriptClass)
		{
			auto func = cp.first->scriptClass->getFunction("onCollide");

			if (func.isFunction())
			{
				func.call(cp.first->scriptClass->classInstance, cp.second);
			}
		}

		if (cp.second->scriptClass)
		{
			auto func = cp.second->scriptClass->getFunction("onCollide");

			if (func.isFunction())
			{
				func.call(cp.second->scriptClass->classInstance, cp.first);
			}
		}
	}
}

BeamCollisionInfo Game::checkBeamIntersection(UnitInstance* inst, SpriteInstance* sprInst, const Vec2& pos, f32 beamWidth)
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

	for (auto unitInst : unitInstances)
	{
		if (unitInst == inst) continue;
		if (!unitInst->collide) continue;

		for (auto sprInst2 : unitInst->spriteInstances)
		{
			if (sprInst == sprInst2) continue;
			if (!sprInst2->collide) continue;

			if (screenMode == ScreenMode::Vertical)
			{
				if (sprInst2->screenRect.y > pos.y) continue;

				if (sprInst2->screenRect.bottom() > pos.y && sprInst2->screenRect.y <= pos.y)
				{
					closest.valid = true;
					closest.distance = 0;
					closest.point = pos;
					closest.unitInst = inst;
					closest.spriteInst = sprInst2;

					return closest;
				}
			}
			else if (screenMode == ScreenMode::Horizontal)
			{
				if (sprInst2->screenRect.right() < pos.x) continue;

				if (sprInst2->screenRect.x < pos.x && sprInst2->screenRect.right() >= pos.x)
				{
					closest.valid = true;
					closest.distance = 0;
					closest.point = pos;
					closest.unitInst = inst;
					closest.spriteInst = sprInst2;

					return closest;
				}
			}

			if (sprInst2->screenRect.overlaps(rc))
			if (screenMode == ScreenMode::Vertical)
			{
				Vec2 col;
				col.x = pos.x;
				col.y = sprInst2->screenRect.bottom();

				// for small sprites
				bool isInsideBeam = rc.contains(sprInst2->screenRect);
				bool isTouchingHalfBeam = false;
				bool pixelCollided = false;

				f32 relativeX = round(col.x - sprInst2->screenRect.x);

				// if the middle of the beam is outside the sprite rect
				// one of the beam haves are touching the sprite, so just set hit to middle of sprite
				if (relativeX < 0 || relativeX >= sprInst2->screenRect.width)
				{
					if (fabs(relativeX) <= beamWidth / 2 || (sprInst2->screenRect.width - relativeX) <= beamWidth / 2)
					{
						isTouchingHalfBeam = true;
						col.y = sprInst2->screenRect.center().y;
					}

					continue;
				}
				else
				{
					Rect frmRc = sprInst2->sprite->getSheetFramePixelRect(sprInst2->animationFrame);

					for (int y = sprInst2->sprite->frameHeight - 1; y >= 0; y--)
					{
						u8* p = (u8*)&sprInst2->sprite->image->imageData[
							(u32)(frmRc.y + y) * sprInst2->sprite->image->width
								+ (u32)(frmRc.x + relativeX)];

						if (p[3] == 0xff)
						{
							pixelCollided = true;
							col.y -= sprInst2->screenRect.height - y;
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
						closest.unitInst = inst;
						closest.spriteInst = sprInst2;
					}
				}
			}
			else if (screenMode == ScreenMode::Horizontal)
			{
				Vec2 col;
				col.x = sprInst2->screenRect.x;
				col.y = pos.y;

				// for small sprites
				bool isInsideBeam = rc.contains(sprInst2->screenRect);
				bool isTouchingHalfBeam = false;
				bool pixelCollided = false;

				f32 relativeY = round(col.y - sprInst2->screenRect.y);

				// if the middle of the beam is outside the sprite rect
				// one of the beam haves are touching the sprite, so just set hit to middle of sprite
				if (relativeY < 0 || relativeY >= sprInst2->screenRect.height)
				{
					if (fabs(relativeY) <= beamWidth / 2 || (sprInst2->screenRect.height - relativeY) <= beamWidth / 2)
					{
						isTouchingHalfBeam = true;
						col.x = sprInst2->screenRect.center().x;
					}

					continue;
				}
				else
				{
					Rect frmRc = sprInst2->sprite->getSheetFramePixelRect(sprInst2->animationFrame);

					for (int x = sprInst2->sprite->frameWidth - 1; x >= 0; x--)
					{
						u8* p = (u8*)&sprInst2->sprite->image->imageData[
							(u32)(frmRc.y + relativeY) * sprInst2->sprite->image->width
								+ (u32)(frmRc.x + x)];

						if (p[3] == 0xff)
						{
							pixelCollided = true;
							col.x -= sprInst2->screenRect.width - x;
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
						closest.unitInst = inst;
						closest.spriteInst = sprInst2;
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
	printf("Preloading sprites...\n");

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

	printf("Packing atlas sprites...\n");
	graphics->atlas->pack();

	printf("Computing sprite params after packing...\n");

	for (auto sprite : resourceLoader->sprites)
	{
		sprite->computeParamsAfterAtlasGeneration();
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
		printf("Level: %s in %s\n", lvlInfoJson["title"].asCString(), lvlInfoJson["file"].asCString());
		levels.push_back(std::make_pair(lvlInfoJson["title"].asCString(), lvlInfoJson["file"].asString()));
	}

	return true;
}

void Game::deleteNonPersistentUnitInstances()
{
	auto iter = unitInstances.begin();

	while (iter != unitInstances.end())
	{
		if ((*iter)->unit->type != UnitResource::Type::Player)
		{
			delete *iter;
			iter = unitInstances.erase(iter);
		}
		else
		{
			++iter;
		}
	}
}

bool Game::changeLevel(i32 index)
{
	deleteNonPersistentUnitInstances();
	projectiles.clear();
	projectiles.resize(maxProjectileCount);

	if (index == -1)
	{
		currentLevelIndex++;
	}

	if (index >= levels.size())
	{
		printf("ERROR: Level index out of range %d\n", index);
		return false;
	}

	currentLevelIndex = index;
	auto level = resourceLoader->loadLevel(levels[currentLevelIndex].second);

	for (auto& layer : level->layers)
	{
		layers.push_back(layer);
	}

	for (auto& inst : level->unitInstances)
	{
		unitInstances.push_back(inst);
	}

	resourceLoader->unload(level);

	return true;
}

SpriteInstance* Game::createSpriteInstance(SpriteResource* sprite)
{
	SpriteInstance* inst = new SpriteInstance();
	inst->sprite = sprite;
	inst->setFrameAnimation("default");

	return inst;
}


UnitInstance* Game::createUnitInstance(UnitResource* unit)
{
	auto unitInst = new UnitInstance();

	unitInst->initializeFrom(unit);
	newUnitInstances.push_back(unitInst);

	return unitInst;
}

WeaponInstance* Game::createWeaponInstance(const std::string& weaponResFilename, struct UnitInstance* unitInst, struct SpriteInstance* spriteInst)
{
	WeaponInstance* weaponInst = new WeaponInstance();
	auto weaponRes = resourceLoader->loadWeapon(weaponResFilename);

	weaponInst->parentUnitInstance = unitInst;
	weaponInst->attachTo = spriteInst;
	weaponInst->initializeFrom(weaponRes);

	return weaponInst;
}

ProjectileInstance* Game::newProjectileInstance()
{
	for (u32 i = 0; i < maxProjectileCount; i++)
	{
		if (!projectiles[i].used)
		{
			ProjectileInstance* inst = &projectiles[i];

			inst->used = true;

			if (lastProjectileIndex < i + 1)
				lastProjectileIndex = i + 1;

			return inst;
		}
	}

	return nullptr;
}

void Game::releaseProjectileInstance(ProjectileInstance* inst)
{
	inst->used = false;
}

}
