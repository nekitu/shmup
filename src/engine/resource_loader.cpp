#include "resource_loader.h"
#include "resource.h"
#include "resources/sprite_resource.h"
#include "resources/sound_resource.h"
#include "resources/unit_resource.h"
#include "resources/level_resource.h"
#include "resources/weapon_resource.h"
#include "resources/script_resource.h"
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

	SpriteResource* res = new SpriteResource();

	res->loader = this;
	res->atlas = atlas;
	res->fileName = filename;
	res->load(json);
	resources[filename] = res;

	return res;
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

	SoundResource* res = new SoundResource();

	res->loader = this;
	res->fileName = filename;
	res->load(json);
	resources[filename] = res;

	return res;
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

	MusicResource* res = new MusicResource();

	res->loader = this;
	res->fileName = filename;
	res->load(json);
	resources[filename] = res;

	return res;
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

	UnitResource* res = new UnitResource();

	res->loader = this;
	res->fileName = filename;
	res->load(json);
	resources[filename] = res;

	return res;
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

	LevelResource* res = new LevelResource();

	res->loader = this;
	res->fileName = filename;
	res->load(json);
	resources[filename] = res;

	return res;
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
	res->script = loadScript(json.get("fireScript", "").asString());
	resources[filename] = res;

	return res;
}

ScriptResource* ResourceLoader::loadScript(const std::string& filename)
{
	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return (ScriptResource*)resources[filename];
	}

	Json::Value json;


	//if (!loadJson(fullFilename + ".json", json))
	//{
	//	return nullptr;
	//}

	ScriptResource* res = new ScriptResource();

	res->loader = this;
	res->fileName = filename;
	res->load(json);
	resources[filename] = res;

	return res;
}

}
