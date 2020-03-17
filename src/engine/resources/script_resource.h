#pragma once
#include "types.h"
#include "resource.h"
#include "lua_scripting.h"

namespace engine
{
struct ScriptClassInstance
{
	struct ScriptResource* script = nullptr;
	LuaIntf::LuaRef classInstance;
	void* object = nullptr;

	LuaIntf::LuaRef getFunction(const std::string& funcName);
};

struct ScriptResource : Resource
{
	std::string code;
	std::vector<ScriptClassInstance*> classInstances;

	bool load(Json::Value& json) override;
	void unload();
	ScriptClassInstance* createClassInstance(void* obj = nullptr);
};

extern bool initializeLua();
extern void shutdownLua();
extern lua_State* getLuaState();

}
