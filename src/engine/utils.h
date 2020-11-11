#pragma once
#include "types.h"
#include "json/value.h"
#include "vec2.h"

namespace engine
{
	template <typename T> T sgn(T val) { return (T(0) < val) - (val < T(0)); }

	struct Timer
	{
		f32 time = 0;
		f32 triggerTime = 1.0f;
		f32 speed = 1.0f;
		bool triggered = false, ongoing = false;

		bool update(f32 deltaTime)
		{
			if (triggered) return true;
			if (!ongoing) return false;

			time += deltaTime * speed;

			if (time >= triggerTime)
			{
				triggered = true;
				ongoing = false;
			}

			return triggered;
		}

		void reset() { triggered = false; time = 0; }
		void start() { ongoing = true; }
	};

	bool clampValue(f32& value, f32 minVal, f32 maxVal);
	void setRandomSeed(i32 seed);
	inline f32 lerp(f32 a, f32 b, f32 t) { return a + t * (b - a); }
	i32 getRandomSeed();
	i32 randomInteger(i32 max);
	f32 randomFloat(f32 low, f32 high);
	f32 randomNormalizedFloat(); // 0.0f -> 1.0f range
	bool utf8ToUtf32(const char* text, UnicodeString& outText);
	u32 singleUtf8ToUtf32(const char* glyph);
	bool loadJson(const std::string& filename, Json::Value& json);
	std::string readTextFile(const std::string& path);
	f32 deg2rad(f32 deg);
	f32 rad2deg(f32 rad);
	f32 dir2deg(const Vec2& dir);
	void replaceAll(std::string& source, const std::string& from, const std::string& to);
	std::string getParentPath(const std::string& path);
}
