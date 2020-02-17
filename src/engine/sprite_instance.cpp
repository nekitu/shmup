#include "sprite_instance.h"
#include "game.h"
#include "resources/sprite_resource.h"
#include "resources/unit_resource.h"
#include "sprite_instance.h"
#include <assert.h>
#include "image_atlas.h"
#include <cmath>

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

void SpriteInstance::initializeFrom(SpriteInstanceResource* res)
{
	name = res->name;
	sprite = res->sprite;
	transform = res->transform;
	orderIndex = res->orderIndex;
	defaultColor = color = res->color;
	colorMode = res->colorMode;
	hitColor = res->hitColor;
	visible = res->visible;
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
		if (frameAnimation->type == SpriteFrameAnimation::Type::Reversed)
		{
			animationDirection = -1;
		}

		animationFrame += (f32)frameAnimation->framesPerSecond * game->deltaTime * animationDirection;

		if (frameAnimation->repeatCount)
		{
			animationRepeatCount++;

			if (animationRepeatCount >= frameAnimation->repeatCount)
			{
				animationIsActive = false;
			}
		}

		f32 frame = (i32)animationFrame;

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

bool SpriteInstance::checkPixelCollision(SpriteInstance* other, Vec2& outCollisionCenter)
{
	if (!visible)
		return false;

	Rect partRc;

	partRc.x = std::fmaxf(rect.x, other->rect.x);
	partRc.y = std::fmaxf(rect.y, other->rect.y);
	partRc.width  = floorf(fminf(rect.x + rect.width,  other->rect.x + other->rect.width) - partRc.x);
	partRc.height = floorf(fminf(rect.y + rect.height, other->rect.y + other->rect.height)- partRc.y);

	if (partRc.width < 1 || partRc.height < 1) return false;

	auto getRectPixels = [](SpriteInstance* spr, const Rect& localRc)
	{
		std::vector<u8> pixels;

		auto frmRc = spr->sprite->getSheetFramePixelRect(spr->animationFrame);
		pixels.resize(localRc.width * localRc.height);
		int i = 0;
		f32 step = 1.0f / spr->transform.scale;
		f32 srcx = 0;
		f32 srcy = 0;
		f32 rcx = round(frmRc.width * (localRc.x / spr->rect.width));
		f32 rcy = round(frmRc.height * (localRc.y / spr->rect.height));

		for (int y = 0; y < localRc.height; y++)
		{
			srcx = 0;

			for (int x = 0; x < localRc.width; x++)
			{
				u8* px = (u8*)&spr->sprite->image->imageData[((u32)frmRc.y + (u32)srcy + (u32)rcy) * spr->sprite->image->width + (u32)frmRc.x + (u32)srcx + (u32)rcx];
				pixels[i++] = px[3];
				srcx += step;
			}

			srcy += step;
		}

		return pixels;
	};

	auto r1 = Rect(partRc.x - rect.x, partRc.y - rect.y, partRc.width, partRc.height);
	auto r2 = Rect(partRc.x - other->rect.x, partRc.y - other->rect.y, partRc.width, partRc.height);
	std::vector<u8> pixels1 = getRectPixels(this, r1);
	std::vector<u8> pixels2 = getRectPixels(other, r2);

	for (int i = 0; i < pixels1.size(); i++)
	{
		if (pixels1[i] != 0 && pixels2[i] != 0)
		{
			outCollisionCenter = partRc.center();
			return true;
		}
	}

	return false;
}

}
