#include "types.h"
#include <string>

namespace engine
{
struct UnitController
{
	struct UnitInstance* unitInstance = nullptr;

	virtual void update(struct Game* game) = 0;
	virtual UnitController* createNew() = 0;

	static UnitController* create(const std::string& ctrlerName, struct UnitInstance* unitInst);
};

struct BackgroundController : UnitController
{
	void update(struct Game* game) override;
	UnitController* createNew() { return new BackgroundController(); }
};

struct SimpleEnemyController : UnitController
{
	void update(struct Game* game) override;
	UnitController* createNew() { return new SimpleEnemyController(); }
};

struct ProjectileController : UnitController
{
	void update(struct Game* game) override;
	UnitController* createNew() { return new ProjectileController(); }
};

struct PlayerController : UnitController
{
	u32 playerIndex = 0;
	struct SoundResource* fireSoundRes = nullptr;
	struct SoundInstance* fireSound = nullptr;
	bool isFirePressed = false;

	PlayerController(struct Game* game);
	void update(struct Game* game) override;
	UnitController* createNew() { return new PlayerController(nullptr); }
};

}
