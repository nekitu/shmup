#pragma once
#include "types.h"

namespace engine
{
	template <typename T> T sgn(T val) { return (T(0) < val) - (val < T(0)); }

	bool clampValue(f32& value, f32 minVal, f32 maxVal);
	void setRandomSeed(i32 seed);
	i32 getRandomSeed();
	i32 randomInteger(i32 max);
	f32 randomFloat(f32 low, f32 high);
	f32 randomNormalizedFloat(); // 0.0f -> 1.0f range
	bool utf8ToUtf32(const char* text, UnicodeString& outText);
}