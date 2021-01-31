#pragma once
#include "types.h"
#include <unordered_map>
#include <string>

namespace engine
{
struct ResourceLoader
{
	std::unordered_map<std::string/*path*/, struct Resource*> resources;
	std::vector<struct ScriptResource*> scripts;
	std::vector<struct SpriteResource*> sprites;
	struct ImageAtlas* atlas = nullptr;

	void unload(struct Resource* res);
	void reloadScripts();
	void reloadWeapons();
	void reloadSprites();
	void reloadAnimations();

	// example: "sprites/sample_sprite"
	struct SpriteResource* loadSprite(const std::string& path);
	struct SoundResource* loadSound(const std::string& path);
	struct MusicResource* loadMusic(const std::string& path);
	struct UnitResource* loadUnit(const std::string& path);
	struct WeaponResource* loadWeapon(const std::string& path);
	struct ScriptResource* loadScript(const std::string& path);
	struct AnimationResource* loadAnimation(const std::string& path);
	struct FontResource* loadFont(const std::string& path);
	struct TilemapResource* loadTilemap(const std::string& path);
	struct TilesetResource* loadTileset(const std::string& path);
};
}
