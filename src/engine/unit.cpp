#include "unit.h"
#include "graphics.h"
#include "game.h"
#include "sprite.h"
#include "sprite_instance.h"
#include "image_atlas.h"
#include "utils.h"
#include "sound.h"

namespace engine
{
void UnitInstance::update(Game* game)
{
	if (controller)
		controller->update(game);

	for (auto inst : spriteInstances)
	{
		inst->update(game);
	}

	computeBoundingBox();
}

void UnitInstance::computeBoundingBox()
{
	boundingBox.x = transform.position.x;
	boundingBox.y = transform.position.y;
	boundingBox.width = 0;
	boundingBox.height = 0;

	for (auto inst : spriteInstances)
	{
		auto pos = transform.position;
		f32 mirrorV = inst->transform.verticalFlip ? -1 : 1;
		f32 mirrorH = inst->transform.horizontalFlip ? -1 : 1;

		pos.x += inst->transform.position.x * mirrorH;
		pos.y += inst->transform.position.y * mirrorV;

		f32 renderW = inst->sprite->frameWidth * inst->transform.scale * transform.scale;
		f32 renderH = inst->sprite->frameHeight * inst->transform.scale * transform.scale;

		pos.x -= renderW / 2.0f;
		pos.y -= renderH / 2.0f;

		Rect spriteRc =
		{
			round(pos.x),
			round(pos.y),
			renderW,
			renderH
		};

		boundingBox.add(spriteRc);
	}
}

void UnitInstance::render(Graphics* gfx)
{
	if (!visible)
		return;

	auto& crtColor = isHit ? hitColor : color;
	gfx->currentColor = crtColor.getRgba();

	for (auto inst : spriteInstances)
	{
		auto pos = transform.position;
		f32 mirrorV = inst->transform.verticalFlip ? -1 : 1;
		f32 mirrorH = inst->transform.horizontalFlip ? -1 : 1;

		if (transform.verticalFlip) mirrorV *= -1;
		if (transform.horizontalFlip) mirrorH *= -1;

		pos.x += inst->transform.position.x * mirrorH;
		pos.y += inst->transform.position.y * mirrorV;

		f32 renderW = inst->sprite->frameWidth * inst->transform.scale * transform.scale;
		f32 renderH = inst->sprite->frameHeight * inst->transform.scale * transform.scale;

		pos.x -= renderW / 2.0f;
		pos.y -= renderH / 2.0f;

		Rect spriteRc = 
		{
				round(pos.x),
				round(pos.y),
				renderW,
				renderH
		};

		Rect uvRc = inst->sprite->getFrameUvRect(inst->spriteAnimationInstance.currentFrame);

		if (mirrorV < 0)
		{
			uvRc.y = uvRc.bottom();
			uvRc.height *= -1.0f;
		}

		if (mirrorH < 0)
		{
			uvRc.x = uvRc.right();
			uvRc.width *= -1.0f;
		}

		if (inst->sprite->image->rotated)
		{
			gfx->drawQuadRot90(spriteRc, uvRc);
		}
		else
		{
			gfx->drawQuad(spriteRc, uvRc);
		}

		if (hasShadows)
		{
			shadowToggle = !shadowToggle;

			if (shadowToggle)
			{
				spriteRc += shadowOffset;
				spriteRc.width *= shadowScale;
				spriteRc.height *= shadowScale;

				gfx->currentColor = Color::black.getRgba();

				if (inst->sprite->image->rotated)
				{
					gfx->drawQuadRot90(spriteRc, uvRc);
				}
				else
				{
					gfx->drawQuad(spriteRc, uvRc);
				}
			}
		}
	}
}

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