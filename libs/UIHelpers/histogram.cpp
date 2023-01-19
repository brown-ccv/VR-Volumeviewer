#include "histogram.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>

#include "Vec2.h"

template <typename T>
T clamp(T x, T min, T max)
{
    if (x < min) {
        return min;
    }
    if (x > max) {
        return max;
    }
    return x;
}

Histogram::Histogram()
{
  m_min_max_val[0] = 0.0f;
  m_min_max_val[1] = 1.0f;
}


void Histogram::setHistogram(const std::vector<float> &hist)
{
	current_histogram = hist;
}

std::vector<float>& Histogram::getHistogram()
{
  return current_histogram;
}

void Histogram::setMinMax(const float min, const float max) {
	m_min_max_val[0] = min;
	m_min_max_val[1] = max;
}


void Histogram::setBlendedHistogram(const std::vector<float>& hist1, const std::vector<float>& hist2, float alpha)
{
	if (hist1.size() != hist2.size())
		return;

	current_histogram.clear();
	for (int i = 0; i < hist1.size(); i++)
		current_histogram.push_back(hist1[i] * alpha + hist2[i] * (1.0f - alpha));
}


void Histogram::draw_histogram()
{
  
  const ImGuiIO& io = ImGui::GetIO();

  ImGui::Text("Histogram");

  vec2f canvas_size = ImGui::GetContentRegionAvail();
  // Note: If you're not using OpenGL for rendering your UI, the setup for
  // displaying the colormap texture in the UI will need to be updated.
 // ImGui::Image(reinterpret_cast<void*>(colormap_img), ImVec2(canvas_size.x, 16));
  vec2f canvas_pos = ImGui::GetCursorScreenPos();
  canvas_size.y -= 250;

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->PushClipRect(canvas_pos, canvas_pos + canvas_size);

  const vec2f view_scale(canvas_size.x, -canvas_size.y);
  const vec2f view_offset(canvas_pos.x , canvas_pos.y + canvas_size.y);

  draw_list->AddRect(canvas_pos, canvas_pos + canvas_size, ImColor(180, 180, 180, 255));

  ImGui::InvisibleButton("hstg_canvas", canvas_size);


  // Draw the alpha control points, and build the points for the polyline
  // which connects them


  //Code to Draw histogram in the UI

  for (int i = 0; i < current_histogram.size(); i++) {
    vec2f lp = vec2f(((float)i) / current_histogram.size(), current_histogram[i]);
    vec2f hp = vec2f(((float)i + 1.0f) / current_histogram.size(), 0.0f);
    vec2f p_min = lp * view_scale + view_offset;  
    vec2f p_max = hp * view_scale + view_offset;
    draw_list->AddRectFilled(p_min, p_max, 0x77777777);
  }


  draw_list->PopClipRect();

 

  
}




