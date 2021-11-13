#include "input.h"
#include "game.h"
#include "graphics.h"

namespace engine
{
void Input::initialize()
{
	registerKey(SDLK_RETURN, "return");
	registerKey(SDLK_ESCAPE, "escape");
	registerKey(SDLK_BACKSPACE, "backspace");
	registerKey(SDLK_TAB, "tab");
	registerKey(SDLK_SPACE, "space");
	registerKey(SDLK_EXCLAIM, "exclaim");
	registerKey(SDLK_QUOTEDBL, "quotedbl");
	registerKey(SDLK_HASH, "hash");
	registerKey(SDLK_PERCENT, "percent");
	registerKey(SDLK_DOLLAR, "dollar");
	registerKey(SDLK_AMPERSAND, "ampersand");
	registerKey(SDLK_QUOTE, "quote");
	registerKey(SDLK_LEFTPAREN, "leftparen");
	registerKey(SDLK_RIGHTPAREN, "rightparen");
	registerKey(SDLK_ASTERISK, "asterisk");
	registerKey(SDLK_PLUS, "plus");
	registerKey(SDLK_COMMA, "comma");
	registerKey(SDLK_MINUS, "minus");
	registerKey(SDLK_PERIOD, "period");
	registerKey(SDLK_SLASH, "slash");
	registerKey(SDLK_0, "0");
	registerKey(SDLK_1, "1");
	registerKey(SDLK_2, "2");
	registerKey(SDLK_3, "3");
	registerKey(SDLK_4, "4");
	registerKey(SDLK_5, "5");
	registerKey(SDLK_6, "6");
	registerKey(SDLK_7, "7");
	registerKey(SDLK_8, "8");
	registerKey(SDLK_9, "9");
	registerKey(SDLK_COLON, "colon");
	registerKey(SDLK_SEMICOLON, "semicolon");
	registerKey(SDLK_LESS, "less");
	registerKey(SDLK_EQUALS, "equals");
	registerKey(SDLK_GREATER, "greater");
	registerKey(SDLK_QUESTION, "question");
	registerKey(SDLK_AT, "at");
	registerKey(SDLK_LEFTBRACKET, "leftbracket");
	registerKey(SDLK_BACKSLASH, "backslash");
	registerKey(SDLK_RIGHTBRACKET, "rightbracket");
	registerKey(SDLK_CARET, "caret");
	registerKey(SDLK_UNDERSCORE, "underscore");
	registerKey(SDLK_BACKQUOTE, "backquote");
	registerKey(SDLK_a, "a");
	registerKey(SDLK_b, "b");
	registerKey(SDLK_c, "c");
	registerKey(SDLK_d, "d");
	registerKey(SDLK_e, "e");
	registerKey(SDLK_f, "f");
	registerKey(SDLK_g, "g");
	registerKey(SDLK_h, "h");
	registerKey(SDLK_i, "i");
	registerKey(SDLK_j, "j");
	registerKey(SDLK_k, "k");
	registerKey(SDLK_l, "l");
	registerKey(SDLK_m, "m");
	registerKey(SDLK_n, "n");
	registerKey(SDLK_o, "o");
	registerKey(SDLK_p, "p");
	registerKey(SDLK_q, "q");
	registerKey(SDLK_r, "r");
	registerKey(SDLK_s, "s");
	registerKey(SDLK_t, "t");
	registerKey(SDLK_u, "u");
	registerKey(SDLK_v, "v");
	registerKey(SDLK_w, "w");
	registerKey(SDLK_x, "x");
	registerKey(SDLK_y, "y");
	registerKey(SDLK_z, "z");
	registerKey(SDLK_CAPSLOCK, "capslock");
	registerKey(SDLK_F1, "f1");
	registerKey(SDLK_F2, "f2");
	registerKey(SDLK_F3, "f3");
	registerKey(SDLK_F4, "f4");
	registerKey(SDLK_F5, "f5");
	registerKey(SDLK_F6, "f6");
	registerKey(SDLK_F7, "f7");
	registerKey(SDLK_F8, "f8");
	registerKey(SDLK_F9, "f9");
	registerKey(SDLK_F10, "f10");
	registerKey(SDLK_F11, "f11");
	registerKey(SDLK_F12, "f12");
	registerKey(SDLK_PRINTSCREEN, "printscreen");
	registerKey(SDLK_SCROLLLOCK, "scrolllock");
	registerKey(SDLK_PAUSE, "pause");
	registerKey(SDLK_INSERT, "insert");
	registerKey(SDLK_HOME, "home");
	registerKey(SDLK_PAGEUP, "pageup");
	registerKey(SDLK_DELETE, "delete");
	registerKey(SDLK_END, "end");
	registerKey(SDLK_PAGEDOWN, "pagedown");
	registerKey(SDLK_RIGHT, "right");
	registerKey(SDLK_LEFT, "left");
	registerKey(SDLK_DOWN, "down");
	registerKey(SDLK_UP, "up");
	registerKey(SDLK_NUMLOCKCLEAR, "numlockclear");
	registerKey(SDLK_KP_DIVIDE, "kp_divide");
	registerKey(SDLK_KP_MULTIPLY, "kp_multiply");
	registerKey(SDLK_KP_MINUS, "kp_minus");
	registerKey(SDLK_KP_PLUS, "kp_plus");
	registerKey(SDLK_KP_ENTER, "kp_enter");
	registerKey(SDLK_KP_1, "kp_1");
	registerKey(SDLK_KP_2, "kp_2");
	registerKey(SDLK_KP_3, "kp_3");
	registerKey(SDLK_KP_4, "kp_4");
	registerKey(SDLK_KP_5, "kp_5");
	registerKey(SDLK_KP_6, "kp_6");
	registerKey(SDLK_KP_7, "kp_7");
	registerKey(SDLK_KP_8, "kp_8");
	registerKey(SDLK_KP_9, "kp_9");
	registerKey(SDLK_KP_0, "kp_0");
	registerKey(SDLK_KP_PERIOD, "kp_period");
	registerKey(SDLK_APPLICATION, "application");
	registerKey(SDLK_POWER, "power");
	registerKey(SDLK_KP_EQUALS, "kp_equals");
	registerKey(SDLK_F13, "f13");
	registerKey(SDLK_F14, "f14");
	registerKey(SDLK_F15, "f15");
	registerKey(SDLK_F16, "f16");
	registerKey(SDLK_F17, "f17");
	registerKey(SDLK_F18, "f18");
	registerKey(SDLK_F19, "f19");
	registerKey(SDLK_F20, "f20");
	registerKey(SDLK_F21, "f21");
	registerKey(SDLK_F22, "f22");
	registerKey(SDLK_F23, "f23");
	registerKey(SDLK_F24, "f24");
	registerKey(SDLK_EXECUTE, "execute");
	registerKey(SDLK_HELP, "help");
	registerKey(SDLK_MENU, "menu");
	registerKey(SDLK_SELECT, "select");
	registerKey(SDLK_STOP, "stop");
	registerKey(SDLK_AGAIN, "again");
	registerKey(SDLK_UNDO, "undo");
	registerKey(SDLK_CUT, "cut");
	registerKey(SDLK_COPY, "copy");
	registerKey(SDLK_PASTE, "paste");
	registerKey(SDLK_FIND, "find");
	registerKey(SDLK_MUTE, "mute");
	registerKey(SDLK_VOLUMEUP, "volumeup");
	registerKey(SDLK_VOLUMEDOWN, "volumedown");
	registerKey(SDLK_KP_COMMA, "kp_comma");
	registerKey(SDLK_KP_EQUALSAS400, "kp_equalsas400");
	registerKey(SDLK_ALTERASE, "alterase");
	registerKey(SDLK_SYSREQ, "sysreq");
	registerKey(SDLK_CANCEL, "cancel");
	registerKey(SDLK_CLEAR, "clear");
	registerKey(SDLK_PRIOR, "prior");
	registerKey(SDLK_RETURN2, "return2");
	registerKey(SDLK_SEPARATOR, "separator");
	registerKey(SDLK_OUT, "out");
	registerKey(SDLK_OPER, "oper");
	registerKey(SDLK_CLEARAGAIN, "clearagain");
	registerKey(SDLK_CRSEL, "crsel");
	registerKey(SDLK_EXSEL, "exsel");
	registerKey(SDLK_KP_00, "kp_00");
	registerKey(SDLK_KP_000, "kp_000");
	registerKey(SDLK_THOUSANDSSEPARATOR, "thousandsseparator");
	registerKey(SDLK_DECIMALSEPARATOR, "decimalseparator");
	registerKey(SDLK_CURRENCYUNIT, "currencyunit");
	registerKey(SDLK_CURRENCYSUBUNIT, "currencysubunit");
	registerKey(SDLK_KP_LEFTPAREN, "kp_leftparen");
	registerKey(SDLK_KP_RIGHTPAREN, "kp_rightparen");
	registerKey(SDLK_KP_LEFTBRACE, "kp_leftbrace");
	registerKey(SDLK_KP_RIGHTBRACE, "kp_rightbrace");
	registerKey(SDLK_KP_TAB, "kp_tab");
	registerKey(SDLK_KP_BACKSPACE, "kp_backspace");
	registerKey(SDLK_KP_A, "kp_a");
	registerKey(SDLK_KP_B, "kp_b");
	registerKey(SDLK_KP_C, "kp_c");
	registerKey(SDLK_KP_D, "kp_d");
	registerKey(SDLK_KP_E, "kp_e");
	registerKey(SDLK_KP_F, "kp_f");
	registerKey(SDLK_KP_XOR, "kp_xor");
	registerKey(SDLK_KP_POWER, "kp_power");
	registerKey(SDLK_KP_PERCENT, "kp_percent");
	registerKey(SDLK_KP_LESS, "kp_less");
	registerKey(SDLK_KP_GREATER, "kp_greater");
	registerKey(SDLK_KP_AMPERSAND, "kp_ampersand");
	registerKey(SDLK_KP_DBLAMPERSAND, "kp_dblampersand");
	registerKey(SDLK_KP_VERTICALBAR, "kp_verticalbar");
	registerKey(SDLK_KP_DBLVERTICALBAR, "kp_dblverticalbar");
	registerKey(SDLK_KP_COLON, "kp_colon");
	registerKey(SDLK_KP_HASH, "kp_hash");
	registerKey(SDLK_KP_SPACE, "kp_space");
	registerKey(SDLK_KP_AT, "kp_at");
	registerKey(SDLK_KP_EXCLAM, "kp_exclam");
	registerKey(SDLK_KP_MEMSTORE, "kp_memstore");
	registerKey(SDLK_KP_MEMRECALL, "kp_memrecall");
	registerKey(SDLK_KP_MEMCLEAR, "kp_memclear");
	registerKey(SDLK_KP_MEMADD, "kp_memadd");
	registerKey(SDLK_KP_MEMSUBTRACT, "kp_memsubtract");
	registerKey(SDLK_KP_MEMMULTIPLY, "kp_memmultiply");
	registerKey(SDLK_KP_MEMDIVIDE, "kp_memdivide");
	registerKey(SDLK_KP_PLUSMINUS, "kp_plusminus");
	registerKey(SDLK_KP_CLEAR, "kp_clear");
	registerKey(SDLK_KP_CLEARENTRY, "kp_clearentry");
	registerKey(SDLK_KP_BINARY, "kp_binary");
	registerKey(SDLK_KP_OCTAL, "kp_octal");
	registerKey(SDLK_KP_DECIMAL, "kp_decimal");
	registerKey(SDLK_KP_HEXADECIMAL, "kp_hexadecimal");
	registerKey(SDLK_LCTRL, "lctrl");
	registerKey(SDLK_LSHIFT, "lshift");
	registerKey(SDLK_LALT, "lalt");
	registerKey(SDLK_LGUI, "lgui");
	registerKey(SDLK_RCTRL, "rctrl");
	registerKey(SDLK_RSHIFT, "rshift");
	registerKey(SDLK_RALT, "ralt");
	registerKey(SDLK_RGUI, "rgui");
	registerKey(SDLK_MODE, "mode");
	registerKey(SDLK_AUDIONEXT, "audionext");
	registerKey(SDLK_AUDIOPREV, "audioprev");
	registerKey(SDLK_AUDIOSTOP, "audiostop");
	registerKey(SDLK_AUDIOPLAY, "audioplay");
	registerKey(SDLK_AUDIOMUTE, "audiomute");
	registerKey(SDLK_MEDIASELECT, "mediaselect");
	registerKey(SDLK_WWW, "www");
	registerKey(SDLK_MAIL, "mail");
	registerKey(SDLK_CALCULATOR, "calculator");
	registerKey(SDLK_COMPUTER, "computer");
	registerKey(SDLK_AC_SEARCH, "ac_search");
	registerKey(SDLK_AC_HOME, "ac_home");
	registerKey(SDLK_AC_BACK, "ac_back");
	registerKey(SDLK_AC_FORWARD, "ac_forward");
	registerKey(SDLK_AC_STOP, "ac_stop");
	registerKey(SDLK_AC_REFRESH, "ac_refresh");
	registerKey(SDLK_AC_BOOKMARKS, "ac_bookmarks");
	registerKey(SDLK_BRIGHTNESSDOWN, "brightnessdown");
	registerKey(SDLK_BRIGHTNESSUP, "brightnessup");
	registerKey(SDLK_DISPLAYSWITCH, "displayswitch");
	registerKey(SDLK_KBDILLUMTOGGLE, "kbdillumtoggle");
	registerKey(SDLK_KBDILLUMDOWN, "kbdillumdown");
	registerKey(SDLK_KBDILLUMUP, "kbdillumup");
	registerKey(SDLK_EJECT, "eject");
	registerKey(SDLK_SLEEP, "sleep");
	registerKey(SDLK_APP1, "app1");
	registerKey(SDLK_APP2, "app2");
	registerKey(SDLK_AUDIOREWIND, "audiorewind");
	registerKey(SDLK_AUDIOFASTFORWARD, "audiofastforward");

	registerMouseButton((u32)MouseControl::LeftButton, "lmb");
	registerMouseButton((u32)MouseControl::RightButton, "rmb");
	registerMouseButton((u32)MouseControl::MiddleButton, "mmb");
	registerMouseWheel((u32)MouseControl::Wheel, "wheel");

	registerGamepadAxis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTX, "axisleftx");
	registerGamepadAxis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_LEFTY, "axislefty");
	registerGamepadAxis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTX, "axisrightx");
	registerGamepadAxis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_RIGHTY, "axisrighty");
	registerGamepadAxis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERLEFT, "axistriggerleft");
	registerGamepadAxis(SDL_GameControllerAxis::SDL_CONTROLLER_AXIS_TRIGGERRIGHT, "axistriggerright");

	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_A, "buttona");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_B, "buttonb");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_X, "buttonx");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_Y, "buttony");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_BACK, "buttonback");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_GUIDE, "buttonguide");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_START, "buttonstart");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSTICK, "buttonleftstick");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSTICK, "buttonrightstick");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_LEFTSHOULDER, "buttonleftshoulder");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_RIGHTSHOULDER, "buttonrightshoulder");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_UP, "buttondpadup");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_DOWN, "buttondpaddown");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_LEFT, "buttondpadleft");
	registerGamepadButton(SDL_GameControllerButton::SDL_CONTROLLER_BUTTON_DPAD_RIGHT, "buttondpadright");

	LOG_INFO("Searching game controllers in {} joysticks...", SDL_NumJoysticks());
	for (int i = 0; i < SDL_NumJoysticks(); i++)
	{
		if (SDL_IsGameController(i))
		{
			auto ctrl = SDL_GameControllerOpen(i);
			gamepads.push_back(ctrl);
			LOG_INFO("Controller {} {} connected", i, SDL_GameControllerName(ctrl));
		}
	}
}

void Input::loadActions(const std::string& path)
{
	auto absPath = Game::instance->dataRoot + path;
	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return;
	}

	for (int i = 0; i < json.getMemberNames().size(); i++)
	{
		auto name = json.getMemberNames()[i];
		auto jsonAction = json.get(name, Json::Value(Json::ValueType::objectValue));

		InputAction ia;
		ia.title = jsonAction.get("title", "").asString();
		ia.info = jsonAction.get("info", "").asString();

		actions[name] = ia;

		LOG_INFO("Action: {}", name);
	}
}

void Input::loadMappings(const std::string& path)
{
	auto absPath = Game::instance->dataRoot + path;
	Json::Value json;

	if (!loadJson(absPath + ".json", json))
	{
		return;
	}

	for (int i = 0; i < json.getMemberNames().size(); i++)
	{
		auto name = json.getMemberNames()[i];
		auto jsonMapping = json.get(name, Json::Value(Json::ValueType::arrayValue));

		InputMapping im;

		for (int j = 0; j < jsonMapping.size(); j++)
		{
			auto str = jsonMapping[j].asString();
			auto beginIndex = str.find('[');
			auto endIndex = str.find(']');

			InputControlMapped mappedCtrl;

			if (beginIndex != std::string::npos && endIndex != std::string::npos)
			{
				mappedCtrl.controlName = str.substr(0, beginIndex);
				mappedCtrl.deviceIndex = atoi(str.substr(beginIndex + 1, endIndex - beginIndex).c_str());
			}
			else
			{
				mappedCtrl.controlName = str;
			}

			LOG_INFO("Mapping: {} index {}", mappedCtrl.controlName, mappedCtrl.deviceIndex);

			im.controls.push_back(mappedCtrl);
		}

		LOG_INFO("Mapping action {}", name);
		mappings[name] = im;
	}
}

bool Input::isDown(const std::string& action)
{
	auto iter = actions.find(action);
	if (iter == actions.end())
	{
		LOG_ERROR("Input::isDown: Unknown input action: {}", action);
		return false;
	}

	return iter->second.down;
}

bool Input::wasPressed(const std::string& action)
{
	auto iter = actions.find(action);
	if (iter == actions.end())
	{
		LOG_ERROR("Input::wasPressed: Unknown input action: {}", action);
		return false;
	}

	return iter->second.pressed;
}

bool Input::wasDown(const std::string& action)
{
	auto iter = actions.find(action);
	if (iter == actions.end())
	{
		LOG_ERROR("Input::wasDown: Unknown input action: {}", action);
		return false;
	}

	return iter->second.downNow;
}

f32 Input::getValue(const std::string& action)
{
	auto iter = actions.find(action);
	if (iter == actions.end())
	{
		LOG_ERROR("Input::getValue: Unknown input action: {}", action);
		return false;
	}

	return iter->second.value;
}

void Input::update()
{
	SDL_Event ev;

	if (dirtyActions)
	{
		for (auto& action : actions)
		{
			action.second.pressed = false;
			action.second.downNow = false;
		}
	}

	SDL_PumpEvents();

	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
		case SDL_WINDOWEVENT:
		{
			switch (ev.window.event)
			{
			case SDL_WINDOWEVENT_RESIZED:
			case SDL_WINDOWEVENT_SIZE_CHANGED:
			case SDL_WINDOWEVENT_MOVED:
			case SDL_WINDOWEVENT_EXPOSED:
				break;
			case SDL_WINDOWEVENT_FOCUS_GAINED:
			{
				if (Game::instance->pauseOnAppDeactivate)
					Game::instance->pauseGame = false;
				break;
			}
			case SDL_WINDOWEVENT_ENTER:
			{
				break;
			}
			case SDL_WINDOWEVENT_FOCUS_LOST:
			{
				if (Game::instance->pauseOnAppDeactivate)
					Game::instance->pauseGame = true;
				break;
			}
			case SDL_WINDOWEVENT_CLOSE:
			{
				Game::instance->exitGame = true;
				break;
			}
			default:
				break;
			}

			break;
		}

		case SDL_KEYDOWN:
		{
			auto deviceIndex = 0;
			for (auto& mapping : mappings)
			{
				for (auto& ctrlMapping : mapping.second.controls)
				{
					auto iterAction = actions.find(mapping.first);

					if (iterAction != actions.end())
					{
						if (ctrlMapping.deviceIndex == deviceIndex
							&& controls[ctrlMapping.controlName].deviceType == InputControl::DeviceType::Keyboard
							&& ev.key.keysym.sym == controls[ctrlMapping.controlName].code)
						{
							if (!iterAction->second.down)
							{
								iterAction->second.downNow = true;
								dirtyActions = true;
							}

							iterAction->second.down = true;
							iterAction->second.value = 1;
						}
					}
				}
			}

			break;
		}

		case SDL_KEYUP:
		{
			auto deviceIndex = 0;
			for (auto& mapping : mappings)
			{
				for (auto& ctrlMapping : mapping.second.controls)
				{
					auto iterAction = actions.find(mapping.first);

					if (iterAction != actions.end())
					{
						if (ctrlMapping.deviceIndex == deviceIndex
							&& controls[ctrlMapping.controlName].deviceType == InputControl::DeviceType::Keyboard
							&& ev.key.keysym.sym == controls[ctrlMapping.controlName].code)
						{
							if (iterAction->second.down)
							{
								iterAction->second.pressed = true;
								dirtyActions = true;
							}

							iterAction->second.down = false;
							iterAction->second.value = 0;
						}
					}
				}
			}

			break;
		}

		case SDL_MOUSEBUTTONDOWN:
		{
			auto deviceIndex = 0;
			for (auto& mapping : mappings)
			{
				for (auto& ctrlMapping : mapping.second.controls)
				{
					auto iterAction = actions.find(mapping.first);

					if (iterAction != actions.end())
					{
						if (ctrlMapping.deviceIndex == deviceIndex
							&& controls[ctrlMapping.controlName].deviceType == InputControl::DeviceType::MouseButton
							&& ev.button.button == controls[ctrlMapping.controlName].code)
						{
							if (!iterAction->second.down)
							{
								iterAction->second.downNow = true;
								dirtyActions = true;
							}

							iterAction->second.down = true;
							iterAction->second.value = 1;
						}
					}
				}
			}
			break;
		}

		case SDL_MOUSEBUTTONUP:
		{
			auto deviceIndex = 0;
			for (auto& mapping : mappings)
			{
				for (auto& ctrlMapping : mapping.second.controls)
				{
					auto iterAction = actions.find(mapping.first);

					if (iterAction != actions.end())
					{
						if (ctrlMapping.deviceIndex == deviceIndex
							&& controls[ctrlMapping.controlName].deviceType == InputControl::DeviceType::MouseButton
							&& ev.button.button == controls[ctrlMapping.controlName].code)
						{
							if (iterAction->second.down)
							{
								iterAction->second.pressed = true;
								dirtyActions = true;
							}

							iterAction->second.down = false;
							iterAction->second.value = 0;
						}
					}
				}
			}
			break;
		}

		case SDL_MOUSEMOTION:
		{
			windowMousePosition = { (f32)ev.motion.x, (f32)ev.motion.y };
			f32 scale = Game::instance->graphics->videoWidth / Game::instance->graphics->blittedRect.width;
			mousePosition.x = (windowMousePosition.x - Game::instance->graphics->blittedRect.x) * scale;
			mousePosition.y = (windowMousePosition.y - Game::instance->graphics->blittedRect.y) * scale;
			break;
		}

		case SDL_CONTROLLERAXISMOTION:
		{
			auto deviceIndex = getGamepadIndex(ev.caxis.which);
			for (auto& mapping : mappings)
			{
				for (auto& ctrlMapping : mapping.second.controls)
				{
					auto iterAction = actions.find(mapping.first);

					if (iterAction != actions.end())
					{
						if (ctrlMapping.deviceIndex == deviceIndex
							&& controls[ctrlMapping.controlName].deviceType == InputControl::DeviceType::GamepadAxis
							&& ev.caxis.axis == controls[ctrlMapping.controlName].code)
						{
							iterAction->second.value = (f32)ev.caxis.value / (f32)SDL_JOYSTICK_AXIS_MAX;
						}
					}
				}
			}

			break;
		}
		case SDL_CONTROLLERBUTTONDOWN:
		{
			auto deviceIndex = getGamepadIndex(ev.caxis.which);
			for (auto& mapping : mappings)
			{
				for (auto& ctrlMapping : mapping.second.controls)
				{
					auto iterAction = actions.find(mapping.first);

					if (iterAction != actions.end())
					{
						if (ctrlMapping.deviceIndex == deviceIndex
							&& controls[ctrlMapping.controlName].deviceType == InputControl::DeviceType::GamepadButton
							&& ev.cbutton.button == controls[ctrlMapping.controlName].code)
						{
							if (!iterAction->second.down)
							{
								iterAction->second.downNow = true;
								dirtyActions = true;
							}

							iterAction->second.down = true;
							iterAction->second.value = 1;
						}
					}
				}
			}

			break;
		}

		case SDL_CONTROLLERBUTTONUP:
		{
			auto deviceIndex = getGamepadIndex(ev.caxis.which);
			for (auto& mapping : mappings)
			{
				for (auto& ctrlMapping : mapping.second.controls)
				{
					auto iterAction = actions.find(mapping.first);

					if (iterAction != actions.end())
					{
						if (ctrlMapping.deviceIndex == deviceIndex
							&& controls[ctrlMapping.controlName].deviceType == InputControl::DeviceType::GamepadButton
							&& ev.cbutton.button == controls[ctrlMapping.controlName].code)
						{
							if (iterAction->second.down)
							{
								iterAction->second.pressed = true;
								dirtyActions = true;
							}

							iterAction->second.down = false;
							iterAction->second.value = 0;
						}
					}
				}
			}

			break;
		}

		default:
			break;
		}
	}
}

u32 Input::getGamepadIndex(SDL_JoystickID id)
{
	u32 idx = 0;

	for (auto& ctrl : gamepads)
	{
		if (ctrl == SDL_GameControllerFromInstanceID(id))
		{
			return idx;
		}

		++idx;
	}

	return 0;
}

void Input::registerKey(u32 key, const std::string& name)
{
	controls[name].code = key;
	controls[name].deviceType = InputControl::DeviceType::Keyboard;
}

void Input::registerMouseButton(u32 btn, const std::string& name)
{
	controls[name].code = btn;
	controls[name].deviceType = InputControl::DeviceType::MouseButton;
}

void Input::registerMouseWheel(u32 index, const std::string& name)
{
	controls[name].code = index;
	controls[name].deviceType = InputControl::DeviceType::MouseWheel;
}

void Input::registerGamepadAxis(u32 axis, const std::string& name)
{
	controls[name].code = axis;
	controls[name].deviceType = InputControl::DeviceType::GamepadAxis;
}

void Input::registerGamepadButton(u32 btn, const std::string& name)
{
	controls[name].code = btn;
	controls[name].deviceType = InputControl::DeviceType::GamepadButton;
}

}
