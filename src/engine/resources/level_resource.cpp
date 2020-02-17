#include "level_resource.h"
#include "json/value.h"
#include "unit_instance.h"
#include "resource_loader.h"
#include "sprite_instance.h"
#include "game.h"

namespace engine
{
bool LevelResource::load(Json::Value& json)
{
	auto layersJson = json.get("layers", Json::Value());
	auto unitInstancesJson = json.get("unitInstances", Json::Value());

	if (!unitInstancesJson.size())
		return true;

	for (auto& item : layersJson)
	{
		Layer layer;
		layer.parallaxScale = item.get("parallaxScale", 1.0f).asFloat();
		layers.push_back(layer);
	}

	for (auto& item : unitInstancesJson)
	{
		UnitInstance* unitInst = new UnitInstance();
		unitInst->load(loader, item);
		unitInstances.push_back(unitInst);
	}

	return true;
}

}
