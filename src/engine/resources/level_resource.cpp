#include "level_resource.h"
#include "json/value.h"
#include "unit.h"
#include "resource_loader.h"
#include "sprite.h"
#include "game.h"
#include "tilemap.h"

namespace engine
{
bool LevelResource::load(Json::Value& json)
{
	auto& layersJson = json.get("layers", Json::Value());
	auto& unitsJson = json.get("units", Json::Value());

	if (!unitsJson.size())
		return true;

	for (auto& item : layersJson)
	{
		Layer layer;

		layer.cameraParallaxScale = item.get("cameraParallaxScale", 1.0f).asFloat();
		layer.cameraParallax = item.get("cameraParallax", true).asBool();
		layer.cameraScroll = item.get("cameraScroll", true).asBool();
		layers.push_back(layer);
	}

	for (auto& item : unitsJson)
	{
		Unit* unit = nullptr;

		if (item["type"].asString() == "tilemap")
		{
			unit = new Tilemap();
		}
		else
		{
			unit = new Unit();
		}

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
