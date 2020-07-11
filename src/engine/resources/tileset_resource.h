#include "types.h"
#include "resource.h"

namespace engine
{
struct TilesetResource : Resource
{
	struct AtlasImage* image = nullptr;
	struct ImageAtlas* atlas = nullptr;

	u32 columns = 0;
	f32 margin = 0;
	f32 spacing = 0;
	u32 tileCount = 0;
	u32 tileWidth = 0;
	u32 tileHeight = 0;

	bool load(Json::Value& json) override;
	void unload() override;
};

}
