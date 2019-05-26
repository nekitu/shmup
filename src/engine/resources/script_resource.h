#pragma once
#include "types.h"
#include "resource.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "luaintf/LuaIntf.h"

namespace engine
{
struct ScriptResource : Resource
{
	std::string code;
	LuaIntf::LuaRef M;

	bool load(Json::Value& json) override;
	void execute();
};

extern bool initializeLua();
extern void shutdownLua();
extern lua_State* getLuaState();

}