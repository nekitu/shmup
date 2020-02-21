#include "resource_loader.h"
#include "resource.h"
#include "resources/sprite_resource.h"
#include "resources/sound_resource.h"
#include "resources/music_resource.h"
#include "resources/unit_resource.h"
#include "resources/level_resource.h"
#include "resources/weapon_resource.h"
#include "resources/script_resource.h"
#include "resources/animation_resource.h"
#include "resources/font_resource.h"
#include "image_atlas.h"
#include <json/json.h>
#include "utils.h"

namespace engine
{
#define checkEmptyFilename(where, filename)\
{\
	if (filename.size() == 0)\
	{\
		printf("%s: Empty filename\n", where);\
	}\
}

void ResourceLoader::unload(struct Resource* res)
{
	res->usageCount--;

	if (res->usageCount <= 0)
	{
		auto iter = resources.find(res->fileName);

		if (iter != resources.end())
		{
			resources.erase(iter);
			delete res;
		}
	}
}

void ResourceLoader::reloadScripts()
{
	for (auto res : scripts)
	{
		res->unload();
	}

	shutdownLua();
	initializeLua();

	for (auto res : scripts)
	{
		Json::Value json;
		res->load(json);
	}
}

SpriteResource* ResourceLoader::loadSprite(const std::string& filename)
{
	checkEmptyFilename("loadSprite", filename);

	if (filename.empty()) return nullptr;

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
	sprites.push_back(res);

	return res;
}

SoundResource* ResourceLoader::loadSound(const std::string& filename)
{
	checkEmptyFilename("loadSound", filename);

	if (filename.empty()) return nullptr;

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
	res->fileName = fullFilename;
	res->load(json);
	resources[filename] = res;

	return res;
}

MusicResource* ResourceLoader::loadMusic(const std::string& filename)
{
	checkEmptyFilename("loadMusic", filename);

	if (filename.empty()) return nullptr;

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
	res->fileName = fullFilename;
	res->load(json);
	resources[filename] = res;

	return res;
}

UnitResource* ResourceLoader::loadUnit(const std::string& filename)
{
	checkEmptyFilename("loadUnit", filename);

	if (filename.empty()) return nullptr;

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
	checkEmptyFilename("loadLevel", filename);

	if (filename.empty()) return nullptr;

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
	checkEmptyFilename("loadWeapon", filename);

	if (filename.empty()) return nullptr;

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
	resources[filename] = res;

	return res;
}

ScriptResource* ResourceLoader::loadScript(const std::string& filename)
{
	checkEmptyFilename("loadScript", filename);

	if (filename.empty()) return nullptr;

	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return (ScriptResource*)resources[filename];
	}

	Json::Value json;
	ScriptResource* res = new ScriptResource();

	res->loader = this;
	res->fileName = filename;
	res->load(json);
	resources[filename] = res;
	scripts.push_back(res);

	return res;
}

AnimationResource* ResourceLoader::loadAnimation(const std::string& filename)
{
	checkEmptyFilename("loadAnimation", filename);
	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return (AnimationResource*)resources[filename];
	}

	Json::Value json;

	if (!loadJson(fullFilename + ".json", json))
	{
		return nullptr;
	}

	AnimationResource* res = new AnimationResource();

	res->loader = this;
	res->fileName = filename;
	res->load(json);
	resources[filename] = res;

	return res;
}

FontResource* ResourceLoader::loadFont(const std::string& filename)
{
	checkEmptyFilename("loadFont", filename);
	std::string fullFilename = root + filename;

	if (resources[filename])
	{
		resources[filename]->usageCount++;
		return (FontResource*)resources[filename];
	}

	Json::Value json;

	if (!loadJson(fullFilename + ".json", json))
	{
		return nullptr;
	}

	FontResource* res = new FontResource();

	res->loader = this;
	res->fileName = filename;
	res->load(json);
	resources[filename] = res;

	return res;
}

}
