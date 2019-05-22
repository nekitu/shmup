#include "resources/script_resource.h"
#include "utils.h"

extern "C"
{
	#include "lua.h"
	#include "lauxlib.h"
}

namespace engine
{
static lua_State* L = nullptr;

bool ScriptResource::load(Json::Value& json)
{
	code = readTextFile(fileName);
	return true;
}

bool initializeLua()
{
	L = luaL_newstate();
	return true;
}

void shutdownLua()
{
	lua_close(L);
}

lua_State* getLuaState()
{
	return L;
}

}