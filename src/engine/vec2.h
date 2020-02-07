#pragma once
#include "types.h"
#include <string>

namespace engine
{
class Vec2
{
public:
	Vec2()
		: x(0.0f)
		, y(0.0f)
	{}

	Vec2(const Vec2& other)
	{
		x = other.x;
		y = other.y;
	}

	Vec2(f32 newX, f32 newY)
	{
		x = newX;
		y = newY;
	}

	Vec2(f32 value)
	{
		x = value;
		y = value;
	}

	inline f32 dot(const Vec2& other) const
	{
		return x * other.x + y * other.y;
	}

	inline void parse(const std::string& vecStr)
	{
		sscanf_s(vecStr.c_str(), "%f %f", &x, &y);
	}

	inline std::string toString() const
	{
		char buff[300] = { 0 };
		sprintf(buff, "%f %f", x, y);
		return buff;
	}

	void rotateAround(const Vec2& center, f32 angleRadians)
	{
		*this = {
			cosf(angleRadians) * (x - center.x) - sinf(angleRadians) * (y - center.y) + center.x,
			sinf(angleRadians) * (x - center.x) + cosf(angleRadians) * (y - center.y) + center.y
		};
	}

	f32 getDistance(const Vec2& other) const
	{
		f32 xx = x - other.x, yy = y - other.y;

		xx = xx * xx + yy * yy;

		if (xx <= 0.0f)
		{
			return 0.0f;
		}

		return (f32)sqrtf(xx);
	}

	inline void makeAbsolute()
	{
		x = fabs(x);
		y = fabs(y);
	}

	f32 getCos(const Vec2& other) const
	{
		f32 m = (x * x + y * y) * (other.x * other.x + other.y * other.y);

		if (m <= 0.0f)
		{
			return 0.0f;
		}

		return (f32)(x * other.x + y * other.y) / sqrt(m);
	}

	void normalize()
	{
		f32 m = x * x + y * y;

		if (m <= 0.0f)
		{
			x = y = 0.0f;
			return;
		}

		m = sqrtf(m);
		x /= m;
		y /= m;
	}

	Vec2 getNormalized() const
	{
		f32 m = x * x + y * y;
		Vec2 value = *this;

		if (m <= 0.0f)
		{
			return Vec2();
		}

		m = sqrtf((f32)m);
		value.x /= m;
		value.y /= m;

		return value;
	}

	Vec2& normalizeTo(Vec2& to)
	{
		f32 m = x * x + y * y;

		if (m <= 0.0f)
		{
			return *this;
		}

		m = sqrtf(m);
		to.x = x / m;
		to.y = y / m;

		return *this;
	}

	Vec2 getNegated() const
	{
		Vec2 value = *this;

		value.x = -x;
		value.y = -y;

		return value;
	}

	inline void negate()
	{
		x = -x;
		y = -y;
	}

	inline Vec2& negateTo(Vec2& to)
	{
		to.x = -x;
		to.y = -y;

		return *this;
	}

	bool isOnLine(const Vec2& lineA, const Vec2& lineB, f32 tolerance = 0.0001) const
	{
		f32 u1, u2;

		u1 = (lineB.x - lineA.x);
		u2 = (lineB.y - lineA.y);

		if (u1 == 0.0f)
			u1 = 1.0f;

		if (u2 == 0.0f)
			u2 = 1.0f;

		u1 = (x - lineA.x) / u1;
		u2 = (y - lineA.y) / u2;

		return (fabs(u1 - u2) <= tolerance);
	}

	inline bool isAlmosEqual(const Vec2& other, f32 tolerance = 0.001f) const
	{
		return fabs(x - other.x) <= tolerance
			&& fabs(y - other.y) <= tolerance;
	}

	inline f32 getLength() const
	{
		return sqrt(x * x + y * y);
	}

	inline f32 getSquaredLength() const
	{
		return x * x + y * y;
	}

	void setLength(f32 length)
	{
		f32 oldLen = getLength();

		if (oldLen < 0.0001f)
		{
			oldLen = 0.0001f;
		}

		f32 l = length / oldLen;

		x *= l;
		y *= l;
	}

	inline Vec2& set(f32 newX, f32 newY)
	{
		x = newX;
		y = newY;

		return *this;
	}

	inline Vec2& clear()
	{
		x = y = 0.0f;

		return *this;
	}

	inline f32 operator [](int index) const
	{
		if (index == 0)
		{
			return x;
		}
		else if (index == 1)
		{
			return y;
		}

		return 0;
	}

	inline Vec2& operator += (const Vec2& value)
	{
		x += value.x;
		y += value.y;

		return *this;
	}

	inline Vec2& operator *= (const Vec2& value)
	{
		x *= value.x;
		y *= value.y;

		return *this;
	}

	inline Vec2& operator -= (const Vec2& value)
	{
		x -= value.x;
		y -= value.y;

		return *this;
	}

	inline Vec2& operator /= (const Vec2& value)
	{
		x /= value.x;
		y /= value.y;

		return *this;
	}

	inline Vec2& operator += (f32 value)
	{
		x += value;
		y += value;

		return *this;
	}

	inline Vec2& operator *= (f32 value)
	{
		x *= value;
		y *= value;

		return *this;
	}

	inline Vec2& operator -= (f32 value)
	{
		x -= value;
		y -= value;

		return *this;
	}

	inline Vec2& operator /= (f32 value)
	{
		x /= value;
		y /= value;

		return *this;
	}

	inline Vec2 operator + (const Vec2& value) const
	{
		Vec2 result;

		result.x = x + value.x;
		result.y = y + value.y;

		return result;
	}

	inline Vec2 operator * (const Vec2& value) const
	{
		Vec2 result;

		result.x = x * value.x;
		result.y = y * value.y;

		return result;
	}

	inline Vec2 operator - (const Vec2& value) const
	{
		Vec2 result;

		result.x = x - value.x;
		result.y = y - value.y;

		return result;
	}

	inline Vec2 operator / (const Vec2& value) const
	{
		Vec2 result;

		result.x = x / value.x;
		result.y = y / value.y;

		return result;
	}

	inline Vec2 operator + (const f32 value) const
	{
		Vec2 result;

		result.x = x + value;
		result.y = y + value;

		return result;
	}

	inline Vec2 operator * (const f32 value) const
	{
		Vec2 result;

		result.x = x * value;
		result.y = y * value;

		return result;
	}

	inline Vec2 operator - (const f32 value) const
	{
		Vec2 result;

		result.x = x - value;
		result.y = y - value;

		return result;
	}

	inline Vec2 operator / (const f32 value) const
	{
		Vec2 result;

		result.x = x / value;
		result.y = y / value;

		return result;
	}

	inline Vec2 operator / (const size_t value) const
	{
		Vec2 result;

		result.x = x / (f32)value;
		result.y = y / (f32)value;

		return result;
	}

	inline Vec2& operator = (const f32 value)
	{
		x = value;
		y = value;

		return *this;
	}

	inline Vec2& operator = (const i32 value)
	{
		x = (f32)value;
		y = (f32)value;

		return *this;
	}

	inline bool operator <= (const Vec2& other) const
	{
		return x <= other.x
			&& y <= other.y;
	}

	inline bool operator >= (const Vec2& other) const
	{
		return x >= other.x
			&& y >= other.y;
	}

	inline bool operator < (const Vec2& other) const
	{
		return x < other.x
			&& y < other.y;
	}

	inline bool operator > (const Vec2& other) const
	{
		return x > other.x
			&& y > other.y;
	}

	inline bool operator != (const Vec2& other) const
	{
		return x != other.x
			|| y != other.y;
	}

	inline Vec2& operator = (const Vec2& other)
	{
		x = other.x;
		y = other.y;

		return *this;
	}

	f32 x, y;
};

}
