#include "types.h"
#include "resource.h"
#include "rect.h"


namespace engine
{
struct TilesetResource : Resource
{
	struct AtlasImage* image = nullptr;

	std::string imageFilename;
	u32 columns = 0;
	u32 margin = 0;
	u32 spacing = 0;
	u32 tileCount = 0;
	u32 tileWidth = 0;
	u32 tileHeight = 0;

	f32 uvMargin = 0;
	f32 uvSpacing = 0;
	f32 uvTileWidth = 0;
	f32 uvTileHeight = 0;

	bool load(Json::Value& json) override;
	void unload() override;
	Rect getTileRectTexCoord(u32 index);
};

}
