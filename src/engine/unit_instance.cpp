#include "unit_instance.h"
#include "unit_controller.h"
#include "sprite_instance.h"
#include "resources/sprite_resource.h"
#include "graphics.h"
#include "game.h"
#include "image_atlas.h"
#include "animation_instance.h"
#include "weapon_instance.h"
#include <algorithm>

namespace engine
{
bool UnitInstance::shadowToggle = true;

void UnitInstance::updateShadowToggle()
{
	shadowToggle = !shadowToggle;
}

void UnitInstance::copyFrom(UnitInstance* other)
{
	boundingBox = other->boundingBox;
	currentAnimationName = other->currentAnimationName;
	hasShadows = other->hasShadows;
	name = other->name;
	shadowOffset = other->shadowOffset;
	shadowScale = other->shadowScale;
	speed = other->speed;
	type = other->type;
	unit = other->unit;
	visible = other->visible;
	deleteMeNow = other->deleteMeNow;
	deleteOnOutOfScreen = other->deleteOnOutOfScreen;
	controller = other->controller->createNew();
	controller->unitInstance = this;
	script = other->script;

	// map from other unit to new sprite instances
	std::map<SpriteInstance*, SpriteInstance*> spriteInstMap;

	// copy sprite instances
	for (auto& otherSprInst : other->spriteInstances)
	{
		auto sprInst = new SpriteInstance();

		sprInst->copyFrom(otherSprInst);
		spriteInstances.push_back(sprInst);
		spriteInstMap[otherSprInst] = sprInst;
	}

	rootSpriteInstance = spriteInstMap[other->rootSpriteInstance];

	// if no root specified, use first sprite instance as root
	if (!rootSpriteInstance && spriteInstances.size())
	{
		rootSpriteInstance = spriteInstances[0];
	}

	// copy sprite instance animations
	for (auto& spriteInstAnim : other->spriteInstanceAnimations)
	{
		auto& animName = spriteInstAnim.first;
		auto& animMap = spriteInstAnim.second;

		spriteInstanceAnimations[animName] = SpriteInstanceAnimationMap();
		auto& crtAnimMap = spriteInstanceAnimations[animName];

		for (auto& anim : animMap)
		{
			AnimationInstance* newAnimInst = new AnimationInstance();

			newAnimInst->copyFrom(anim.second);
			crtAnimMap[spriteInstMap[anim.first]] = newAnimInst;
		}
	}

	// copy weapon instances
	for (auto& wi : other->weapons)
	{
		WeaponInstance* wiNew = new WeaponInstance();

		wiNew->copyFrom(wi);
		wiNew->parentUnitInstance = this;
		wiNew->attachTo = spriteInstMap[wi->attachTo];
		weapons.push_back(wiNew);
	}

	setAnimation(currentAnimationName);
}

void UnitInstance::instantiateFrom(UnitResource* res)
{
	unit = res;
	name = res->name;
	speed = res->speed;
	type = res->type;
	visible = res->visible;
	script = res->scriptResource;

	// map from other unit to new sprite instances
	std::map<SpriteInstanceResource*, SpriteInstance*> spriteInstMap;

	// create the sprite instances for this unit instance
	for (auto& iter : res->spriteInstances)
	{
		SpriteInstance* sprInst = new SpriteInstance();

		sprInst->instantiateFrom(iter.second);
		spriteInstances.push_back(sprInst);
		spriteInstMap[iter.second] = sprInst;
	}

	std::sort(spriteInstances.begin(), spriteInstances.end(), [](const SpriteInstance* a, const SpriteInstance* b) { return a->orderIndex > b->orderIndex; });

	if (res->rootSpriteInstanceName.size())
	{
		rootSpriteInstance = spriteInstMap[res->spriteInstances[res->rootSpriteInstanceName]];
	}

	// if no root specified, use first sprite instance as root
	if (!rootSpriteInstance && spriteInstances.size())
	{
		rootSpriteInstance = spriteInstances[0];
	}

	// copy sprite instance animations
	for (auto& spriteInstAnim : res->spriteInstances)
	{
		auto& animName = spriteInstAnim.first;
		auto& sprInstRes = spriteInstAnim.second;

		spriteInstanceAnimations[animName] = SpriteInstanceAnimationMap();
		auto& crtAnimMap = spriteInstanceAnimations[animName];

		for (auto& anim : sprInstRes->animations)
		{
			AnimationInstance* newAnimInst = new AnimationInstance();

			newAnimInst->instantiateFrom(anim.second);
			crtAnimMap[spriteInstMap[sprInstRes]] = newAnimInst;
		}
	}

	if (type == UnitResource::Type::Enemy)
	{
		controller = new SimpleEnemyController();
		controller->unitInstance = this;
	}
}

void UnitInstance::update(Game* game)
{
	if (controller)
	{
		controller->update(game);
	}

	if (spriteInstanceAnimationMap)
	{
		for (auto iter : *spriteInstanceAnimationMap)
		{
			auto sprInst = iter.first;
			auto sprAnimInst = iter.second;

			sprAnimInst->update(game->deltaTime);
			sprAnimInst->animateSpriteInstance(sprInst);
		}
	}

	for (auto sprInst : spriteInstances)
	{
		sprInst->update(game);
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
	if (rootSpriteInstance)
	{
		boundingBox.width = rootSpriteInstance->sprite->frameWidth * rootSpriteInstance->transform.scale;
		boundingBox.height = rootSpriteInstance->sprite->frameHeight * rootSpriteInstance->transform.scale;
		boundingBox.x = rootSpriteInstance->transform.position.x - boundingBox.width / 2;
		boundingBox.y = rootSpriteInstance->transform.position.y - boundingBox.height / 2;
	}
	
	for (auto sprInst : spriteInstances)
	{
		if (!sprInst->visible || sprInst == rootSpriteInstance) continue;

		auto pos = rootSpriteInstance->transform.position;
		f32 mirrorV = sprInst->transform.verticalFlip ? -1 : 1;
		f32 mirrorH = sprInst->transform.horizontalFlip ? -1 : 1;

		pos.x += sprInst->transform.position.x * mirrorH;
		pos.y += sprInst->transform.position.y * mirrorV;

		f32 renderW = sprInst->sprite->frameWidth * sprInst->transform.scale * rootSpriteInstance->transform.scale;
		f32 renderH = sprInst->sprite->frameHeight * sprInst->transform.scale * rootSpriteInstance->transform.scale;

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

void UnitInstance::setAnimation(const std::string& animName)
{
	if (spriteInstanceAnimations.find(animName) != spriteInstanceAnimations.end())
	{
		spriteInstanceAnimationMap = &spriteInstanceAnimations[currentAnimationName];
	}
}

void UnitInstance::render(Graphics* gfx)
{
	if (!visible)
		return;

	for (auto sprInst : spriteInstances)
	{
		if (!sprInst->visible) continue;

		auto pos = sprInst != rootSpriteInstance ? rootSpriteInstance->transform.position : Vec2();

		f32 mirrorV = sprInst->transform.verticalFlip ? -1 : 1;
		f32 mirrorH = sprInst->transform.horizontalFlip ? -1 : 1;

		if (sprInst != rootSpriteInstance)
		{
			if (rootSpriteInstance->transform.verticalFlip) mirrorV *= -1;
			if (rootSpriteInstance->transform.horizontalFlip) mirrorH *= -1;
		}

		pos.x += sprInst->transform.position.x * mirrorH;
		pos.y += sprInst->transform.position.y * mirrorV;

		f32 renderW = sprInst->sprite->frameWidth * sprInst->transform.scale * ((sprInst != rootSpriteInstance) ? rootSpriteInstance->transform.scale : 1.0f);
		f32 renderH = sprInst->sprite->frameHeight * sprInst->transform.scale * ((sprInst != rootSpriteInstance) ? rootSpriteInstance->transform.scale : 1.0f);

		pos.x -= renderW / 2.0f;
		pos.y -= renderH / 2.0f;

		Rect spriteRc = 
		{
				round(pos.x),
				round(pos.y),
				renderW,
				renderH
		};

		Rect uvRc = sprInst->sprite->getFrameUvRect(sprInst->animationFrame);

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

		gfx->currentColor = sprInst->color.getRgba();
		gfx->currentColorMode = (u32)sprInst->colorMode;

		if (sprInst->sprite->image->rotated)
		{
			gfx->drawQuadRot90(spriteRc, uvRc);
		}
		else
		{
			gfx->drawQuad(spriteRc, uvRc);
		}

		if (hasShadows)
		{
			if (shadowToggle)
			{
				spriteRc += shadowOffset;
				spriteRc.width *= shadowScale;
				spriteRc.height *= shadowScale;

				gfx->currentColor = 0;
				gfx->currentColorMode = (u32)ColorMode::Mul;

				if (sprInst->sprite->image->rotated)
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

}
