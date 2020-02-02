#include "sprite_resource.h"
#include <stb_image.h>
#include "image_atlas.h"
#include "game.h"

namespace engine
{
AtlasImage* SpriteResource::loadImage(const std::string& filename)
{
	int width = 0;
	int height = 0;
	int comp = 0;

	stbi_uc* data = stbi_load(filename.c_str(), &width, &height, &comp, 4);

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

	u32 col = frame % columns;
	u32 row = frame / columns;

	if (image->rotated)
	{
		rc.x = image->uvRect.x + uvFrameWidth * (f32)row;
		rc.y = image->uvRect.y + uvFrameHeight * (f32)col;
		rc.width = uvFrameWidth;
		rc.height = uvFrameHeight;
	}
	else
	{
		rc.x = image->uvRect.x + uvFrameWidth * (f32)col;
		rc.y = image->uvRect.y + uvFrameHeight * (f32)row;
		rc.width = uvFrameWidth;
		rc.height = uvFrameHeight;
	}

	return rc;
}

bool SpriteResource::load(Json::Value& json)
{
	auto imageFilename = fileName + ".png";
	
	frameWidth = json.get("frameWidth", Json::Value(0)).asInt();
	frameHeight = json.get("frameHeight", Json::Value(0)).asInt();
	color.parse(json.get("color", color.toString()).asString());
	auto colMode = json.get("colorMode", "Add").asString();

	if (colMode == "Add") colorMode = ColorMode::Add;
	if (colMode == "Sub") colorMode = ColorMode::Sub;
	if (colMode == "Mul") colorMode = ColorMode::Mul;

	Json::Value animationsJson = json.get("animations", Json::Value());
	auto animNames = animationsJson.getMemberNames();

	for (auto& animName : animNames)
	{
		auto& animJson = animationsJson.get(animName, Json::Value());
		SpriteFrameAnimation* anim = new SpriteFrameAnimation();

		anim->name = animName;
		anim->startFrame = animJson.get("start", Json::Value(0)).asInt();
		anim->frameCount = animJson.get("frames", Json::Value(0)).asInt();
		anim->framesPerSecond = animJson.get("fps", Json::Value(0)).asInt();
		anim->repeatCount = animJson.get("repeat", Json::Value(0)).asInt();
		frameAnimations[animName] = anim;

		std::string type = animJson.get("type", Json::Value("normal")).asString();
		if (type == "Normal") anim->type = SpriteFrameAnimation::Type::Normal;
		if (type == "Reversed") anim->type = SpriteFrameAnimation::Type::Reversed;
		if (type == "PingPong") anim->type = SpriteFrameAnimation::Type::PingPong;
	}

	image = loadImage(Game::makeFullDataPath(imageFilename));
	//TODO: put this after the global atlas packing

	if (image->rotated)
	{
		uvFrameWidth = image->uvRect.width / (f32)(image->rect.height / frameHeight);
		uvFrameHeight = image->uvRect.height / (f32)(image->rect.width / frameWidth);
	}
	else
	{
		uvFrameWidth = image->uvRect.width / (f32)(image->rect.width / frameWidth);
		uvFrameHeight = image->uvRect.height / (f32)(image->rect.height / frameHeight);
	}

	columns = image->width / frameWidth;
	rows = image->height / frameHeight;

	return true;
}

}
