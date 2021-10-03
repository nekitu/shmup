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
#include "resources/sound_resource.h"

namespace engine
{
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
	for (auto& spr : sprites) delete spr;

	for (auto& sprAnimPair : spriteAnimations)
	{
		for (auto& sprAnim : sprAnimPair.second)
		{
			delete sprAnim.second;
		}
	}

	for (auto& wpn : weapons) delete wpn.second;

	for (auto& ctrl : controllers)
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

void Unit::onAnimationEvent(struct Sprite* sprite, const std::string& eventName)
{
	if (sprite)
	{
		CALL_LUA_FUNC("onAnimationEvent", sprite, eventName);

		for (auto& ctrl : controllers)
		{
			CALL_LUA_FUNC2(ctrl.second, "onAnimationEvent", sprite, eventName);
		}
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
		if (unitResource->unitType == UnitType::Player)
			wiNew->fireSound.channel = SoundChannel::Player;
		if (unitResource->unitType == UnitType::Enemy)
			wiNew->fireSound.channel = SoundChannel::Enemy;
		if (unitResource->unitType == UnitType::Item)
			wiNew->fireSound.channel = SoundChannel::Item;
	}

	// copy sounds
	for (auto& snd : other->sounds)
	{
		Sound* newSnd = new Sound();

		newSnd->channel = snd.first;
		newSnd->soundResource = snd.second->soundResource;
		sounds[snd.first] = newSnd;
	}

	setAnimation(currentAnimationName);

	// controller script instances
	for (auto& ctrl : unitResource->controllers)
	{
		auto ctrlClassInst = ctrl.second.script->createClassInstance<Unit>(this);
		CALL_LUA_FUNC2(ctrlClassInst, "setup", &ctrl.second.parameters);
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
		weapon->groupIndex = weaponRes.second->groupIndex;
		weapon->attachTo = spriteMap[weaponRes.second->attachTo];
		weapon->active = weaponRes.second->active;
		weapon->params.ammo = weaponRes.second->ammo;
		weapon->autoFire = weaponRes.second->autoFire;
		weapon->params.position = weaponRes.second->localPosition;
		if (unitResource->unitType == UnitType::Player)
			weapon->fireSound.channel = SoundChannel::Player;
		if (unitResource->unitType == UnitType::Enemy)
			weapon->fireSound.channel = SoundChannel::Enemy;
		if (unitResource->unitType == UnitType::Item)
			weapon->fireSound.channel = SoundChannel::Item;
		weapons[weaponRes.first] = weapon;
	}

	for (auto& snd : res->sounds)
	{
		// search if the channel was created
		// we only create a sound object for each channel
		// then we can play sound on specific channels
		auto iter = sounds.find(snd.second.channel);

		if (iter == sounds.end())
		{
			Sound* newSnd = new Sound();
			newSnd->channel = snd.second.channel;
			sounds[snd.second.channel] = newSnd;
		}
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
	if (!visible)
		return;

	f32 deltaTime = isPlayer() ? game->realDeltaTime : game->deltaTime;

	deleteQueuedSprites();
	computeHealth();

	if (unitResource && appeared)
	{
		int index = 0;
		for (auto& stage : unitResource->stages)
		{
			if (health <= stage->triggerOnHealth && stage != currentStage)
			{
				//TODO: maybe just use a currentStageIndex
				auto iter = std::find(triggeredStages.begin(), triggeredStages.end(), stage);

				if (iter != triggeredStages.end()) continue;

				triggeredStages.push_back(stage);
				CALL_LUA_FUNC("onStageChange", currentStage ? currentStage->name : "", stage->name);
				currentStage = stage;
				stageIndex = index;
				break;
			}

			++index;
		}
	}

	if (spriteAnimationMap)
	{
		for (auto& iter : *spriteAnimationMap)
		{
			auto& spr = iter.first;
			auto& sprAnim = iter.second;

			if (sprAnim->active)
			{
				sprAnim->update(deltaTime);
				sprAnim->animateSprite(spr);
			}
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

	for (auto& spr : sprites)
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
			if (!game->cameraState.offscreenBoundary.contains(boundingBox))
			{
				deleteMeNow = true;
			}
		}

		age += deltaTime;
	}

	//TODO: call as fixed step/fps ?
	// maybe also have a onFixedUpdate like in Unity
	CALL_LUA_FUNC("onUpdate");
}

void Unit::computeHealth()
{
	health = 0;
	maxHealth = 0.0f;

	for (auto& spr : sprites)
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

		root->localRect = boundingBox;
		boundingBox = Game::instance->worldToScreen(boundingBox, layerIndex);
		root->rect = boundingBox;
		root->rect.x = roundf(root->rect.x);
		root->rect.y = roundf(root->rect.y);
		root->rect.width = roundf(root->rect.width);
		root->rect.height = roundf(root->rect.height);

		// compute bbox for the rest of the sprites
		for (auto& spr : sprites)
		{
			if (!spr->visible || spr == root) continue;

			Vec2 pos;

			// if relative to root sprite
			if (spr->relativeToRoot)
			{
				pos = root->position;
			}

			pos.x += spr->position.x;
			pos.y += spr->position.y;

			f32 renderW = spr->spriteResource->frameWidth * spr->scale.x * root->scale.x;
			f32 renderH = spr->spriteResource->frameHeight * spr->scale.y * root->scale.y;

			pos.x -= renderW / 2.0f;
			pos.y -= renderH / 2.0f;
			spr->localRect = Rect(roundf(pos.x), roundf(pos.y), roundf(renderW), roundf(renderH));
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
	if (!visible || !root)
		return;

	CALL_LUA_FUNC("onBeforeRender");

	// draw shadows first
	if (shadow && gfx->shadowToggle)
	for (auto& spr : sprites)
	{
		spr->renderShadow(gfx);
	}

	// draw color sprites
	for (auto& spr : sprites)
	{
		spr->render(gfx);
	}

	for (auto& weapon : weapons)
	{
		weapon.second->render(gfx);
	}

	CALL_LUA_FUNC("onAfterRender");
}

Sprite* Unit::findSprite(const std::string& sname)
{
	for (auto& spr : sprites)
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

	for (auto& sprInst1 : sprites)
	{
		for (auto& sprInst2 : other->sprites)
		{
			if (sprInst1->health > 0 && sprInst2->health > 0 && sprInst1->checkPixelCollision(sprInst2, collisionCenter))
			{
				collisions.push_back({sprInst1, sprInst2, collisionCenter});
				collided = true;
			}
		}
	}

	return collided;
}

void Unit::deleteSprite(struct Sprite* spr)
{
	if (spr == root)
	{
		LOG_DEBUG("Cannot delete root sprite of unit {0}", name);
		return;
	}

	auto iter = std::find(sprites.begin(), sprites.end(), spr);

	if (iter != sprites.end())
	{
		removeSpriteAnimations(spr);
		delete spr;
		sprites.erase(iter);
	}
}

void Unit::deleteSprite(const std::string& sname)
{
	auto spr = findSprite(sname);

	if (spr)
	{
		deleteSprite(spr);
	}
}

void Unit::queueDeleteSprite(Sprite* what)
{
	deleteSpritesQueue.insert(what);
}

void Unit::deleteQueuedSprites()
{
	for (auto& spr : deleteSpritesQueue)
	{
		deleteSprite(spr);
	}

	deleteSpritesQueue.clear();
}

void Unit::replaceSprite(const std::string& what, const std::string& with)
{
	auto sprWith = findSprite(with);
	auto sprWhat = findSprite(what);

	queueDeleteSprite(sprWhat);
	if (sprWith) sprWith->visible = true;
}

void Unit::removeSpriteAnimations(struct Sprite* spr)
{
	for (auto& sprAnimMapPair : spriteAnimations)
	{
		for (auto& sprAnimPair : sprAnimMapPair.second)
		{
			if (sprAnimPair.first == spr)
			{
				delete sprAnimPair.second;
				sprAnimMapPair.second.erase(spr);
				break;
			}
		}
	}
}

void Unit::hideAllSprites()
{
	for (auto& spr : sprites)
	{
		spr->visible = false;
	}
}

void Unit::disableAllWeapons()
{
	for (auto& wpn : weapons)
	{
		wpn.second->active = false;
	}
}

void Unit::playSound(const std::string& sndName)
{
	// find in resource
	auto iter = unitResource->sounds.find(sndName);

	if (iter == unitResource->sounds.end())
	{
		LOG_WARN("Cannot find sound to play: {0}", sndName);
		return;
	}

	sounds[iter->second.channel]->soundResource = iter->second.soundResource;
	sounds[iter->second.channel]->play();
	LOG_INFO("Playing {0} on {1}", sounds[iter->second.channel]->soundResource->path, iter->second.channel);
}

bool Unit::isSoundPlaying(const std::string& name)
{
	auto iter = unitResource->sounds.find(name);

	if (iter == unitResource->sounds.end())
		return false;

	return sounds[iter->second.channel]->isPlaying();
}

bool Unit::isPlayer() const
{
	if (!unitResource) return false;
	return unitResource->unitType == UnitType::Player
		|| unitResource->unitType == UnitType::PlayerProjectile;
}

}
