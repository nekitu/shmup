#include "resource_loader.h"
#include "resources/sprite_resource.h"
#include "resources/sound_resource.h"
#include "resources/unit_resource.h"
#include "resources/level_resource.h"
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

bool loadJson(const std::string& fullFilename, Json::Value& root)
{
	Json::Reader reader;
	auto json = readTextFile((fullFilename + ".json").c_str());
	bool ok = reader.parse(json, root);

	if (!ok)
	{
		printf(reader.getFormatedErrorMessages().c_str());
		return nullptr;
	}

	return true;
}

SpriteResource* ResourceLoader::loadSprite(const char* filename, ImageAtlas* atlas)
{
	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return dynamic_cast<SpriteResource*>(resources[filename]);
	}

	Json::Value root;

	if (!loadJson(fullFilename, ))
	
	SpriteResource* sprite = new SpriteResource();



	return sprite;
}

SoundResource* ResourceLoader::loadSound(const char* filename)
{
	std::string fullFilename = root + filename;

	if (sounds[filename])
	{
		sounds[filename]->usageCount++;
		return sounds[filename];
	}

	SoundResource* sound = new SoundResource();

	sound->load(filename);

	return sound;
}


MusicResource* ResourceLoader::loadMusic(const char* filename)
{
	std::string fullFilename = root + filename;

	if (musics[filename])
	{
		musics[filename]->usageCount++;
		return musics[filename];
	}

	MusicResource* music = new MusicResource();

	music->load(filename);

	return music;
}

UnitResource* ResourceLoader::loadUnit(const char* filename)
{
	std::string fullFilename = root + filename;

	if (units[filename])
	{
		units[filename]->usageCount++;
		return units[filename];
	}

	UnitResource* unit = new UnitResource();

	unit->load(filename);

	return unit;
}

LevelResource* ResourceLoader::loadLevel(const char* filename)
{
	std::string fullFilename = root + filename;

	if (levels[filename])
	{
		levels[filename]->usageCount++;
		return levels[filename];
	}

	LevelResource* level = new LevelResource();

	level->load(filename);

	return level;
}

}