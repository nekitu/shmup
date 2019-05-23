#pragma once
#include "types.h"
#include "resource.h"

struct lua_State;

namespace engine
{
struct ScriptResource : Resource
{
	std::string code;

	bool load(Json::Value& json) override;
	void execute();
};

extern bool initializeLua();
extern void shutdownLua();
extern lua_State* getLuaState();

}