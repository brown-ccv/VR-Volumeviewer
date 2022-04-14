
#ifdef _WIN32
#include "GL/glew.h"
#include "GL/wglew.h"
#elif (!defined(__APPLE__))
#include "GL/glxew.h"
#endif

// OpenGL Headers
#if defined(WIN32)
#define NOMINMAX
#include <windows.h>
#include <GL/gl.h>
#elif defined(__APPLE__)
#define GL_GLEXT_PROTOTYPES
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif

#include "transfer_function_widget.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "embedded_colormaps.h"
#include "../../include/render/FontHandler.h"


#ifndef TFN_WIDGET_NO_STB_IMAGE_IMPL
#define STB_IMAGE_IMPLEMENTATION
#endif

#include "stb_image.h"

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

Colormap::Colormap(const std::string& name, const std::vector<uint8_t>& img)
  : name(name), colormap(img)
{
}



TransferFunctionWidget::TransferFunctionWidget()
{
  // Load up the embedded colormaps as the default options
  load_embedded_preset(cool_warm_extended, sizeof(cool_warm_extended), "Cool Warm Extended");
  load_embedded_preset(paraview_cool_warm, sizeof(paraview_cool_warm), "ParaView Cool Warm");
  load_embedded_preset(rainbow, sizeof(rainbow), "Rainbow");
  load_embedded_preset(matplotlib_plasma, sizeof(matplotlib_plasma), "Matplotlib Plasma");
  load_embedded_preset(matplotlib_virdis, sizeof(matplotlib_virdis), "Matplotlib Virdis");
  load_embedded_preset(samsel_linear_green, sizeof(samsel_linear_green), "Samsel Linear Green");
  load_embedded_preset(samsel_linear_ygb_1211g, sizeof(samsel_linear_ygb_1211g), "Samsel Linear YGB 1211G");
  load_embedded_preset(blackbody, sizeof(blackbody), "Black Body");
  load_embedded_preset(jet, sizeof(jet), "Jet");
  load_embedded_preset(blue_gold, sizeof(blue_gold), "Blue Gold");
  load_embedded_preset(ice_fire, sizeof(ice_fire), "Ice Fire");
  load_embedded_preset(nic_edge, sizeof(nic_edge), "nic Edge");
  //load_embedded_preset(Algae, sizeof(Algae), "Algae");

  // Initialize the colormap alpha channel w/ a linear ramp
  update_colormap();

  for (int i = 0; i < 256; i++)
  {
    //current_histogram.push_back(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
    current_histogram.push_back(0);
  }



  m_min_max_val[0] = 0.0f;
  m_min_max_val[1] = 1.0f;

  m_quantiles[0] = 0.05f;
  m_quantiles[1] = 0.95f;
}

void TransferFunctionWidget::add_colormap(const Colormap& map)
{
  colormaps.push_back(map);
}

void TransferFunctionWidget::draw_ui()
{
  update_gpu_image();
  colormap_changed = false;

  const ImGuiIO& io = ImGui::GetIO();

  ImGui::Text("Transfer Function");
  ImGui::TextWrapped(
    "Left click to add a point, right click remove. "
    "Left click + drag to move points.");

  if (ImGui::BeginCombo("Colormap", colormaps[selected_colormap].name.c_str())) {
    for (size_t i = 0; i < colormaps.size(); ++i) {
      if (ImGui::Selectable(colormaps[i].name.c_str(), selected_colormap == i)) {
        selected_colormap = i;
        update_colormap();
      }
    }
    ImGui::EndCombo();
  }

  vec2f canvas_size = ImGui::GetContentRegionAvail();
  // Note: If you're not using OpenGL for rendering your UI, the setup for
  // displaying the colormap texture in the UI will need to be updated.
  ImGui::Image(reinterpret_cast<void*>(colormap_img), ImVec2(canvas_size.x, 16));
  vec2f canvas_pos = ImGui::GetCursorScreenPos();
  canvas_size.y -= 80;

  const float point_radius = 20.f;

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->PushClipRect(canvas_pos, canvas_pos + canvas_size);

  const vec2f view_scale(canvas_size.x, -canvas_size.y);
  const vec2f view_offset(canvas_pos.x, canvas_pos.y + canvas_size.y);

  draw_list->AddRect(canvas_pos, canvas_pos + canvas_size, ImColor(180, 180, 180, 255));

  ImGui::InvisibleButton("tfn_canvas", canvas_size);
  if (ImGui::IsItemHovered() || selected_point != (size_t)-1) {

    vec2f mouse_pos = (vec2f(io.MousePos) - view_offset) / view_scale;
    mouse_pos.x = clamp(mouse_pos.x, 0.f, 1.f);
    mouse_pos.y = clamp(mouse_pos.y, 0.f, 1.f);


    if (io.MouseDown[0]) {
      if (selected_point != (size_t)-1) {
        alpha_control_pts[selected_point] = mouse_pos;

        //make sure point does not cross

          // Keep the first and last control points at the edges
        if (selected_point == 0) {
          alpha_control_pts[selected_point].x = 0.f;
        }
        else if (selected_point == alpha_control_pts.size() - 1) {
          alpha_control_pts[selected_point].x = 1.f;
        }
        //make sure the other points do not cross
        else
        {
          alpha_control_pts[selected_point].x = (alpha_control_pts[selected_point].x < alpha_control_pts[selected_point - 1].x)
            ? alpha_control_pts[selected_point - 1].x + 0.01 : alpha_control_pts[selected_point].x;
          alpha_control_pts[selected_point].x = (alpha_control_pts[selected_point].x > alpha_control_pts[selected_point + 1].x)
            ? alpha_control_pts[selected_point + 1].x - 0.01 : alpha_control_pts[selected_point].x;
        }
      }
      else {
        // See if we're selecting a point or adding one
        if (io.MousePos.x - canvas_pos.x <= point_radius) {
          selected_point = 0;
        }
        else if (io.MousePos.x - canvas_pos.x >= canvas_size.x - point_radius) {
          selected_point = alpha_control_pts.size() - 1;
        }
        else {
          auto fnd = std::find_if(
            alpha_control_pts.begin(), alpha_control_pts.end(), [&](const vec2f& p) {
              const vec2f pt_pos = p * view_scale + view_offset;
              float dist = (pt_pos - vec2f(io.MousePos)).length();
              return dist <= point_radius;
            });
          // No nearby point, we're adding a new one
          if (fnd == alpha_control_pts.end()) {
            alpha_control_pts.push_back(mouse_pos);
            // Keep alpha control points ordered by x coordinate, update
            // selected point index to match
            std::sort(alpha_control_pts.begin(),
              alpha_control_pts.end(),
              [](const vec2f& a, const vec2f& b) { return a.x < b.x; });
            if (selected_point != 0 && selected_point != alpha_control_pts.size() - 1) {
              fnd = std::find_if(
                alpha_control_pts.begin(), alpha_control_pts.end(), [&](const vec2f& p) {
                  const vec2f pt_pos = p * view_scale + view_offset;
                  float dist = (pt_pos - vec2f(io.MousePos)).length();
                  return dist <= point_radius;
                });
            }
          }
          selected_point = std::distance(alpha_control_pts.begin(), fnd);
        }
      }
      update_colormap();
    }
    else if (ImGui::IsMouseClicked(1)) {
      selected_point = -1;
      // Find and remove the point
      auto fnd = std::find_if(
        alpha_control_pts.begin(), alpha_control_pts.end(), [&](const vec2f& p) {
          const vec2f pt_pos = p * view_scale + view_offset;
          float dist = (pt_pos - vec2f(io.MousePos)).length();
          return dist <= point_radius;
        });
      // We also want to prevent erasing the first and last points
      if (fnd != alpha_control_pts.end() && fnd != alpha_control_pts.begin() &&
        fnd != alpha_control_pts.end() - 1) {
        alpha_control_pts.erase(fnd);
      }
      update_colormap();
    }
    else {
      selected_point = -1;
    }
  }

  // Draw the alpha control points, and build the points for the polyline
  // which connects them
  std::vector<ImVec2> polyline_pts;
  for (const auto& pt : alpha_control_pts) {
    const vec2f pt_pos = pt * view_scale + view_offset;
    polyline_pts.push_back(pt_pos);
    draw_list->AddCircleFilled(pt_pos, point_radius, 0xFFFFFFFF);
  }


  draw_list->AddPolyline(polyline_pts.data(), polyline_pts.size(), 0xFFFFFFFF, false, 2.f);
  draw_list->PopClipRect();

  //Add Label tick marks
  int nbTicks = 5;
  vec2f tick_pos = ImGui::GetCursorScreenPos();
  tick_pos.y -= ImGui::GetStyle().ItemSpacing.y;
  vec2f tick_size = ImGui::GetContentRegionAvail();
  tick_size.y = 5;
  draw_list->PushClipRect(tick_pos, ImVec2(tick_pos.x + tick_size.x, tick_pos.y + tick_size.y));

  for (int i = 0; i < nbTicks; i++) {
    float percentage = float(i) / (nbTicks - 1);

    draw_list->AddLine(ImVec2(tick_pos.x + percentage * (tick_size.x - 1), tick_size.y),
      ImVec2(tick_pos.x + percentage * (tick_size.x - 1), tick_pos.y + tick_size.y), ImColor(255, 255, 255, 255), 1);
  }
  draw_list->PopClipRect();

  //Add Label text
  const float ItemSpacing = ImGui::GetStyle().ItemSpacing.x;
  for (int i = 0; i < nbTicks; i++) {
    float percentage = float(i) / (nbTicks - 1);
    float val = (m_min_max_val[1] - m_min_max_val[0]) * percentage + m_min_max_val[0];
    std::stringstream text;
    text << std::fixed << std::setprecision(4) << val;
    if (i == 0) {
    }
    else if (i == nbTicks - 1) {
      ImGui::SameLine(ImGui::GetWindowWidth() - ItemSpacing - ImGui::CalcTextSize(text.str().c_str()).x);
    }
    else {
      ImGui::SameLine((ImGui::GetWindowWidth()) * percentage - ImGui::CalcTextSize(text.str().c_str()).x * 0.5);
    }
    ImGui::Text(text.str().c_str());
  }
}

bool TransferFunctionWidget::changed() const
{
  return colormap_changed;
}

std::vector<uint8_t> TransferFunctionWidget::get_colormap()
{
  return current_colormap;
}

std::vector<float> TransferFunctionWidget::get_colormapf()
{
  std::vector<float> colormapf(current_colormap.size(), 0.f);
  for (size_t i = 0; i < current_colormap.size(); ++i) {
    colormapf[i] = current_colormap[i] / 255.f;
  }
  return colormapf;
}

void TransferFunctionWidget::setHistogram(const std::vector<float>& hist)
{
  current_histogram = hist;
}

std::vector<float>& TransferFunctionWidget::getHistogram()
{
  return current_histogram;
}

void TransferFunctionWidget::setMinMax(const float min, const float max) {
  m_min_max_val[0] = min;
  m_min_max_val[1] = max;
}


void TransferFunctionWidget::setBlendedHistogram(const std::vector<float>& hist1, const std::vector<float>& hist2, float alpha)
{
  if (hist1.size() != hist2.size())
    return;

  current_histogram.clear();
  for (int i = 0; i < hist1.size(); i++)
    current_histogram.push_back(hist1[i] * alpha + hist2[i] * (1.0f - alpha));
}


void TransferFunctionWidget::get_colormapf(std::vector<float>& color, std::vector<float>& opacity)
{
  color.resize((current_colormap.size() / 4) * 3);
  opacity.resize(current_colormap.size() / 4);
  for (size_t i = 0; i < current_colormap.size() / 4; ++i) {
    color[i * 3] = current_colormap[i * 4] / 255.f;
    color[i * 3 + 1] = current_colormap[i * 4 + 1] / 255.f;
    color[i * 3 + 2] = current_colormap[i * 4 + 2] / 255.f;
    opacity[i] = current_colormap[i * 4 + 3] / 255.f;
  }
}

void TransferFunctionWidget::drawLegend() {
  GLint viewport[4];
  GLfloat projection[16];
  GLfloat modelview[16];

  int min[2] = { 50 , 100 };
  int max[2] = { 400 , 120 };


  glGetIntegerv(GL_VIEWPORT, &viewport[0]);
  glGetFloatv(GL_PROJECTION_MATRIX, &projection[0]);
  glGetFloatv(GL_MODELVIEW_MATRIX, &modelview[0]);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(0, viewport[2], 0, viewport[3], -1, 1);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glEnable(GL_TEXTURE_2D);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  glBindTexture(GL_TEXTURE_2D, colormap_img);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glBegin(GL_QUADS);
  glTexCoord2f(0, 1);
  glVertex3f(min[0], max[1], 0);
  glTexCoord2f(1, 1);
  glVertex3f(max[0], max[1], 0);
  glTexCoord2f(1, 0);
  glVertex3f(max[0], 0.5f * (min[1] + max[1]), 0);
  glTexCoord2f(0, 0);
  glVertex3f(min[0], 0.5f * (min[1] + max[1]), 0);
  glEnd();
  glDisable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindTexture(GL_TEXTURE_2D, colormap_img);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glBegin(GL_QUADS);
  glTexCoord2f(0, 1);
  glVertex3f(min[0], 0.5f * (min[1] + max[1]), 0);
  glTexCoord2f(1, 1);
  glVertex3f(max[0], 0.5f * (min[1] + max[1]), 0);
  glTexCoord2f(1, 0);
  glVertex3f(max[0], min[1], 0);
  glTexCoord2f(0, 0);
  glVertex3f(min[0], min[1], 0);
  glEnd();
  glDisable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0);



  //Add Label tick marks
  int nbTicks = 4;
  float diff = (max[0] - min[0]) / nbTicks;
  for (int i = 0; i <= nbTicks; i++) {
    float pos_x = min[0] + i * diff;
    glBegin(GL_LINES);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glVertex3f(pos_x, min[1], 0);
    glVertex3f(pos_x, min[1] - 5, 0);
    glEnd();

    float percentage = float(i) / (nbTicks);
    float val = (m_min_max_val[1] - m_min_max_val[0]) * percentage + m_min_max_val[0];
    std::stringstream text;
    text << std::fixed << std::setprecision(4) << val;

    FontHandler::getInstance()->renderTextBox(text.str(), pos_x - 100, min[1] - 15, 0, 200.0, 10, TextAlignment::CENTER);
  }

  glEnable(GL_LIGHTING);

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(projection);
  glMatrixMode(GL_MODELVIEW);
  glLoadMatrixf(modelview);



}

void TransferFunctionWidget::drawLegend( float legend_pos_x, float legend_pos_y, float legend_width, float legend_height)
{
  bool show_legend = true;
  ImGui::SetNextWindowPos(ImVec2(legend_pos_x, legend_pos_y));
  ImGui::SetNextWindowSize(ImVec2(legend_width, legend_height));
  ImGui::Begin("##legend", &show_legend, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
  ImGuiIO& io = ImGui::GetIO();
  ImTextureID my_tex_id = io.Fonts->TexID;
  float my_tex_w = (float)io.Fonts->TexWidth;
  float my_tex_h = (float)io.Fonts->TexHeight;
  std::string max_str = std::to_string(m_min_max_val[1]);
  float offset = 0.008 * (my_tex_w * max_str.size());
  ImGui::Image(reinterpret_cast<void*>(colormap_img), ImVec2(legend_width, 16));
  ImGui::Text("%.0f", m_min_max_val[0]);
  ImGui::SameLine((legend_width /2) );
  ImGui::Text("%.0f", (m_min_max_val[0]+ m_min_max_val[1]) /2);
  ImGui::SameLine(ImGui::GetWindowWidth() - offset);
  ImGui::Text(" %.0f", m_min_max_val[1]);
  ImGui::End();
}

void TransferFunctionWidget::draw_histogram()
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
  const vec2f view_offset(canvas_pos.x, canvas_pos.y + canvas_size.y);

  draw_list->AddRect(canvas_pos, canvas_pos + canvas_size, ImColor(180, 180, 180, 255));

  ImGui::InvisibleButton("hstg_canvas", canvas_size);


  // Draw the alpha control points, and build the points for the polyline
  // which connects them


  //Code to Draw histogram in the UI
  for (int i = 0; i < current_histogram.size(); i++) {
    vec2f lp = vec2f(((float)i) / current_histogram.size(), 0.0f);
    vec2f hp = vec2f(((float)i + 1.0f) / current_histogram.size(), current_histogram[i]);
    draw_list->AddRectFilled(lp * view_scale + view_offset, hp * view_scale + view_offset, 0x77777777);
  }


  draw_list->PopClipRect();




}

void TransferFunctionWidget::update_gpu_image()
{
  GLint prev_tex_2d = 0;
  glGetIntegerv(GL_TEXTURE_BINDING_2D, &prev_tex_2d);

  if (colormap_img == (GLuint)-1) {
    glGenTextures(1, &colormap_img);
    glBindTexture(GL_TEXTURE_2D, colormap_img);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  }
  if (colormap_changed) {
    glBindTexture(GL_TEXTURE_2D, colormap_img);
    glTexImage2D(GL_TEXTURE_2D,
      0,
      GL_RGBA,
      current_colormap.size() / 4,
      1,
      0,
      GL_RGBA,
      GL_UNSIGNED_BYTE,
      current_colormap.data());
  }
  if (prev_tex_2d != 0) {
    glBindTexture(GL_TEXTURE_2D, prev_tex_2d);
  }
}

void TransferFunctionWidget::update_colormap()
{
  colormap_changed = true;
  current_colormap = colormaps[selected_colormap].colormap;
  // We only change opacities for now, so go through and update the opacity
  // by blending between the neighboring control points
  auto a_it = alpha_control_pts.begin();
  const size_t npixels = current_colormap.size() / 4;

  for (size_t i = 0; i < npixels; ++i) {
    float x = static_cast<float>(i) / npixels;
    auto high = a_it + 1;
    if (x > high->x) {
      ++a_it;
      ++high;
    }
    float t = (x - a_it->x) / (high->x - a_it->x);
    float alpha = (1.f - t) * a_it->y + t * high->y;
    current_colormap[i * 4 + 3] = static_cast<uint8_t>(clamp(alpha * 255.f, 0.f, 255.f));
  }
}

void TransferFunctionWidget::set_Quantiles(float min, float max)
{
  m_quantiles[0] = min;
  m_quantiles[1] = max;
}


void TransferFunctionWidget::get_Quantiles(float& min, float& max)
{
  min = m_quantiles[0];
  max = m_quantiles[1];
}

void TransferFunctionWidget::load_embedded_preset(const uint8_t* buf,
  size_t size,
  const std::string& name)
{
  int w, h, n;
  uint8_t* img_data = stbi_load_from_memory(buf, size, &w, &h, &n, 4);
  auto img = std::vector<uint8_t>(img_data, img_data + w * 1 * 4);
  stbi_image_free(img_data);
  colormaps.emplace_back(name, img);
}

