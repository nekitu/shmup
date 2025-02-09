#pragma once
#include "types.h"
#include "resource.h"
#include "lua_scripting.h"
#include "utils.h"

namespace engine
{
#define CALL_LUA_FUNC2(scriptClass, name, ...)\
		if (scriptClass)\
		{\
			auto func = scriptClass->getFunction(name);\
			if (func.isFunction())\
			{\
				try\
				{\
					func.call(scriptClass->classInstance, ##__VA_ARGS__);\
				}\
				catch (LuaIntf::LuaException* e)\
				{\
					std::string str = e->what();\
					replaceAll(str, "\r", " ");\
					LOG_ERROR("Lua Error in {}: {}", scriptClass->script->path, str);\
				}\
			}\
		}

#define CALL_LUA_FUNC(name, ...) CALL_LUA_FUNC2(scriptClass, name, ##__VA_ARGS__)

extern bool initializeLua();
extern void shutdownLua();
extern lua_State* getLuaState();

struct ScriptResource;

struct ScriptClassInstanceBase
{
	struct ScriptResource* script = nullptr;
	LuaIntf::LuaRef classInstance;
	std::string serializedData;

	virtual bool createInstance() = 0;
	LuaIntf::LuaRef getFunction(const std::string& funcName);
	virtual ~ScriptClassInstanceBase();
};

template<typename T>
struct ScriptClassInstance : ScriptClassInstanceBase
{
	T* object = nullptr;

	ScriptClassInstance(T* obj) { object = obj; }

	bool createInstance() override
	{
		if (!script)
		{
			LOG_ERROR("No script associated with this class instance");
			return false;
		}

		if (script->code.empty())
		{
			LOG_WARN("No code in: '{}'", script->path);
			return false;
		}

		auto res = luaL_loadstring(getLuaState(), script->code.c_str());

		if (res != 0)
		{
			std::string str = lua_tostring(getLuaState(), -1);
			replaceAll(str, "\r", " ");
			LOG_ERROR("createClassInstance: Lua error: {}", str);
			return false;
		}

		auto result = lua_pcall(getLuaState(), 0, LUA_MULTRET, 0);

		if (result)
		{
			std::string str = lua_tostring(getLuaState(), -1);
			replaceAll(str, "\r", " ");
			LOG_ERROR("createClassInstance: Lua error: {}", str);
			return false;
		}

		auto cfunc = LuaIntf::LuaRef::popFromStack(getLuaState());

		if (cfunc.isFunction())
		{
			try
			{
				classInstance = cfunc.call<LuaIntf::LuaRef>(object);
			}
			catch (LuaIntf::LuaException e)
			{
				std::string str = e.what();
				replaceAll(str, "\r", " ");
				LOG_ERROR("Lua Error in {}: {}", script->path, str);
			}
		}
		else
		{
			LOG_ERROR("Lua: Please return class table in '{}'", script->path);
			return false;
		}

		//LOG_INFO("Lua: Created class instance for {}'", script->path);

		return true;
	}
};

struct ScriptResource : Resource
{
	std::string code;
	std::vector<ScriptClassInstanceBase*> classInstances;
	LuaIntf::LuaRef serializedInstancesTable;
	std::string serializedInstancesString;

	bool load(Json::Value& json) override;
	void unload() override;

	void serialize();
	void deserialize();

	template<typename T>
	ScriptClassInstanceBase* createClassInstance(T* obj)
	{
		ScriptClassInstanceBase* classInst = new ScriptClassInstance<T>(obj);

		classInst->script = this;
		classInst->createInstance();
		classInstances.push_back(classInst);

		return classInst;
	}
};

}
