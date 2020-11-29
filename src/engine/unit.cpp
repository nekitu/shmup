#include "unit.h"
#include "utils.h"
#include "sprite.h"
#include "resources/sprite_resource.h"
#include "graphics.h"
#include "game.h"
#include "image_atlas.h"
#include "animation.h"
#include "weapon.h"
#include <algorithm>
#include "resources/script_resource.h"
#include "resource_loader.h"
#include "resources/weapon_resource.h"

namespace engine
{
bool Unit::shadowToggle = true;
u64 Unit::lastId = 1;

Unit::Unit()
{
	id = lastId++;
}

Unit::~Unit()
{
	reset();
}

void Unit::reset()
{
	for (auto spr : sprites) delete spr;

	for (auto sprAnimPair : spriteAnimations)
	{
		for (auto sprAnim : sprAnimPair.second)
		{
			delete sprAnim.second;
		}
	}

	for (auto wpn : weapons) delete wpn.second;

	for (auto ctrl : controllers)
	{
		delete ctrl.second;
	}

	controllers.clear();
	spriteAnimations.clear();
	sprites.clear();
	weapons.clear();
	triggeredStages.clear();
	root = nullptr;
	currentStage = nullptr;
	unitResource = nullptr;
	delete scriptClass;
	scriptClass = nullptr;
	deleteMeNow = false;
	age = 0;
	spriteAnimationMap = nullptr;
	appeared = false;
	stageIndex = 0;
}

void Unit::updateShadowToggle()
{
	shadowToggle = !shadowToggle;
}

void Unit::onAnimationEvent(struct Sprite* sprite, const std::string& eventName)
{
	if (sprite)
	{
		CALL_LUA_FUNC("onAnimationEvent", sprite, eventName);
	}
}

void Unit::copyFrom(Unit* other)
{
	reset();
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
	unitResource = other->unitResource;
	deleteMeNow = other->deleteMeNow;
	scriptClass = other->unitResource->script ? other->unitResource->script->createClassInstance(this) : nullptr;

	// map from other unit to new sprites
	std::map<Sprite*, Sprite*> spriteMap;

	// copy sprites
	for (auto& otherSpr : other->sprites)
	{
		auto spr = new Sprite();

		spr->copyFrom(otherSpr);
		spr->unit = this;
		sprites.push_back(spr);
		spriteMap[otherSpr] = spr;
	}

	root = spriteMap[other->root];

	// if no root specified, use first sprite as root
	if (!root && sprites.size())
	{
		root = sprites[0];
	}

	if (other->root)
	{
		root->position = other->root->position;
		root->scale = other->root->scale;
		root->rotation = other->root->rotation;
		root->verticalFlip = other->root->verticalFlip;
		root->horizontalFlip = other->root->horizontalFlip;
	}

	// copy sprite animations
	for (auto& spriteAnim : other->spriteAnimations)
	{
		auto& animName = spriteAnim.first;
		auto& animMap = spriteAnim.second;

		spriteAnimations[animName] = SpriteAnimationMap();
		auto& crtAnimMap = spriteAnimations[animName];

		for (auto& anim : animMap)
		{
			Animation* newAnim = new Animation();

			newAnim->copyFrom(anim.second);
			crtAnimMap[spriteMap[anim.first]] = newAnim;
		}
	}

	// copy weapons
	for (auto& wi : other->weapons)
	{
		Weapon* wiNew = new Weapon();

		wiNew->copyFrom(wi.second);
		// reparent to this
		wiNew->parentUnit = this;
		// attach to new sprite
		wiNew->attachTo = spriteMap[wi.second->attachTo];
		weapons[wi.first] = wiNew;
	}

	setAnimation(currentAnimationName);

	// controller script instances
	for (auto& ctrl : unitResource->controllers)
	{
		auto ctrlClassInst = ctrl.second.script->createClassInstance<Unit>(this);
		CALL_LUA_FUNC2(ctrlClassInst, "setup", &ctrl.second);
		controllers[ctrl.first] = ctrlClassInst;
	}
}

void Unit::initializeFrom(UnitResource* res)
{
	reset();
	unitResource = res;
	name = res->name;
	speed = res->speed;
	visible = res->visible;
	scriptClass = res->script ? res->script->createClassInstance(this) : nullptr;
	collide = res->collide;
	shadow = res->shadow;

	// map from other unit to new sprites
	std::map<SpriteInstanceResource*, Sprite*> spriteMap;

	// create the sprites for this unit
	for (auto& iter : res->sprites)
	{
		Sprite* spr = new Sprite();

		spr->initializeFrom(iter.second);
		spr->unit = this;
		sprites.push_back(spr);
		spriteMap[iter.second] = spr;
	}

	std::sort(sprites.begin(), sprites.end(), [](const Sprite* a, const Sprite* b) { return a->orderIndex < b->orderIndex; });

	if (res->rootName.size())
	{
		root = spriteMap[res->sprites[res->rootName]];
	}

	// if no root specified, use first sprite as root
	if (!root && sprites.size())
	{
		root = sprites[0];
	}

	// copy sprite animations
	for (auto& spriteIter : res->sprites)
	{
		auto& sprName = spriteIter.first;
		auto& sprRes = spriteIter.second;

		if (sprRes)
		{
			for (auto& anim : sprRes->animations)
			{
				Animation* newAnim = new Animation();

				newAnim->initializeFrom(anim.second);
				newAnim->unit = this;
				spriteAnimations[anim.first][spriteMap[sprRes]] = newAnim;
			}
		}
	}

	// create weapons
	for (auto& weaponRes : res->weapons)
	{
		Weapon* weapon = new Weapon();

		weapon->initializeFrom(weaponRes.second->weaponResource);
		weapon->parentUnit = this;
		weapon->attachTo = spriteMap[weaponRes.second->attachTo];
		weapon->active = weaponRes.second->active;
		weapon->params.position = weaponRes.second->localPosition;
		weapons[weaponRes.first] = weapon;
	}

	// controller script instances
	// has to be last since they will try to references sprites, weapons, etc.
	for (auto& ctrl : res->controllers)
	{
		auto ctrlClassInst = ctrl.second.script->createClassInstance<Unit>(this);
		CALL_LUA_FUNC2(ctrlClassInst, "setup", &ctrl.second);
		controllers[ctrl.first] = ctrlClassInst;
	}
}

void Unit::load(TilemapObject& object)
{
	name = object.name;
	currentAnimationName = object.properties["animationName"];
	visible = object.visible;
	shadow = object.properties["shadow"] == "true";
	speed = atof(object.properties["speed"].c_str());
	if (!root) root = new Sprite();
	if (root) root->position = object.position;
	stageIndex = 0;
}

void Unit::update(Game* game)
{
	computeHealth();

	if (unitResource)
	{
		for (auto stage : unitResource->stages)
		{
			if (health <= stage->triggerOnHealth && stage != currentStage)
			{
				//TODO: maybe just use a currentStageIndex
				auto iter = std::find(triggeredStages.begin(), triggeredStages.end(), stage);

				if (iter != triggeredStages.end()) continue;

				triggeredStages.push_back(stage);
				CALL_LUA_FUNC("onStageChange", currentStage ? currentStage->name : "", stage->name);
				currentStage = stage;
				break;
			}
		}
	}

	if (spriteAnimationMap)
	{
		for (auto iter : *spriteAnimationMap)
		{
			auto spr = iter.first;
			auto sprAnim = iter.second;

			sprAnim->update(game->deltaTime);
			sprAnim->animateSprite(spr);
		}
	}

	for (auto& ctrl : controllers)
	{
		CALL_LUA_FUNC2(ctrl.second, "onUpdate");
	}

	for (auto& wp : weapons)
	{
		wp.second->update(game);
	}

	for (auto spr : sprites)
	{
		spr->update(game);
	}

	computeBoundingBox();

	if (appeared && unitResource)
	{
		if (unitResource->autoDeleteType == AutoDeleteType::EndOfScreen)
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
		else if (unitResource->autoDeleteType == AutoDeleteType::OutOfScreen)
		{
			if (boundingBox.x > game->graphics->videoWidth
				|| boundingBox.y > game->graphics->videoHeight
				|| boundingBox.right() < 0
				|| boundingBox.bottom() < 0)
			{
				deleteMeNow = true;
			}
		}
		else if (unitResource->autoDeleteType == AutoDeleteType::OffscreenBoundary)
		{
			if (!game->offscreenBoundary.contains(boundingBox))
			{
				deleteMeNow = true;
			}
		}

		age += game->deltaTime;
	}

	//TODO: call as fixed step/fps ?
	// maybe also have a onFixedUpdate like in Unity
	CALL_LUA_FUNC("onUpdate");
}

void Unit::computeHealth()
{
	health = 0;
	maxHealth = 0.0f;

	for (auto spr : sprites)
	{
		if (!spr->visible) continue;

		health += spr->health;
		maxHealth += spr->maxHealth;
	}

	health = health / maxHealth * 100;
}

void Unit::setAnimation(const std::string& animName)
{
	if (spriteAnimations.find(animName) != spriteAnimations.end())
	{
		currentAnimationName = animName;
		spriteAnimationMap = &spriteAnimations[currentAnimationName];

		for (auto& animIter : *spriteAnimationMap)
		{
			animIter.second->rewind();
		}
	}
	else
	{
		currentAnimationName = "";
		spriteAnimationMap = nullptr;
	}
}

void Unit::computeBoundingBox()
{
	if (root)
	{
		if (root->spriteResource)
		{
			if (root->rotation != 0)
			{
				boundingBox.width = root->spriteResource->frameWidth * root->scale.x;
				boundingBox.height = root->spriteResource->frameHeight * root->scale.y;
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
				boundingBox.width = root->spriteResource->frameWidth * root->scale.x;
				boundingBox.height = root->spriteResource->frameHeight * root->scale.y;
				boundingBox.x = root->position.x - boundingBox.width / 2;
				boundingBox.y = root->position.y - boundingBox.height / 2;
			}
		}
		else
		{
			boundingBox.setPosition(root->position);
			boundingBox.setSize(root->scale);
		}

		boundingBox = Game::instance->worldToScreen(boundingBox, layerIndex);
		root->rect = boundingBox;
		root->rect.x = roundf(root->rect.x);
		root->rect.y = roundf(root->rect.y);
		root->rect.width = roundf(root->rect.width);
		root->rect.height = roundf(root->rect.height);
	}

	// compute bbox for the rest of the sprites
	for (auto spr : sprites)
	{
		if (!spr->visible || spr == root) continue;

		Vec2 pos;

		// if relative to root sprite
		if (spr->relativeToRoot)
		{
			pos = root->position;
		}

		f32 mirrorV = spr->verticalFlip ? -1 : 1;
		f32 mirrorH = spr->horizontalFlip ? -1 : 1;

		pos.x += spr->position.x * mirrorH;
		pos.y += spr->position.y * mirrorV;

		f32 renderW = spr->spriteResource->frameWidth * spr->scale.x * root->scale.x;
		f32 renderH = spr->spriteResource->frameHeight * spr->scale.y * root->scale.y;

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

		if (spr->rotation != 0)
		{
			Vec2 v0(spriteRc.topLeft());
			Vec2 v1(spriteRc.topRight());
			Vec2 v2(spriteRc.bottomRight());
			Vec2 v3(spriteRc.bottomLeft());

			Vec2 center = spriteRc.center();
			auto angle = deg2rad(spr->rotation);

			v0.rotateAround(center, angle);
			v1.rotateAround(center, angle);
			v2.rotateAround(center, angle);
			v3.rotateAround(center, angle);

			spr->rect = Rect(center.x, center.y, 0, 0);
			spr->rect.add(v0);
			spr->rect.add(v1);
			spr->rect.add(v2);
			spr->rect.add(v3);

			boundingBox.add(v0);
			boundingBox.add(v1);
			boundingBox.add(v2);
			boundingBox.add(v3);
		}
		else
		{
			boundingBox.add(spriteRc);
			spr->rect = spriteRc;
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
			CALL_LUA_FUNC("onAppeared");
		}
	}
}

void Unit::render(Graphics* gfx)
{
	if (!visible)
		return;

	for (auto spr : sprites)
	{
		if (!spr->visible) continue;

		Vec2 pos;

		if (spr->relativeToRoot)
		{
			pos = spr != root ? root->position : Vec2();
		}

		f32 mirrorV = spr->verticalFlip ? -1 : 1;
		f32 mirrorH = spr->horizontalFlip ? -1 : 1;

		if (spr != root)
		{
			if (root->verticalFlip) mirrorV *= -1;
			if (root->horizontalFlip) mirrorH *= -1;
		}

		Rect uvRc = spr->spriteResource->getFrameUvRect(spr->animationFrame);

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

		auto shadowRc = spr->rect;
		shadowRc += unitResource->shadowOffset;
		shadowRc.width *= unitResource->shadowScale;
		shadowRc.height *= unitResource->shadowScale;
		spr->uvRect = uvRc;
		spr->shadowRect = shadowRc;
	}

	// draw shadows first
	if (shadow && shadowToggle)
	for (auto spr : sprites)
	{
		if (!spr->visible || !spr->shadow) continue;

		Game::instance->graphics->atlasTextureIndex = spr->spriteResource->image->atlasTexture->textureIndex;
		gfx->color = 0x00FFFFFF;
		gfx->colorMode = (u32)ColorMode::Sub;

		if (spr->rotation > 0)
		{
			if (spr->spriteResource->image->rotated)
			{
				gfx->drawRotatedQuadWithTexCoordRotated90(spr->shadowRect, spr->uvRect, spr->rotation);
			}
			else
			{
				gfx->drawRotatedQuad(spr->shadowRect, spr->uvRect, spr->rotation);
			}
		}
		else
		{
			if (spr->spriteResource->image->rotated)
			{
				gfx->drawQuadWithTexCoordRotated90(spr->shadowRect, spr->uvRect);
			}
			else
			{
				gfx->drawQuad(spr->shadowRect, spr->uvRect);
			}
		}
	}

	// draw color sprites
	for (auto spr : sprites)
	{
		if (!spr->visible) continue;

		gfx->color = spr->color.getRgba();
		gfx->colorMode = (u32)spr->colorMode;

		if (spr->rotation > 0)
		{
			if (spr->spriteResource->image->rotated)
			{
				gfx->drawRotatedQuadWithTexCoordRotated90(spr->rect, spr->uvRect, spr->rotation);
			}
			else
			{
				gfx->drawRotatedQuad(spr->rect, spr->uvRect, spr->rotation);
			}
		}
		else
		{
			if (spr->spriteResource->image->rotated)
			{
				gfx->drawQuadWithTexCoordRotated90(spr->rect, spr->uvRect);
			}
			else
			{
				gfx->drawQuad(spr->rect, spr->uvRect);
			}
		}
	}

	for (auto weapon : weapons)
	{
		weapon.second->render();
	}
}

Sprite* Unit::findSprite(const std::string& sname)
{
	for (auto spr : sprites)
	{
		if (sname == spr->name)
			return spr;
	}

	return nullptr;
}

bool Unit::checkPixelCollision(struct Unit* other, std::vector<SpriteCollision>& collisions)
{
	bool collided = false;
	Vec2 collisionCenter;

	for (auto sprInst1 : sprites)
	{
		for (auto sprInst2 : other->sprites)
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
