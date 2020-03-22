#include "resources/script_resource.h"
#include "resources/unit_resource.h"
#include "resources/font_resource.h"
#include "resources/sprite_resource.h"
#include "utils.h"
#include "resource_loader.h"
#include "weapon_instance.h"
#include "unit_instance.h"
#include "sprite_instance.h"
#include "game.h"
#include "graphics.h"
#include "projectile_instance.h"

namespace engine
{
static lua_State* L = nullptr;

bool ScriptResource::load(Json::Value& json)
{
	code = readTextFile(loader->root + fileName + ".lua");
	auto res = luaL_loadstring(L, code.c_str());

	// if we already have some class instances, recreate them with the new script
	for (auto& ci : classInstances)
	{
		lua_pushlightuserdata(L, ci->object);

		//TODO: cant we store the creator function only ? do we have to call the entire code ?
		auto result = lua_pcall(L, 0, LUA_MULTRET, 0);

		if (result)
		{
			printf("Lua error: %s\n", lua_tostring(L, -1));
			continue;
		}

		if (lua_istable(L, -1)) {
			ci->classInstance = LuaIntf::LuaRef::popFromStack(L);
		}
		else
		{
			printf("Lua: Please return a class table in '%s'\n", fileName.c_str());
		}
	}

	return res == 0;
}

void ScriptResource::unload()
{
	code = "";

	for (auto& ci : classInstances)
	{
		ci->classInstance = LuaIntf::LuaRef();
	}
}

ScriptClassInstance* ScriptResource::createClassInstance(void* obj)
{
	if (code.empty())
	{
		printf("No code in: '%s'\n", fileName.c_str());
		return nullptr;
	}

	auto res = luaL_loadstring(L, code.c_str());

	auto result = lua_pcall(L, 0, LUA_MULTRET, 0);

	if (result)
	{
		printf("createClassInstance: Lua error: %s\n", lua_tostring(L, -1));
		return nullptr;
	}

	LuaIntf::LuaRef inst;

	auto cfunc = LuaIntf::LuaRef::popFromStack(L);

	printf("Creating instance '%s'\n", fileName.c_str());

	if (cfunc.isFunction())
	{
		inst = cfunc.call<LuaIntf::LuaRef>(LuaIntf::LuaRef::fromPtr(L, obj));
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

LuaIntf::LuaRef ScriptClassInstance::getFunction(const std::string& funcName)
{
	if (!script)
		return LuaIntf::LuaRef();

	if (classInstance.has(funcName))
	{
		auto f = classInstance.get(funcName);

		if (!f.isFunction())
		{
			printf("Could not find the function '%s' in script '%s'\n", funcName.c_str(), script->fileName.c_str());
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

	auto LUA = LuaIntf::LuaBinding(L);

	LuaIntf::LuaContext l(L);

	LUA.beginClass<Game>("game")
		.addVariable("deltaTime", &Game::deltaTime)
		//.addFunction("log", &engine_log)
		.addVariable("cameraSpeed", &Game::cameraSpeed)
		.addVariable("hiscore", &Game::hiscore)
		.addFunction("player1", [](Game* g) { return g->players[0].unitInstance; })
		.addFunction("player2", [](Game* g) { return g->players[1].unitInstance; })
		.addVariable("credit", &Game::credit)
		.addFunction("animateCameraSpeed", &Game::animateCameraSpeed)
		.addFunction("shakeCamera", &Game::shakeCamera)
		.addFunction("fadeScreen", &Game::fadeScreen)
		.addFunction("changeLevel", &Game::changeLevel)
		.addFunction("loadNextLevel", [](Game* g) { g->changeLevel(~0); })
		.addFunction("spawn", [](Game* g, const std::string& unit, const std::string& name, const Vec2& position)
			{
				auto uinst = g->createUnitInstance(Game::instance->resourceLoader->loadUnit(unit));
				uinst->name = name;
				uinst->root->position = position;
				return uinst;
			}
		)
		.addFunction("loadFont", [](Game* g, const std::string& filename) { return g->resourceLoader->loadFont(filename); })
		.addFunction("loadSprite", [](Game* g, const std::string& filename) { return g->resourceLoader->loadSprite(filename); })
		.addVariable("cameraParallaxOffset", &Game::cameraParallaxOffset)
		.addVariable("cameraParallaxScale", &Game::cameraParallaxScale)
		.addVariableRef("cameraParallaxOffset", &Game::cameraPosition)
		.addVariableRef("cameraParallaxOffset", &Game::cameraPositionOffset)
		.addVariable("cameraSpeed", &Game::cameraSpeed)
		.addVariable("cameraSpeedAnimateSpeed", &Game::cameraSpeedAnimateSpeed)
		.addVariable("cameraSpeedAnimateTime", &Game::cameraSpeedAnimateTime)
		.addVariable("animatingCameraSpeed", &Game::animatingCameraSpeed)
		.addFunction("isPlayerMoveUp", &Game::isPlayerMoveUp)
		.addFunction("isPlayerMoveDown", &Game::isPlayerMoveDown)
		.addFunction("isPlayerMoveLeft", &Game::isPlayerMoveLeft)
		.addFunction("isPlayerMoveRight", &Game::isPlayerMoveRight)
		.addFunction("isPlayerFire1", &Game::isPlayerFire1)
		.addFunction("isPlayerFire2", &Game::isPlayerFire2)
		.addFunction("isPlayerFire3", &Game::isPlayerFire3)
		.endClass();

	LUA.beginModule("util")
		.addFunction("clampValue", [](f32& val, f32 minVal, f32 maxVal)
			{
				return clampValue(val, minVal, maxVal);
			})
		.endModule();

	LUA.beginModule("gfx")
		.addVariable("videoWidth", &Game::instance->graphics->videoWidth)
		.addVariable("videoHeight", &Game::instance->graphics->videoHeight)
		.addFunction("drawText", [](FontResource* font, const Vec2& pos, const std::string& text)
			{
				Game::instance->graphics->drawText(font, pos, text);
			})
		.endModule();

	LUA.beginClass<WeaponResource::Parameters>("WeaponParams")
		.addVariableRef("direction", &WeaponResource::Parameters::direction)
		.endClass();

	LUA.beginClass<WeaponInstance>("WeaponInstance")
		.addVariable("active", &WeaponInstance::active)
		.addVariable("angle", &WeaponInstance::angle)
		.addVariableRef("params", &WeaponInstance::params)
		.addFunction("fire", &WeaponInstance::fire)
		.addFunction("debug", &WeaponInstance::debug)
		.endClass();

	LUA.beginClass<UnitResource>("UnitResource")
		.addVariable("name", &UnitResource::name)
		.addVariable("fileName", &UnitResource::fileName)
		.addVariable("type", &UnitResource::type)
		.endClass();

	LUA.beginExtendClass<SpriteResource, UnitResource>("SpriteResource")
		.addVariable("frameCount", &SpriteResource::frameCount)
		.endClass();

	LUA.beginExtendClass<FontResource, UnitResource>("FontResource")
		.endClass();

	LUA.beginClass<UnitLifeStage>("UnitLifeStage")
		.addVariable("name", &UnitLifeStage::name)
		.addVariable("triggerOnHealth", &UnitLifeStage::triggerOnHealth)
		.endClass();

	LUA.beginClass<UnitInstance>("UnitInstance")
		.addVariable("id", &UnitInstance::id, false)
		.addVariable("name", &UnitInstance::name, true)
		.addVariable("layerIndex", &UnitInstance::layerIndex)
		.addVariable("appeared", &UnitInstance::appeared)
		.addVariable("unitResource", &UnitInstance::unit, false)
		.addVariable("age", &UnitInstance::age, false)
		.addVariable("health", &UnitInstance::health)
		.addVariable("stage", &UnitInstance::currentStage)
		.addVariable("deleteMeNow", &UnitInstance::deleteMeNow)
		.addVariable("root", &UnitInstance::root)
		.addFunction("findWeapon",
			[](UnitInstance* uinst, const std::string& weaponName)
			{
				auto iter = uinst->weapons.find(weaponName);

				if (iter != uinst->weapons.end())
				{
					return iter->second;
				}

				return (WeaponInstance*)nullptr;
			}
		)
		.addFunction("checkPixelCollision", [](UnitInstance* thisObj, UnitInstance* other, LuaIntf::LuaRef& collisions)
			{
				std::vector<SpriteInstanceCollision> cols;
				bool collided = thisObj->checkPixelCollision(other, cols);
				int i = 1;

				for (auto& col : cols)
				{
					collisions.set(i++, col);
				}

				return collided;
			}
		)
		.addFunction("fire", [](UnitInstance* inst)
			{
				for (auto& wp : inst->weapons)
				{
					wp.second->fire();
					wp.second->update(Game::instance);
				}
			})
		.endClass();

	LUA.beginExtendClass<ProjectileInstance, UnitInstance>("ProjectileInstance")
		.addVariableRef("weapon", &ProjectileInstance::weapon)
		.addVariableRef("velocity", &ProjectileInstance::velocity)
		.addVariable("minSpeed", &ProjectileInstance::minSpeed)
		.addVariable("maxSpeed", &ProjectileInstance::maxSpeed)
		.addVariable("acceleration", &ProjectileInstance::acceleration)
		.endClass();

	LUA.beginClass<Color>("Color")
		.addConstructor(LUA_ARGS(f32, f32, f32, f32))
		.addVariable("r", &Color::r)
		.addVariable("g", &Color::g)
		.addVariable("b", &Color::b)
		.addVariable("a", &Color::a)
		.endClass();

	LUA.beginClass<Vec2>("Vec2")
		.addConstructor(LUA_ARGS(LuaIntf::_opt<f32>, LuaIntf::_opt<f32>))
		.addVariable("x", &Vec2::x)
		.addVariable("y", &Vec2::y)
		.addFunction("dir2deg", [](Vec2* v) { return dir2deg(*v); })
		.addFunction("normalize", &Vec2::normalize)
		.addFunction("getLength", &Vec2::getLength)
		.addFunction("__add", [](Vec2* v1, Vec2* v2) { return *v1 + *v2; })
		.addFunction("__sub", [](Vec2* v1, Vec2* v2) { return *v1 - *v2; })
		.addFunction("__mul", [](Vec2* v1, Vec2* v2) { return *v1 * *v2; })
		.addFunction("__div", [](Vec2* v1, Vec2* v2) { return *v1 / *v2; })
		.addFunction("add", [](Vec2* v1, Vec2* v2) { *v1 += *v2; return v1; })
		.addFunction("sub", [](Vec2* v1, Vec2* v2) { *v1 -= *v2; return v1; })
		.addFunction("mul", [](Vec2* v1, Vec2* v2) { *v1 *= *v2; return v1; })
		.addFunction("div", [](Vec2* v1, Vec2* v2) { *v1 /= *v2; return v1; })
		.addFunction("addReturn", [](Vec2* v1, Vec2* v2) { return *v1 + *v2; })
		.addFunction("subReturn", [](Vec2* v1, Vec2* v2) { return *v1 - *v2; })
		.addFunction("mulReturn", [](Vec2* v1, Vec2* v2) { return *v1 * *v2; })
		.addFunction("divReturn", [](Vec2* v1, Vec2* v2) { return *v1 / *v2; })
		.addFunction("addScalar", [](Vec2* v1, f32 val) { *v1 += val; return v1; })
		.addFunction("subScalar", [](Vec2* v1, f32 val) { *v1 -= val; return v1; })
		.addFunction("mulScalar", [](Vec2* v1, f32 val) { *v1 *= val; return v1; })
		.addFunction("divScalar", [](Vec2* v1, f32 val) { *v1 /= val; return v1; })
		.addFunction("addScalarReturn", [](Vec2* v1, f32 val) { return *v1 + val; })
		.addFunction("subScalarReturn", [](Vec2* v1, f32 val) { return *v1 - val; })
		.addFunction("mulScalarReturn", [](Vec2* v1, f32 val) { return *v1 * val; })
		.addFunction("divScalarReturn", [](Vec2* v1, f32 val) { return *v1 / val; })
		.endClass();

	LUA.beginClass<Rect>("Rect")
		.addConstructor(LUA_ARGS(LuaIntf::_opt<f32>, LuaIntf::_opt<f32>, LuaIntf::_opt<f32>, LuaIntf::_opt<f32>))
		.addVariable("x", &Rect::x)
		.addVariable("y", &Rect::y)
		.addVariable("width", &Rect::width)
		.addVariable("height", &Rect::height)
		.addFunction("center", &Rect::center)
		.endClass();

	LUA.beginClass<SpriteInstance>("SpriteInstance")
		.addVariableRef("position", &SpriteInstance::position)
		.addVariable("rotation", &SpriteInstance::rotation)
		.addVariable("scale", &SpriteInstance::scale)
		.addVariable("verticalFlip", &SpriteInstance::verticalFlip)
		.addVariable("horizontalFlip", &SpriteInstance::horizontalFlip)
		.addVariableRef("screenPosition", &SpriteInstance::screenPosition)
		.addVariableRef("sprite", &SpriteInstance::sprite)
		.addVariable("frame", &SpriteInstance::animationFrame)
		.addVariable("screenRect", &SpriteInstance::screenRect)
		.addFunction("setFrameAnimation", &SpriteInstance::setFrameAnimation)
		.addFunction("setFrameAnimationFromAngle", &SpriteInstance::setFrameAnimationFromAngle)
		.addFunction("checkPixelCollision", &SpriteInstance::checkPixelCollision)
		.addFunction("hit", &SpriteInstance::hit)
		.addFunction("getFrameFromAngle", &SpriteInstance::getFrameFromAngle)
		.endClass();

	LUA.beginClass<SpriteInstanceCollision>("SpriteInstanceCollision")
		.addVariable("a", &SpriteInstanceCollision::a)
		.addVariable("b", &SpriteInstanceCollision::b)
		.addVariable("collisionCenter", &SpriteInstanceCollision::collisionCenter)
		.endClass();

	l.setGlobal("game", Game::instance);
	l.setGlobal("ColorMode_Add", ColorMode::Add);
	l.setGlobal("ColorMode_Sub", ColorMode::Sub);
	l.setGlobal("ColorMode_Mul", ColorMode::Mul);

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
