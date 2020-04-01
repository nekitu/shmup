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
#include "weapon_instance.h"
#include "game.h"
#include <json/json.h>
#include "utils.h"

namespace engine
{
#define checkEmptyFilename(where, filename)\
{\
	if (filename.size() == 0)\
	{\
		LOG_ERROR("{0}: Empty filename", where);\
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
	LOG_INFO("Unloading script instances...");

	for (auto res : scripts)
	{
		res->unload();
	}

	LOG_INFO("Shutdown Lua...");
	shutdownLua();
	LOG_INFO("Initializing Lua...");
	initializeLua();

	Json::Value json;

	LOG_INFO("Recreating Lua class instances...");

	for (auto res : scripts)
	{
		res->load(json);
	}

	reloadWeapons();
}

void ResourceLoader::reloadWeapons()
{
	Json::Value json;

	LOG_INFO("Recreating weapons...");

	for (auto res : resources)
	{
		if (res.second->type == ResourceType::Weapon)
		{
			std::string fullFilename = root + res.first;

			if (!loadJson(fullFilename + ".json", json))
			{
				continue;
			}

			res.second->load(json);
		}
	}

	for (auto unit : Game::instance->unitInstances)
	{
		for (auto wpn : unit->weapons)
		{
			wpn.second->initializeFrom(wpn.second->weaponResource);
		}
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

	res->type = ResourceType::Sprite;
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

	res->type = ResourceType::Sound;
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

	res->type = ResourceType::Music;
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

	res->type = ResourceType::Unit;
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

	res->type = ResourceType::Level;
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

	res->type = ResourceType::Weapon;
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

	res->type = ResourceType::Script;
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

	res->type = ResourceType::Animation;
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

	res->type = ResourceType::Font;
	res->loader = this;
	res->fileName = filename;
	res->load(json);
	resources[filename] = res;

	return res;
}

}
