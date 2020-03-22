#include "unit_instance.h"
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
}

void UnitInstance::updateShadowToggle()
{
	shadowToggle = !shadowToggle;
}

void UnitInstance::copyFrom(UnitInstance* other)
{
	layerIndex = other->layerIndex;
	name = other->name;
	currentAnimationName = other->currentAnimationName;
	boundingBox = other->boundingBox;
	visible = other->visible;
	appeared = other->appeared;
	speed = other->speed;
	health = other->health;
	maxHealth = other->maxHealth;
	age = other->age;
	stageIndex = other->stageIndex;
	collide = other->collide;
	shadow = other->shadow;
	unit = other->unit;
	deleteMeNow = other->deleteMeNow;
	scriptClass = other->unit->script ? other->unit->script->createClassInstance(this) : nullptr;

	// map from other unit to new sprite instances
	std::map<SpriteInstance*, SpriteInstance*> spriteInstMap;

	// copy sprite instances
	for (auto& otherSprInst : other->spriteInstances)
	{
		auto sprInst = new SpriteInstance();

		sprInst->copyFrom(otherSprInst);
		spriteInstances.push_back(sprInst);
		spriteInstMap[otherSprInst] = sprInst;
	}

	root = spriteInstMap[other->root];

	// if no root specified, use first sprite instance as root
	if (!root && spriteInstances.size())
	{
		root = spriteInstances[0];
	}

	if (other->root)
	{
		root->position = other->root->position;
		root->scale = other->root->scale;
		root->rotation = other->root->rotation;
		root->verticalFlip = other->root->verticalFlip;
		root->horizontalFlip = other->root->horizontalFlip;
	}

	// copy sprite instance animations
	for (auto& spriteInstAnim : other->spriteInstanceAnimations)
	{
		auto& animName = spriteInstAnim.first;
		auto& animMap = spriteInstAnim.second;

		spriteInstanceAnimations[animName] = SpriteInstanceAnimationMap();
		auto& crtAnimMap = spriteInstanceAnimations[animName];

		for (auto& anim : animMap)
		{
			AnimationInstance* newAnimInst = new AnimationInstance();

			newAnimInst->copyFrom(anim.second);
			crtAnimMap[spriteInstMap[anim.first]] = newAnimInst;
		}
	}

	// copy weapon instances
	for (auto& wi : other->weapons)
	{
		WeaponInstance* wiNew = new WeaponInstance();

		wiNew->copyFrom(wi.second);
		// reparent to this
		wiNew->parentUnitInstance = this;
		// attach to new sprite instance
		wiNew->attachTo = spriteInstMap[wi.second->attachTo];
		weapons[wi.first] = wiNew;
	}

	setAnimation(currentAnimationName);
}

void UnitInstance::initializeFrom(UnitResource* res)
{
	unit = res;
	name = res->name;
	speed = res->speed;
	visible = res->visible;
	scriptClass = res->script ? res->script->createClassInstance(this) : nullptr;
	collide = res->collide;
	shadow = res->shadow;

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

	std::sort(spriteInstances.begin(), spriteInstances.end(), [](const SpriteInstance* a, const SpriteInstance* b) { return a->orderIndex < b->orderIndex; });

	if (res->rootName.size())
	{
		root = spriteInstMap[res->spriteInstances[res->rootName]];
	}

	// if no root specified, use first sprite instance as root
	if (!root && spriteInstances.size())
	{
		root = spriteInstances[0];
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
	shadow = json.get("shadow", visible).asBool();
	speed = json.get("speed", speed).asFloat();
	layerIndex = json.get("layerIndex", layerIndex).asInt();
	root->position.parse(json.get("position", "0 0").asString());
	stageIndex = 0;
}

void UnitInstance::update(Game* game)
{
	computeHealth();

	for (auto stage : unit->stages)
	{
		if (health <= stage->triggerOnHealth && stage != currentStage)
		{
			//TODO: maybe just use a currentStageIndex
			auto iter = std::find(triggeredStages.begin(), triggeredStages.end(), stage);

			if (iter != triggeredStages.end()) continue;

			triggeredStages.push_back(stage);

			if (scriptClass)
			{
				auto func = scriptClass->getFunction("onStageChange");

				if (func.isFunction()) func.call(scriptClass->classInstance, currentStage ? currentStage->name : "", stage->name);
			}

			currentStage = stage;
			break;
		}
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

	for (auto& wp : weapons)
	{
		wp.second->update(game);
	}

	for (auto sprInst : spriteInstances)
	{
		sprInst->update(game);
	}

	computeBoundingBox();

	if (appeared && unit->autoDeleteType == AutoDeleteType::EndOfScreen)
	{
		if (game->screenMode == ScreenMode::Vertical)
		{
			if (boundingBox.y > game->graphics->videoHeight)
				deleteMeNow = true;
		}
		else if (game->screenMode == ScreenMode::Horizontal)
		{
			if (boundingBox.right() < 0)
				deleteMeNow = true;
		}
	}

	if (unit->autoDeleteType == AutoDeleteType::OutOfScreen)
	if (boundingBox.x > game->graphics->videoWidth
		|| boundingBox.y > game->graphics->videoHeight
		|| boundingBox.right() < 0
		|| boundingBox.bottom() < 0)
	{
		deleteMeNow = true;
	}

	age += game->deltaTime;

	if (scriptClass)
	{
		auto func = scriptClass->getFunction("onUpdate");
		if (func.isFunction()) func.call(scriptClass->classInstance);
	}
}

void UnitInstance::computeHealth()
{
	health = 0;
	maxHealth = 0.0f;

	for (auto sprInst : spriteInstances)
	{
		if (!sprInst->visible) continue;

		health += sprInst->health;
		maxHealth += sprInst->maxHealth;
	}

	health = health / maxHealth * 100;
}

void UnitInstance::setAnimation(const std::string& animName)
{
	if (spriteInstanceAnimations.find(animName) != spriteInstanceAnimations.end())
	{
		spriteInstanceAnimationMap = &spriteInstanceAnimations[currentAnimationName];
	}
}

void UnitInstance::computeBoundingBox()
{
	if (root)
	{
		if (root->rotation != 0)
		{
			boundingBox.width = root->sprite->frameWidth * root->scale;
			boundingBox.height = root->sprite->frameHeight * root->scale;
			boundingBox.x = root->position.x - boundingBox.width / 2;
			boundingBox.y = root->position.y - boundingBox.height / 2;

			Vec2 v0(boundingBox.topLeft());
			Vec2 v1(boundingBox.topRight());
			Vec2 v2(boundingBox.bottomRight());
			Vec2 v3(boundingBox.bottomLeft());

			Vec2 center = boundingBox.center();
			auto angle = deg2rad(root->rotation);

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
			boundingBox.width = root->sprite->frameWidth * root->scale;
			boundingBox.height = root->sprite->frameHeight * root->scale;
			boundingBox.x = root->position.x - boundingBox.width / 2;
			boundingBox.y = root->position.y - boundingBox.height / 2;
		}

		boundingBox = Game::instance->worldToScreen(boundingBox, layerIndex);

		root->screenRect = boundingBox;

		root->screenRect.x = roundf(root->screenRect.x);
		root->screenRect.y = roundf(root->screenRect.y);
		root->screenRect.width = roundf(root->screenRect.width);
		root->screenRect.height = roundf(root->screenRect.height);
	}

	// compute bbox for the rest of the sprite instances
	for (auto sprInst : spriteInstances)
	{
		if (!sprInst->visible || sprInst == root) continue;

		Vec2 pos;

		// if relative to root sprite instance
		if (!sprInst->notRelativeToRoot)
		{
			pos = root->position;
		}

		f32 mirrorV = sprInst->verticalFlip ? -1 : 1;
		f32 mirrorH = sprInst->horizontalFlip ? -1 : 1;

		pos.x += sprInst->position.x * mirrorH;
		pos.y += sprInst->position.y * mirrorV;

		f32 renderW = sprInst->sprite->frameWidth * sprInst->scale * root->scale;
		f32 renderH = sprInst->sprite->frameHeight * sprInst->scale * root->scale;

		pos.x -= renderW / 2.0f;
		pos.y -= renderH / 2.0f;

		pos = Game::instance->worldToScreen(pos, layerIndex);

		Rect spriteRc =
		{
			roundf(pos.x),
			roundf(pos.y),
			roundf(renderW),
			roundf(renderH)
		};

		if (sprInst->rotation != 0)
		{
			Vec2 v0(spriteRc.topLeft());
			Vec2 v1(spriteRc.topRight());
			Vec2 v2(spriteRc.bottomRight());
			Vec2 v3(spriteRc.bottomLeft());

			Vec2 center = spriteRc.center();
			auto angle = deg2rad(sprInst->rotation);

			v0.rotateAround(center, angle);
			v1.rotateAround(center, angle);
			v2.rotateAround(center, angle);
			v3.rotateAround(center, angle);

			sprInst->screenRect = Rect(center.x, center.y, 0, 0);
			sprInst->screenRect.add(v0);
			sprInst->screenRect.add(v1);
			sprInst->screenRect.add(v2);
			sprInst->screenRect.add(v3);

			boundingBox.add(v0);
			boundingBox.add(v1);
			boundingBox.add(v2);
			boundingBox.add(v3);
		}
		else
		{
			boundingBox.add(spriteRc);
			sprInst->screenRect = spriteRc;
		}
	}

	boundingBox.x = roundf(boundingBox.x);
	boundingBox.y = roundf(boundingBox.y);
	boundingBox.width = roundf(boundingBox.width);
	boundingBox.height = roundf(boundingBox.height);

	if (!appeared)
	{
		if ((Game::instance->screenMode == ScreenMode::Vertical && boundingBox.bottom() > 0)
			|| (Game::instance->screenMode == ScreenMode::Horizontal && boundingBox.x < Game::instance->graphics->videoWidth))
		{
			appeared = true;

			if (scriptClass)
			{
				auto func = scriptClass->getFunction("onAppeared");

				if (func.isFunction())
				{
					func.call(scriptClass->classInstance);
				}
			}
		}
	}
}

void UnitInstance::render(Graphics* gfx)
{
	if (!visible)
		return;

	for (auto sprInst : spriteInstances)
	{
		if (!sprInst->visible) continue;

		Vec2 pos;

		if (!sprInst->notRelativeToRoot)
		{
			pos = sprInst != root ? root->position : Vec2();
		}

		f32 mirrorV = sprInst->verticalFlip ? -1 : 1;
		f32 mirrorH = sprInst->horizontalFlip ? -1 : 1;

		if (sprInst != root)
		{
			if (root->verticalFlip) mirrorV *= -1;
			if (root->horizontalFlip) mirrorH *= -1;
		}

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

		auto shadowRc = sprInst->screenRect;
		shadowRc += unit->shadowOffset;
		shadowRc.width *= unit->shadowScale;
		shadowRc.height *= unit->shadowScale;
		sprInst->uvRect = uvRc;
		sprInst->shadowRect = shadowRc;
	}

	// draw shadows first
	if (shadow && shadowToggle)
	for (auto sprInst : spriteInstances)
	{
		if (!sprInst->visible || !sprInst->shadow) continue;

		Game::instance->graphics->atlasTextureIndex = sprInst->sprite->image->atlasTexture->textureIndex;
		gfx->currentColor = 0;
		gfx->currentColorMode = (u32)ColorMode::Mul;

		if (sprInst->rotation > 0)
		{
			if (sprInst->sprite->image->rotated)
			{
				gfx->drawRotatedQuadWithTexCoordRotated90(sprInst->shadowRect, sprInst->uvRect, sprInst->rotation);
			}
			else
			{
				gfx->drawRotatedQuad(sprInst->shadowRect, sprInst->uvRect, sprInst->rotation);
			}
		}
		else
		{
			if (sprInst->sprite->image->rotated)
			{
				gfx->drawQuadWithTexCoordRotated90(sprInst->shadowRect, sprInst->uvRect);
			}
			else
			{
				gfx->drawQuad(sprInst->shadowRect, sprInst->uvRect);
			}
		}
	}

	// draw color sprites
	for (auto sprInst : spriteInstances)
	{
		if (!sprInst->visible) continue;

		gfx->currentColor = sprInst->color.getRgba();
		gfx->currentColorMode = (u32)sprInst->colorMode;

		if (sprInst->rotation > 0)
		{
			if (sprInst->sprite->image->rotated)
			{
				gfx->drawRotatedQuadWithTexCoordRotated90(sprInst->screenRect, sprInst->uvRect, sprInst->rotation);
			}
			else
			{
				gfx->drawRotatedQuad(sprInst->screenRect, sprInst->uvRect, sprInst->rotation);
			}
		}
		else
		{
			if (sprInst->sprite->image->rotated)
			{
				gfx->drawQuadWithTexCoordRotated90(sprInst->screenRect, sprInst->uvRect);
			}
			else
			{
				gfx->drawQuad(sprInst->screenRect, sprInst->uvRect);
			}
		}
	}

	for (auto weapon : weapons)
	{
		weapon.second->render();
	}
}

SpriteInstance* UnitInstance::findSpriteInstance(const std::string& sname)
{
	for (auto sprInst : spriteInstances)
	{
		if (sname == sprInst->name)
			return sprInst;
	}

	return nullptr;
}

bool UnitInstance::checkPixelCollision(struct UnitInstance* other, std::vector<SpriteInstanceCollision>& collisions)
{
	bool collided = false;
	Vec2 collisionCenter;

	for (auto sprInst1 : spriteInstances)
	{
		for (auto sprInst2 : other->spriteInstances)
		{
			if (sprInst1->checkPixelCollision(sprInst2, collisionCenter))
			{
				collisions.push_back({sprInst1, sprInst2, collisionCenter});
				collided = true;
			}
		}
	}

	return collided;
}

}
