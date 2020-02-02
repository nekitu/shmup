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
#include "SDL_mixer.h"
#include "animation_instance.h"
#include "unit_controller.h"
#include "resources/sound_resource.h"
#include "resources/unit_resource.h"
#include "resources/sprite_resource.h"
#include "resources/weapon_resource.h"
#include "resources/level_resource.h"
#include "resources/script_resource.h"
#include "unit_instance.h"
#include "sprite_instance.h"
#include "music_instance.h"
#include "sound_instance.h"
#include "weapon_instance.h"

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

	int err = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_AUDIO);

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
		windowWidth, windowHeight, SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI);

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
	mapSdlToControl[SDLK_RCTRL] = InputControl::Player1_Fire1;
	mapSdlToControl[SDLK_RSHIFT] = InputControl::Player1_Fire2;
	mapSdlToControl[SDLK_RALT] = InputControl::Player1_Fire3;
	mapSdlToControl[SDLK_a] = InputControl::Player2_MoveLeft;
	mapSdlToControl[SDLK_d] = InputControl::Player2_MoveRight;
	mapSdlToControl[SDLK_w] = InputControl::Player2_MoveUp;
	mapSdlToControl[SDLK_s] = InputControl::Player2_MoveDown;
	mapSdlToControl[SDLK_b] = InputControl::Player2_Fire1;
	mapSdlToControl[SDLK_g] = InputControl::Player2_Fire2;
	mapSdlToControl[SDLK_t] = InputControl::Player2_Fire3;

	//TODO: remove
	music = new MusicInstance();
	music->musicResource = resourceLoader->loadMusic("music/Retribution.ogg");
	music->play();
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
		players[i]->instantiateFrom(resourceLoader->loadUnit("units/player"));
		unitInstances.push_back(players[i]);
		players[i]->type = UnitResource::Type::Player;
		players[i]->name = "Player" + std::to_string(i + 1);
		players[i]->speed = 120;
		players[i]->hasShadows = true;
		players[i]->shadowOffset.set(40, 40);
		players[i]->shadowScale = 0.4f;
		players[i]->rootSpriteInstance->transform.position.x = graphics->videoWidth / 2;
		players[i]->rootSpriteInstance->transform.position.y = graphics->videoHeight / 2;
		players[i]->controller = new PlayerController(this);
		players[i]->controller->unitInstance = players[i];
	
		auto wp = createWeaponInstance("weapons/default", players[i], nullptr);
		players[i]->weapons.push_back(wp);
		((PlayerController*)players[i]->controller)->playerIndex = i;
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

		if (isControlDown(InputControl::Exit))
			exitGame = true;

		// update and render the game graphics render target
		graphics->setupRenderTargetRendering();
		graphics->beginFrame();

		UnitInstance::updateShadowToggle();

		for (u32 i = 0; i < unitInstances.size(); i++)
		{
			unitInstances[i]->update(this);
		}

		auto iter = unitInstances.begin();

		while (iter != unitInstances.end())
		{
			if ((*iter)->deleteMeNow)
			{
				delete *iter;
				iter = unitInstances.erase(iter);
			}
			else
			{
				iter++;
			}
		}

		for (auto inst : unitInstances)
		{
			inst->render(graphics);
		}

		graphics->endFrame();

		// display the render target contents in the main backbuffer
		graphics->blitRenderTarget();
		SDL_GL_SwapWindow(window);
	}
}

void Game::computeDeltaTime()
{
	f32 ticks = SDL_GetTicks();
	deltaTime = (ticks - lastTime) / 1000.0f;
	lastTime = ticks;
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
		auto lvl = resourceLoader->loadLevel(lvlInfoJson["file"].asString());
		
		if (lvl)
		{
			levels.push_back(lvl);
		}
	}

	return true;
}

void Game::deleteNonPersistentUnitInstances()
{
	auto iter = unitInstances.begin();

	while (iter != unitInstances.end())
	{
		if ((*iter)->type == UnitResource::Type::Enemy
			|| (*iter)->type == UnitResource::Type::Item
			|| (*iter)->type == UnitResource::Type::EnemyProjectile
			|| (*iter)->type == UnitResource::Type::PlayerProjectile)
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

	LevelResource* lev = levels[index];

	// clone level unit instances to main game
	for (u32 i = 0; i < lev->unitInstances.size(); i++)
	{
		auto uinst = new UnitInstance();
		uinst->copyFrom(lev->unitInstances[i]);
		unitInstances.push_back(uinst);
	}

	currentLevelIndex = index;
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

	unitInst->instantiateFrom(unit);
	unitInstances.push_back(unitInst);

	return unitInst;
}

WeaponInstance* Game::createWeaponInstance(const std::string& weaponResFilename, struct UnitInstance* unitInst, struct SpriteInstance* spriteInst)
{
	WeaponInstance* weaponInst = new WeaponInstance();
	auto weaponRes = resourceLoader->loadWeapon(weaponResFilename);

	weaponInst->parentUnitInstance = unitInst;
	weaponInst->attachTo = spriteInst;
	weaponInst->setWeaponResource(weaponRes);

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
