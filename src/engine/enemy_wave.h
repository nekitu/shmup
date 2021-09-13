#pragma once
#include "unit.h"

namespace engine
{
struct EnemyWave : Unit
{
	f32 activeTime = 10;
	f32 spawnDelay = 2;
};
}
