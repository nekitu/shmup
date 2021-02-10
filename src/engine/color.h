#pragma once
#include "types.h"
#include <string>
#include <unordered_map>

namespace engine
{
#define packRGB(r,g,b) (u32)((u8)b<<16|(u8)g<<8|(u8)r)
#define packRGBA(r,g,b,a) (u32)((u8)a<<24|(u8)b<<16|(u8)g<<8|(u8)r)

struct Color
{
	Color() {}
	Color(u32 color)
	{
		setFromRgba(color);
	}

	Color(f32 R, f32 G, f32 B, f32 A)
		: r(R), g(G), b(B), a(A)
	{}

	u32 getRgba() const;
	u32 getArgb() const;
	void parse(const std::string& colorStr);
	std::string toString() const;
	void setFromRgba(u32 value)
	{
		u8 *color = (u8*)&value;

		r = (f32)color[0] / 255.0f;
		g = (f32)color[1] / 255.0f;
		b = (f32)color[2] / 255.0f;
		a = (f32)color[3] / 255.0f;
	}

	Color operator * (f32 other) const
	{
		return { r * other, g * other, b * other, a * other };
	}

	Color operator * (const Color& other) const
	{
		return { r * other.r, g * other.g, b * other.b, a * other.a };
	}

	Color operator - (const Color& other) const
	{
		return { r - other.r, g - other.g, b - other.b, a - other.a };
	}

	Color operator + (const Color& other) const
	{
		return { r + other.r, g + other.g, b + other.b, a + other.a };
	}

	static Color random();
	static const Color transparent;
	static const Color white;
	static const Color black;
	static const Color red;
	static const Color darkRed;
	static const Color veryDarkRed;
	static const Color green;
	static const Color darkGreen;
	static const Color veryDarkGreen;
	static const Color blue;
	static const Color darkBlue;
	static const Color veryDarkBlue;
	static const Color yellow;
	static const Color darkYellow;
	static const Color veryDarkYellow;
	static const Color magenta;
	static const Color cyan;
	static const Color darkCyan;
	static const Color veryDarkCyan;
	static const Color orange;
	static const Color darkOrange;
	static const Color darkGray;
	static const Color gray;
	static const Color lightGray;
	static const Color sky;

	f32 r = 0.0f, g = 0.0f, b = 0.0f, a = 1.0f;
};

}
