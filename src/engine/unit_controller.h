#include "types.h"
#include <string>
#include "json/value.h"
#include "vec2.h"
#include "color.h"
#include "resources/sprite_resource.h"

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

struct ScreenFxController : UnitController
{
	Vec2 shakeForce;
	f32 shakeDuration, shakeTimer = 0;
	u32 shakeCounter;
	u32 shakeCount;
	bool doingShake = false;

	f32 fadeDuration, fadeTimer = 0;
	Color fadeColor;
	ColorMode fadeColorMode;
	f32 fadeTimerDir = 1;
	bool fadeRevertBackAfter = false;
	bool doingFade = false;

	void shakeCamera(const Vec2& force, f32 duration, u32 count);
	void fadeScreen(const Color& color, ColorMode colorMode, f32 duration, bool revertBackAfter);
	void update(struct Game* game) override;
	UnitController* createNew() { return new ScreenFxController(); }
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
	bool isFirePressed = false;
	bool active = true;

	PlayerController(struct Game* game);
	void update(struct Game* game) override;
	UnitController* createNew() { return new PlayerController(nullptr); }
};

}
