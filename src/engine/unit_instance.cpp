#include "unit_instance.h"
#include "unit_controller.h"
#include "utils.h"
#include "sprite_instance.h"
#include "resources/sprite_resource.h"
#include "graphics.h"
#include "game.h"
#include "image_atlas.h"
#include "animation_instance.h"
#include "weapon_instance.h"
#include <algorithm>
#include "resources/script_resource.h"
#include "resource_loader.h"
#include "resources/weapon_resource.h"

namespace engine
{
bool UnitInstance::shadowToggle = true;
u64 UnitInstance::lastId = 1;

UnitInstance::UnitInstance()
{
	id = lastId++;
	//printf("CRT UID %lld\n", id);
}

void UnitInstance::updateShadowToggle()
{
	shadowToggle = !shadowToggle;
}

//void UnitInstance::copyFrom(UnitInstance* other)
//{
//	boundingBox = other->boundingBox;
//	currentAnimationName = other->currentAnimationName;
//	hasShadows = other->hasShadows;
//	name = other->name;
//	shadowOffset = other->shadowOffset;
//	shadowScale = other->shadowScale;
//	speed = other->speed;
//	type = other->type;
//	unit = other->unit;
//	visible = other->visible;
//	deleteMeNow = other->deleteMeNow;
//	deleteOnOutOfScreen = other->deleteOnOutOfScreen;
//
//	if (other->controller)
//	{
//		controller = other->controller->createNew();
//		controller->unitInstance = this;
//	}
//
//	script = other->script;
//
//	// map from other unit to new sprite instances
//	std::map<SpriteInstance*, SpriteInstance*> spriteInstMap;
//
//	// copy sprite instances
//	for (auto& otherSprInst : other->spriteInstances)
//	{
//		auto sprInst = new SpriteInstance();
//
//		sprInst->copyFrom(otherSprInst);
//		spriteInstances.push_back(sprInst);
//		spriteInstMap[otherSprInst] = sprInst;
//	}
//
//	rootSpriteInstance = spriteInstMap[other->rootSpriteInstance];
//
//	// if no root specified, use first sprite instance as root
//	if (!rootSpriteInstance && spriteInstances.size())
//	{
//		rootSpriteInstance = spriteInstances[0];
//	}
//
//	if (other->rootSpriteInstance)
//		rootSpriteInstance->transform = other->rootSpriteInstance->transform;
//
//	// copy sprite instance animations
//	for (auto& spriteInstAnim : other->spriteInstanceAnimations)
//	{
//		auto& animName = spriteInstAnim.first;
//		auto& animMap = spriteInstAnim.second;
//
//		spriteInstanceAnimations[animName] = SpriteInstanceAnimationMap();
//		auto& crtAnimMap = spriteInstanceAnimations[animName];
//
//		for (auto& anim : animMap)
//		{
//			AnimationInstance* newAnimInst = new AnimationInstance();
//
//			newAnimInst->copyFrom(anim.second);
//			crtAnimMap[spriteInstMap[anim.first]] = newAnimInst;
//		}
//	}
//
//	// copy weapon instances
//	for (auto& wi : other->weapons)
//	{
//		WeaponInstance* wiNew = new WeaponInstance();
//
//		wiNew->copyFrom(wi);
//		wiNew->parentUnitInstance = this;
//		wiNew->attachTo = spriteInstMap[wi->attachTo];
//		weapons.push_back(wiNew);
//	}
//
//	setAnimation(currentAnimationName);
//}

void UnitInstance::initializeFrom(UnitResource* res)
{
	unit = res;
	name = res->name;
	speed = res->speed;
	visible = res->visible;
	script = res->script;
	collide = res->collide;

	// map from other unit to new sprite instances
	std::map<SpriteInstanceResource*, SpriteInstance*> spriteInstMap;

	// create the sprite instances for this unit instance
	for (auto& iter : res->spriteInstances)
	{
		SpriteInstance* sprInst = new SpriteInstance();

		sprInst->initializeFrom(iter.second);
		spriteInstances.push_back(sprInst);
		spriteInstMap[iter.second] = sprInst;
	}

	std::sort(spriteInstances.begin(), spriteInstances.end(), [](const SpriteInstance* a, const SpriteInstance* b) { return a->orderIndex > b->orderIndex; });

	if (res->rootSpriteInstanceName.size())
	{
		rootSpriteInstance = spriteInstMap[res->spriteInstances[res->rootSpriteInstanceName]];
	}

	// if no root specified, use first sprite instance as root
	if (!rootSpriteInstance && spriteInstances.size())
	{
		rootSpriteInstance = spriteInstances[0];
	}

	// copy sprite instance animations
	for (auto& spriteInstAnim : res->spriteInstances)
	{
		auto& animName = spriteInstAnim.first;
		auto& sprInstRes = spriteInstAnim.second;

		spriteInstanceAnimations[animName] = SpriteInstanceAnimationMap();
		auto& crtAnimMap = spriteInstanceAnimations[animName];

		for (auto& anim : sprInstRes->animations)
		{
			AnimationInstance* newAnimInst = new AnimationInstance();

			newAnimInst->initializeFrom(anim.second);
			crtAnimMap[spriteInstMap[sprInstRes]] = newAnimInst;
		}
	}

	// create weapon instances
	for (auto& weaponInstRes : res->weapons)
	{
		WeaponInstance* weaponInst = new WeaponInstance();
		weaponInst->initializeFrom(weaponInstRes.second->weapon);
		weaponInst->parentUnitInstance = this;
		weaponInst->attachTo = spriteInstMap[weaponInstRes.second->attachTo];
		weaponInst->active = weaponInstRes.second->active;
		weaponInst->params.position = weaponInstRes.second->localPosition;
		weapons[weaponInstRes.first] = weaponInst;
	}

	controller = UnitController::create(res->controllerName, this);
}

void UnitInstance::load(ResourceLoader* loader, const Json::Value& json)
{
	name = json.get("name", name).asCString();
	auto unitFilename = json["unit"].asString();

	if (unitFilename == "")
	{
		printf("No unit filename specified for unit instance (%s)\n", name.c_str());
		return;
	}

	auto unit = loader->loadUnit(unitFilename);
	initializeFrom(unit);
	name = json.get("name", name).asCString();
	currentAnimationName = json.get("animationName", "").asString();
	boundingBox.parse(json.get("boundingBox", "0 0 0 0").asString());
	visible = json.get("visible", visible).asBool();
	speed = json.get("speed", speed).asFloat();
	health = json.get("health", health).asFloat();
	rootSpriteInstance->transform.position.parse(json.get("position", "0 0").asString());
	stageIndex = 0;
}

void UnitInstance::update(Game* game)
{
	if (controller)
	{
		controller->update(game);
	}

	if (spriteInstanceAnimationMap)
	{
		for (auto iter : *spriteInstanceAnimationMap)
		{
			auto sprInst = iter.first;
			auto sprAnimInst = iter.second;

			sprAnimInst->update(game->deltaTime);
			sprAnimInst->animateSpriteInstance(sprInst);
		}
	}

	for (auto sprInst : spriteInstances)
	{
		sprInst->update(game);
	}

	computeBoundingBox();

	if (unit->deleteOnOutOfScreen)
	{
		if (boundingBox.x > game->graphics->videoWidth
			|| boundingBox.y > game->graphics->videoHeight
			|| boundingBox.right() < 0
			|| boundingBox.top() < 0)
		{
			deleteMeNow = true;
		}
	}

	age += game->deltaTime;

	if (script)
	{
		auto func = script->getFunction("onUpdate");
		if (func.isFunction()) func.call(this);
	}
}

void UnitInstance::computeBoundingBox()
{
	if (rootSpriteInstance)
	{
		if (rootSpriteInstance->transform.rotation != 0)
		{
			boundingBox.width = rootSpriteInstance->sprite->frameWidth * rootSpriteInstance->transform.scale;
			boundingBox.height = rootSpriteInstance->sprite->frameHeight * rootSpriteInstance->transform.scale;
			boundingBox.x = rootSpriteInstance->transform.position.x - boundingBox.width / 2;
			boundingBox.y = rootSpriteInstance->transform.position.y - boundingBox.height / 2;

			Vec2 v0(boundingBox.topLeft());
			Vec2 v1(boundingBox.topRight());
			Vec2 v2(boundingBox.bottomRight());
			Vec2 v3(boundingBox.bottomLeft());

			Vec2 center = boundingBox.center();
			auto angle = deg2rad(rootSpriteInstance->transform.rotation);

			v0.rotateAround(center, angle);
			v1.rotateAround(center, angle);
			v2.rotateAround(center, angle);
			v3.rotateAround(center, angle);

			boundingBox.set(center.x, center.y, 0, 0);
			boundingBox.add(v0);
			boundingBox.add(v1);
			boundingBox.add(v2);
			boundingBox.add(v3);
		}
		else
		{
			boundingBox.width = rootSpriteInstance->sprite->frameWidth * rootSpriteInstance->transform.scale;
			boundingBox.height = rootSpriteInstance->sprite->frameHeight * rootSpriteInstance->transform.scale;
			boundingBox.x = rootSpriteInstance->transform.position.x - boundingBox.width / 2;
			boundingBox.y = rootSpriteInstance->transform.position.y - boundingBox.height / 2;
		}

		rootSpriteInstance->rect = boundingBox;
	}
	
	for (auto sprInst : spriteInstances)
	{
		if (!sprInst->visible || sprInst == rootSpriteInstance) continue;

		auto pos = rootSpriteInstance->transform.position;
		f32 mirrorV = sprInst->transform.verticalFlip ? -1 : 1;
		f32 mirrorH = sprInst->transform.horizontalFlip ? -1 : 1;

		pos.x += sprInst->transform.position.x * mirrorH;
		pos.y += sprInst->transform.position.y * mirrorV;

		f32 renderW = sprInst->sprite->frameWidth * sprInst->transform.scale * rootSpriteInstance->transform.scale;
		f32 renderH = sprInst->sprite->frameHeight * sprInst->transform.scale * rootSpriteInstance->transform.scale;

		pos.x -= renderW / 2.0f;
		pos.y -= renderH / 2.0f;

		Rect spriteRc =
		{
			round(pos.x),
			round(pos.y),
			renderW,
			renderH
		};

		if (sprInst->transform.rotation != 0)
		{
			Vec2 v0(spriteRc.topLeft());
			Vec2 v1(spriteRc.topRight());
			Vec2 v2(spriteRc.bottomRight());
			Vec2 v3(spriteRc.bottomLeft());

			Vec2 center = spriteRc.center();
			auto angle = deg2rad(sprInst->transform.rotation);

			v0.rotateAround(center, angle);
			v1.rotateAround(center, angle);
			v2.rotateAround(center, angle);
			v3.rotateAround(center, angle);

			sprInst->rect = Rect(center.x, center.y, 0, 0);
			sprInst->rect.add(v0);
			sprInst->rect.add(v1);
			sprInst->rect.add(v2);
			sprInst->rect.add(v3);

			boundingBox.add(v0);
			boundingBox.add(v1);
			boundingBox.add(v2);
			boundingBox.add(v3);
		}
		else
		{
			boundingBox.add(spriteRc);
			sprInst->rect = spriteRc;
		}
	}
}

void UnitInstance::computeHealth()
{
	health = 0.0f;

	for (auto sprInst : spriteInstances)
	{
		health += sprInst->health;
	}

	health /= spriteInstances.size();
}

void UnitInstance::setAnimation(const std::string& animName)
{
	if (spriteInstanceAnimations.find(animName) != spriteInstanceAnimations.end())
	{
		spriteInstanceAnimationMap = &spriteInstanceAnimations[currentAnimationName];
	}
}

void UnitInstance::render(Graphics* gfx)
{
	if (!visible)
		return;

	for (auto sprInst : spriteInstances)
	{
		if (!sprInst->visible) continue;

		auto pos = sprInst != rootSpriteInstance ? rootSpriteInstance->transform.position : Vec2();

		f32 mirrorV = sprInst->transform.verticalFlip ? -1 : 1;
		f32 mirrorH = sprInst->transform.horizontalFlip ? -1 : 1;

		if (sprInst != rootSpriteInstance)
		{
			if (rootSpriteInstance->transform.verticalFlip) mirrorV *= -1;
			if (rootSpriteInstance->transform.horizontalFlip) mirrorH *= -1;
		}

		pos.x += sprInst->transform.position.x * mirrorH;
		pos.y += sprInst->transform.position.y * mirrorV;

		f32 renderW = sprInst->sprite->frameWidth * sprInst->transform.scale * ((sprInst != rootSpriteInstance) ? rootSpriteInstance->transform.scale : 1.0f);
		f32 renderH = sprInst->sprite->frameHeight * sprInst->transform.scale * ((sprInst != rootSpriteInstance) ? rootSpriteInstance->transform.scale : 1.0f);

		pos.x -= renderW / 2.0f;
		pos.y -= renderH / 2.0f;

		Rect spriteRc = 
		{
				round(pos.x),
				round(pos.y),
				renderW,
				renderH
		};

		Rect uvRc = sprInst->sprite->getFrameUvRect(sprInst->animationFrame);

		if (mirrorV < 0)
		{
			uvRc.y = uvRc.bottom();
			uvRc.height *= -1.0f;
		}

		if (mirrorH < 0)
		{
			uvRc.x = uvRc.right();
			uvRc.width *= -1.0f;
		}

		if (unit->hasShadows && shadowToggle)
		{
			auto shadowRc = spriteRc;
			shadowRc += unit->shadowOffset;
			shadowRc.width *= unit->shadowScale;
			shadowRc.height *= unit->shadowScale;

			gfx->currentColor = 0;
			gfx->currentColorMode = (u32)ColorMode::Mul;
			//TODO: handle shadow rotation
			if (sprInst->sprite->image->rotated)
			{
				gfx->drawQuadWithTexCoordRotated90(shadowRc, uvRc);
			}
			else
			{
				gfx->drawQuad(shadowRc, uvRc);
			}
		}

		gfx->currentColor = sprInst->color.getRgba();
		gfx->currentColorMode = (u32)sprInst->colorMode;

		if (sprInst->transform.rotation > 0)
		{
			if (sprInst->sprite->image->rotated)
			{
				gfx->drawRotatedQuadWithTexCoordRotated90(spriteRc, uvRc, sprInst->transform.rotation);
			}
			else
			{
				gfx->drawRotatedQuad(spriteRc, uvRc, sprInst->transform.rotation);
			}
		}
		else
		{
			if (sprInst->sprite->image->rotated)
			{
				gfx->drawQuadWithTexCoordRotated90(spriteRc, uvRc);
			}
			else
			{
				gfx->drawQuad(spriteRc, uvRc);
			}
		}
	}
}

}
