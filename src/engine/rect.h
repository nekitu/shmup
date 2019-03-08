#pragma once
#include "types.h"
#include "vec2.h"

namespace engine
{
struct Rect
{
    f32 x = 0, y = 0, width = 0, height = 0;

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

    inline Rect expand(f32 amount)
    {
        return {
            x - amount,
            y - amount,
            width + 2.0f * amount,
            height + 2.0f * amount
        };
    }

    inline Rect contract(f32 amount)
    {
        return expand(-amount);
    }

	inline void add(const Rect& rc)
	{
		if (rc.x < x) {	auto r = right(); x = rc.x; width = r - rc.x; }
		if (rc.y < y) { auto b = bottom(); y = rc.y; height = b - rc.y; }
		if (rc.right() > right()) width = rc.right() - x;
		if (rc.bottom() > bottom()) height = rc.bottom() - y;
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