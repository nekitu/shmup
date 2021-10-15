#pragma once
#include "types.h"
#include <unordered_map>
#include <string>
#include "vec2.h"

namespace engine
{
struct InputMapping
{
	std::vector<InputControl> controls;
};

struct InputAction
{
	bool down = false;
	f32 value = 0;
};

struct InputControl
{
	enum class DeviceType
	{
		Keyboard,
		Gamepad,
		Mouse
	};

	DeviceType deviceType = DeviceType::Keyboard;
	u32 code = 0;
};

struct Input
{
	std::unordered_map<std::string, InputMapping> mappings;
	std::unordered_map<std::string, InputAction> actions;
	std::unordered_map<std::string, InputControl> controls;
	Vec2 mousePosition;
	Vec2 windowMousePosition;

	Input();
	void loadActions(const std::string& path);
	void loadMappings(const std::string& path);
	bool isDown(const std::string& action);
	void update();
private:
	void registerKey(u32 key, const std::string& name);
	void registerGPad();
};

}
