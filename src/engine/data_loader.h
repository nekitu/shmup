#pragma once
#include "types.h"
#include <unordered_map>
#include <string>

namespace engine
{
struct DataLoader
{
	std::string root = "../data/";
	std::unordered_map<std::string/*filename*/, struct Sprite*> sprites;
	std::unordered_map<std::string/*filename*/, struct Sound*> sounds;
	std::unordered_map<std::string/*filename*/, struct Unit*> units;
	std::unordered_map<std::string/*filename*/, struct Level*> levels;

	// example: "sprites/sample_sprite"
	Sprite* loadSprite(const char* filename, struct ImageAtlas* atlas);
	Sound* loadSound(const char* filename);
	Unit* loadUnit(const char* filename);
	Level* loadLevel(const char* filename);
};
}