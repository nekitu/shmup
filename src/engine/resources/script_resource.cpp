#include "resources/script_resource.h"
#include "utils.h"
#include "resource_loader.h"

namespace engine
{
static lua_State* L = nullptr;

bool ScriptResource::load(Json::Value& json)
{
	code = readTextFile(loader->root + fileName);
	luaL_loadstring(L, code.c_str());
	auto result = lua_pcall(L, 0, LUA_MULTRET, 0);
	
	if (result)
	{
		printf("Lua error: %s\n", lua_tostring(L, -1));
		return false;
	}

	if (lua_istable(L, -1)) {
		M = LuaIntf::LuaRef::popFromStack(L);
	}
	else
	{
		printf("Lua: Please return M table in %s\n", fileName.c_str());
	}

	return true;
}

void ScriptResource::execute()
{
	LuaIntf::LuaRef onUpdateFunc = M.get("onUpdate");

	onUpdateFunc(111, "Coco");
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