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
#include "unit_controller.h"

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

void ScriptResource::unload()
{
	M = LuaIntf::LuaRef();
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

	auto LUA = LuaIntf::LuaBinding(L);

	LUA.beginModule("game")
		.addVariable("deltaTime", &Game::instance->deltaTime)
		.addFunction("log", engine_log)
		.addVariable("cameraSpeed", &Game::instance->cameraSpeed)
		.addVariable("score", &Game::instance->score)
		.addVariableRef("player1", &Game::instance->players[0])
		.addVariableRef("player2", &Game::instance->players[1])
		.addVariable("credit", &Game::instance->credit)
		.addFunction("shakeCamera", [](UnitInstance* inst, const std::string& ctrlName, const Vec2& force, f32 duration, u32 count)
			{
				auto ctrl = static_cast<ScreenFxController*>(inst->findController(ctrlName));
				if (ctrl)
				{
					ctrl->shakeCamera(force, duration, count);
				}
			})
		.addFunction("fadeScreen", [](UnitInstance* inst, const std::string& ctrlName, const Color& color, u32 colorMode, f32 duration, bool revertBackAfter)
			{
				auto ctrl = static_cast<ScreenFxController*>(inst->findController(ctrlName));
				if (ctrl)
				{
					ctrl->fadeScreen(color, (ColorMode)colorMode, duration, revertBackAfter);
				}
			})
				.addFunction("animateCameraSpeed", [](f32 towards, f32 speed)
			{
				Game::instance->animateCameraSpeed(towards, speed);
			})
		.addFunction("changeLevel", [](int index) { Game::instance->changeLevel(index); })
		.addFunction("loadNextLevel", []() { Game::instance->changeLevel(~0); })
		.addFunction("spawn", [](const std::string& unit, const std::string& name, const Vec2& position)
			{
				auto uinst = Game::instance->createUnitInstance(Game::instance->resourceLoader->loadUnit(unit));
				uinst->name = name;
				uinst->rootSpriteInstance->transform.position = position;
				return uinst;
			}
		)
		.addFunction("loadFont", [](const std::string& filename) { return Game::instance->resourceLoader->loadFont(filename); })
		.addFunction("loadSprite", [](const std::string& filename) { return Game::instance->resourceLoader->loadSprite(filename); })
		.endModule();

	LUA.beginModule("gfx")
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
		.addVariable("unit", &UnitInstance::unit, false)
		.addVariable("age", &UnitInstance::age, false)
		.addVariable("health", &UnitInstance::health)
		.addVariable("stage", &UnitInstance::currentStage)
		.addVariable("deleteMeNow", &UnitInstance::deleteMeNow)
		.addVariable("rootSpriteInstance", &UnitInstance::rootSpriteInstance)
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

	LUA.beginClass<Color>("Color")
		.addConstructor(LUA_ARGS(f32, f32, f32, f32))
		.addVariable("r", &Color::r)
		.addVariable("g", &Color::g)
		.addVariable("b", &Color::b)
		.addVariable("a", &Color::a)
		.endClass();

	LUA.beginClass<Vec2>("Vec2")
		.addConstructor(LUA_ARGS(f32, f32))
		.addVariable("x", &Vec2::x)
		.addVariable("y", &Vec2::y)
		.addFunction("dir2deg", [](Vec2* v) { return dir2deg(*v); })
		.endClass();

	LUA.beginClass<Rect>("Rect")
		.addVariable("x", &Rect::x)
		.addVariable("y", &Rect::y)
		.addVariable("width", &Rect::width)
		.addVariable("height", &Rect::height)
		.addFunction("center", &Rect::center)
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
		.addVariableRef("sprite", &SpriteInstance::sprite)
		.addVariable("animationFrame", &SpriteInstance::animationFrame)
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
