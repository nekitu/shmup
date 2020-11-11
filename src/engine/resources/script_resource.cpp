#include "resources/script_resource.h"
#include "resources/unit_resource.h"
#include "resources/font_resource.h"
#include "resources/sprite_resource.h"
#include "resources/animation_resource.h"
#include "utils.h"
#include "resource_loader.h"
#include "weapon.h"
#include "unit.h"
#include "sprite.h"
#include "game.h"
#include "graphics.h"
#include "projectile.h"

namespace LuaIntf
{
	LUA_USING_LIST_TYPE(std::vector)
}

namespace engine
{
static lua_State* L = nullptr;

ScriptClassInstanceBase::~ScriptClassInstanceBase()
{
	if (script)
	{
		auto iter = std::find(script->classInstances.begin(), script->classInstances.end(), this);

		if (iter != script->classInstances.end())
		{
			LOG_INFO("Destroying the script class instance for script: {0} ", script->fileName);
			script->classInstances.erase(iter);
		}
		else
		{
			LOG_ERROR("Cannot find the script class instance for script: {0} ", script->fileName);
		}
	}
}

bool ScriptResource::load(Json::Value& json)
{
	code = readTextFile(Game::makeFullDataPath(fileName + ".lua"));
	auto res = luaL_loadstring(L, code.c_str());

	// if we already have some class instances, recreate them with the new script
	for (auto& ci : classInstances)
	{
		ci->createInstance();
	}

	return res == 0;
}

void ScriptResource::unload()
{
	code = "";
	LOG_INFO("Unloading script {0} with {1} instances", fileName, classInstances.size());

	for (auto& ci : classInstances)
	{
		CALL_LUA_FUNC2(ci, "onUnload");
		ci->classInstance = LuaIntf::LuaRef();
	}
}

LuaIntf::LuaRef ScriptClassInstanceBase::getFunction(const std::string& funcName)
{
	if (!script || !getLuaState() || !classInstance.isValid())
		return LuaIntf::LuaRef();

	if (classInstance.has(funcName))
	{
		auto f = classInstance.get(funcName);

		if (!f.isFunction())
		{
			spdlog::error("Could not find the function '%s' in script '%s'\n", funcName.c_str(), script->fileName.c_str());
			return LuaIntf::LuaRef::fromPtr(L, nullptr);
		}

		return f;
	}

	return LuaIntf::LuaRef();
}

bool initializeLua()
{
	L = luaL_newstate();
	luaL_openlibs(L);

	auto LUA = LuaIntf::LuaBinding(L);

	LuaIntf::LuaContext l(L);

	LUA.beginModule("log")
		.addFunction("info", [](const std::string& str) { LOG_INFO("LUA: {0}", str); })
		.addFunction("error", [](const std::string& str) { LOG_ERROR("LUA: {0}", str); })
		.addFunction("debug", [](const std::string& str) { LOG_DEBUG("LUA: {0}", str); })
		.addFunction("warn", [](const std::string& str) { LOG_WARN("LUA: {0}", str); })
		.addFunction("critical", [](const std::string& str) { LOG_CRITICAL("LUA: {0}", str); })
		.endModule();

	LUA.beginClass<Game>("game")
		.addVariable("deltaTime", &Game::deltaTime)
		.addVariable("cameraSpeed", &Game::cameraSpeed)
		.addFunction("getProjectileCount", [](Game* game) { return Game::instance->projectiles.size(); })
		.addVariable("hiscore", &Game::hiscore)
		.addFunction("player1", [](Game* g) { return g->players[0].unit; })
		.addFunction("player2", [](Game* g) { return g->players[1].unit; })
		.addVariable("credit", &Game::credit)
		.addFunction("animateCameraSpeed", &Game::animateCameraSpeed)
		.addFunction("shakeCamera", &Game::shakeCamera)
		.addFunction("fadeScreen", &Game::fadeScreen)
		.addFunction("changeLevel", &Game::changeLevel)
		.addFunction("loadNextLevel", [](Game* g) { g->changeLevel(~0); })
		.addFunction("spawn", [](Game* g, const std::string& unitResource, const std::string& name, const Vec2& position)
			{
				auto u = g->createUnit(Game::instance->resourceLoader->loadUnit(unitResource));
				u->name = name;
				u->root->position = position;
				return u;
			}
		)
		.addFunction("loadFont", [](Game* g, const std::string& filename) { return g->resourceLoader->loadFont(filename); })
		.addFunction("loadSprite", [](Game* g, const std::string& filename) { return g->resourceLoader->loadSprite(filename); })
		.addVariable("cameraParallaxOffset", &Game::cameraParallaxOffset)
		.addVariable("cameraParallaxScale", &Game::cameraParallaxScale)
		.addVariableRef("cameraPosition", &Game::cameraPosition)
		.addVariableRef("cameraPositionOffset", &Game::cameraPositionOffset)
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
		.addFunction("clampValue", [](f32 val, f32 minVal, f32 maxVal)
			{
				clampValue(val, minVal, maxVal);
				return val;
			})
		.endModule();

	LUA.beginClass<Graphics>("Graphics")
		.addVariable("videoWidth", &Graphics::videoWidth)
		.addVariable("videoHeight", &Graphics::videoHeight)
		.addVariable("color", &Graphics::color)
		.addVariable("colorMode", &Graphics::colorMode)
		.addVariable("alphaMode", &Graphics::alphaMode)
		.addFunction("drawText", &Graphics::drawText)
		.endClass();

	LUA.beginClass<WeaponResource::Parameters>("WeaponParams")
		.addVariableRef("direction", &WeaponResource::Parameters::direction)
		.endClass();

	LUA.beginClass<Weapon>("Weapon")
		.addVariable("active", &Weapon::active)
		.addVariable("angle", &Weapon::angle)
		.addVariable("fireAngleOffset", &Weapon::fireAngleOffset)
		.addVariableRef("params", &Weapon::params)
		.addFunction("fire", &Weapon::fire)
		.endClass();

	LUA.beginClass<Resource>("Resource")
		.addVariable("type", &Resource::type)
		.endClass();

	LUA.beginExtendClass<UnitResource, Resource>("UnitResource")
		.addVariable("name", &UnitResource::name)
		.addVariable("fileName", &UnitResource::fileName)
		.addVariable("unitType", &UnitResource::unitType)
		.endClass();

	LUA.beginExtendClass<SpriteResource, UnitResource>("SpriteResource")
		.addVariable("frameCount", &SpriteResource::frameCount)
		.endClass();

	LUA.beginExtendClass<FontResource, UnitResource>("FontResource")
		.endClass();

	LUA.beginExtendClass<AnimationResource, UnitResource>("AnimationResource")
		.endClass();

	LUA.beginClass<AnimationKey>("AnimationKey")
		.endClass();

	LUA.beginClass<AnimationTrack>("AnimationTrack")
		.endClass();

	LUA.beginClass<UnitLifeStage>("UnitLifeStage")
		.addVariable("name", &UnitLifeStage::name)
		.addVariable("triggerOnHealth", &UnitLifeStage::triggerOnHealth)
		.endClass();

	LUA.beginClass<Unit>("Unit")
		.addVariable("id", &Unit::id, false)
		.addVariable("name", &Unit::name, true)
		.addVariable("layerIndex", &Unit::layerIndex)
		.addVariable("appeared", &Unit::appeared)
		.addVariable("unitResource", &Unit::unitResource, false)
		.addVariable("age", &Unit::age, false)
		.addVariable("health", &Unit::health)
		.addVariable("speed", &Unit::speed)
		.addVariable("stage", &Unit::currentStage)
		.addVariable("deleteMeNow", &Unit::deleteMeNow)
		.addVariable("root", &Unit::root)
		.addFunction("findSprite", &Unit::findSprite)
		.addFunction("findWeapon",
			[](Unit* unit, const std::string& weaponName)
			{
				auto iter = unit->weapons.find(weaponName);

				if (iter != unit->weapons.end())
				{
					return iter->second;
				}

				return (Weapon*)nullptr;
			}
		)
		.addFunction("getWeapons", [](Unit* thisObj)
			{
				LuaIntf::LuaRef wpns = LuaIntf::LuaRef::createTable(getLuaState());
				int i = 1;

				for (auto& wpn : thisObj->weapons)
				{
					wpns.set(i, wpn.second);
					++i;
				}

				return wpns;
			})
		.addFunction("checkPixelCollision", [](Unit* thisObj, Unit* other, LuaIntf::LuaRef& collisions)
			{
				std::vector<SpriteCollision> cols;
				bool collided = thisObj->checkPixelCollision(other, cols);
				int i = 1;

				for (auto& col : cols)
				{
					collisions.set(i++, col);
				}

				return collided;
			}
		)
		.addFunction("fire", [](Unit* unit)
			{
				for (auto& wp : unit->weapons)
				{
					wp.second->fire();
					wp.second->update(Game::instance);
				}
			})
		.addFunction("setAnimation", &Unit::setAnimation)
		.endClass();

	LUA.beginExtendClass<Projectile, Unit>("Projectile")
		.addVariableRef("weaponResource", &Projectile::weaponResource)
		.addVariableRef("velocity", &Projectile::velocity)
		.addVariable("minSpeed", &Projectile::minSpeed)
		.addVariable("maxSpeed", &Projectile::maxSpeed)
		.addVariable("acceleration", &Projectile::acceleration)
		.endClass();

	LUA.beginClass<ControllerInstanceResource>("ControllerInstanceResource")
		.addFunction("getInt", [](ControllerInstanceResource* cls, const std::string& name, int defaultVal) { return cls->json.get(name, defaultVal).asInt(); })
		.addFunction("getFloat", [](ControllerInstanceResource* cls, const std::string& name, f32 defaultVal) { return cls->json.get(name, defaultVal).asFloat(); })
		.addFunction("getString", [](ControllerInstanceResource* cls, const std::string& name, const std::string& defaultVal) { return cls->json.get(name, defaultVal).asString(); })
		.addFunction("getBool", [](ControllerInstanceResource* cls, const std::string& name, bool defaultVal) { return cls->json.get(name, defaultVal).asBool(); })
		.addFunction("getVec2", [](ControllerInstanceResource* cls, const std::string& name, const Vec2& defaultVal)
			{

				auto str = cls->json.get(name, defaultVal.toString()).asString();
				Vec2 v;
				v.parse(str);
				return v;
			})
		.endClass();

	LUA.beginClass<Color>("Color")
		.addConstructor(LUA_ARGS(f32, f32, f32, f32))
		.addVariable("r", &Color::r)
		.addVariable("g", &Color::g)
		.addVariable("b", &Color::b)
		.addVariable("a", &Color::a)
		.addStaticVariableRef("red", &Color::red)
		.addStaticVariableRef("green", &Color::green)
		.addStaticVariableRef("black", &Color::black)
		.addStaticVariableRef("sky", &Color::sky)
		.addFunction("getRgba", &Color::getRgba)
		.endClass();

	LUA.beginClass<Vec2>("Vec2")
		.addConstructor(LUA_ARGS(LuaIntf::_opt<f32>, LuaIntf::_opt<f32>))
		.addVariable("x", &Vec2::x)
		.addVariable("y", &Vec2::y)
		.addFunction("getCopy", [](Vec2* v) {return Vec2(v->x, v->y); })
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

	LUA.beginClass<Sprite>("Sprite")
		.addVariableRef("position", &Sprite::position)
		.addVariable("name", &Sprite::name)
		.addVariable("rotation", &Sprite::rotation)
		.addVariable("scale", &Sprite::scale)
		.addVariable("verticalFlip", &Sprite::verticalFlip)
		.addVariable("horizontalFlip", &Sprite::horizontalFlip)
		.addVariableRef("spriteResource", &Sprite::spriteResource)
		.addVariable("frame", &Sprite::animationFrame)
		.addVariable("rect", &Sprite::rect)
		.addVariable("relativeToRoot", &Sprite::relativeToRoot)
		.addFunction("setFrameAnimation", &Sprite::setFrameAnimation)
		.addFunction("setFrameAnimationFromAngle", &Sprite::setFrameAnimationFromAngle)
		.addFunction("checkPixelCollision", &Sprite::checkPixelCollision)
		.addFunction("hit", &Sprite::hit)
		.addFunction("getFrameFromAngle", &Sprite::getFrameFromAngle)
		.endClass();

	LUA.beginClass<SpriteCollision>("SpriteCollision")
		.addVariable("sprite1", &SpriteCollision::sprite1)
		.addVariable("sprite2", &SpriteCollision::sprite2)
		.addVariable("collisionCenter", &SpriteCollision::collisionCenter)
		.endClass();

	l.setGlobal("game", Game::instance);
	l.setGlobal("gfx", Game::instance->graphics);
	l.setGlobal("package.path", "../data/scripts/?.lua;?.lua");

	// constants and enums
	l.setGlobal("ColorMode_Add", ColorMode::Add);
	l.setGlobal("ColorMode_Sub", ColorMode::Sub);
	l.setGlobal("ColorMode_Mul", ColorMode::Mul);

	l.setGlobal("AlphaMode_Blend", AlphaMode::Blend);
	l.setGlobal("AlphaMode_Mask", AlphaMode::Mask);

	l.setGlobal("AnimationLoopMode_None", AnimationLoopMode::None);
	l.setGlobal("AnimationLoopMode_Normal", AnimationLoopMode::Normal);
	l.setGlobal("AnimationLoopMode_Reversed", AnimationLoopMode::Reversed);
	l.setGlobal("AnimationLoopMode_PingPong", AnimationLoopMode::PingPong);

	l.setGlobal("AnimationTrackType_Unknown", AnimationTrackType::Unknown);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);
	l.setGlobal("AnimationTrackType_", AnimationTrackType::);

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
