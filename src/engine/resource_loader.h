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

	// example: "sprites/sample_sprite"
	SpriteResource* loadSprite(const char* filename, struct ImageAtlas* atlas);
	SoundResource* loadSound(const char* filename);
	MusicResource* loadMusic(const char* filename);
	UnitResource* loadUnit(const char* filename);
	LevelResource* loadLevel(const char* filename);
};
}