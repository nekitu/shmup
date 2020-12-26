#include "resource_loader.h"
#include "resource.h"
#include "resources/sprite_resource.h"
#include "resources/sound_resource.h"
#include "resources/music_resource.h"
#include "resources/unit_resource.h"
#include "resources/weapon_resource.h"
#include "resources/script_resource.h"
#include "resources/animation_resource.h"
#include "resources/font_resource.h"
#include "resources/tilemap_resource.h"
#include "resources/tileset_resource.h"
#include "graphics.h"
#include "image_atlas.h"
#include "weapon.h"
#include "game.h"
#include <json/json.h>
#include "utils.h"

namespace engine
{
#define checkForEmptyPath(where, path)\
{\
	if (path.size() == 0)\
	{\
		LOG_ERROR("{0}: Empty path", where);\
	}\
}

void ResourceLoader::unload(struct Resource* res)
{
	res->usageCount--;

	if (res->usageCount <= 0)
	{
		auto iter = resources.find(res->path);

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

	// setup controllers for units again
	for (auto unit : Game::instance->units)
	{
		for (auto ctrl : unit->controllers)
		{
			CALL_LUA_FUNC2(ctrl.second, "setup", &unit->unitResource->controllers[ctrl.first]);
		}
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
			auto absPath = Game::instance->dataRoot + res.first;

			if (!loadJson(absPath + ".json", json))
			{
				continue;
			}

			res.second->load(json);
		}
	}

	for (auto unitResource : Game::instance->units)
	{
		for (auto wpn : unitResource->weapons)
		{
			wpn.second->initializeFrom(wpn.second->weaponResource);
		}
	}
}

void ResourceLoader::reloadSprites()
{
	LOG_INFO("Reloading sprites...");
	Json::Value json;

	Game::instance->graphics->atlas->create(Graphics::textureAtlasWidth, Graphics::textureAtlasWidth);

	for (auto& res : resources)
	{
		if (res.second->type == ResourceType::Sprite)
		{
			auto absPath = Game::instance->dataRoot + res.first;

			if (!loadJson(absPath + ".json", json))
			{
				continue;
			}

			res.second->load(json);
		}
	}

	// resolve/reload the tilesets also
	for (auto& res : resources)
	{
		if (res.second->type == ResourceType::Tileset)
		{
			auto absPath = Game::instance->dataRoot + res.first;

			if (!loadJson(absPath + ".json", json))
			{
				continue;
			}

			res.second->load(json);
		}
		else if (res.second->type == ResourceType::Tilemap)
		{
			auto absPath = Game::instance->dataRoot + res.first;

			if (!loadJson(absPath + ".json", json))
			{
				continue;
			}

			TilemapResource* tr = (TilemapResource*)res.second;

			for (auto& layer : tr->layers)
			{
				if (layer.type == TilemapLayer::Type::Image)
				{
					layer.loadImage();
				}
			}
		}
	}

	LOG_INFO("Packing atlas sprites...");
	Game::instance->graphics->atlas->packWithLastUsedParams();

	LOG_INFO("Computing sprite params after packing...");

	for (auto& spriteResource : sprites)
	{
		spriteResource->computeParamsAfterAtlasGeneration();
	}
}

SpriteResource* ResourceLoader::loadSprite(const std::string& path)
{
	checkForEmptyPath("loadSprite", path);

	if (path.empty()) return nullptr;

	auto absPath = Game::instance->dataRoot + path;

	if (resources[path])
	{
		resources[path]->usageCount++;
		return dynamic_cast<SpriteResource*>(resources[path]);
	}

	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return nullptr;
	}

	SpriteResource* res = new SpriteResource();

	res->type = ResourceType::Sprite;
	res->loader = this;
	res->atlas = atlas;
	res->path = path;
	res->load(json);
	resources[path] = res;
	sprites.push_back(res);

	return res;
}

SoundResource* ResourceLoader::loadSound(const std::string& path)
{
	checkForEmptyPath("loadSound", path);

	if (path.empty()) return nullptr;

	auto absPath = Game::instance->dataRoot + path;

	if (resources[path])
	{
		resources[path]->usageCount++;
		return (SoundResource*)resources[path];
	}

	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return nullptr;
	}

	SoundResource* res = new SoundResource();

	res->type = ResourceType::Sound;
	res->loader = this;
	res->path = absPath;
	res->load(json);
	resources[path] = res;

	return res;
}

MusicResource* ResourceLoader::loadMusic(const std::string& path)
{
	checkForEmptyPath("loadMusic", path);

	if (path.empty()) return nullptr;

	auto absPath = Game::instance->dataRoot + path;

	if (resources[path])
	{
		resources[path]->usageCount++;
		return (MusicResource*)resources[path];
	}

	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return nullptr;
	}

	MusicResource* res = new MusicResource();

	res->type = ResourceType::Music;
	res->loader = this;
	res->path = path;
	res->load(json);
	resources[path] = res;

	return res;
}

UnitResource* ResourceLoader::loadUnit(const std::string& path)
{
	checkForEmptyPath("loadUnit", path);

	if (path.empty()) return nullptr;

	auto absPath = Game::instance->dataRoot + path;

	if (resources[path])
	{
		resources[path]->usageCount++;
		return (UnitResource*)resources[path];
	}

	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return nullptr;
	}

	UnitResource* res = new UnitResource();

	res->type = ResourceType::Unit;
	res->loader = this;
	res->path = path;
	res->load(json);
	resources[path] = res;

	return res;
}

WeaponResource* ResourceLoader::loadWeapon(const std::string& path)
{
	checkForEmptyPath("loadWeapon", path);

	if (path.empty()) return nullptr;

	auto absPath = Game::instance->dataRoot + path;

	if (resources[path])
	{
		resources[path]->usageCount++;
		return (WeaponResource*)resources[path];
	}

	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return nullptr;
	}

	WeaponResource* res = new WeaponResource();

	res->type = ResourceType::Weapon;
	res->loader = this;
	res->path = path;
	res->load(json);
	resources[path] = res;

	return res;
}

ScriptResource* ResourceLoader::loadScript(const std::string& path)
{
	checkForEmptyPath("loadScript", path);

	if (path.empty()) return nullptr;

	auto absPath = Game::instance->dataRoot + path;

	if (resources[path])
	{
		resources[path]->usageCount++;
		return (ScriptResource*)resources[path];
	}

	Json::Value json;
	ScriptResource* res = new ScriptResource();

	res->type = ResourceType::Script;
	res->loader = this;
	res->path = path;
	res->load(json);
	resources[path] = res;
	scripts.push_back(res);

	return res;
}

AnimationResource* ResourceLoader::loadAnimation(const std::string& path)
{
	checkForEmptyPath("loadAnimation", path);
	auto absPath = Game::instance->dataRoot + path;

	if (resources[path])
	{
		resources[path]->usageCount++;
		return (AnimationResource*)resources[path];
	}

	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return nullptr;
	}

	AnimationResource* res = new AnimationResource();

	res->type = ResourceType::Animation;
	res->loader = this;
	res->path = path;
	res->load(json);
	resources[path] = res;

	return res;
}

FontResource* ResourceLoader::loadFont(const std::string& path)
{
	checkForEmptyPath("loadFont", path);
	auto absPath = Game::instance->dataRoot + path;

	if (resources[path])
	{
		resources[path]->usageCount++;
		return (FontResource*)resources[path];
	}

	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return nullptr;
	}

	FontResource* res = new FontResource();

	res->type = ResourceType::Font;
	res->loader = this;
	res->path = path;
	res->load(json);
	resources[path] = res;

	return res;
}

TilemapResource* ResourceLoader::loadTilemap(const std::string& path)
{
	checkForEmptyPath("loadTilemap", path);
	auto absPath = Game::instance->dataRoot + path;

	if (resources[path])
	{
		resources[path]->usageCount++;
		return (TilemapResource*)resources[path];
	}

	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return nullptr;
	}

	TilemapResource* res = new TilemapResource();

	res->type = ResourceType::Tilemap;
	res->loader = this;
	res->path = path;
	res->load(json);
	resources[path] = res;

	return res;
}

TilesetResource* ResourceLoader::loadTileset(const std::string& path)
{
	checkForEmptyPath("loadTileset", path);
	auto absPath = Game::instance->dataRoot + path;

	if (resources[path])
	{
		resources[path]->usageCount++;
		return (TilesetResource*)resources[path];
	}

	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return nullptr;
	}

	TilesetResource* res = new TilesetResource();

	res->type = ResourceType::Tileset;
	res->loader = this;
	res->path = path;
	res->load(json);
	resources[path] = res;

	return res;
}

}
