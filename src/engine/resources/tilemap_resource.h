#include "types.h"
#include "resource.h"

namespace engine
{
struct TilemapResource : Resource
{
	bool load(Json::Value& json) override;
	void unload() override;
};

}
