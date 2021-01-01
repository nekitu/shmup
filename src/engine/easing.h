#pragma once
#include "types.h"

namespace engine
{
// Copyright (C) Robert Penner's Easing Functions
// http://www.robertpenner.com/easing/

// Function parameter meaning:
// t - is the current time(or position) of the tween.This can be seconds or frames, steps, seconds, ms, whatever – as long as the unit is the same as is used for the total time[3].
// b - is the beginning value of the property.
// c - is the change between the beginning and destination value of the property.
// d - is the total time of the tween.

typedef f32 (*EaseFunction)(f32 t, f32 b, f32 c, f32 d);

struct Easing
{
enum class Type
{
	inLinear,
	outLinear,
	inOutLinear,
	inBack,
	outBack,
	inOutBack,
	inBounce,
	outBounce,
	inOutBounce,
	inCirc,
	outCirc,
	inOutCirc,
	inCubic,
	outCubic,
	inOutCubic,
	inElastic,
	outElastic,
	inOutElastic,
	inExpo,
	outExpo,
	inOutExpo,
	inQuad,
	outQuad,
	inOutQuad,
	inQuart,
	outQuart,
	inOutQuart,
	inQuint,
	outQuint,
	inOutQuint,
	inSine,
	outSine,
	inOutSine
};

static f32 inLinear(f32 t, f32 b, f32 c, f32 d);
static f32 outLinear(f32 t, f32 b, f32 c, f32 d);
static f32 inOutLinear(f32 t, f32 b, f32 c, f32 d);

static f32 inBack(f32 t, f32 b, f32 c, f32 d);
static f32 outBack(f32 t, f32 b, f32 c, f32 d);
static f32 inOutBack(f32 t, f32 b, f32 c, f32 d);

static f32 inBounce(f32 t, f32 b, f32 c, f32 d);
static f32 outBounce(f32 t, f32 b, f32 c, f32 d);
static f32 inOutBounce(f32 t, f32 b, f32 c, f32 d);

static f32 inCirc(f32 t, f32 b, f32 c, f32 d);
static f32 outCirc(f32 t, f32 b, f32 c, f32 d);
static f32 inOutCirc(f32 t, f32 b, f32 c, f32 d);

static f32 inCubic(f32 t, f32 b, f32 c, f32 d);
static f32 outCubic(f32 t, f32 b, f32 c, f32 d);
static f32 inOutCubic(f32 t, f32 b, f32 c, f32 d);

static f32 inElastic(f32 t, f32 b, f32 c, f32 d);
static f32 outElastic(f32 t, f32 b, f32 c, f32 d);
static f32 inOutElastic(f32 t, f32 b, f32 c, f32 d);

static f32 inExpo(f32 t, f32 b, f32 c, f32 d);
static f32 outExpo(f32 t, f32 b, f32 c, f32 d);
static f32 inOutExpo(f32 t, f32 b, f32 c, f32 d);

static f32 inQuad(f32 t, f32 b, f32 c, f32 d);
static f32 outQuad(f32 t, f32 b, f32 c, f32 d);
static f32 inOutQuad(f32 t, f32 b, f32 c, f32 d);

static f32 inQuart(f32 t, f32 b, f32 c, f32 d);
static f32 outQuart(f32 t, f32 b, f32 c, f32 d);
static f32 inOutQuart(f32 t, f32 b, f32 c, f32 d);

static f32 inQuint(f32 t, f32 b, f32 c, f32 d);
static f32 outQuint(f32 t, f32 b, f32 c, f32 d);
static f32 inOutQuint(f32 t, f32 b, f32 c, f32 d);

static f32 inSine(f32 t, f32 b, f32 c, f32 d);
static f32 outSine(f32 t, f32 b, f32 c, f32 d);
static f32 inOutSine(f32 t, f32 b, f32 c, f32 d);

inline static f32 easeValue(Type type, f32 t, f32 b, f32 c, f32 d)
{
	switch (type)
	{
	case Type::inLinear: return inLinear(t, b, c, d); break;
	case Type::outLinear: return outLinear(t, b, c, d); break;
	case Type::inOutLinear: return inOutLinear(t, b, c, d); break;
	case Type::inBack: return inBack(t, b, c, d); break;
	case Type::outBack: return outBack(t, b, c, d); break;
	case Type::inOutBack: return inOutBack(t, b, c, d); break;
	case Type::inBounce: return inBounce(t, b, c, d); break;
	case Type::outBounce: return outBounce(t, b, c, d); break;
	case Type::inOutBounce: return inOutBounce(t, b, c, d); break;
	case Type::inCirc: return inCirc(t, b, c, d); break;
	case Type::outCirc: return outCirc(t, b, c, d); break;
	case Type::inOutCirc: return inOutCirc(t, b, c, d); break;
	case Type::inCubic: return inCubic(t, b, c, d); break;
	case Type::outCubic: return outCubic(t, b, c, d); break;
	case Type::inOutCubic: return inOutCubic(t, b, c, d); break;
	case Type::inElastic: return inElastic(t, b, c, d); break;
	case Type::outElastic: return outElastic(t, b, c, d); break;
	case Type::inOutElastic: return inOutElastic(t, b, c, d); break;
	case Type::inExpo: return inExpo(t, b, c, d); break;
	case Type::outExpo: return outExpo(t, b, c, d); break;
	case Type::inOutExpo: return inOutExpo(t, b, c, d); break;
	case Type::inQuad: return inQuad(t, b, c, d); break;
	case Type::outQuad: return outQuad(t, b, c, d); break;
	case Type::inOutQuad: return inOutQuad(t, b, c, d); break;
	case Type::inQuart: return inQuart(t, b, c, d); break;
	case Type::outQuart: return outQuart(t, b, c, d); break;
	case Type::inOutQuart: return inOutQuart(t, b, c, d); break;
	case Type::inQuint: return inQuint(t, b, c, d); break;
	case Type::outQuint: return outQuint(t, b, c, d); break;
	case Type::inOutQuint: return inOutQuint(t, b, c, d); break;
	case Type::inSine: return inSine(t, b, c, d); break;
	case Type::outSine: return outSine(t, b, c, d); break;
	case Type::inOutSine: return inOutSine(t, b, c, d); break;
	};

	return 0.0f;
}

inline static Type getTypeFromString(const std::string& name)
{
	if (name == "inLinear") return Type::inLinear;
	if (name == "outLinear") return Type::outLinear;
	if (name == "inOutLinear") return Type::inOutLinear;
	if (name == "inBack") return Type::inBack;
	if (name == "outBack") return Type::outBack;
	if (name == "inOutBack") return Type::inOutBack;
	if (name == "inBounce") return Type::inBounce;
	if (name == "outBounce") return Type::outBounce;
	if (name == "inOutBounce") return Type::inOutBounce;
	if (name == "inCirc") return Type::inCirc;
	if (name == "outCirc") return Type::outCirc;
	if (name == "inOutCirc") return Type::inOutCirc;
	if (name == "inCubic") return Type::inCubic;
	if (name == "outCubic") return Type::outCubic;
	if (name == "inOutCubic") return Type::inOutCubic;
	if (name == "inElastic") return Type::inElastic;
	if (name == "outElastic") return Type::outElastic;
	if (name == "inOutElastic") return Type::inOutElastic;
	if (name == "inExpo") return Type::inExpo;
	if (name == "outExpo") return Type::outExpo;
	if (name == "inOutExpo") return Type::inOutExpo;
	if (name == "inQuad") return Type::inQuad;
	if (name == "outQuad") return Type::outQuad;
	if (name == "inOutQuad") return Type::inOutQuad;
	if (name == "inQuart") return Type::inQuart;
	if (name == "outQuart") return Type::outQuart;
	if (name == "inOutQuart") return Type::inOutQuart;
	if (name == "inQuint") return Type::inQuint;
	if (name == "outQuint") return Type::outQuint;
	if (name == "inOutQuint") return Type::inOutQuint;
	if (name == "inSine") return Type::inSine;
	if (name == "outSine") return Type::outSine;
	if (name == "inOutSine") return Type::inOutSine;
	return Type::inLinear;
}

};

}
