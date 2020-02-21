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
#include "unit_controller.h"
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
	
	//int result = 0;
	//int flags = MIX_INIT_MP3;

	//if (flags != (result = Mix_Init(flags))) {
	//	printf("Could not initialize mixer (result: %d).\n", result);
	//	printf("Mix_Init: %s\n", Mix_GetError());
	//	exit(1);
	//}

	//initializeAudio();

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

	currentMainScript = resourceLoader->loadScript("scripts/ingame_screen.lua");
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
		players[i] = new UnitInstance();
		players[i]->initializeFrom(resourceLoader->loadUnit("units/player"));
		unitInstances.push_back(players[i]);
		players[i]->name = "Player" + std::to_string(i + 1);
		players[i]->rootSpriteInstance->transform.position.x = graphics->videoWidth / 2;
		players[i]->rootSpriteInstance->transform.position.y = graphics->videoHeight / 2;
		static_cast<PlayerController*>(players[i]->findController("main"))->playerIndex = i;
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

		if (currentMainScript)
		if (currentMainScript->getFunction("onUpdate").isFunction())
			currentMainScript->getFunction("onUpdate").call(deltaTime);

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

		// add the new created instances
		unitInstances.insert(unitInstances.end(), newUnitInstances.begin(), newUnitInstances.end());
		newUnitInstances.clear();

		UnitInstance::updateShadowToggle();

		auto iter = unitInstances.begin();
		checkCollisions();

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

		// update and render the game graphics render target
		graphics->setupRenderTargetRendering();
		graphics->beginFrame();

		for (auto inst : unitInstances)
		{
			inst->render(graphics);
		}

		if (currentMainScript)
		{
			if (currentMainScript->getFunction("onRender").isFunction())
				currentMainScript->getFunction("onRender").call(0);
		}

		static FontResource* fnt = nullptr;

		//TODO: remove
		if (!fnt) fnt = resourceLoader->loadFont("fonts/default");
		static f32 time = 0;

		graphics->currentColorMode = (u32)ColorMode::Mul;
		graphics->currentColor = Color::red.getRgba();
		graphics->drawText(fnt, { 90, 2 }, "HISCORE");
		graphics->currentColorMode = (u32)ColorMode::Add;
		graphics->currentColor = Color::green.getRgba();
		graphics->drawText(fnt, { 90, 12 }, "OOOOOOO");

		if (time > 0.2 && time < 1)
		{
			graphics->currentColorMode = (u32)ColorMode::Mul;
			graphics->currentColor = Color::red.getRgba();
			graphics->drawText(fnt, { 8, 17 }, "   PLEASE");
			graphics->currentColorMode = (u32)ColorMode::Add;
			graphics->currentColor = Color::black.getRgba();
			graphics->drawText(fnt, { 10, 25 }, "INSERT COIN");

			graphics->currentColorMode = (u32)ColorMode::Add;
			graphics->currentColor = Color::sky.getRgba();
			graphics->drawText(fnt, { 135, 17 }, "   PLEASE");
			graphics->currentColorMode = (u32)ColorMode::Add;
			graphics->currentColor = Color::black.getRgba();
			graphics->drawText(fnt, { 140, 25 }, "INSERT COIN");
		}

		time += deltaTime;
		if (time > 1) time = 0;

		graphics->endFrame();

		// display the render target contents in the main backbuffer
		graphics->blitRenderTarget();
		SDL_GL_SwapWindow(window);
	}
}

void Game::checkCollisions()
{
	std::unordered_map<UnitInstance*, UnitInstance*> collisionPairs;

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

	for (auto& cp : collisionPairs)
	{
		if (cp.first->script)
		{
			auto func = cp.first->script->getFunction("onCollide");
			if (func.isFunction())
			{
				func.call(cp.first, cp.second);
			}
		}

		if (cp.second->script)
		{
			auto func2 = cp.second->script->getFunction("onCollide");
			if (func2.isFunction())
			{
				func2.call(cp.second, cp.first);
			}
		}
	}
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

UnitController* Game::createUnitController(const std::string& name)
{
	if (name == "player")
	{
		return new PlayerController(this);
	}
	else if (name == "simple_enemy")
	{
		return new SimpleEnemyController();
	}
	else if (name == "projectile")
	{
		return new ProjectileController();
	}

	else if (name == "background")
	{
		return new BackgroundController();
	}

	std::cout << "Unknown unit controller type: " << name << std::endl;

	return nullptr;
}

}
