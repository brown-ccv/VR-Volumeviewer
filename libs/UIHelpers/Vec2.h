#include "imgui/imgui.h"
struct vec2f {
  float x, y;

  vec2f(float c = 0.f);
  vec2f(float x, float y);
  vec2f(const ImVec2& v);

  float length() const;

  vec2f operator+(const vec2f& b) const;
  vec2f operator-(const vec2f& b) const;
  vec2f operator/(const vec2f& b) const;
  vec2f operator*(const vec2f& b) const;
  operator ImVec2() const;
};