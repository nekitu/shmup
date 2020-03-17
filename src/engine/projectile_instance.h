#include "unit_instance.h"

namespace engine
{
struct ProjectileInstance : UnitInstance
{
	struct WeaponInstance* weapon = nullptr;
	Vec2 velocity;
	f32 minSpeed = 1, maxSpeed = 100000;
	f32 acceleration = 0;

	void update(struct Game* game) override;
};

}
