#pragma once
#include "types.h"
#include "resources/unit_resource.h"
#include "lua_scripting.h"
#include "unit.h"
#include "color.h"

namespace engine
{
struct Particle
{
	f32 age = 0.0f;
	f32 maxAge = 1.0f;
	f32 acceleration = 1.0f;
	f32 speed = 1.0f;
	Color color;
	Vec2 position;
	Vec2 velocity;
	Vec2 size;
};

template <typename T>
struct ParticleParamRange
{
	ParticleParamRange() {}
	ParticleParamRange(const T& minVal, const T& maxVal)
		: min(minVal), max(maxVal) {}
	T min, max;
};

enum class ParticleSpawnPosition
{
	Point,
	Ellipse,
	FillEllipse,
	Rect,
	FillRect,
	Line,
	Polyline,

	Count
};

enum class ParticleSpawnVelocity
{
	Random,
	Conical,
	Unidirectional,
	Zero,

	Count
};

struct ParticleSystemParams
{
	ParticleParamRange<f32> age{5, 5};
	ParticleParamRange<f32> acceleration{0, 50};
	ParticleParamRange<f32> speed{77, 77};
	ParticleParamRange<Color> color{Color::white, Color::red};
	ParticleParamRange<Vec2> size{Vec2(1, 1), Vec2(1, 1)};
	ParticleSpawnPosition spawnPosition = ParticleSpawnPosition::Point;
	ParticleSpawnVelocity spawnVelocity = ParticleSpawnVelocity::Random;
	ParticleParamRange<f32> coneSpawnAngle{-10, 10};
	u32 particlesPerSecond = 1000;
	f32 life = 0; // 0 infinite
};

struct ParticleForce
{
	virtual void affectParticle(Particle* particle) = 0;
};

struct ParticleGravity : ParticleForce
{
	f32 acceleration = 9.8;
	Vec2 gravityDirection = { 0, 1 };

	void affectParticle(Particle* particle) override;
};

struct ParticleSystem : Unit
{
	ParticleSystem();
	virtual ~ParticleSystem();
	virtual void copyFrom(Unit* other) override;
	virtual void initializeFrom(UnitResource* res) override;
	virtual void update(struct Game* game) override;
	virtual void render(struct Graphics* gfx) override;

	ParticleSystemParams params;

private:
	Vec2 getNewSpawnPosition();
	Vec2 getNewSpawnVelocity(const Vec2& pos);

	f32 spawnTimer = 0;
	std::vector<Particle*> particles;
	std::vector<ParticleForce*> forces;
};

}
