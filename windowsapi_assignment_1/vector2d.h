#pragma once

#include <cmath>

struct Vector2d
{
public:
	double x;
	double y;

public:
	Vector2d()
	{
		x = 0.0;
		y = 0.0;
	}

	Vector2d(double _x, double _y)
	{
		x = _x;
		y = _y;
	}

	Vector2d operator+() const
	{
		return *this;
	}

	Vector2d operator-() const
	{
		return Vector2d(-x, -y);
	}

	Vector2d operator+(const Vector2d other) const
	{
		return Vector2d(x + other.x, y + other.y);
	}

	Vector2d operator-(const Vector2d other) const
	{
		return *this + (-other);
	}

	Vector2d operator*(const double s) const
	{
		return Vector2d(x * s, y * s);
	}

	Vector2d operator/(const double s) const
	{
		return Vector2d(x / s, y / s);
	}

	double SqrMagnitude() const
	{
		return x * x + y * y;
	}

	double Magnitude() const
	{
		return sqrt(SqrMagnitude());
	}

	Vector2d Normalize() const
	{
		return *this / Magnitude();
	}
};