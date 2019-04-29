#include "level_resource.h"
#include "json/value.h"

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
	}

	return true;
}

}