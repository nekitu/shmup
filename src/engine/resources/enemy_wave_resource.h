#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct EnemyWaveResource : Resource
{
	f32 activeTime = 10;
	f32 spawnDelay = 2;
	std::vector<struct UnitResource*> unitResources;

	struct ScriptResource* moverScript = nullptr;

	bool load(Json::Value& json) override;
	void unload() override;
};

}
