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
void BackgroundController::update(struct Game* game)
{
	if (!unitInstance || !unitInstance->rootSpriteInstance) return;

	unitInstance->rootSpriteInstance->transform.position.y += unitInstance->speed * game->deltaTime;
}

void SimpleEnemyController::update(struct Game* game)
{
	if (!unitInstance || !unitInstance->rootSpriteInstance) return;

	unitInstance->rootSpriteInstance->transform.position.y += unitInstance->speed * game->deltaTime;
	unitInstance->rootSpriteInstance->transform.position.x -= sinf(unitInstance->rootSpriteInstance->transform.position.y)*10.3f * game->deltaTime;

	if (unitInstance->rootSpriteInstance->transform.position.y > game->graphics->videoHeight)
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

PlayerController::PlayerController(Game* game)
{
	if (game)
	{
		fireSoundRes = game->resourceLoader->loadSound("sounds/Laser01.wav");
		fireSound = new SoundInstance();
		fireSound->soundResource = fireSoundRes;
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

	clampValue(unitInstance->rootSpriteInstance->transform.position.x, unitInstance->boundingBox.width / 2, game->graphics->videoWidth - unitInstance->boundingBox.width / 2);
	clampValue(unitInstance->rootSpriteInstance->transform.position.y, unitInstance->boundingBox.height, game->graphics->videoHeight - unitInstance->boundingBox.height / 2);
}

}
