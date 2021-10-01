#pragma once
#include "types.h"
#include "color.h"
#include "vec2.h"
#include <string>
#include <unordered_map>
#include "json/value.h"

namespace engine
{
struct Parameters
{
	union ParamValue
	{
		int intVal;
		bool boolVal;
		f32 floatVal;

		ParamValue() {}
		~ParamValue() {}
	};

	struct Parameter
	{
		enum class Type
		{
			Int,
			Float,
			Bool,
			String,
			Vec2,
			Color,

			Count
		};

		Type type;
		ParamValue value;
		bool wasParsed = false;
		std::string stringVal;
		Vec2 vec2Val;
		Color colorVal;

		Parameter() { type = Type::String; }
		~Parameter() {}
	};

	std::unordered_map<std::string /*param name*/, Parameter> parameters;

	bool load(Json::Value& json);
	int getInt(const std::string& name, int defaultValue);
	bool getBool(const std::string& name, bool defaultValue);
	f32 getFloat(const std::string& name, f32 defaultValue);
	std::string getString(const std::string& name, const std::string& defaultValue);
	Vec2 getVec2(const std::string& name, const Vec2& defaultValue);
	Color getColor(const std::string& name, const Color& defaultValue);
};

}
