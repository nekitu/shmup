#include "sprite_instance.h"
#include "game.h"
#include "resources/sprite_resource.h"
#include "resources/unit_resource.h"
#include "sprite_instance.h"
#include <assert.h>

namespace engine
{
void SpriteInstance::copyFrom(SpriteInstance* other)
{
	name = other->name;
	sprite = other->sprite;
	transform = other->transform;
	orderIndex = other->orderIndex;
	visible = other->visible;
	collide = other->collide;
	health = other->health;
	defaultColor = other->defaultColor;
	color = other->color;
	colorMode = other->colorMode;
		
	hitColor = other->hitColor;
	hitOldColorMode = other->hitOldColorMode;
	hitColorFlashSpeed = other->hitColorFlashSpeed;
	hitFlashCount = other->hitFlashCount;
	hitFlashActive = other->hitFlashActive;
	hitColorTimer = other->hitColorTimer;
	currentHitFlashCount = other->currentHitFlashCount;

	frameAnimation = other->frameAnimation;
	animationFrame = other->animationFrame;
	animationRepeatCount = other->animationRepeatCount;
	animationDirection = other->animationDirection;
	animationIsActive = other->animationIsActive;

	play();
}

void SpriteInstance::instantiateFrom(SpriteInstanceResource* res)
{
	name = res->name;
	sprite = res->sprite;
	transform = res->transform;
	orderIndex = res->orderIndex;
	defaultColor = color = res->color;
	colorMode = res->colorMode;
	hitColor = res->hitColor;
	animationFrame = 0;
	animationRepeatCount = 0;
	animationDirection = 1;
	animationIsActive = true;
	std::string animName = res->animationName;

	if (animName.empty() && res->animations.size())
	{
		animName = res->animations.begin()->first;
	}

	setFrameAnimation(animName);
}

void SpriteInstance::update(struct Game* game)
{
	// update the sprite frame animation
	if (frameAnimation && animationIsActive)
	{
		animationFrame += (f32)frameAnimation->framesPerSecond * game->deltaTime * animationDirection;

		if (frameAnimation->repeatCount)
		{
			animationRepeatCount++;

			if (animationRepeatCount >= frameAnimation->repeatCount)
			{
				animationIsActive = false;
			}
		}

		f32 frame = (u32)animationFrame;

		if (frameAnimation->type == SpriteFrameAnimation::Type::Normal)
		{
			if (frame > frameAnimation->startFrame + frameAnimation->frameCount - 1)
			{
				animationFrame = frameAnimation->startFrame;
			}
		}
		else if (frameAnimation->type == SpriteFrameAnimation::Type::Reversed)
		{
			if (frame < frameAnimation->startFrame)
			{
				animationFrame = frameAnimation->startFrame + frameAnimation->frameCount - 1;
			}
		}
		else if (frameAnimation->type == SpriteFrameAnimation::Type::PingPong)
		{
			if (animationDirection > 0 && frame > frameAnimation->startFrame + frameAnimation->frameCount - 1)
			{
				animationFrame = frameAnimation->startFrame + frameAnimation->frameCount - 1;
				animationDirection = -1;
			}
			else if (animationDirection < 0 && frame < frameAnimation->startFrame)
			{
				animationFrame = frameAnimation->startFrame;
				animationDirection = 1;
			}
		}
	}

	// update hit color
	if (hitFlashActive)
	{
		color = defaultColor + (hitColor - defaultColor) * hitColorTimer;
		hitColorTimer += hitColorFlashSpeed * game->deltaTime;

		if (hitColorTimer > 1.0f)
		{
			hitColorTimer = 0.0;
			currentHitFlashCount++;

			if (currentHitFlashCount >= hitFlashCount)
			{
				hitFlashActive = false;
				colorMode = hitOldColorMode;
				color = defaultColor;
			}
		}
	}
}

void SpriteInstance::setFrameAnimation(const std::string& name)
{
	if (sprite && sprite->frameAnimations.size())
	{
		frameAnimation = sprite->frameAnimations[name];
		play();
	}
}

void SpriteInstance::play()
{
	if (!frameAnimation) return;
	animationDirection = 1;
	animationIsActive = true;
	animationFrame = frameAnimation->startFrame;
}

void SpriteInstance::hit(f32 hitDamage)
{
	health -= hitDamage;
	if (hitFlashActive) return;
	hitFlashActive = true;
	hitOldColorMode = colorMode;
	colorMode = ColorMode::Add;
	hitColorTimer = 0.0f;
	currentHitFlashCount = 0;
}

}
