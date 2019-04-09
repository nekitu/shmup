#include "types.h"

namespace engine
{
struct UnitController
{
	UnitInstance* unitInstance = nullptr;

	virtual void update(struct Game* game) = 0;
};

struct BackgroundController : UnitController
{
	void update(struct Game* game) override;
};

struct SimpleEnemyController : UnitController
{
	void update(struct Game* game) override;
};

struct PlayerController : UnitController
{
	u32 playerIndex = 0;
	struct Sound* fireSound = nullptr;
	f32 fire1Rate = 15;
	f32 fire1PlayingTime = FLT_MAX;

	PlayerController();
	void update(struct Game* game) override;
};

}