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
#include "music.h"
#include "graphics.h"
#include "projectile.h"
#include "image_atlas.h"
#include <SDL_mixer.h>

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
			LOG_INFO("Destroying the script class instance for script: {} ", script->path);
			script->classInstances.erase(iter);
		}
		else
		{
			LOG_ERROR("Cannot find the script class instance for script: {} ", script->path);
		}
	}
}

bool ScriptResource::load(Json::Value& json)
{
	code = readTextFile(Game::makeFullDataPath(path + ".lua"));
	auto res = luaL_loadstring(L, code.c_str());

	// if we already have some class instances, recreate them with the new script
	for (auto& ci : classInstances)
	{
		ci->script = this;
		ci->createInstance();
	}

	if (classInstances.size())
		deserialize();

	return res == 0;
}

void ScriptResource::unload()
{
	code = "";
	serialize();
}

void ScriptResource::serialize()
{
	LOG_INFO("Serializing script {} with {} instances", path, classInstances.size());

	serializedInstancesTable = LuaIntf::LuaRef::createTable(getLuaState());
	int i = 1;
	for (auto& ci : classInstances)
	{
		LuaIntf::LuaRef tbl = LuaIntf::LuaRef::createTable(getLuaState());
		CALL_LUA_FUNC2(ci, "onSerialize", tbl);
		serializedInstancesTable[i] = tbl;
		i++;
	}

	LuaIntf::LuaRef luaSerialize = LuaIntf::LuaRef(getLuaState(), "pickle");

	serializedInstancesString = luaSerialize.call<std::string>(serializedInstancesTable);

	serializedInstancesTable = LuaIntf::LuaRef();

	for (auto& ci : classInstances)
	{
		ci->classInstance = LuaIntf::LuaRef();
	}
}

void ScriptResource::deserialize()
{
	LOG_INFO("Deserializing script {} with {} instances", path, classInstances.size());

	LuaIntf::LuaRef luaDeserialize = LuaIntf::LuaRef(getLuaState(), "unpickle");

	serializedInstancesTable = luaDeserialize.call<LuaIntf::LuaRef>(serializedInstancesString);

	int i = 1;
	for (auto& ci : classInstances)
	{
		CALL_LUA_FUNC2(ci, "onDeserialize", serializedInstancesTable.get(i));
		i++;
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
			spdlog::error("Could not find the function '%s' in script '%s'\n", funcName.c_str(), script->path.c_str());
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

	l.setGlobal("package.path", "../data/scripts/?.lua;?.lua");

	LUA.beginModule("log")
		.addFunction("info", [](const std::string& str) { LOG_INFO("LUA: {}", str); })
		.addFunction("error", [](const std::string& str) { LOG_ERROR("LUA: {}", str); })
		.addFunction("debug", [](const std::string& str) { LOG_DEBUG("LUA: {}", str); })
		.addFunction("warn", [](const std::string& str) { LOG_WARN("LUA: {}", str); })
		.addFunction("critical", [](const std::string& str) { LOG_CRITICAL("LUA: {}", str); })
		.endModule();

	LUA.beginClass<GameScreen>("GameScreen")
		.addConstructor(LUA_ARGS())
		.addVariable("id", &GameScreen::id)
		.addVariable("name", &GameScreen::name)
		.addVariable("path", &GameScreen::path)
		.endClass();

	LUA.beginClass<CameraState>("CameraState")
		.addConstructor(LUA_ARGS())
		.addVariable("speed", &CameraState::speed)
		.addVariable("parallaxOffset", &CameraState::parallaxOffset)
		.addVariable("parallaxScale", &CameraState::parallaxScale)
		.addVariableRef("position", &CameraState::position)
		.addVariableRef("positionOffset", &CameraState::positionOffset)
		.addVariable("speedAnimateSpeed", &CameraState::speedAnimateSpeed)
		.addVariable("speedAnimateTime", &CameraState::speedAnimateTime)
		.addVariable("animatingSpeed", &CameraState::animatingSpeed)
		.endClass();

	LUA.beginClass<GameState>("GameState")
		.addVariable("score", &GameState::score)
		.endClass();

	LUA.beginClass<Game>("Game")
		.addVariable("deltaTime", &Game::deltaTime)
		.addVariable("prebakedAtlas", &Game::prebakedAtlas)
		.addVariable("realDeltaTime", &Game::realDeltaTime)
		.addFunction("animateTimeScale", &Game::animateTimeScale)
		.addVariableRef("cameraState", &Game::cameraState)
		.addFunction("createPlayers", &Game::createPlayers)
		.addFunction("getProjectileCount", [](Game* game) { return Game::instance->projectiles.size(); })
		.addFunction("getUnitCount", [](Game* game) { return Game::instance->units.size(); })
		.addFunction("getUnit", [](Game* game, int idx) { return Game::instance->units[idx]; })
		.addFunction("findUnitById", &Game::findUnitById)
		.addFunction("findUnitByName", &Game::findUnitByName)
		.addFunction("findGameScreenById", &Game::findGameScreenById)
		.addVariable("gameState", &Game::gameState)
		.addFunction("player1", [](Game* g) { return g->playerState[0].unit; })
		.addFunction("player2", [](Game* g) { return g->playerState[1].unit; })
		.addFunction("getLastLayerIndex", [](Game* g) { if (!g->map) return (size_t)0; return g->map->layers.size() - 1; })
		.addFunction("animateCameraSpeed", &Game::animateCameraSpeed)
		.addFunction("shakeCamera", &Game::shakeCamera)
		.addFunction("fadeScreen", &Game::fadeScreen)
		.addFunction("changeMapByIndex", &Game::changeMapByIndex)
		.addFunction("changeMapByName", &Game::changeMapByName)
		.addFunction("renderUnits", &Game::renderUnits)
		.addFunction("setScreenActive", &Game::setScreenActive)
		.addFunction("loadNextMap", [](Game* g) { g->changeMapByIndex(~0); })
		.addFunction("spawn", [](Game* g, const std::string& unitResource, const std::string& name, const Vec2& position)
			{
				auto u = g->createUnit(Game::instance->resourceLoader->loadUnit(unitResource));
				u->name = name;
				u->root->position = position;
				return u;
			}
		)
		.addFunction("loadFont", [](Game* g, const std::string& path) { return g->resourceLoader->loadFont(path); })
		.addFunction("loadSprite", [](Game* g, const std::string& path) { return g->resourceLoader->loadSprite(path); })
		.addFunction("showMousePointer", &Game::showMousePointer)
		.addFunction("worldToScreen", [](Game* game, const Vec2& v, int layerIndex)
			{
				return game->worldToScreen(v, layerIndex);
			})
		.addFunction("screenToWorld", [](Game* game, const Vec2& v, int layerIndex)
			{
				return game->screenToWorld(v, layerIndex);
			})
		.addFunction("fadeOutMusic", [](Game* game, i32 msec) { game->music->fadeOutMusic(msec); })
		.addFunction("changeMusic",
			[](Game* game, const std::string& path, i32 fadeOutMsec, i32 fadeInMsec)
			{
				game->music->changeMusic(path, fadeInMsec, fadeOutMsec);
			}, LUA_ARGS(const std::string& path, LuaIntf::_def<i32, 3000>, LuaIntf::_def<i32, 3000>))
		.addFunction("setChannelVolume", [](Game* game, i32 channel, f32 vol)
		{
			Mix_Volume(channel, vol * 128);
		})
		.addFunction("setMusicVolume", [](Game* game, f32 vol)
		{
			Mix_VolumeMusic(vol * 128);
		})

		.endClass();

	LUA.beginClass<Input>("Input")
		.addFunction("isDown", &Input::isDown)
		.addFunction("wasDown", &Input::wasDown)
		.addFunction("wasPressed", &Input::wasPressed)
		.addFunction("getValue", &Input::getValue)
		.addVariableRef("mousePosition", &Input::mousePosition)
		.addVariableRef("windowMousePosition", &Input::windowMousePosition)
	.endClass();

	LUA.beginModule("util")
		.addFunction("clampValue", [](f32 val, f32 minVal, f32 maxVal)
			{
				clampValue(val, minVal, maxVal);
				return val;
			})
		.addFunction("saveAtlas", [](const std::string path)
			{
				return Game::instance->graphics->atlas->save(path);
			}
			)
		.endModule();

	LUA.beginClass<Graphics>("Graphics")
		.addVariable("videoWidth", &Graphics::videoWidth)
		.addVariable("videoHeight", &Graphics::videoHeight)
		.addVariable("color", &Graphics::color)
		.addVariable("colorMode", &Graphics::colorMode)
		.addVariable("alphaMode", &Graphics::alphaMode)
		.addFunction("setupColor", &Graphics::setupColor)
		.addFunction("drawText", &Graphics::drawText)
		.addFunction("getTextSize", &Graphics::getTextSize)
		.addFunction("createUserPalette", &Graphics::createUserPalette)
		.addFunction("freeUserPalette", &Graphics::freeUserPalette)
		.addFunction("drawSpriteCustomQuad", [](Graphics* gfx, struct SpriteResource* spr, const Vec2& topLeft, const Vec2& topRight, const Vec2& btmRight, const Vec2& btmLeft, u32 frame, f32 angle)
		{
			gfx->drawSpriteCustomQuad(spr, topLeft, topRight, btmRight, btmLeft, frame, angle);
		})
		.addFunction("drawPalettedSpriteCustomQuad", [](Graphics* gfx, struct SpriteResource* spr, const Vec2& topLeft, const Vec2& topRight, const Vec2& btmRight, const Vec2& btmLeft, u32 frame, f32 angle, ColorPalette* palette)
		{
			gfx->drawSpriteCustomQuad(spr, topLeft, topRight, btmRight, btmLeft, frame, angle, palette);
		})
		.addFunction("drawSprite", [](Graphics* gfx, SpriteResource* spr, const Rect& rc, u32 frame, f32 angle)
			{
				gfx->drawSprite(spr, rc, frame, angle);
			})
		.addFunction("drawPalettedSprite", [](Graphics* gfx, SpriteResource* spr, const Rect& rc, u32 frame, f32 angle, ColorPalette* userPalette)
			{
				gfx->drawSprite(spr, rc, frame, angle, userPalette);
			})
		.addFunction("drawLine", &Graphics::drawLine)
		.addFunction("drawRect", &Graphics::drawRect)
		.endClass();

	LUA.beginClass<ColorPalette>("ColorPalette")
		.addFunction("setColor", &ColorPalette::setColor)
		.addFunction("getColor", &ColorPalette::getColor)
		.addFunction("copyFromSprite", &ColorPalette::copyFromSprite)
		.endClass();

	LUA.beginClass<Parameters>("Parameters")
		.addFunction("getInt", &Parameters::getInt)
		.addFunction("getFloat", &Parameters::getFloat)
		.addFunction("getString", &Parameters::getString)
		.addFunction("getBool", &Parameters::getBool)
		.addFunction("getVec2", &Parameters::getVec2)
		.addFunction("getColor", &Parameters::getColor)
		.endClass();

	LUA.beginClass<WeaponResource::Parameters>("WeaponParams")
		.addVariable("fireRate", &WeaponResource::Parameters::fireRate)
		.addVariableRef("direction", &WeaponResource::Parameters::direction)
		.endClass();

	LUA.beginClass<Weapon>("Weapon")
		.addVariable("active", &Weapon::active)
		.addVariable("angle", &Weapon::angle)
		.addVariable("fireAngleOffset", &Weapon::fireAngleOffset)
		.addVariableRef("params", &Weapon::params)
		.addFunction("fire", &Weapon::fire)
		.addFunction("stopFire", &Weapon::stopFire)
		.endClass();

	LUA.beginClass<Resource>("Resource")
		.addVariable("type", &Resource::type)
		.addVariable("path", &Resource::path)
		.endClass();

	LUA.beginExtendClass<UnitResource, Resource>("UnitResource")
		.addVariable("name", &UnitResource::name)
		.addVariable("unitType", &UnitResource::unitType)
		.addVariableRef("parameters", &UnitResource::parameters)
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
		.addVariable("stageIndex", &Unit::stageIndex)
		.addVariable("appeared", &Unit::appeared)
		.addVariable("selected", &Unit::selected)
		.addVariable("persistentOnChangeMap", &Unit::persistentOnChangeMap)
		.addVariable("unitResource", &Unit::unitResource, false)
		.addVariable("age", &Unit::age, false)
		.addVariable("health", &Unit::health)
		.addVariable("speed", &Unit::speed)
		.addVariable("stage", &Unit::currentStage)
		.addVariableRef("boundingBox", &Unit::boundingBox)
		.addVariable("deleteMeNow", &Unit::deleteMeNow)
		.addVariable("root", &Unit::root)
		.addFunction("replaceSprite", &Unit::replaceSprite)
		.addFunction("findSprite", &Unit::findSprite)
		.addFunction("hideAllSprites", &Unit::hideAllSprites)
		.addFunction("disableAllWeapons", &Unit::disableAllWeapons)
		.addFunction("localToScreen", [](Unit* unit, const Vec2& v)
			{
				Vec2 sv = v;
				sv += unit->root->position;
				return Game::instance->worldToScreen(sv, unit->layerIndex);
			}
		)
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
		.addFunction("getGroupWeapons", [](Unit* thisObj, int groupIndex)
			{
				LuaIntf::LuaRef wpns = LuaIntf::LuaRef::createTable(getLuaState());
				int i = 1;

				for (auto& wpn : thisObj->weapons)
				{
					if (wpn.second->groupIndex == groupIndex)
					{
						wpns.set(i, wpn.second);
						++i;
					}
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
		.addFunction("playSound", &Unit::playSound)
		.addFunction("isSoundPlaying", &Unit::isSoundPlaying)
		.addFunction("addController", [](Unit* unit, const std::string& scriptPath, const std::string& name, Parameters* params)
			{
				auto script = Game::instance->resourceLoader->loadScript(scriptPath);
				auto ctrl = script->createClassInstance<Unit>(unit);
				ctrl->script = script;
				unit->controllers.insert(std::make_pair(name, ctrl));
				CALL_LUA_FUNC2(ctrl, "setup", params);
			})
		.addFunction("getSpriteCount", [](Unit* unit) { return unit->sprites.size(); })
		.addFunction("getSprite", [](Unit* unit, int idx) { return unit->sprites[idx]; })
		.endClass();

	LUA.beginExtendClass<Projectile, Unit>("Projectile")
		.addVariableRef("weaponResource", &Projectile::weaponResource)
		.addVariableRef("velocity", &Projectile::velocity)
		.addVariable("minSpeed", &Projectile::minSpeed)
		.addVariable("maxSpeed", &Projectile::maxSpeed)
		.addVariable("acceleration", &Projectile::acceleration)
		.endClass();

	LUA.beginClass<ControllerInstanceResource>("ControllerInstanceResource")
		.addVariable("name", &ControllerInstanceResource::name)
		.addVariableRef("parameters", &ControllerInstanceResource::parameters)
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
		.addStaticVariableRef("transparent", &Color::transparent)
		.addFunction("getRgba", &Color::getRgba)
		.endClass();

	LUA.beginClass<Vec2>("Vec2")
		.addConstructor(LUA_ARGS(LuaIntf::_opt<f32>, LuaIntf::_opt<f32>))
		.addVariable("x", &Vec2::x)
		.addVariable("y", &Vec2::y)
		.addFunction("set", &Vec2::set)
		.addFunction("getCopy", [](Vec2* v) {return Vec2(v->x, v->y); })
		.addFunction("dir2deg", [](Vec2* v) { return dir2deg(*v); })
		.addFunction("normalize", &Vec2::normalize)
		.addFunction("getNormalized", &Vec2::getNormalized)
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
		.addFunction("lerp", [](Vec2* v1, Vec2* v2, f32 t) { return *v1 + (*v2 - *v1) * t; })
		.addFunction("dir2deg", [](Vec2* v) {
				return dir2deg(*v);
			})
		.endClass();

	LUA.beginClass<Rect>("Rect")
		.addConstructor(LUA_ARGS(LuaIntf::_opt<f32>, LuaIntf::_opt<f32>, LuaIntf::_opt<f32>, LuaIntf::_opt<f32>))
		.addVariable("x", &Rect::x)
		.addVariable("y", &Rect::y)
		.addVariable("width", &Rect::width)
		.addVariable("height", &Rect::height)
		.addFunction("center", &Rect::center)
		.endClass();

	LUA.beginClass<Sound>("Sound")
		.addVariable("channel", &Sound::channel)
		.endClass();

	LUA.beginClass<Music>("Music")
		.endClass();

	LUA.beginClass<Sprite>("Sprite")
		.addVariableRef("position", &Sprite::position)
		.addVariable("name", &Sprite::name)
		.addVariable("rotation", &Sprite::rotation)
		.addVariableRef("scale", &Sprite::scale)
		.addVariable("verticalFlip", &Sprite::verticalFlip)
		.addVariable("horizontalFlip", &Sprite::horizontalFlip)
		.addVariable("health", &Sprite::health)
		.addVariable("visible", &Sprite::visible)
		.addVariable("selected", &Sprite::selected)
		.addVariable("collide", &Sprite::collide)
		.addVariableRef("spriteResource", &Sprite::spriteResource)
		.addVariable("frame", &Sprite::animationFrame)
		.addVariableRef("rect", &Sprite::rect)
		.addVariableRef("localRect", &Sprite::localRect)
		.addVariable("animationIsActive", &Sprite::animationIsActive)
		.addVariable("relativeToRoot", &Sprite::relativeToRoot)
		.addFunction("getFrameAnimationName", [](Sprite* spr) { if (!spr->frameAnimation) return std::string(""); return spr->frameAnimation->name; })
		.addFunction("setFrameAnimation", &Sprite::setFrameAnimation)
		.addFunction("setFrameAnimationFromAngle", &Sprite::setFrameAnimationFromAngle)
		.addFunction("checkPixelCollision", &Sprite::checkPixelCollision)
		.addFunction("hit", &Sprite::hit)
		.addFunction("getFrameFromAngle", &Sprite::getFrameFromAngle)
		.addFunction("copyPaletteFromResource", &Sprite::copyPaletteFromResource)
		.addFunction("setPaletteEntry", &Sprite::setPaletteEntry)
		.endClass();

	LUA.beginClass<SpriteCollision>("SpriteCollision")
		.addVariable("sprite1", &SpriteCollision::sprite1)
		.addVariable("sprite2", &SpriteCollision::sprite2)
		.addVariable("collisionCenter", &SpriteCollision::collisionCenter)
		.endClass();

	l.setGlobal("game", Game::instance);
	l.setGlobal("input", &Game::instance->input);
	l.setGlobal("gfx", Game::instance->graphics);

	LUA.addFunction("randomFloat", &randomFloat);

	// constants and enums
	l.setGlobal("UnitType_Enemy", UnitType::Enemy);
	l.setGlobal("UnitType_EnemyProjectile", UnitType::EnemyProjectile);
	l.setGlobal("UnitType_Item", UnitType::Item);
	l.setGlobal("UnitType_Player", UnitType::Player);
	l.setGlobal("UnitType_PlayerProjectile", UnitType::PlayerProjectile);
	l.setGlobal("UnitType_None", UnitType::None);

	l.setGlobal("ColorMode_Add", ColorMode::Add);
	l.setGlobal("ColorMode_Sub", ColorMode::Sub);
	l.setGlobal("ColorMode_Mul", ColorMode::Mul);
	l.setGlobal("ColorMode_Set", ColorMode::Set);

	l.setGlobal("AlphaMode_Blend", AlphaMode::Blend);
	l.setGlobal("AlphaMode_Mask", AlphaMode::Mask);

	l.setGlobal("AnimationLoopMode_None", AnimationLoopMode::None);
	l.setGlobal("AnimationLoopMode_Normal", AnimationLoopMode::Normal);
	l.setGlobal("AnimationLoopMode_Reversed", AnimationLoopMode::Reversed);
	l.setGlobal("AnimationLoopMode_PingPong", AnimationLoopMode::PingPong);

	l.setGlobal("AnimationTrackType_Unknown", AnimationTrackType::Unknown);
	l.setGlobal("AnimationTrackType_PositionX", AnimationTrackType::PositionX);
	l.setGlobal("AnimationTrackType_PositionY", AnimationTrackType::PositionY);
	l.setGlobal("AnimationTrackType_ScaleX", AnimationTrackType::ScaleX);
	l.setGlobal("AnimationTrackType_ScaleY", AnimationTrackType::ScaleY);
	l.setGlobal("AnimationTrackType_UniformScale", AnimationTrackType::UniformScale);
	l.setGlobal("AnimationTrackType_VerticalFlip", AnimationTrackType::VerticalFlip);
	l.setGlobal("AnimationTrackType_HorizontalFlip", AnimationTrackType::HorizontalFlip);
	l.setGlobal("AnimationTrackType_Rotation", AnimationTrackType::Rotation);
	l.setGlobal("AnimationTrackType_Visible", AnimationTrackType::Visible);
	l.setGlobal("AnimationTrackType_Shadow", AnimationTrackType::Shadow);
	l.setGlobal("AnimationTrackType_ShadowOffsetX", AnimationTrackType::ShadowOffsetX);
	l.setGlobal("AnimationTrackType_ShadowOffsetY", AnimationTrackType::ShadowOffsetY);
	l.setGlobal("AnimationTrackType_ShadowScaleX", AnimationTrackType::ShadowScaleX);
	l.setGlobal("AnimationTrackType_ShadowScaleY", AnimationTrackType::ShadowScaleY);
	l.setGlobal("AnimationTrackType_ShadowUniformScale", AnimationTrackType::ShadowUniformScale);
	l.setGlobal("AnimationTrackType_ColorR", AnimationTrackType::ColorR);
	l.setGlobal("AnimationTrackType_ColorG", AnimationTrackType::ColorG);
	l.setGlobal("AnimationTrackType_ColorB", AnimationTrackType::ColorB);
	l.setGlobal("AnimationTrackType_ColorA", AnimationTrackType::ColorA);
	l.setGlobal("AnimationTrackType_ColorMode", AnimationTrackType::ColorMode);

	l.setGlobal("SoundChannel_Other", SoundChannel::Other);
	l.setGlobal("SoundChannel_Music", SoundChannel::Music);
	l.setGlobal("SoundChannel_Player", SoundChannel::Player);
	l.setGlobal("SoundChannel_Enemy", SoundChannel::Enemy);
	l.setGlobal("SoundChannel_Item", SoundChannel::Item);

	// load util by default
	auto path = Game::makeFullDataPath("scripts/util.lua");
	auto code = readTextFile(path);
	auto res = luaL_loadstring(getLuaState(), code.c_str());
	auto result = lua_pcall(getLuaState(), 0, LUA_MULTRET, 0);

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
