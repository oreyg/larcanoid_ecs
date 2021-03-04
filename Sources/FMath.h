#pragma once
#include <type_traits>
#include <math.h>

struct Vector2
{
	float x{};
	float y{};
};

// Convenience operators for Vector2

constexpr Vector2 operator*(const Vector2& v, const float f)
{
	return { v.x * f, v.y * f };
}

constexpr Vector2 operator*(const float f, const Vector2& v)
{
	return v * f;
}

constexpr Vector2 operator*(const Vector2& v1, const Vector2& v2)
{
	return { v1.x * v2.x, v1.y * v2.y };
}

constexpr Vector2 operator/(const Vector2& v, const float f)
{
	return { v.x / f, v.y / f };
}

constexpr Vector2 operator/(const float f, const Vector2& v)
{
	return { f / v.x, f / v.y};
}

constexpr Vector2 operator-(const Vector2& v1, const Vector2& v2)
{
	return { v1.x - v2.x, v1.y - v2.y };
}

constexpr Vector2 operator+(const Vector2& v1, const Vector2& v2)
{
	return { v1.x + v2.x, v1.y + v2.y };
}

// Axis-Aligned rectangle (min, max)
struct Bounds
{
	Vector2 min;
	Vector2 max;
};

// Axis-Aligned rectangle (center, size)
struct Rect
{
	Vector2 position;
	Vector2 dimensions;
};

// Circle (center, radius)
struct Circle
{
	Vector2 position;
	float   radius;
};

namespace fmath
{
	constexpr float PI = 3.14159265359f;
	constexpr float conv_to_rad = PI / 180.0f;

	template<class t>
	constexpr t min(t x1, t x2)
	{
		static_assert(std::is_arithmetic_v<t>, "fmath::max(x1, x2) expects arithmetic type");
		return x1 < x2 ? x1 : x2;
	}

	template<class t>
	constexpr t max(t x1, t x2)
	{
		static_assert(std::is_arithmetic_v<t>, "fmath::max(x1, x2) expects arithmetic type");
		return x1 > x2 ? x1 : x2;
	}

	template<class t>
	constexpr t clamp(t x, t xmin, t xmax)
	{
		static_assert(std::is_arithmetic_v<t>, "fmath::max(x1, x2) expects arithmetic type");
		return min(max(x, xmin), xmax);
	}

	constexpr Bounds rect_to_bounds(const Rect& rect)
	{
		const float half_width  = rect.dimensions.x / 2.0f;
		const float half_height = rect.dimensions.y / 2.0f;
		return {
			{ rect.position.x - half_width, rect.position.y - half_height, },
			{ rect.position.x + half_width, rect.position.y + half_height  }
		};
	}

	constexpr Rect circle_to_rect(const Circle& c)
	{
		return {
			c.position,
			{ c.radius * 2, c.radius * 2 }
		};
	}

	constexpr float magnitude_sqr(const Vector2& v)
	{
		return v.x * v.x + v.y * v.y;
	}

	inline Vector2 rotated(const Vector2& v, const float angle) {
		float sin = sinf(angle);
		float cos = cosf(angle);
		float tx = v.x;
		float ty = v.y;
		return {
			(cos * tx) - (sin * ty),
			(sin * tx) + (cos * ty)
		};
	}

	inline float magnitude(const Vector2& v)
	{
		return sqrtf(magnitude_sqr(v));
	}

	inline Vector2 proj_to_hemi(const float max, const float delta_x, const float x)
	{
		const float p = delta_x / x * 2.0f;
		const float a = fmath::clamp(p * max, -max, max);
		return fmath::rotated(Vector2{ 0.0f, -1 }, -a);
	}

	constexpr bool has_intersection(const Circle& c, const Bounds& b)
	{
		const float dx = max(b.min.x, min(c.position.x, b.max.x)) - c.position.x;
		const float dy = max(b.min.y, min(c.position.y, b.max.y)) - c.position.y;
		return (dx * dx + dy * dy) <= c.radius * c.radius;
	}

	constexpr bool has_intersection(const Circle& c, const Rect& r)
	{
		return has_intersection(c, rect_to_bounds(r));
	}

	constexpr bool has_intersection(const Bounds& b1, const Bounds& b2)
	{
		return (b1.min.x <= b2.max.x) && (b1.max.x >= b2.min.x) &&
			   (b1.min.y <= b2.max.y) && (b1.max.y >= b2.min.y);
	}

	constexpr bool has_intersection(const Rect& r1, const Rect& r2)
	{
		return has_intersection(rect_to_bounds(r1), rect_to_bounds(r2));
	}

}