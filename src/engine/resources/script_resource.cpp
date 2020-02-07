#include "resources/script_resource.h"
#include "resources/unit_resource.h"
#include "utils.h"
#include "resource_loader.h"
#include "weapon_instance.h"
#include "unit_instance.h"
#include "sprite_instance.h"
#include "game.h"

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

LuaIntf::LuaRef ScriptResource::getFunction(const std::string& funcName)
{
	if (M.has(funcName))
	{
		auto f = M.get(funcName);

		if (!f.isFunction())
		{
			printf("Could not find the function '%s' in script '%s'\n", funcName.c_str(), fileName.c_str());
			return LuaIntf::LuaRef::fromPtr(L, nullptr);
		}

		return f;
	}

	return LuaIntf::LuaRef();
}

void engine_log(const char* str)
{
	printf("LOG: %s\n", str);
}

bool initializeLua()
{
	L = luaL_newstate();
	
	luaL_openlibs(L);

	auto& LUA = LuaIntf::LuaBinding(L);

	LUA.beginModule("engine")
		.addFunction("log", engine_log)
		.addFunction("changeLevel", [](int index) { Game::instance->changeLevel(index); })
		.addFunction("loadNextLevel", []() { Game::instance->changeLevel(~0); })
		.addFunction("spawnUnitInstance", [](const std::string& unit, const std::string& name, const Vec2& position)
			{
				auto uinst = Game::instance->createUnitInstance(Game::instance->resourceLoader->loadUnit(unit));
				uinst->name = name;
				uinst->rootSpriteInstance->transform.position = position;
				return uinst;
			}
		)
		.endModule();

	LUA.beginClass<WeaponInstance>("WeaponInstance")
		.addFunction("fire", &WeaponInstance::fire)
		.addFunction("debug", &WeaponInstance::debug)
		.endClass();

	LUA.beginClass<UnitResource>("UnitResource")
		.addVariable("type", &UnitResource::type)
		.endClass();

	LUA.beginClass<UnitInstance>("UnitInstance")
		.addVariable("id", &UnitInstance::id, false)
		.addVariable("name", &UnitInstance::name, true)
		.addVariable("unit", &UnitInstance::unit, false)
		.addVariable("age", &UnitInstance::age, false)
		.addVariable("deleteMeNow", &UnitInstance::deleteMeNow)
		.addVariable("rootSpriteInstance", &UnitInstance::rootSpriteInstance)
		.addFunction("fire", [](UnitInstance* inst)
			{
				for (auto& wp : inst->weapons)
				{
					wp.second->fire();
					wp.second->update(Game::instance);
				}
			})
		.endClass();

	LUA.beginClass<Vec2>("Vec2")
		.addVariable("x", &Vec2::x)
		.addVariable("y", &Vec2::y)
		.endClass();

	LUA.beginClass<Rect>("Rect")
		.addVariable("x", &Rect::x)
		.addVariable("y", &Rect::y)
		.addVariable("width", &Rect::width)
		.addVariable("height", &Rect::height)
		.endClass();

	LUA.beginClass<Transform>("Transform")
		.addVariableRef("position", &Transform::position)
		.addVariable("rotation", &Transform::rotation)
		.addVariable("scale", &Transform::scale)
		.addVariable("verticalFlip", &Transform::verticalFlip)
		.addVariable("horizontalFlip", &Transform::horizontalFlip)
		.endClass();

	LUA.beginClass<SpriteInstance>("SpriteInstance")
		.addVariableRef("transform", &SpriteInstance::transform)
		.addFunction("checkPixelCollision", &SpriteInstance::checkPixelCollision)
		.addFunction("hit", &SpriteInstance::hit)
		.endClass();

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
