#pragma once
#include "types.h"
#include "vec2.h"

namespace engine
{
struct Rect
{
    f32 x = 0, y = 0, width = 0, height = 0;

	enum class ClipType
	{
		Outside = BIT(0),
		IsEqual = BIT(1),
		IsContained = BIT(2),
		Contains = BIT(3),
		Clips = BIT(4)
	};

    Rect()
        : x(0)
        , y(0)
        , width(0)
        , height(0)
    {}

    Rect(f32 newX, f32 newY, f32 newWidth, f32 newHeight)
    {
        x = newX;
        y = newY;
        width = newWidth;
        height = newHeight;
    }

    void set(f32 newX, f32 newY, f32 newWidth, f32 newHeight)
    {
        x = newX;
        y = newY;
        width = newWidth;
        height = newHeight;
    }

    inline bool isZero() const
    {
        return x == 0.0f && y == 0.0f && width == 0.0f && height == 0.0f;
    }

	void parse(const std::string& str)
	{
		sscanf(str.c_str(), "%f %f %f %f", &x, &y, &width, &height);
	}

	void setPosition(const Vec2& pos)
	{
		x = pos.x;
		y = pos.y;
	}

	void setSize(const Vec2& size)
	{
		width = size.x;
		height = size.y;
	}

	std::string toString() const
	{
		char buff[300] = { 0 };
		sprintf(buff, "%f %f %f %f", x, y, width, height);
		return buff;
	}

    inline f32 left() const { return x; }
    inline f32 top() const { return y; }
    inline f32 right() const { return x + width; }
    inline f32 bottom() const { return y + height; }
    inline Vec2 topLeft() const { return Vec2(x, y); }
    inline Vec2 topRight() const { return Vec2(x + width, y); }
    inline Vec2 bottomLeft() const { return Vec2(x, y + height); }
    inline Vec2 bottomRight() const { return Vec2(x + width, y + height); }
    inline Vec2 center() const { return Vec2(x + width / 2, y + height / 2); }
    inline bool contains(const Vec2& pt) const
    {
        return pt.x >= x && pt.x < (x + width) && pt.y >= y && (pt.y < y + height);
    }

    inline bool contains(f32 X, f32 Y) const
    {
        return X >= x && X < (x + width) && Y >= y && Y < (y + height);
    }

    inline bool contains(const Rect& other) const
    {
        return x <= other.x && (other.right()) < right()
            && y <= other.y && (other.bottom()) < bottom();
    }

    inline bool outside(const Rect& other) const
    {
        return (x > other.right() || right() < other.x)
            || (y > other.bottom() || bottom() < other.y);
    }

    Rect clipInside(const Rect& parentRect) const
    {
        Rect newRect = *this;

        if (parentRect.x > newRect.x)
        {
            newRect.width -= parentRect.x - newRect.x;
            newRect.x = parentRect.x;
        }

        if (parentRect.y > newRect.y)
        {
            newRect.height -= parentRect.y - newRect.y;
            newRect.y = parentRect.y;
        }

        if (parentRect.right() < right())
        {
            newRect.width -= right() - parentRect.right();
        }

        if (parentRect.bottom() < bottom())
        {
            newRect.height -= bottom() - parentRect.bottom();
        }

        if (newRect.width < 0) newRect.width = 0;
        if (newRect.height < 0) newRect.height = 0;

        return newRect;
    }

	ClipType getBoxLineTest(f32 v0, f32 v1, f32 w0, f32 w1)
	{
		if ((v1 < w0) || (v0 > w1))
			return ClipType::Outside;
		else if ((v0 == w0) && (v1 == w1))
			return ClipType::IsEqual;
		else if ((v0 >= w0) && (v1 <= w1))
			return ClipType::IsContained;
		else if ((v0 <= w0) && (v1 >= w1))
			return ClipType::Contains;
		else
			return ClipType::Clips;
	}

	bool overlaps(const Rect& b)
	{
		return (x <= b.right() &&
			b.x <= right() &&
			y <= b.bottom() &&
			b.y <= bottom());
	}

	ClipType intersect2(const Rect& rc)
	{
		u32 andCode = 0xFFFF;
		u32 orCode = 0;
		u32 cx, cy;

		cx = (u32)getBoxLineTest(x, right(), rc.x, rc.right());
		andCode &= cx;
		orCode |= cx;

		cy = (u32)getBoxLineTest(y, bottom(), rc.y, rc.bottom());
		andCode &= cy;
		orCode |= cy;

		if (orCode == 0)
		{
			return ClipType::Outside;
		}
		else if (andCode != 0)
		{
			return (ClipType)andCode;
		}
		else
		{
			if (cx && cy)
				return ClipType::Clips;
			else
				return ClipType::Outside;
		}
	}

    inline Rect getExpanded(f32 amount)
    {
        return {
            x - amount,
            y - amount,
            width + 2.0f * amount,
            height + 2.0f * amount
        };
    }

	inline Rect getCenterScaled(f32 amount)
	{
		auto c = center();

		return {
			c.x - width * amount / 2.0f,
			c.y - height * amount / 2.0f,
			width * amount,
			height * amount
		};
	}

    inline Rect getContracted(f32 amount)
    {
        return getExpanded(-amount);
    }

	inline void add(const Rect& rc)
	{
		if (rc.x < x) {	auto r = right(); x = rc.x; width = r - rc.x; }
		if (rc.y < y) { auto b = bottom(); y = rc.y; height = b - rc.y; }
		if (rc.right() > right()) width = rc.right() - x;
		if (rc.bottom() > bottom()) height = rc.bottom() - y;
	}

	inline void add(const Vec2& pt)
	{
		if (pt.x < x) { auto r = right(); x = pt.x; width = r - pt.x; }
		if (pt.y < y) { auto b = bottom(); y = pt.y; height = b - pt.y; }
		if (pt.x > right()) width = pt.x - x;
		if (pt.y > bottom()) height = pt.y - y;
	}

	inline Rect operator + (const Vec2& pt) const
    {
        return { x + pt.x, y + pt.y, width, height };
    }

    inline Rect& operator += (const Vec2& pt)
    {
        x += pt.x;
        y += pt.y;
        return *this;
    }

    inline Rect operator - (const Vec2& pt) const
    {
        return { x - pt.x, y - pt.y, width, height };
    }

	inline Rect operator * (const f32 s) const
	{
		return { x, y, width * s, height * s };
	}

	inline Rect operator / (const f32 s) const
	{
		return { x, y, width / s, height / s };
	}

	inline Rect& operator -= (const Vec2& pt)
    {
        x -= pt.x;
        y -= pt.y;
        return *this;
    }

    inline bool operator != (const Rect& other) const
    {
        constexpr f32 epsilon = 0.00001f;
        return fabs(x - other.x) > epsilon
            || fabs(y - other.y) > epsilon
            || fabs(width - other.width) > epsilon
            || fabs(height - other.height) > epsilon;
    }
};
}
