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

	if (ctrlerName == "Background")
	{
		auto controller = new BackgroundController();
		controller->unitInstance = unitInst;
		return controller;
	}

	if (ctrlerName == "Player")
	{
		auto controller = new PlayerController(Game::instance);
		controller->unitInstance = unitInst;
		return controller;
	}

	if (ctrlerName == "Follow")
	{
		auto controller = new FollowController();
		controller->unitInstance = unitInst;
		return controller;
	}

	return nullptr;
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

	unitInstance->rootSpriteInstance->transform.position += projInst->velocity * unitInstance->speed * game->deltaTime;
	projInst->speed += projInst->speed * projInst->acceleration * game->deltaTime;
	clampValue(projInst->speed, projInst->minSpeed, projInst->maxSpeed);
}

void FollowController::acquireOffset()
{
	if (!follow) follow = unitInstance->rootSpriteInstance;

	if (follower && follow)
	{
		follower->noRootParent = true; // force to not transform locally to parent
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
		auto worldPos = follow->transform.position + offset;

		follower->transform.position =
			follower->transform.position +
			(worldPos - follower->transform.position) * speed * game->deltaTime;
	}
}

void FollowController::initializeFromJson(const Json::Value& value)
{
	follow = unitInstance->findSpriteInstance(value.get("follow", "").asString());
	follower = unitInstance->findSpriteInstance(value.get("follower", "").asString());
	speed = value.get("speed", 1.0f).asFloat();
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
	if (!unitInstance || !unitInstance->rootSpriteInstance) return;
	static f32 t = 0;

	if (game->isPlayerFire1(playerIndex) && !isFirePressed)
	{
		isFirePressed = true;
		for (auto& wp: unitInstance->weapons)
		{
			wp.second->fire();
			//fireSound->play();
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

	if (game->isPlayerMoveLeft(playerIndex))
	{
		unitInstance->rootSpriteInstance->transform.position.x -= game->deltaTime * unitInstance->speed;
	}

	if (game->isPlayerMoveRight(playerIndex))
	{
		unitInstance->rootSpriteInstance->transform.position.x += game->deltaTime * unitInstance->speed;
	}

	if (game->isPlayerMoveUp(playerIndex))
	{
		unitInstance->rootSpriteInstance->transform.position.y -= game->deltaTime * unitInstance->speed;
	}

	if (game->isPlayerMoveDown(playerIndex))
	{
		unitInstance->rootSpriteInstance->transform.position.y += game->deltaTime * unitInstance->speed;
	}

	game->cameraSideOffset = (game->graphics->videoWidth / 2 - unitInstance->rootSpriteInstance->transform.position.x) * 0.5f;

	//clampValue(unitInstance->rootSpriteInstance->transform.position.x, unitInstance->boundingBox.width / 2, game->graphics->videoWidth - unitInstance->boundingBox.width / 2);
	//clampValue(unitInstance->rootSpriteInstance->transform.position.y, unitInstance->boundingBox.height, game->graphics->videoHeight - unitInstance->boundingBox.height / 2);
}

}
