#include "particle_system.h"
#include "game.h"
#include "sprite.h"
#include "graphics.h"

namespace engine
{
ParticleSystem::ParticleSystem()
{
	root = new Sprite();
	//forces.push_back(new ParticleGravity);
}

ParticleSystem::~ParticleSystem()
{
	delete root;
}

void ParticleSystem::copyFrom(Unit* other)
{
}

void ParticleSystem::initializeFrom(UnitResource* res)
{
}

void ParticleSystem::update(struct Game* game)
{
	Unit::update(game);

	f32 slice = 1.0f / (f32)params.particlesPerSecond;

	if (spawnTimer >= slice)
	{
		u32 particlesToSpawn = spawnTimer / slice;
		
		//LOG_INFO("particles {} slice {} timer {} dt {}", particlesToSpawn, slice, spawnTimer, game->deltaTime);
		spawnTimer = 0;
		for (auto i = 0; i < particlesToSpawn; ++i)
		{
			auto pos = getNewSpawnPosition();
			Particle* particle = new Particle();

			particle->acceleration = randomFloat(params.acceleration.min, params.acceleration.max);
			particle->maxAge = randomFloat(params.age.min, params.age.max);
			particle->size.x = randomFloat(params.size.min.x, params.size.max.x);
			particle->size.y = randomFloat(params.size.min.y, params.size.max.y);
			particle->speed = randomFloat(params.speed.min, params.speed.max);
			particle->position = pos;
			particle->velocity = getNewSpawnVelocity(pos);
			particles.push_back(particle);
		}
	}
	else
	{
		spawnTimer += game->deltaTime;
	}

	auto iter = particles.begin();

	while (iter != particles.end())
	{
		auto particle = *iter;

		if (particle->age >= particle->maxAge)
		{
			delete particle;
			iter = particles.erase(iter);
			continue;
		}

		for (auto& force : forces)
		{
			force->affectParticle(particle);
		}

		particle->speed += particle->acceleration * game->deltaTime;
		particle->position += particle->velocity * particle->speed * game->deltaTime;
		particle->age += game->deltaTime;
		f32 t = particle->age / particle->maxAge;
		particle->color.r = lerp(params.color.min.r, params.color.max.r, t);
		particle->color.g = lerp(params.color.min.g, params.color.max.g, t);
		particle->color.b = lerp(params.color.min.b, params.color.max.b, t);
		particle->color.a = lerp(params.color.min.a, params.color.max.a, t);

		++iter;
	}
}

void ParticleSystem::render(struct Graphics* gfx)
{
	for (auto& particle : particles)
	{
		Rect rc;

		rc.set(particle->position.x - particle->size.x / 2,
			particle->position.y - particle->size.y / 2,
			particle->size.x,
			particle->size.y);

		rc = Game::instance->worldToScreen(rc, layerIndex);
		gfx->setupColor(particle->color.getRgba(), ColorMode::Mul);
		gfx->drawQuad(rc, gfx->atlas->whiteImage->uvRect);
	}
}

Vec2 ParticleSystem::getNewSpawnPosition()
{
	switch (params.spawnPosition)
	{
	case ParticleSpawnPosition::Point:
	{
		return root->position;
		break;
	}
	default:
		break;
	}

	return Vec2();
}

Vec2 ParticleSystem::getNewSpawnVelocity(const Vec2& pos)
{
	switch (params.spawnVelocity)
	{
	case ParticleSpawnVelocity::Random:
	{
		return Vec2(
			randomFloat(-1, 1),
			randomFloat(-1, 1)
		);
		break;
	}
	case ParticleSpawnVelocity::Conical:
	{
		break;
	}

	default:
		break;
	}

	return Vec2();
}

void ParticleGravity::affectParticle(Particle* particle)
{
	particle->velocity += gravityDirection * acceleration * Game::instance->deltaTime;
}

}
