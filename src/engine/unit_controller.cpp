#include "unit_controller.h"
#include "unit_instance.h"
#include "weapon_instance.h"
#include "game.h"
#include "utils.h"
#include "resource_loader.h"
#include "graphics.h"
#include "sound_instance.h"
#include "projectile_instance.h"
#include "resources/sound_resource.h"

namespace engine
{
void BackgroundController::update(struct Game* game)
{
	if (!unitInstance) return;

	unitInstance->transform.position.y += unitInstance->speed * game->deltaTime;
}

void SimpleEnemyController::update(struct Game* game)
{
	if (!unitInstance) return;

	unitInstance->transform.position.y += unitInstance->speed * game->deltaTime;
	unitInstance->transform.position.x += sinf(unitInstance->transform.position.y)*50.3f * game->deltaTime;

	if (unitInstance->transform.position.y > game->graphics->videoHeight)
		unitInstance->transform.position.y = -32;
}

void ProjectileController::update(struct Game* game)
{
	if (!unitInstance) return;
	
	auto projInst = (ProjectileInstance*)unitInstance;

	unitInstance->transform.position += projInst->velocity * unitInstance->speed * game->deltaTime;
	projInst->speed += projInst->speed * projInst->acceleration * game->deltaTime;
	clampValue(projInst->speed, projInst->minSpeed, projInst->maxSpeed);
}

PlayerController::PlayerController(Game* game)
{
	fireSoundRes = game->resourceLoader->loadSound("sounds/Laser01.wav");
	fireSound = new SoundInstance();
	fireSound->soundResource = fireSoundRes;
}

void PlayerController::update(struct Game* game)
{
	if (!unitInstance) return;
	static f32 t = 0;

	if (game->isPlayerFire1(playerIndex) && !isFirePressed)
	{
		isFirePressed = true;
		for (auto& wp: unitInstance->weapons)
		{
			wp->fire();
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
			wp->update(game);
		}
	}

	if (game->isPlayerMoveLeft(playerIndex))
	{
		unitInstance->transform.position.x -= game->deltaTime * unitInstance->speed;
	}

	if (game->isPlayerMoveRight(playerIndex))
	{
		unitInstance->transform.position.x += game->deltaTime * unitInstance->speed;
	}

	if (game->isPlayerMoveUp(playerIndex))
	{
		unitInstance->transform.position.y -= game->deltaTime * unitInstance->speed;
	}

	if (game->isPlayerMoveDown(playerIndex))
	{
		unitInstance->transform.position.y += game->deltaTime * unitInstance->speed;
	}

	clampValue(unitInstance->transform.position.x, unitInstance->boundingBox.width / 2, game->graphics->videoWidth - unitInstance->boundingBox.width / 2);
	clampValue(unitInstance->transform.position.y, unitInstance->boundingBox.height, game->graphics->videoHeight - unitInstance->boundingBox.height / 2);
}

}