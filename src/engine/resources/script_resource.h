#pragma once
#include "types.h"
#include "resource.h"
#include "lua_scripting.h"

namespace engine
{
extern bool initializeLua();
extern void shutdownLua();
extern lua_State* getLuaState();

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

	template<typename T>
	ScriptClassInstance* createClassInstance(T* obj)
	{
		if (code.empty())
		{
			printf("No code in: '%s'\n", fileName.c_str());
			return nullptr;
		}

		auto res = luaL_loadstring(getLuaState(), code.c_str());

		auto result = lua_pcall(getLuaState(), 0, LUA_MULTRET, 0);

		if (result)
		{
			printf("createClassInstance: Lua error: %s\n", lua_tostring(getLuaState(), -1));
			return nullptr;
		}

		LuaIntf::LuaRef inst;

		auto cfunc = LuaIntf::LuaRef::popFromStack(getLuaState());

		//printf("Creating instance '%s'\n", fileName.c_str());

		if (cfunc.isFunction())
		{
			inst = cfunc.call<LuaIntf::LuaRef>(obj);
		}
		else
		{
			printf("Lua: Please return class table in '%s'\n", fileName.c_str());
			return nullptr;
		}

		ScriptClassInstance* classInst = new ScriptClassInstance();

		classInst->script = this;
		classInst->classInstance = inst;
		classInst->object = obj;

		classInstances.push_back(classInst);

		return classInst;
	}

};

}
