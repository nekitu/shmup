#include "sprite_instance.h"
#include "game.h"
#include "resources/sprite_resource.h"
#include "sprite_instance.h"

namespace engine
{
void SpriteInstance::update(struct Game* game)
{
	if (spriteAnimationInstance.spriteAnimation && spriteAnimationInstance.active)
	{
		auto spriteAnim = spriteAnimationInstance.spriteAnimation;
		
		spriteAnimationInstance.currentFrame += (f32)spriteAnim->framesPerSecond * game->deltaTime * spriteAnimationInstance.direction;

		if (spriteAnim->repeatCount)
		{
			spriteAnimationInstance.currentRepeatCount++;

			if (spriteAnimationInstance.currentRepeatCount >= spriteAnim->repeatCount)
			{
				spriteAnimationInstance.active = false;
			}
		}

		f32 frame = (u32)spriteAnimationInstance.currentFrame;

		if (spriteAnim->type == SpriteAnimation::Type::Normal)
		{
			if (frame > spriteAnim->startFrame + spriteAnim->frameCount - 1)
			{
				spriteAnimationInstance.currentFrame = spriteAnim->startFrame;
			}
		}
		else if (spriteAnim->type == SpriteAnimation::Type::Reversed)
		{
			if (frame < spriteAnim->startFrame)
			{
				spriteAnimationInstance.currentFrame = spriteAnim->startFrame + spriteAnim->frameCount - 1;
			}
		}
		else if (spriteAnim->type == SpriteAnimation::Type::PingPong)
		{
			if (spriteAnimationInstance.direction > 0 && frame > spriteAnim->startFrame + spriteAnim->frameCount - 1)
			{
				spriteAnimationInstance.currentFrame = spriteAnim->startFrame + spriteAnim->frameCount - 1;
				spriteAnimationInstance.direction = -1;
			}
			else if (spriteAnimationInstance.direction < 0 && frame < spriteAnim->startFrame)
			{
				spriteAnimationInstance.currentFrame = spriteAnim->startFrame;
				spriteAnimationInstance.direction = 1;
			}
		}
	}
}

void SpriteInstance::setAnimation(const std::string& name)
{
	if (sprite)
	{
		spriteAnimationInstance.spriteAnimation = sprite->animations[name];
		spriteAnimationInstance.play();
	}
}

}