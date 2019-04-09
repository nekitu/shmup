#include "game.h"
#include "graphics.h"
#define GLEW_STATIC
#include <GL/glew.h>
#include "color.h"
#include "texture_array.h"
#include "utils.h"
#include "resource_loader.h"
#include "image_atlas.h"
#include "SDL_mixer.h"
#include "resources/sound_resource.h"
#include "resources/unit_resource.h"
#include "resources/sprite_resource.h"
#include "sprite_instance.h"
#include "animation.h"
#include <iostream>

namespace engine
{
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

	music = new MusicInstance();
	music->musicResource = resourceLoader->loadMusic("../data/music/Retribution.ogg");
	music->play();


	lastTime = SDL_GetTicks();
	graphics = new Graphics(this);
	resourceLoader = new ResourceLoader();
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
}

void Game::shutdown()
{
	Mix_CloseAudio();
}

void Game::createPlayers()
{
	UnitInstance* enemy = new UnitInstance();

	enemy->team = Team::Neutral;
	enemy->color = Color::white;
	enemy->speed = 10;
	auto ctrler = new BackgroundController();
	ctrler->unitInstance = enemy;
	enemy->controller = ctrler;

	SpriteInstance* inst = new SpriteInstance();
	inst->sprite = resourceLoader->loadSprite("sprites/clouds2", graphics->atlas);

	enemy->transform.position.x = graphics->videoWidth / 2;
	enemy->transform.position.y = -(f32)inst->sprite->image->height / 2.0f;

	enemy->spriteInstances.push_back(inst);

	unitInstances.push_back(enemy);

	//TODO: remove when level is made
	for (u32 i = 0; i < 5; i++)
	{
		UnitInstance* enemy = new UnitInstance();
		
		enemy->team = Team::Enemy;
		enemy->color = Color::green;
		enemy->speed = randomFloat(4, 20);
		enemy->transform.scale = 2.0f;
		enemy->transform.position.x = randomFloat(0, graphics->videoWidth);
		enemy->transform.position.y = randomFloat(0, graphics->videoHeight);
		enemy->transform.verticalFlip = true;
		auto ctrler = new SimpleEnemyController();
		ctrler->unitInstance = enemy;
		enemy->controller = ctrler;

		SpriteInstance* inst = new SpriteInstance();
		inst->sprite = resourceLoader->loadSprite("sprites/sample_sprite", graphics->atlas);
		inst->setAnimation("default");
		enemy->spriteInstances.push_back(inst);

		unitInstances.push_back(enemy);
	}

	for (u32 i = 0; i < 1; i++)
	{
		players[i] = new UnitInstance();
		unitInstances.push_back(players[i]);
		players[i]->team = Team::Player;
		players[i]->name = "Player" + std::to_string(i + 1);
		players[i]->speed = 120;
		//players[i]->transform.verticalFlip = true;
		players[i]->hasShadows = true;
		players[i]->shadowOffset.set(40, 40);
		players[i]->shadowScale = 0.4f;
		SpriteInstance* inst = new SpriteInstance();
		inst->sprite = resourceLoader->loadSprite("sprites/sample_sprite", graphics->atlas);
		inst->setAnimation("default");
		players[i]->spriteInstances.push_back(inst);

		players[i]->transform.position.x = graphics->videoWidth / 2;
		players[i]->transform.position.y = graphics->videoHeight - 64;
		players[i]->color = Color::white;
		players[i]->controller = new PlayerController();
		players[i]->controller->unitInstance = players[i];
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
		
		for (auto inst : unitInstances)
		{
			inst->update(this);
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

SpriteInstance* Game::createSpriteInstance(Sprite* sprite)
{
	SpriteInstance* inst = new SpriteInstance();
	inst->sprite = sprite;
	inst->setAnimation("default");

	return inst;
}

UnitInstance* Game::createUnitInstance(UnitResource* unit)
{
	auto unitInst = new UnitInstance();
	unitInstances.push_back(unitInst);
	unitInst->unit = unit;
	unitInst->team = unit->team;
	unitInst->name = unit->name;
	unitInst->speed = unit->speed;
	unitInst->transform = unit->transform;
	
	for (u32 i = 0; i < unit->spriteInstanceCount; i++)
	{
		SpriteInstance* sprInst = new SpriteInstance();
		sprInst->sprite = unit->spriteInstances[i].sprite;
		sprInst->setAnimation(unit->spriteInstances[i].spriteAnimationInstance.spriteAnimation->name);
		unitInst->spriteInstances.push_back(sprInst);
	}

	for (u32 i = 0; i < unit->spriteInstanceAnimationCount; i++)
	{
		unitInst->spriteInstanceAnimations[unit->spriteInstanceAnimations[i].animation->name] = unit->spriteInstanceAnimations[i].animation;
	}

	return unitInst;
}

ProjectileInstance* Game::createProjectileInstance(ProjectileResource* projectile)
{
	
}

UnitController* Game::createUnitController(const std::string& name)
{
	if (name == "player")
	{
		return new PlayerController();
	}

	std::cout << "Unknown unit controller type: " << name << std::endl;

	return nullptr;
}


}