#include "unit_instance.h"
#include "unit_controller.h"
#include "sprite_instance.h"
#include "resources/sprite_resource.h"
#include "graphics.h"
#include "game.h"
#include "image_atlas.h"
#include "animation.h"
#include "weapon_instance.h"

namespace engine
{
void UnitInstance::cloneTo(UnitInstance* clone)
{
	clone->boundingBox = boundingBox;
	clone->collide = collide;
	clone->color = color;
	clone->currentSpriteInstanceAnimation = currentSpriteInstanceAnimation;
	clone->hasShadows = hasShadows;
	clone->hitColor = hitColor;
	clone->isHit = isHit;
	clone->name = name;
	clone->shadowOffset = shadowOffset;
	clone->shadowScale = shadowScale;
	clone->shadowToggle = shadowToggle;
	clone->speed = speed;
	clone->team = team;
	clone->transform = transform;
	clone->type = type;
	clone->unit = unit;
	clone->visible = visible;
	clone->deleteMeNow = deleteMeNow;
	clone->deleteOnOutOfScreen = deleteOnOutOfScreen;
	clone->controller = controller;
	clone->script = script;

	// clone sprite instances
	for (auto& spriteInst : spriteInstances)
	{
		auto sprInst = new SpriteInstance();
		sprInst->name = spriteInst->name;
		sprInst->sprite = spriteInst->sprite;
		sprInst->transform = spriteInst->transform;
		sprInst->spriteAnimationInstance = spriteInst->spriteAnimationInstance;
		sprInst->setAnimation("default");
		clone->spriteInstances.push_back(sprInst);
	}

	// clone sprite instance animations
	for (auto& spriteInstAnim : spriteInstanceAnimations)
	{
		auto& arr = clone->spriteInstanceAnimations[spriteInstAnim.first] = SpriteInstanceAnimations();

		for (auto& anim : spriteInstAnim.second)
		{
			SpriteInstanceAnimation sia;
			sia.spriteInstance = clone->spriteInstances[indexOfSpriteInstance(anim.spriteInstance)];
			sia.animationInstance = new AnimationInstance();
			*sia.animationInstance = *anim.animationInstance;
			arr.push_back(sia);
		}
	}

	// clone weapon instances
	for (auto& wi : weapons)
	{
		WeaponInstance* wiNew = new WeaponInstance();

		*wiNew = *wi;
		wiNew->parentUnitInstance = clone;
		wiNew->attachTo = clone->spriteInstances[indexOfSpriteInstance(wi->attachTo)];
		clone->weapons.push_back(wiNew);
	}
}

void UnitInstance::update(Game* game)
{
	if (controller)
	{
		controller->update(game);
	}

	for (auto inst : spriteInstances)
	{
		inst->update(game);
	}

	computeBoundingBox();

	if (deleteOnOutOfScreen)
	{
		if (boundingBox.x > game->graphics->videoWidth
			|| boundingBox.y > game->graphics->videoHeight
			|| boundingBox.right() < 0
			|| boundingBox.top() < 0)
		{
			deleteMeNow = true;
		}
	}
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

size_t UnitInstance::indexOfSpriteInstance(struct SpriteInstance* spriteInst)
{
	auto iter = std::find(spriteInstances.begin(), spriteInstances.end(), spriteInst);

	if (iter == spriteInstances.end())
		return -1;

	return std::distance(spriteInstances.begin(), iter);
}

}
