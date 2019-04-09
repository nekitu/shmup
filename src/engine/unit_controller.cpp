#include "unit_controller.h"

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
	unitInstance->transform.position.x += sinf(unitInstance->transform.position.y)*0.3f;

	if (unitInstance->transform.position.y > game->graphics->videoHeight)
		unitInstance->transform.position.y = -32;
}

PlayerController::PlayerController()
{
	fireSound = new Sound();
	fireSound->load("..\\data\\sounds\\Laser01.wav");
}

void PlayerController::update(struct Game* game)
{
	if (!unitInstance) return;

	fire1PlayingTime += game->deltaTime;

	if (game->isPlayerFire1(playerIndex) && fire1PlayingTime > 1.0f / fire1Rate)
	{
		fire1PlayingTime = 0;
		fireSound->play();
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