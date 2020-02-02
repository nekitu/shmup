#include "level_resource.h"
#include "json/value.h"
#include "unit_instance.h"
#include "resource_loader.h"
#include "game.h"

namespace engine
{
bool LevelResource::load(Json::Value& json)
{
	auto unitInstancesJson = json.get("unitInstances", Json::Value());

	if (!unitInstancesJson.size())
		return true;

	for (auto& item : unitInstancesJson)
	{
		printf("Item name %s\n", item["name"].asCString());
		auto unitFilename = item["unit"].asString();

		if (unitFilename == "")
		{
			printf("No unit filename specified for unit instance (%s)\n", item["name"].asCString());
			continue;
		}

		auto unit = loader->loadUnit(unitFilename);
		auto unitInst = new UnitInstance();
		unitInst->instantiateFrom(unit);
		unitInstances.push_back(unitInst);
	}

	return true;
}

}
