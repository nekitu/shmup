#include "utils.h"
#include "libs/utf8/source/utf8.h"
#include "json/reader.h"
#include "json/writer.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <cstring>
#include <filesystem>

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

u32 singleUtf8ToUtf32(const char* glyph)
{
	u32 out;

	try
	{
		out = utf8::peek_next(
			(char*)glyph,
			(char*)(glyph + strlen(glyph)));
	}

	catch (utf8::invalid_utf8 ex)
	{
		return 0;
	}

	return out;
}

bool loadJson(const std::string& absPath, Json::Value& root)
{
	LOG_INFO("Loading \"{0}\"", absPath);
	Json::Reader reader;
	auto json = readTextFile(absPath);

	if (json.empty())
	{
		LOG_ERROR("Empty JSON file {0}", absPath);
		return false;
	}

	bool ok = reader.parse(json, root);

	if (!ok)
	{
		LOG_ERROR("JSON error in '{0}' {1}", absPath, reader.getFormatedErrorMessages());
		return false;
	}

	return true;
}

std::string readTextFile(const std::string& path)
{
	FILE* file = fopen(path.c_str(), "rb");

	if (!file)
		return std::string("");

	fseek(file, 0, SEEK_END);
	long size = ftell(file);
	std::string text;

	if (size != -1)
	{
		fseek(file, 0, SEEK_SET);

		char* buffer = new char[size + 1];
		buffer[size] = 0;

		if (fread(buffer, 1, size, file) == (unsigned long)size)
			text = buffer;

		delete[] buffer;
	}

	fclose(file);

	return text;
}

bool writeTextFile(const std::string& path, const std::string& text)
{
	FILE* file = fopen(path.c_str(), "wb");

	if (!file)
		return false;

	auto written = fwrite(text.data(), 1, text.size(), file);
	fclose(file);

	return written == text.size();
}

f32 deg2rad(f32 deg) { return deg * M_PI / 180.f; }
f32 rad2deg(f32 rad) { return rad * 180.f / M_PI; }
f32 dir2deg(const Vec2& dir)
{
	return rad2deg(atan2f(dir.x, dir.y));
}

void replaceAll(std::string& source, const std::string& from, const std::string& to)
{
	std::string newString;
	newString.reserve(source.length());

	std::string::size_type lastPos = 0;
	std::string::size_type findPos;

	while (std::string::npos != (findPos = source.find(from, lastPos)))
	{
		newString.append(source, lastPos, findPos - lastPos);
		newString += to;
		lastPos = findPos + from.length();
	}

	newString += source.substr(lastPos);

	source.swap(newString);
}

std::string getParentPath(const std::string& path)
{
	std::filesystem::path p(path);

	return p.parent_path().generic_u8string();
}

std::string jsonAsString(const Json::Value& json)
{
	std::string result;
	Json::StreamWriterBuilder wbuilder;

	wbuilder["indentation"] = ""; // Optional
	result = Json::writeString(wbuilder, json);
	return result;
}

}
