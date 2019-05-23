#include "resources/script_resource.h"
#include "utils.h"
#include "resource_loader.h"

extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "luaintf/LuaIntf.h"

namespace engine
{
static lua_State* L = nullptr;

bool ScriptResource::load(Json::Value& json)
{
	code = readTextFile(loader->root + fileName);
	return true;
}

void ScriptResource::execute()
{
	luaL_dostring(L, code.c_str());
}

void engine_log(const char* str)
{
	printf("LOG: %s\n", str);
}

bool initializeLua()
{
	L = luaL_newstate();
	
	luaL_openlibs(L);

	LuaIntf::LuaBinding(L).beginModule("engine")
		.addFunction("log", engine_log)
		.endModule();

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