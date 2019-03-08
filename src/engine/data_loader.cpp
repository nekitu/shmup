#include "data_loader.h"
#include "sprite.h"
#include "sound.h"
#include "unit.h"
#include "level.h"
#include "image_atlas.h"
#include <json/json.h>

namespace engine
{
static std::string readTextFile(const char* path)
{
	FILE* file = fopen(path, "rb");

	if (!file)
		return std::string("");

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	std::string text;

	if (size != -1)
	{
		fseek(file, 0, SEEK_SET);

		char* buffer = new char[size + 1];
		buffer[size] = 0;

		if (fread(buffer, 1, size, file) == (unsigned long)size)
			text = buffer;

		delete[] buffer;
	}

	fclose(file);

	return text;
}


Sprite* DataLoader::loadSprite(const char* filename, ImageAtlas* atlas)
{
	std::string fullFilename = root + filename;

	if (sprites[filename]) return sprites[filename];

	Sprite* sprite = new Sprite();
	Json::Reader reader;
	Json::Value root;
	auto json = readTextFile((fullFilename + ".json").c_str());
	bool ok = reader.parse(json, root);

	if (!ok)
	{
		printf(reader.getFormatedErrorMessages().c_str());
		delete sprite;
		return nullptr;
	}

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

	return sprite;
}

Sound* DataLoader::loadSound(const char* filename)
{
	Sound* sound = new Sound();

	return sound;
}

Unit* DataLoader::loadUnit(const char* filename)
{
	Unit* unit = new Unit();

	return unit;
}

Level* DataLoader::loadLevel(const char* filename)
{
	Level* level = new Level();

	return level;
}

}