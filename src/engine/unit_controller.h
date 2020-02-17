#include "types.h"
#include <string>
#include "json/value.h"
#include "vec2.h"

namespace engine
{
struct UnitController
{
	struct UnitInstance* unitInstance = nullptr;
	std::string name;

	virtual void update(struct Game* game) = 0;
	virtual UnitController* createNew() = 0;
	virtual void initializeFromJson(const Json::Value& value) {}

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

struct FollowController : UnitController
{
	struct SpriteInstance* follower = nullptr;
	struct SpriteInstance* follow = nullptr;
	Vec2 offset;
	f32 speed = 1;
	bool constantSpeed = false;
	bool offsetAcquired = false;

	void acquireOffset();
	void update(struct Game* game) override;
	void initializeFromJson(const Json::Value& value) override;
	UnitController* createNew() { return new FollowController(); }
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
