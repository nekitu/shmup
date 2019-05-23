#pragma once
#include "types.h"
#include <unordered_map>
#include <string>

namespace engine
{
struct ResourceLoader
{
	std::string root = "../data/";
	std::unordered_map<std::string/*filename*/, struct Resource*> resources;
	struct ImageAtlas* atlas = nullptr;

	// example: "sprites/sample_sprite"
	struct SpriteResource* loadSprite(const std::string& filename);
	struct SoundResource* loadSound(const std::string& filename);
	struct MusicResource* loadMusic(const std::string& filename);
	struct UnitResource* loadUnit(const std::string& filename);
	struct LevelResource* loadLevel(const std::string& filename);
	struct WeaponResource* loadWeapon(const std::string& filename);
	struct ScriptResource* loadScript(const std::string& filename);
};
}