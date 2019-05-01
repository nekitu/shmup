#include "types.h"

namespace engine
{
struct UnitController
{
	struct UnitInstance* unitInstance = nullptr;

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

struct ProjectileController : UnitController
{
	void update(struct Game* game) override;
};

struct PlayerController : UnitController
{
	u32 playerIndex = 0;
	struct SoundResource* fireSoundRes = nullptr;
	struct SoundInstance* fireSound = nullptr;
	bool isFirePressed = false;

	PlayerController(struct Game* game);
	void update(struct Game* game) override;
};

}