#pragma once
#include "types.h"
#include "resource.h"

namespace engine
{
struct ScriptResource : Resource
{
	std::string code;
};

extern bool initializeLua();
extern void shutdownLua();
extern struct lua_State* getLuaState();

}