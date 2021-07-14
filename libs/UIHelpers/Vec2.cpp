#include "Vec2.h"
#include <cmath>

vec2f::vec2f(float c) : x(c), y(c) {}

vec2f::vec2f(float x, float y) : x(x), y(y) {}

vec2f::vec2f(const ImVec2& v) : x(v.x), y(v.y) {}

float vec2f::length() const
{
  return std::sqrt(x * x + y * y);
}

vec2f vec2f::operator+(
  const vec2f& b) const
{
  return vec2f(x + b.x, y + b.y);
}

vec2f vec2f::operator-(
  const vec2f& b) const
{
  return vec2f(x - b.x, y - b.y);
}

vec2f vec2f::operator/(
  const vec2f& b) const
{
  return vec2f(x / b.x, y / b.y);
}

vec2f vec2f::operator*(
  const vec2f& b) const
{
  return vec2f(x * b.x, y * b.y);
}

vec2f::operator ImVec2() const
{
  return ImVec2(x, y);
}