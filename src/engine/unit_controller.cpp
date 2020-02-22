#include "unit_controller.h"
#include "unit_instance.h"
#include "weapon_instance.h"
#include "game.h"
#include "utils.h"
#include "resource_loader.h"
#include "graphics.h"
#include "sound_instance.h"
#include "projectile_instance.h"
#include "sprite_instance.h"
#include "resources/sound_resource.h"

namespace engine
{
UnitController* UnitController::create(const std::string& ctrlerName, UnitInstance* unitInst)
{
	if (ctrlerName == "SimpleEnemy")
	{
		auto controller = new SimpleEnemyController();
		controller->unitInstance = unitInst;
		return controller;
	}
	else if (ctrlerName == "Background")
	{
		auto controller = new BackgroundController();
		controller->unitInstance = unitInst;
		return controller;
	}
	else if (ctrlerName == "Player")
	{
		auto controller = new PlayerController(Game::instance);
		controller->unitInstance = unitInst;
		return controller;
	}
	else if (ctrlerName == "Follow")
	{
		auto controller = new FollowController();
		controller->unitInstance = unitInst;
		return controller;
	}
	else if (ctrlerName == "ScreenFx")
	{
		auto controller = new ScreenFxController();
		controller->unitInstance = unitInst;
		return controller;
	}

	return nullptr;
}

void ScreenFxController::shakeCamera(const Vec2& force, f32 duration, u32 count)
{
	doingShake = true;
	shakeTimer = 0;
	shakeForce = force;
	shakeCount = shakeCounter = count;
	shakeDuration = duration;
}

void ScreenFxController::fadeScreen(const Color& color, ColorMode colorMode, f32 duration, bool revertBackAfter)
{
	doingFade = true;
	fadeTimer = 0;
	fadeTimerDir = 1;
	fadeColor = color;
	fadeDuration = duration;
	fadeColorMode = colorMode;
	fadeRevertBackAfter = revertBackAfter;
}

void ScreenFxController::update(struct Game* game)
{
	if (doingShake)
	{
		f32 slice = shakeDuration / (f32)shakeCount;
		shakeTimer += game->deltaTime;

		if (shakeTimer >= slice && !shakeCounter)
		{
			doingShake = false;
			game->cameraPositionOffset.clear();
		}
		else
		if (shakeTimer >= slice && shakeCounter)
		{
			shakeTimer = 0;
			f32 x = shakeForce.x * (f32)shakeCounter / (f32)shakeCount;
			f32 y = shakeForce.y * (f32)shakeCounter / (f32)shakeCount;
			game->cameraPositionOffset.x = randomFloat(-x, x);
			game->cameraPositionOffset.y = randomFloat(-y, y);
			shakeCounter--;
		}
	}

	if (doingFade)
	{
		fadeTimer += game->deltaTime * fadeTimerDir * 1.0 / fadeDuration;

		if (fadeTimer >= 1 && fadeTimerDir > 0)
		{
			if (fadeRevertBackAfter)
			{
				fadeTimer = 1;
				fadeTimerDir = -1;
				fadeRevertBackAfter = false;
			}
			else
			{
				doingFade = false;
				fadeTimer = 1;
				game->graphics->renderTargetColor = fadeColor.getRgba();
			}
		}
		else if (fadeTimerDir < 0 && fadeTimer < 0)
		{
			doingFade = false;
			game->graphics->renderTargetColor = Color(0, 0, 0, 1).getRgba();
			game->graphics->renderTargetColorMode = ColorMode::Add;
		}
		else
		{
			Color c = fadeColor;

			if (fadeColorMode == ColorMode::Add
				|| fadeColorMode == ColorMode::Sub)
			{
				c.r *= fadeTimer;
				c.g *= fadeTimer;
				c.b *= fadeTimer;
				c.a = 1;
			}
			else if (fadeColorMode == ColorMode::Mul)
			{
				c.r = 1.0 + fadeTimer * (c.r - 1.0f);
				c.g = 1.0 + fadeTimer * (c.g - 1.0f);
				c.b = 1.0 + fadeTimer * (c.b - 1.0f);
				c.a = 1;
			}

			game->graphics->renderTargetColor = c.getRgba();
			game->graphics->renderTargetColorMode = fadeColorMode;
		}
	}
}

void BackgroundController::update(struct Game* game)
{
	if (!unitInstance || !unitInstance->rootSpriteInstance) return;

	unitInstance->rootSpriteInstance->transform.position.y += unitInstance->speed * game->deltaTime;
}

void SimpleEnemyController::update(struct Game* game)
{
	if (!unitInstance || !unitInstance->rootSpriteInstance) return;

	unitInstance->rootSpriteInstance->transform.position.y += unitInstance->speed * game->deltaTime;

	if (unitInstance->rootSpriteInstance->transform.position.y + game->cameraPosition.y > game->graphics->videoHeight)

	unitInstance->rootSpriteInstance->transform.position.y = -32;
}

void ProjectileController::update(struct Game* game)
{
	if (!unitInstance || !unitInstance->rootSpriteInstance) return;
	
	auto projInst = (ProjectileInstance*)unitInstance;

	// keep projectile in the screen space
	unitInstance->rootSpriteInstance->transform.position.y -= game->cameraSpeed * game->deltaTime;

	unitInstance->rootSpriteInstance->transform.position += projInst->velocity * unitInstance->speed * game->deltaTime;
	projInst->speed += projInst->speed * projInst->acceleration * game->deltaTime;
	clampValue(projInst->speed, projInst->minSpeed, projInst->maxSpeed);
}

void FollowController::acquireOffset()
{
	if (!follow) follow = unitInstance->rootSpriteInstance;

	if (follower && follow)
	{
		follower->notRelativeToRoot = true; // force to not transform locally to parent
		offset = follower->transform.position;
		follower->transform.position = follow->transform.position + offset;
		offsetAcquired = true;
	}
}

void FollowController::update(struct Game* game)
{
	if (!offsetAcquired) acquireOffset();

	if (follow && follower)
	{
		auto targetPos = follow->transform.position + offset;

		if (constantSpeed)
		{
			auto delta = targetPos - follower->transform.position;
			auto dist = delta.getLength();

			if (dist <= speed * game->deltaTime)
			{
				follower->transform.position = targetPos;
			}
			else
			{
				delta.normalize();
				follower->transform.position += delta * speed * game->deltaTime;
			}
		}
		else
		{
			follower->transform.position =
				follower->transform.position +
				(targetPos - follower->transform.position) * speed * game->deltaTime;
		}
	}
}

void FollowController::initializeFromJson(const Json::Value& value)
{
	follow = unitInstance->findSpriteInstance(value.get("follow", "").asString());
	follower = unitInstance->findSpriteInstance(value.get("follower", "").asString());
	speed = value.get("speed", speed).asFloat();
	constantSpeed = value.get("constantSpeed", constantSpeed).asBool();
}

PlayerController::PlayerController(Game* game)
{
	//TODO
	if (game)
	{
		//fireSoundRes = game->resourceLoader->loadSound("sounds/Laser01.wav");
		//fireSound = new SoundInstance();
		//fireSound->soundResource = fireSoundRes;
	}
}

void PlayerController::update(struct Game* game)
{
	if (!active)
		return;

	if (!unitInstance || !unitInstance->rootSpriteInstance) return;
	static f32 t = 0;

	unitInstance->rootSpriteInstance->transform.position.y -= game->cameraSpeed * game->deltaTime;

	if (game->isPlayerFire1(playerIndex) && !isFirePressed)
	{
		isFirePressed = true;
		for (auto& wp: unitInstance->weapons)
		{
			wp.second->fire();
		}
	}
	
	if (!game->isPlayerFire1(playerIndex))
	{
		isFirePressed = false;
	}

	if (isFirePressed)
	{
		for (auto& wp : unitInstance->weapons)
		{
			wp.second->active = true;
		}
	}
	else
	{
		for (auto& wp : unitInstance->weapons)
		{
			wp.second->active = false;
		}
	}

	if (game->isControlDown(InputControl::Player1_Fire2))
	{
		unitInstance->rootSpriteInstance->hit(10);
	}

	Vec2 moveDir;

	if (game->isPlayerMoveLeft(playerIndex))
	{
		moveDir.x = -1;
	}

	if (game->isPlayerMoveRight(playerIndex))
	{
		moveDir.x = 1;
	}

	if (game->isPlayerMoveUp(playerIndex))
	{
		moveDir.y = -1;
	}

	if (game->isPlayerMoveDown(playerIndex))
	{
		moveDir.y = 1;
	}

	moveDir.normalize();

	unitInstance->rootSpriteInstance->transform.position += moveDir * game->deltaTime * unitInstance->speed;

	clampValue(unitInstance->rootSpriteInstance->transform.position.x, -game->cameraParallaxOffset, game->graphics->videoWidth - game->cameraParallaxOffset);
	clampValue(unitInstance->rootSpriteInstance->transform.position.y,
		-game->cameraPosition.y,
		-game->cameraPosition.y + game->graphics->videoHeight);

	game->cameraParallaxOffset = (game->graphics->videoWidth / 2 - unitInstance->rootSpriteInstance->transform.position.x) * 0.3f;
}

}
