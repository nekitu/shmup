#include "types.h"
#include <json/value.h>

namespace engine
{
struct Resource
{
	std::string fileName;
	u32 usageCount = 1;

	virtual ~Resource() {}
	virtual bool load(Json::Value& json) { return false; }
};
}