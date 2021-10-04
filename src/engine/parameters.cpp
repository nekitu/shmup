#include "parameters.h"

namespace engine
{
bool Parameters::load(Json::Value& json)
{
	Parameter p;

	for (int j = 0; j < json.getMemberNames().size(); j++)
	{
		auto paramName = json.getMemberNames()[j];
		auto paramJson = json[paramName];

		switch (paramJson.type())
		{
		case Json::ValueType::intValue:
			p.type = Parameter::Type::Int;
			p.value.intVal = paramJson.asInt();
			break;
		case Json::ValueType::booleanValue:
			p.type = Parameter::Type::Bool;
			p.value.boolVal = paramJson.asBool();
			break;
		case Json::ValueType::realValue:
			p.type = Parameter::Type::Float;
			p.value.floatVal = paramJson.asFloat();
			break;
		case Json::ValueType::stringValue:
			p.type = Parameter::Type::String;
			p.stringVal = paramJson.asString();
			break;
		}

		parameters.insert(std::make_pair(paramName, p));
	}

	return true;
}

int Parameters::getInt(const std::string& name, int defaultValue)
{
	auto iter = parameters.find(name);
	if (iter == parameters.end()) return defaultValue;

	if (iter->second.type == Parameter::Type::Float)
		return iter->second.value.floatVal;

	return iter->second.value.intVal;
}

bool Parameters::getBool(const std::string& name, bool defaultValue)
{
	auto iter = parameters.find(name);
	if (iter == parameters.end()) return defaultValue;
	return iter->second.value.boolVal;
}

f32 Parameters::getFloat(const std::string& name, f32 defaultValue)
{
	auto iter = parameters.find(name);
	if (iter == parameters.end()) return defaultValue;

	if (iter->second.type == Parameter::Type::Int)
		return iter->second.value.intVal;

	return iter->second.value.floatVal;
}

std::string Parameters::getString(const std::string& name, const std::string& defaultValue)
{
	auto iter = parameters.find(name);
	if (iter == parameters.end()) return defaultValue;
	return iter->second.stringVal;
}

Vec2 Parameters::getVec2(const std::string& name, const Vec2& defaultValue)
{
	auto iter = parameters.find(name);
	if (iter == parameters.end()) return defaultValue;

	if (!iter->second.wasParsed)
	{
		iter->second.vec2Val.parse(iter->second.stringVal);
		iter->second.wasParsed = true;
	}

	return iter->second.vec2Val;
}

Color Parameters::getColor(const std::string& name, const Color& defaultValue)
{
	auto iter = parameters.find(name);
	if (iter == parameters.end()) return defaultValue;

	if (!iter->second.wasParsed)
	{
		iter->second.colorVal.parse(iter->second.stringVal);
		iter->second.wasParsed = true;
	}

	return iter->second.colorVal;
}

}
