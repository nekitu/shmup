#include "input.h"
#define SDL_MAIN_HANDLED
#include <SDL.h>
#include <SDL_version.h>
#include <SDL_audio.h>

namespace engine
{
Input::Input()
{
	registerKey(SDLK_RETURN, "return");
	registerKey(SDLK_ESCAPE, "escape");
	registerKey(SDLK_BACKSPACE, "backspace");
	registerKey(SDLK_TAB, "tab");
	registerKey(SDLK_SPACE, "");
	registerKey(SDLK_EXCLAIM, "");
	registerKey(SDLK_QUOTEDBL, "");
	registerKey(SDLK_HASH, "");
	registerKey(SDLK_PERCENT, "");
	registerKey(SDLK_DOLLAR, "");
	registerKey(SDLK_AMPERSAND, "");
	registerKey(SDLK_QUOTE, "");
	registerKey(SDLK_LEFTPAREN, "");
	registerKey(SDLK_RIGHTPAREN, "");
	registerKey(SDLK_ASTERISK, "");
	registerKey(SDLK_PLUS, "");
	registerKey(SDLK_COMMA, "");
	registerKey(SDLK_MINUS, "");
	registerKey(SDLK_PERIOD, "");
	registerKey(SDLK_SLASH, "");
	registerKey(SDLK_0, "");
	registerKey(SDLK_1, "");
	registerKey(SDLK_2, "");
	registerKey(SDLK_3, "");
	registerKey(SDLK_4, "");
	registerKey(SDLK_5, "");
	registerKey(SDLK_6, "");
	registerKey(SDLK_7, "");
	registerKey(SDLK_8, "");
	registerKey(SDLK_9, "");
	registerKey(SDLK_COLON, "");
	registerKey(SDLK_SEMICOLON, "");
	registerKey(SDLK_LESS, "");
	registerKey(SDLK_EQUALS, "");
	registerKey(SDLK_GREATER, "");
	registerKey(SDLK_QUESTION, "");
	registerKey(SDLK_AT, "");
	registerKey(SDLK_LEFTBRACKET, "");
	registerKey(SDLK_BACKSLASH, "");
	registerKey(SDLK_RIGHTBRACKET, "");
	registerKey(SDLK_CARET, "");
	registerKey(SDLK_UNDERSCORE, "");
	registerKey(SDLK_BACKQUOTE, "");
	registerKey(SDLK_a, "");
	registerKey(SDLK_b, "");
	registerKey(SDLK_c, "");
	registerKey(SDLK_d, "");
	registerKey(SDLK_e, "");
	registerKey(SDLK_f, "");
	registerKey(SDLK_g, "");
	registerKey(SDLK_h, "");
	registerKey(SDLK_i, "");
	registerKey(SDLK_j, "");
	registerKey(SDLK_k, "");
	registerKey(SDLK_l, "");
	registerKey(SDLK_m, "");
	registerKey(SDLK_n, "");
	registerKey(SDLK_o, "");
	registerKey(SDLK_p, "");
	registerKey(SDLK_q, "");
	registerKey(SDLK_r, "");
	registerKey(SDLK_s, "");
	registerKey(SDLK_t, "");
	registerKey(SDLK_u, "");
	registerKey(SDLK_v, "");
	registerKey(SDLK_w, "");
	registerKey(SDLK_x, "");
	registerKey(SDLK_y, "");
	registerKey(SDLK_z, "");
	registerKey(SDLK_CAPSLOCK, "");
	registerKey(SDLK_F1, "");
	registerKey(SDLK_F2, "");
	registerKey(SDLK_F3, "");
	registerKey(SDLK_F4, "");
	registerKey(SDLK_F5, "");
	registerKey(SDLK_F6, "");
	registerKey(SDLK_F7, "");
	registerKey(SDLK_F8, "");
	registerKey(SDLK_F9, "");
	registerKey(SDLK_F10, "");
	registerKey(SDLK_F11, "");
	registerKey(SDLK_F12, "");
	registerKey(SDLK_PRINTSCREEN, "");
	registerKey(SDLK_SCROLLLOCK, "");
	registerKey(SDLK_PAUSE, "");
	registerKey(SDLK_INSERT, "");
	registerKey(SDLK_HOME, "");
	registerKey(SDLK_PAGEUP, "");
	registerKey(SDLK_DELETE, "");
	registerKey(SDLK_END, "");
	registerKey(SDLK_PAGEDOWN, "");
	registerKey(SDLK_RIGHT, "");
	registerKey(SDLK_LEFT, "");
	registerKey(SDLK_DOWN, "");
	registerKey(SDLK_UP, "");
	registerKey(SDLK_NUMLOCKCLEAR, "");
	registerKey(SDLK_KP_DIVIDE, "");
	registerKey(SDLK_KP_MULTIPLY, "");
	registerKey(SDLK_KP_MINUS, "");
	registerKey(SDLK_KP_PLUS, "");
	registerKey(SDLK_KP_ENTER, "");
	registerKey(SDLK_KP_1, "");
	registerKey(SDLK_KP_2, "");
	registerKey(SDLK_KP_3, "");
	registerKey(SDLK_KP_4, "");
	registerKey(SDLK_KP_5, "");
	registerKey(SDLK_KP_6, "");
	registerKey(SDLK_KP_7, "");
	registerKey(SDLK_KP_8, "");
	registerKey(SDLK_KP_9, "");
	registerKey(SDLK_KP_0, "");
	registerKey(SDLK_KP_PERIOD, "");
	registerKey(SDLK_APPLICATION, "");
	registerKey(SDLK_POWER, "");
	registerKey(SDLK_KP_EQUALS, "");
	registerKey(SDLK_F13, "");
	registerKey(SDLK_F14, "");
	registerKey(SDLK_F15, "");
	registerKey(SDLK_F16, "");
	registerKey(SDLK_F17, "");
	registerKey(SDLK_F18, "");
	registerKey(SDLK_F19, "");
	registerKey(SDLK_F20, "");
	registerKey(SDLK_F21, "");
	registerKey(SDLK_F22, "");
	registerKey(SDLK_F23, "");
	registerKey(SDLK_F24, "");
	registerKey(SDLK_EXECUTE, "");
	registerKey(SDLK_HELP, "");
	registerKey(SDLK_MENU, "");
	registerKey(SDLK_SELECT, "");
	registerKey(SDLK_STOP, "");
	registerKey(SDLK_AGAIN, "");
	registerKey(SDLK_UNDO, "");
	registerKey(SDLK_CUT, "");
	registerKey(SDLK_COPY, "");
	registerKey(SDLK_PASTE, "");
	registerKey(SDLK_FIND, "");
	registerKey(SDLK_MUTE, "");
	registerKey(SDLK_VOLUMEUP, "");
	registerKey(SDLK_VOLUMEDOWN, "");
	registerKey(SDLK_KP_COMMA, "");
	registerKey(SDLK_KP_EQUALSAS400, "");
	registerKey(SDLK_ALTERASE, "");
	registerKey(SDLK_SYSREQ, "");
	registerKey(SDLK_CANCEL, "");
	registerKey(SDLK_CLEAR, "");
	registerKey(SDLK_PRIOR, "");
	registerKey(SDLK_RETURN2, "");
	registerKey(SDLK_SEPARATOR, "");
	registerKey(SDLK_OUT, "");
	registerKey(SDLK_OPER, "");
	registerKey(SDLK_CLEARAGAIN, "");
	registerKey(SDLK_CRSEL, "");
	registerKey(SDLK_EXSEL, "");
	registerKey(SDLK_KP_00, "");
	registerKey(SDLK_KP_000, "");
	registerKey(SDLK_THOUSANDSSEPARATOR, "");
	registerKey(SDLK_DECIMALSEPARATOR, "");
	registerKey(SDLK_CURRENCYUNIT, "");
	registerKey(SDLK_CURRENCYSUBUNIT, "");
	registerKey(SDLK_KP_LEFTPAREN, "");
	registerKey(SDLK_KP_RIGHTPAREN, "");
	registerKey(SDLK_KP_LEFTBRACE, "");
	registerKey(SDLK_KP_RIGHTBRACE, "");
	registerKey(SDLK_KP_TAB, "");
	registerKey(SDLK_KP_BACKSPACE, "");
	registerKey(SDLK_KP_A, "");
	registerKey(SDLK_KP_B, "");
	registerKey(SDLK_KP_C, "");
	registerKey(SDLK_KP_D, "");
	registerKey(SDLK_KP_E, "");
	registerKey(SDLK_KP_F, "");
	registerKey(SDLK_KP_XOR, "");
	registerKey(SDLK_KP_POWER, "");
	registerKey(SDLK_KP_PERCENT, "");
	registerKey(SDLK_KP_LESS, "");
	registerKey(SDLK_KP_GREATER, "");
	registerKey(SDLK_KP_AMPERSAND, "");
	registerKey(SDLK_KP_DBLAMPERSAND, "");
	registerKey(SDLK_KP_VERTICALBAR, "");
	registerKey(SDLK_KP_DBLVERTICALBAR, "");
	registerKey(SDLK_KP_COLON, "");
	registerKey(SDLK_KP_HASH, "");
	registerKey(SDLK_KP_SPACE, "");
	registerKey(SDLK_KP_AT, "");
	registerKey(SDLK_KP_EXCLAM, "");
	registerKey(SDLK_KP_MEMSTORE, "");
	registerKey(SDLK_KP_MEMRECALL, "");
	registerKey(SDLK_KP_MEMCLEAR, "");
	registerKey(SDLK_KP_MEMADD, "");
	registerKey(SDLK_KP_MEMSUBTRACT, "");
	registerKey(SDLK_KP_MEMMULTIPLY, "");
	registerKey(SDLK_KP_MEMDIVIDE, "");
	registerKey(SDLK_KP_PLUSMINUS, "");
	registerKey(SDLK_KP_CLEAR, "");
	registerKey(SDLK_KP_CLEARENTRY, "");
	registerKey(SDLK_KP_BINARY, "");
	registerKey(SDLK_KP_OCTAL, "");
	registerKey(SDLK_KP_DECIMAL, "");
	registerKey(SDLK_KP_HEXADECIMAL, "");
	registerKey(SDLK_LCTRL, "");
	registerKey(SDLK_LSHIFT, "");
	registerKey(SDLK_LALT, "");
	registerKey(SDLK_LGUI, "");
	registerKey(SDLK_RCTRL, "");
	registerKey(SDLK_RSHIFT, "");
	registerKey(SDLK_RALT, "");
	registerKey(SDLK_RGUI, "");
	registerKey(SDLK_MODE, "");
	registerKey(SDLK_AUDIONEXT, "");
	registerKey(SDLK_AUDIOPREV, "");
	registerKey(SDLK_AUDIOSTOP, "");
	registerKey(SDLK_AUDIOPLAY, "");
	registerKey(SDLK_AUDIOMUTE, "");
	registerKey(SDLK_MEDIASELECT, "");
	registerKey(SDLK_WWW, "");
	registerKey(SDLK_MAIL, "");
	registerKey(SDLK_CALCULATOR, "");
	registerKey(SDLK_COMPUTER, "");
	registerKey(SDLK_AC_SEARCH, "");
	registerKey(SDLK_AC_HOME, "");
	registerKey(SDLK_AC_BACK, "");
	registerKey(SDLK_AC_FORWARD, "");
	registerKey(SDLK_AC_STOP, "");
	registerKey(SDLK_AC_REFRESH, "");
	registerKey(SDLK_AC_BOOKMARKS, "");
	registerKey(SDLK_BRIGHTNESSDOWN, "");
	registerKey(SDLK_BRIGHTNESSUP, "");
	registerKey(SDLK_DISPLAYSWITCH, "");
	registerKey(SDLK_KBDILLUMTOGGLE, "");
	registerKey(SDLK_KBDILLUMDOWN, "");
	registerKey(SDLK_KBDILLUMUP, "");
	registerKey(SDLK_EJECT, "");
	registerKey(SDLK_SLEEP, "");
	registerKey(SDLK_APP1, "");
	registerKey(SDLK_APP2, "");
	registerKey(SDLK_AUDIOREWIND, "");
	registerKey(SDLK_AUDIOFASTFORWARD, "");
}

void Input::loadActions(const std::string& path)
{

}

void Input::loadMappings(const std::string& path)
{

}

bool Input::isDown(const std::string& action)
{
	return actions[action].down;
}

void Input::update()
{
	SDL_PumpEvents();

	SDL_Event ev;

	while (SDL_PollEvent(&ev))
	{
		switch (ev.type)
		{
		case SDL_KEYDOWN:
		{
			for (auto& item : mapSdlToControl)
			{
				if (item.first == (u32)ev.key.keysym.sym)
				{
					controls[(u32)item.second] = true;
				}
			}

			break;
		}

		case SDL_KEYUP:
		{
			for (auto& item : mapSdlToControl)
			{
				if (item.first == (u32)ev.key.keysym.sym)
				{
					controls[(u32)item.second] = false;
				}
			}

			break;
		}
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
				if (pauseOnAppDeactivate)
					pauseGame = false;
				break;
			}
			case SDL_WINDOWEVENT_ENTER:
			{
				break;
			}
			case SDL_WINDOWEVENT_FOCUS_LOST:
			{
				if (pauseOnAppDeactivate)
					pauseGame = true;
				break;
			}
			case SDL_WINDOWEVENT_CLOSE:
			{
				exitGame = true;
				break;
			}
			default:
				break;
			}

			break;
		}

		case SDL_MOUSEBUTTONDOWN:
			mouseButtonDown[ev.button.button] = true;
			break;

		case SDL_MOUSEBUTTONUP:
			mouseButtonDown[ev.button.button] = false;
			break;

		case SDL_MOUSEMOTION:
		{
			windowMousePosition = { (f32)ev.motion.x, (f32)ev.motion.y };
			f32 scale = graphics->videoWidth / graphics->blittedRect.width;
			mousePosition.x = (windowMousePosition.x - graphics->blittedRect.x) * scale;
			mousePosition.y = (windowMousePosition.y - graphics->blittedRect.y) * scale;
			break;
		}
		default:
			break;
		}
	}
}

}
