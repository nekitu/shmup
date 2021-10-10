#include "sprite.h"
#include "game.h"
#include "resources/sprite_resource.h"
#include "resources/unit_resource.h"
#include "sprite.h"
#include "graphics.h"
#include <assert.h>
#include "image_atlas.h"
#include <cmath>
#include "utils.h"

namespace engine
{
std::vector<Sprite*> Sprite::allSprites;

Sprite::Sprite()
{
	allSprites.push_back(this);
}

Sprite::~Sprite()
{
	auto iter = std::find(allSprites.begin(), allSprites.end(), this);

	if (iter != allSprites.end())
	{
		allSprites.erase(iter);
	}
}

void Sprite::copyFrom(Sprite* other)
{
	name = other->name;
	spriteResource = other->spriteResource;
	position = other->position;
	scale = other->scale;
	verticalFlip = other->verticalFlip;
	horizontalFlip = other->horizontalFlip;
	rotation = other->rotation;
	orderIndex = other->orderIndex;
	visible = other->visible;
	shadow = other->shadow;
	collide = other->collide;
	health = other->health;
	damage = other->damage;
	defaultColor = other->defaultColor;
	color = other->color;
	colorMode = other->colorMode;
	relativeToRoot = other->relativeToRoot;
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

	if (other->spriteResource->paletteInfo.isPaletted)
		paletteSlot = Game::instance->graphics->allocPaletteSlot();

	play();
}

void Sprite::initializeFrom(SpriteInstanceResource* res)
{
	name = res->name;
	spriteResource = res->spriteResource;
	position = res->position;
	scale = res->scale;
	verticalFlip = res->verticalFlip;
	horizontalFlip = res->horizontalFlip;
	rotation = res->rotation;
	orderIndex = res->orderIndex;
	defaultColor = color = res->color;
	colorMode = res->colorMode;
	hitColor = res->hitColor;
	visible = res->visible;
	shadow = res->shadow;
	maxHealth = health = res->health;
	damage = res->damage;
	relativeToRoot = res->relativeToRoot;
	animationFrame = 0;
	animationRepeatCount = 0;
	animationDirection = 1;
	animationIsActive = true;

	if (res->spriteResource->paletteInfo.isPaletted)
		paletteSlot = Game::instance->graphics->allocPaletteSlot();

	std::string animName = res->animationName;

	if (animName.empty() && res->animations.size())
	{
		animName = res->animations.begin()->first;
	}

	setFrameAnimation(animName);
}

void Sprite::initializeFromSpriteResource(SpriteResource* res)
{
	spriteResource = res;
	defaultColor = color = res->color;
	colorMode = res->colorMode;
	animationFrame = 0;
	animationRepeatCount = 0;
	animationDirection = 1;
	animationIsActive = true;

	if (res->paletteInfo.isPaletted)
		paletteSlot = Game::instance->graphics->allocPaletteSlot();

	std::string animName;

	if (res->frameAnimations.size())
	{
		animName = res->frameAnimations.begin()->first;
	}

	setFrameAnimation(animName);
}

void Sprite::update(struct Game* game)
{
	f32 deltaTime = game->deltaTime;

	if (unit)
	{
		if (unit->isPlayer())
			deltaTime = game->realDeltaTime;
	}

	// update the sprite frame animation
	if (frameAnimation && animationIsActive)
	{
		if (frameAnimation->type == SpriteFrameAnimation::Type::Reversed)
		{
			animationDirection = -1;
		}

		animationFrame += (f32)frameAnimation->framesPerSecond * deltaTime * animationDirection;

		f32 frame = (i32)animationFrame;
		bool reachedEnd = false;
		u32 freezeFrame = 0;

		if (frameAnimation->type == SpriteFrameAnimation::Type::Normal)
		{
			if (frame > frameAnimation->startFrame + frameAnimation->frameCount - 1)
			{
				animationFrame = frameAnimation->startFrame;
				freezeFrame = frameAnimation->startFrame + frameAnimation->frameCount - 1;
				reachedEnd = true;
			}
		}
		else if (frameAnimation->type == SpriteFrameAnimation::Type::Reversed)
		{
			if (frame < frameAnimation->startFrame)
			{
				animationFrame = frameAnimation->startFrame + frameAnimation->frameCount - 1;
				freezeFrame = frameAnimation->startFrame;
				reachedEnd = true;
			}
		}
		else if (frameAnimation->type == SpriteFrameAnimation::Type::PingPong)
		{
			if (animationDirection > 0 && frame > frameAnimation->startFrame + frameAnimation->frameCount - 1)
			{
				animationFrame = frameAnimation->startFrame + frameAnimation->frameCount - 1;
				animationDirection = -1;
				reachedEnd = true;
			}
			else if (animationDirection < 0 && frame < frameAnimation->startFrame)
			{
				animationFrame = frameAnimation->startFrame;
				animationDirection = 1;
				reachedEnd = true;
			}

			if (animationDirection > 0)
				freezeFrame = frameAnimation->startFrame + frameAnimation->frameCount - 1;
			else
				freezeFrame = frameAnimation->startFrame;
		}

		if (reachedEnd && frameAnimation->repeatCount != 0)
		{
			animationRepeatCount++;

			if (animationRepeatCount >= frameAnimation->repeatCount)
			{
				animationFrame = freezeFrame;
				animationIsActive = false;
			}
		}
	}

	// update hit color
	if (hitFlashActive)
	{
		color = defaultColor + (hitColor - defaultColor) * hitColorTimer;
		hitColorTimer += hitColorFlashSpeed * deltaTime;

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

	// update rendering info
	if (visible)
	{
		Vec2 pos;

		if (unit && relativeToRoot)
		{
			pos = this != unit->root ? unit->root->position : Vec2();
		}

		f32 flipV = verticalFlip ? -1 : 1;
		f32 flipH = horizontalFlip ? -1 : 1;

		if (unit && this != unit->root)
		{
			if (unit->root->verticalFlip) flipV *= -1;
			if (unit->root->horizontalFlip) flipH *= -1;
		}

		Rect uvRc = spriteResource->getFrameUvRect(animationFrame);

		if (flipV < 0)
		{
			uvRc.y = uvRc.bottom();
			uvRc.height *= -1.0f;
		}

		if (flipH < 0)
		{
			uvRc.x = uvRc.right();
			uvRc.width *= -1.0f;
		}

		auto shadowRc = rect;

		if (unit && unit->unitResource)
		{
			shadowRc += unit->unitResource->shadowOffset;
			shadowRc.width *= unit->unitResource->shadowScale;
			shadowRc.height *= unit->unitResource->shadowScale;
		}

		uvRect = uvRc;
		shadowRect = shadowRc;
	}
}

void Sprite::renderShadow(Graphics* gfx)
{
	if (!visible) return;
	if (!shadow) return;

	gfx->atlasTextureIndex = spriteResource->image->atlasTexture->textureIndex;
	gfx->color = 0x00FFFFFF;
	gfx->colorMode = (u32)ColorMode::Sub;

	if (rotation > 0)
	{
		gfx->drawRotatedQuad(shadowRect, uvRect, spriteResource->image->rotated, rotation);
	}
	else
	{
		gfx->drawQuad(shadowRect, uvRect, spriteResource->image->rotated);
	}
}

void Sprite::render(Graphics* gfx)
{
	if (!visible) return;

	gfx->color = color.getRgba();
	gfx->colorMode = (u32)colorMode;
	auto usePalette = spriteResource->paletteInfo.isPaletted;
	gfx->currentGpuProgram->setUIntValue((u32)usePalette, "usePalette");

	if (usePalette)
	{
		gfx->currentGpuProgram->setUIntArrayValue(palette.size(), palette.data(), "palette");
	}

	if (rotation > 0)
	{
		gfx->drawRotatedQuad(rect, uvRect, spriteResource->image->rotated, rotation);
	}
	else
	{
		gfx->drawQuad(rect, uvRect, spriteResource->image->rotated);
	}

	if (usePalette)
	{
		gfx->currentGpuProgram->setUIntValue(0, "usePalette");
	}
}

void Sprite::setFrameAnimation(const std::string& name)
{
	if (spriteResource && spriteResource->frameAnimations.size())
	{
		currentFrameAnimationName = name;
		frameAnimation = spriteResource->frameAnimations[name];

		if (!frameAnimation)
		{
			currentFrameAnimationName = "default";
			frameAnimation = spriteResource->frameAnimations[currentFrameAnimationName];
		}

		play();
	}
}

void Sprite::play()
{
	if (!frameAnimation) return;
	animationDirection = 1;
	animationIsActive = true;
	animationFrame = frameAnimation->startFrame;
}

void Sprite::hit(f32 hitDamage)
{
	health -= hitDamage;
	clampValue(health, 0, maxHealth);
	if (hitFlashActive || health == 0) return;
	hitFlashActive = true;
	hitOldColorMode = colorMode;
	colorMode = ColorMode::Add;
	hitColorTimer = 0.0f;
	currentHitFlashCount = 0;
}

bool Sprite::checkPixelCollision(Sprite* other, Vec2& outCollisionCenter)
{
	if (!visible)
		return false;

	Rect partRc;

	partRc.x = std::fmaxf(rect.x, other->rect.x);
	partRc.y = std::fmaxf(rect.y, other->rect.y);
	partRc.width  = floorf(fminf(rect.x + rect.width,  other->rect.x + other->rect.width) - partRc.x);
	partRc.height = floorf(fminf(rect.y + rect.height, other->rect.y + other->rect.height)- partRc.y);

	if (partRc.width < 1 || partRc.height < 1) return false;

	auto getRectPixels = [](Sprite* spr, const Rect& localRc)
	{
		std::vector<u8> pixels;

		auto frmRc = spr->spriteResource->getSheetFramePixelRect(spr->animationFrame);
		pixels.resize(localRc.width * localRc.height);
		int i = 0;

		f32 stepX = 1.0f / spr->scale.x;
		f32 stepY = 1.0f / spr->scale.y;
		f32 srcx = 0;
		f32 srcy = 0;
		f32 rcx = floorf(frmRc.width * (localRc.x / spr->rect.width));
		f32 rcy = floorf(frmRc.height * (localRc.y / spr->rect.height));
		int offsLocalRc = 0;
		u32 newSrcY = 0;
		u32 newSrcX = 0;

		for (int y = 0; y < localRc.height; y++)
		{
			srcx = 0;

			for (int x = 0; x < localRc.width; x++)
			{
				if (!spr->horizontalFlip && !spr->verticalFlip)
				{
					newSrcY = ((u32)frmRc.y + srcy + (u32)rcy);
					newSrcX = (u32)frmRc.x + (u32)srcx + (u32)rcx;
				}
				else
				{
					newSrcY = ((u32)frmRc.y + srcy + (u32)rcy);
					newSrcX = (u32)frmRc.x + (u32)srcx + (u32)rcx;

					if (spr->horizontalFlip)
					{
						newSrcX = (u32)frmRc.x + ((spr->rect.width - 1) - ((u32)srcx + (u32)rcx));
					}

					if (spr->verticalFlip)
					{
						newSrcY = (u32)frmRc.y + ((spr->rect.width - 1) - (srcy + (u32)rcy));
					}
				}

				u32 offs = newSrcY * spr->spriteResource->image->width + newSrcX;
				u8* px = (u8*)&spr->spriteResource->image->imageData[offs];

				if (!spr->horizontalFlip && !spr->verticalFlip)
				{
					pixels[i++] = px[3];
				}
				else
				{
					int newX = x, newY = y;

					if (spr->horizontalFlip)
						newX = localRc.width - 1 - x;

					if (spr->verticalFlip)
						newY = localRc.height - 1 - y;

					offsLocalRc = newY * localRc.width + newX;
					pixels[offsLocalRc] = px[3];
				}

				srcx += stepX;
			}

			srcy += stepY;
		}

		return pixels;
	};

	auto r1 = Rect(partRc.x - rect.x, partRc.y - rect.y, partRc.width, partRc.height);
	auto r2 = Rect(partRc.x - other->rect.x, partRc.y - other->rect.y, partRc.width, partRc.height);
	r1.x = floorf(r1.x);
	r1.y = floorf(r1.y);
	r1.width = floorf(r1.width);
	r1.height = floorf(r1.height);
	r2.x = floorf(r2.x);
	r2.y = floorf(r2.y);
	r2.width = floorf(r2.width);
	r2.height = floorf(r2.height);

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

void Sprite::copyPaletteFromResource()
{
	palette = spriteResource->paletteInfo.colors;
}

void Sprite::setPaletteEntry(u32 index, const Color& newColor)
{
	palette[index] = newColor.getRgba();
}

f32 Sprite::getFrameFromAngle(f32 angle)
{
	return (f32)(spriteResource->frameCount - 1) * fabs(angle) / 360.0f;
}

void Sprite::setFrameAnimationFromAngle(f32 angle)
{
	if (!spriteResource->rotationAnimCount) return;

	// correct it, since our origin is at left-top
	angle -= 180;
	angle = fmod(fabs(angle), 360);

	int idx = (f32)(spriteResource->rotationAnimCount - 1) * angle / 360.0f;
	char buf[10] = { 0 };

	itoa(idx, buf, 10);

	f32 relFrame = frameAnimation ? animationFrame - frameAnimation->startFrame : 0;

	setFrameAnimation(spriteResource->rotationAnimPrefix + buf);

	if (frameAnimation)
	{
		animationFrame = frameAnimation->startFrame + relFrame;
	}
}

}
