#include "utils.h"
#include "libs/utf8/source/utf8.h"

namespace engine
{
static u32 mathRandomSeed = 500;

bool clampValue(f32& value, f32 minVal, f32 maxVal)
{
	if (value < minVal)
	{
		value = minVal;
		return true;
	}

	if (value > maxVal)
	{
		value = maxVal;
		return true;
	}

	return false;
}

void setRandomSeed(i32 seed)
{
	mathRandomSeed = seed;
	srand(seed);
}

i32 getRandomSeed()
{
	return mathRandomSeed;
}

i32 randomInteger(i32 max)
{
	const i32 a = 809;
	const i32 c = 13;
	const i32 m = 1993;

	return (i32)(mathRandomSeed = ((a * mathRandomSeed + c) % m)) % max;
}

f32 randomFloat(f32 low, f32 high)
{
	return (((f32)(rand() & 0x7fff) / (f32)0x7fff) * (high - low) + low);
}

f32 randomNormalizedFloat()
{
	return (f32)rand() / (f32)RAND_MAX;
}

bool utf8ToUtf32(const char* text, UnicodeString& outText)
{
	if (!text)
		return false;

	outText.clear();

	try
	{
		utf8::utf8to32(
			(char*)text,
			(char*)(text + strlen(text)),
			std::back_inserter(outText));
	}

	catch (utf8::invalid_utf8 ex)
	{
		return false;
	}

	return true;
}

}