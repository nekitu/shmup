#pragma once
#include "types.h"
#include <unordered_map>
#include <string>
#include "vec2.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_version.h>

namespace engine
{
enum class MouseControl
{
	None = 0,
	LeftButton,
	RightButton,
	MiddleButton,
	Wheel,

	Count
};

struct InputAction
{
	std::string title, info;
	bool down = false;
	bool downNow = false;
	bool pressed = false;
	f32 value = 0;
};

struct InputControl
{
	enum class DeviceType
	{
		Keyboard,
		GamepadAxis,
		GamepadButton,
		MouseButton,
		MouseWheel
	};

	DeviceType deviceType = DeviceType::Keyboard;
	u32 code = 0;
};

struct InputControlMapped
{
	std::string controlName;
	u32 deviceIndex = 0;
};

struct InputMapping
{
	std::vector<InputControlMapped> controls;
};

struct Input
{
	std::unordered_map<std::string, InputMapping> mappings;
	std::unordered_map<std::string, InputAction> actions;
	std::unordered_map<std::string, InputControl> controls;
	Vec2 mousePosition;
	Vec2 windowMousePosition;
	std::vector<SDL_GameController*> gamepads;
	bool dirtyActions = false;

	void loadActions(const std::string& path);
	void loadMappings(const std::string& path);
	bool isDown(const std::string& action);
	bool wasPressed(const std::string& action);
	bool wasDown(const std::string& action);
	f32 getValue(const std::string& action);
	void update();
	void searchControllers();
	void initialize();

private:
	u32 getGamepadIndex(SDL_JoystickID id);

	void registerKey(u32 key, const std::string& name);
	void registerMouseButton(u32 btn, const std::string& name);
	void registerMouseWheel(u32 index, const std::string& name);
	void registerGamepadAxis(u32 axis, const std::string& name);
	void registerGamepadButton(u32 btn, const std::string& name);
};

}
