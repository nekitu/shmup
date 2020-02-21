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
	printf("Loaded image: %s %dx%d\n", filename.c_str(), width, height);

	if (!data)
		return nullptr;

	auto img = atlas->addImage((Rgba32*)data, width, height);

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
		rc.y = image->uvRect.y + uvFrameHeight * (f32)(columns - col - 1);
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

Rect SpriteResource::getFramePixelRect(u32 frame)
{
	Rect rc;

	u32 col = frame % columns;
	u32 row = frame / columns;

	if (image->rotated)
	{
		rc.x = image->rect.x + frameWidth * row;
		rc.y = image->rect.y + frameHeight * col;
		rc.width = frameWidth;
		rc.height = frameHeight;
	}
	else
	{
		rc.x = image->rect.x + frameWidth * col;
		rc.y = image->rect.y + frameHeight * row;
		rc.width = frameWidth;
		rc.height = frameHeight;
	}

	return rc;
}

Rect SpriteResource::getSheetFramePixelRect(u32 frame)
{
	Rect rc;

	u32 col = frame % columns;
	u32 row = frame / columns;

	rc.x = ceilf(frameWidth * col);
	rc.y = ceilf(frameHeight * row);
	rc.width = ceilf(frameWidth);
	rc.height = ceilf(frameHeight);

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

	Json::Value rotAnimsJson = json.get("rotationAnims", Json::Value());

	if (rotAnimsJson.isObject())
	{
		rotationAnimPrefix = rotAnimsJson.get("prefix", rotationAnimPrefix).asString();
		auto frmCount = rotAnimsJson.get("frameCount", 0).asInt();
		rotationAnimCount = rotAnimsJson.get("count", 0).asInt();
		auto fps = rotAnimsJson.get("fps", Json::Value(0)).asInt();
		auto repeatCount = rotAnimsJson.get("repeat", Json::Value(0)).asInt();
		std::string type = rotAnimsJson.get("type", Json::Value("normal")).asString();
		SpriteFrameAnimation::Type animType = SpriteFrameAnimation::Type::Normal;

		if (type == "Normal") animType = SpriteFrameAnimation::Type::Normal;
		if (type == "Reversed") animType = SpriteFrameAnimation::Type::Reversed;
		if (type == "PingPong") animType = SpriteFrameAnimation::Type::PingPong;

		for (size_t i = 0; i < rotationAnimCount; i++)
		{
			SpriteFrameAnimation* anim = new SpriteFrameAnimation();

			anim->frameCount = frmCount;
			anim->framesPerSecond = fps;
			char buf[10] = { 0 };
			itoa(i, buf, 10);
			anim->name = rotationAnimPrefix + buf;
			anim->repeatCount = repeatCount;
			anim->startFrame = i * frmCount;
			anim->type = animType;
			frameAnimations[anim->name] = anim;
		}
	}

	Json::Value animationsJson = json.get("animations", Json::Value());
	auto animNames = animationsJson.getMemberNames();

	for (auto& animName : animNames)
	{
		auto animJson = animationsJson.get(animName, Json::Value());
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

	return true;
}

void SpriteResource::computeParamsAfterAtlasGeneration()
{
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
	frameCount = rows * columns;
}

}
