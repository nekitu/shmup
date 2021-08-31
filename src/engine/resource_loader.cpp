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
#include "sprite.h"
#include <json/json.h>
#include "utils.h"

namespace engine
{
#define checkForEmptyPath(where, path)\
{\
	if (path.size() == 0)\
	{\
		LOG_ERROR("{0}: Empty path", where);\
		_CrtDbgBreak();\
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

	for (auto& res : scripts)
	{
		res->unload();
	}

	LOG_INFO("Shutdown Lua...");
	shutdownLua();
	LOG_INFO("Initializing Lua...");
	initializeLua();

	Json::Value json;

	LOG_INFO("Recreating Lua class instances...");

	for (auto& res : scripts)
	{
		res->load(json);
	}

	// setup controllers for units again
	for (auto& unit : Game::instance->units)
	{
		for (auto& ctrl : unit->controllers)
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

	for (auto& res : resources)
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

	for (auto& unitResource : Game::instance->units)
	{
		for (auto& wpn : unitResource->weapons)
		{
			auto active = wpn.second->active;
			auto& pos = wpn.second->params.position;
			auto ammo = wpn.second->params.ammo;

			wpn.second->initializeFrom(wpn.second->weaponResource);
			wpn.second->active = active;
			wpn.second->params.ammo = ammo;
			wpn.second->params.position = pos;
		}
	}
}

void ResourceLoader::reloadAnimations()
{
	LOG_INFO("Reloading animations...");
	Json::Value json;

	for (auto& res : resources)
	{
		if (res.second->type == ResourceType::Animation)
		{
			auto absPath = Game::instance->dataRoot + res.first;

			if (!loadJson(absPath + ".json", json))
			{
				continue;
			}

			res.second->load(json);
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


	for (auto& spr : Sprite::allSprites)
	{
		spr->setFrameAnimation(spr->currentFrameAnimationName);
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
	auto res = loadResource<SpriteResource, ResourceType::Sprite>(path);
	sprites.push_back(res);
	return res;
}

SoundResource* ResourceLoader::loadSound(const std::string& path)
{
	checkForEmptyPath("loadSound", path);
	auto res = loadResource<SoundResource, ResourceType::Sound>(path);
	return res;
}

MusicResource* ResourceLoader::loadMusic(const std::string& path)
{
	checkForEmptyPath("loadMusic", path);
	auto res = loadResource<MusicResource, ResourceType::Music>(path);
	return res;
}

UnitResource* ResourceLoader::loadUnit(const std::string& path)
{
	checkForEmptyPath("loadUnit", path);
	auto res = loadResource<UnitResource, ResourceType::Unit>(path);
	return res;
}

WeaponResource* ResourceLoader::loadWeapon(const std::string& path)
{
	checkForEmptyPath("loadWeapon", path);
	auto res = loadResource<WeaponResource, ResourceType::Weapon>(path);
	return res;
}

ScriptResource* ResourceLoader::loadScript(const std::string& path)
{
	if (path.empty()) return nullptr;

	auto absPath = Game::instance->dataRoot + path;
	auto iter = resources.find(path);

	if (iter != resources.end())
	{
		iter->second->usageCount++;
		return dynamic_cast<ScriptResource*>(iter->second);
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
	auto res = loadResource<AnimationResource, ResourceType::Animation>(path);
	return res;
}

FontResource* ResourceLoader::loadFont(const std::string& path)
{
	checkForEmptyPath("loadFont", path);
	auto res = loadResource<FontResource, ResourceType::Font>(path);
	return res;
}

TilemapResource* ResourceLoader::loadTilemap(const std::string& path)
{
	checkForEmptyPath("loadTilemap", path);
	auto res = loadResource<TilemapResource, ResourceType::Tilemap>(path);
	return res;
}

TilesetResource* ResourceLoader::loadTileset(const std::string& path)
{
	checkForEmptyPath("loadTileset", path);
	auto res = loadResource<TilesetResource, ResourceType::Tileset>(path);
	return res;
}

}
