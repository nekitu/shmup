#pragma once
#include "types.h"
#include <json/value.h>
#include <string>

namespace engine
{
enum class ResourceType
{
	None = 0,
	Animation,
	Font,
	Level,
	Music,
	Script,
	Sound,
	Sprite,
	Tilemap,
	Unit,
	Weapon,

	Count
};

struct Resource
{
	ResourceType type = ResourceType::None;
	struct ResourceLoader* loader = nullptr;
	std::string fileName;
	u32 usageCount = 1;
	bool mustReload = false;

	virtual ~Resource() {}
	virtual bool load(Json::Value& json) { return false; }
	virtual void unload() {}
};
}
