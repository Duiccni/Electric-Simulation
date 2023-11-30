#pragma once

#define _CRT_SECURE_NO_WARNINGS
#include <utility>

#define clamp(x, a, b) (x < a ? a : (x > b ? b : x))

typedef struct {
	int x, y;
} Point;

constexpr Point pZero = { 0, 0 };
constexpr Point pOne = { 1, 1 };
constexpr Point pForward = { 0, 1 };

Point operator+(const Point& a, const Point& b)
{
	return { a.x + b.x, a.y + b.y };
}

Point operator-(const Point& a, const Point& b)
{
	return { a.x - b.x, a.y - b.y };
}

Point operator-(const Point& a)
{
	return { -a.x, -a.y };
}

Point operator*(const Point& a, const Point& b)
{
	return { a.x * b.x, a.y * b.y };
}

Point operator/(const Point& a, const Point& b)
{
	return { a.x / b.x, a.y / b.y };
}

Point operator%(const Point& a, const Point& b)
{
	return { a.x % b.x, a.y % b.y };
}

Point operator+(const Point& a, const int& b)
{
	return { a.x + b, a.y + b };
}

Point operator-(const Point& a, const int& b)
{
	return { a.x - b, a.y - b };
}

Point operator*(const Point& a, const int& b)
{
	return { a.x * b, a.y * b };
}

Point operator/(const Point& a, const int& b)
{
	return { a.x / b, a.y / b };
}

Point operator<<(const Point& a, const int& b)
{
	return { a.x << b, a.y << b };
}

Point operator>>(const Point& a, const int& b)
{
	return { a.x >> b, a.y >> b };
}

Point operator%(const Point& a, const int& b)
{
	return { a.x % b, a.y % b };
}

bool operator==(const Point& a, const Point& b)
{
	return a.x == b.x && a.y == b.y;
}

bool operator!=(const Point& a, const Point& b)
{
	return a.x != b.x || a.y != b.y;
}

void operator+=(Point& a, const Point& b)
{
	a.x += b.x;
	a.y += b.y;
}

void operator+=(Point& a, const int& b)
{
	a.x += b;
	a.y += b;
}

void operator-=(Point& a, const Point& b)
{
	a.x -= b.x;
	a.y -= b.y;
}

void operator-=(Point& a, const int& b)
{
	a.x -= b;
	a.y -= b;
}

void operator*=(Point& a, const Point& b)
{
	a.x *= b.x;
	a.y *= b.y;
}

void operator/=(Point& a, const Point& b)
{
	a.x /= b.x;
	a.y /= b.y;
}

void operator<<=(Point& a, const Point& b)
{
	a.x <<= b.x;
	a.y <<= b.y;
}

void operator>>=(Point& a, const Point& b)
{
	a.x >>= b.x;
	a.y >>= b.y;
}

inline void clampPoint(Point& point, const Point& a, const Point& b)
{
	point.x = clamp(point.x, a.x, b.x);
	point.y = clamp(point.y, a.y, b.y);
}

inline void swapPoint(Point& a, Point& b)
{
	if (a.x > b.x)
		std::swap(a.x, b.x);
	if (a.y > b.y)
		std::swap(a.y, b.y);
}
