#include "sprite_resource.h"
#include <stb_image.h>
#include "image_atlas.h"

namespace engine
{
AtlasImage* SpriteResource::loadImage(const char* filename, ImageAtlas* atlas)
{
	int width = 0;
	int height = 0;
	int comp;
	stbi_uc* data = stbi_load(filename, &width, &height, &comp, 4);

	if (!data)
		return nullptr;

	auto img = atlas->addImage((Rgba32*)data, width, height);
	delete[] data;
	atlas->pack();

	return img;
}

Rect SpriteResource::getFrameUvRect(u32 frame)
{
	Rect rc;

	u32 cols = (f32)image->width / (f32)frameWidth;
	u32 rows = (f32)image->height / (f32)frameHeight;
	u32 col = frame % cols;
	u32 row = frame / cols;

	rc.x = image->uvRect.x + uvFrameWidth * (f32)col;
	rc.y = image->uvRect.y + uvFrameHeight * (f32)row;
	rc.width = uvFrameWidth;
	rc.height = uvFrameHeight;

	return rc;
}

bool SpriteResource::load(const std::string& filename)
{

	sprite->frameWidth = root.get("frameWidth", Json::Value(0)).asInt();
	sprite->frameHeight = root.get("frameHeight", Json::Value(0)).asInt();

	Json::Value animations = root.get("animations", Json::Value());
	auto animNames = animations.getMemberNames();

	for (auto& animName : animNames)
	{
		auto& animJson = animations.get(animName, Json::Value());
		SpriteAnimation* anim = new SpriteAnimation();

		anim->startFrame = animJson.get("start", Json::Value(0)).asInt();
		anim->frameCount = animJson.get("frames", Json::Value(0)).asInt();
		anim->framesPerSecond = animJson.get("fps", Json::Value(0)).asInt();
		anim->repeatCount = animJson.get("repeat", Json::Value(0)).asInt();
		sprite->animations[animName] = anim;

		std::string type = animJson.get("type", Json::Value("normal")).asString();
		if (type == "normal") anim->type = SpriteAnimation::Type::Normal;
		if (type == "reversed") anim->type = SpriteAnimation::Type::Reversed;
		if (type == "pingpong") anim->type = SpriteAnimation::Type::PingPong;
	}

	sprite->image = sprite->loadImage((fullFilename + ".png").c_str(), atlas);
	//TODO: put this after the global atlas packing
	sprite->uvFrameWidth = sprite->image->uvRect.width / (f32)(sprite->image->width / sprite->frameWidth);
	sprite->uvFrameHeight = sprite->image->uvRect.height / (f32)(sprite->image->height / sprite->frameHeight);
	sprites[filename] = sprite;
	return true;
}

}