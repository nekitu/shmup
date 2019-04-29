#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct ScriptResource : Resource
{
	std::string code;
};

}