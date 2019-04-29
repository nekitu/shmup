#include "resource_loader.h"
#include "resource.h"
#include "resources/sprite_resource.h"
#include "resources/sound_resource.h"
#include "resources/unit_resource.h"
#include "resources/level_resource.h"
#include "resources/weapon_resource.h"
#include "image_atlas.h"
#include <json/json.h>
#include "utils.h"

namespace engine
{
SpriteResource* ResourceLoader::loadSprite(const std::string& filename)
{
	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return dynamic_cast<SpriteResource*>(resources[filename]);
	}

	Json::Value json;

	if (!loadJson(fullFilename + ".json", json))
	{
		return nullptr;
	}

	SpriteResource* sprite = new SpriteResource();

	sprite->loader = this;
	sprite->atlas = atlas;
	sprite->fileName = filename;
	sprite->load(json);
	resources[filename] = sprite;

	return sprite;
}

SoundResource* ResourceLoader::loadSound(const std::string& filename)
{
	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return (SoundResource*)resources[filename];
	}

	Json::Value json;

	if (!loadJson(fullFilename + ".json", json))
	{
		return nullptr;
	}

	SoundResource* sound = new SoundResource();

	sound->loader = this;
	sound->fileName = filename;
	sound->load(json);
	resources[filename] = sound;

	return sound;
}

MusicResource* ResourceLoader::loadMusic(const std::string& filename)
{
	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return (MusicResource*)resources[filename];
	}

	Json::Value json;
	
	if (!loadJson(fullFilename + ".json", json))
	{
		return nullptr;
	}

	MusicResource* music = new MusicResource();

	music->loader = this;
	music->fileName = filename;
	music->load(json);
	resources[filename] = music;

	return music;
}

UnitResource* ResourceLoader::loadUnit(const std::string& filename)
{
	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return (UnitResource*)resources[filename];
	}

	Json::Value json;
	
	if (!loadJson(fullFilename + ".json", json))
	{
		return nullptr;
	}

	UnitResource* unit = new UnitResource();

	unit->loader = this;
	unit->fileName = filename;
	unit->load(json);
	resources[filename] = unit;

	return unit;
}

LevelResource* ResourceLoader::loadLevel(const std::string& filename)
{
	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return (LevelResource*)resources[filename];
	}

	Json::Value json;

	if (!loadJson(fullFilename + ".json", json))
	{
		return nullptr;
	}

	LevelResource* level = new LevelResource();

	level->loader = this;
	level->fileName = filename;
	level->load(json);
	resources[filename] = level;

	return level;
}

WeaponResource* ResourceLoader::loadWeapon(const std::string& filename)
{
	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return (WeaponResource*)resources[filename];
	}

	Json::Value json;

	if (!loadJson(fullFilename + ".json", json))
	{
		return nullptr;
	}

	WeaponResource* res = new WeaponResource();

	res->loader = this;
	res->fileName = filename;
	res->load(json);
	res->projectileUnit = loadUnit(json.get("projectileUnit", "").asString());
	resources[filename] = res;

	return res;
}

}