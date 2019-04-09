#include "unit_instance.h"

namespace engine
{
struct ProjectileInstance : UnitInstance
{
    struct WeaponInstance* weapon = nullptr;
    Vec2 velocity;
};

}