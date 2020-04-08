#include "level_resource.h"
#include "json/value.h"
#include "unit.h"
#include "resource_loader.h"
#include "sprite.h"
#include "game.h"

namespace engine
{
bool LevelResource::load(Json::Value& json)
{
	auto layersJson = json.get("layers", Json::Value());
	auto unitsJson = json.get("units", Json::Value());

	if (!unitsJson.size())
		return true;

	for (auto& item : layersJson)
	{
		Layer layer;

		layer.parallaxScale = item.get("parallaxScale", 1.0f).asFloat();
		layers.push_back(layer);
	}

	for (auto& item : unitsJson)
	{
		Unit* unit = new Unit();

		unit->load(loader, item);
		units.push_back(unit);
	}

	return true;
}

void LevelResource::unload()
{
	layers.clear();

	for (auto& unit : units)
	{
		delete unit;
	}

	units.clear();
}

}
