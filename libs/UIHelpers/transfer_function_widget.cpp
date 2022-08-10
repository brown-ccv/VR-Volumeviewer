#include "transfer_function_widget.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "embedded_colormaps.h"

#ifndef TFN_WIDGET_NO_STB_IMAGE_IMPL
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#endif

#include "stb_image.h"
#include "stb_image_resize.h"

static const float alpha_points_min_distance = 0.006f;

namespace tfnw
{
  template <typename T>
  inline T clamp(T x, T min, T max)
  {
    if (x < min)
    {
      return min;
    }
    if (x > max)
    {
      return max;
    }
    return x;
  }

  inline float srgb_to_linear(const float x)
  {
    if (x <= 0.04045f)
    {
      return x / 12.92f;
    }
    else
    {
      return std::pow((x + 0.055f) / 1.055f, 2.4f);
    }
  }

  Colormap::Colormap(const std::string &name,
                     const std::vector<uint8_t> &img,
                     const ColorSpace color_space)
      : name(name), colormap(img), color_space(color_space)
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

    // Initialize the colormap alpha channel w/ a linear ramp
    update_colormap();

    for (int i = 0; i < 256; i++)
    {
      current_histogram.push_back(0);
    }

    m_min_max_val[0] = 0.0f;
    m_min_max_val[1] = 1.0f;

    m_quantiles[0] = 0.05f;
    m_quantiles[1] = 0.95f;
  }

  void TransferFunctionWidget::add_colormap(const Colormap &map)
  {
    colormaps.push_back(map);
    if (colormaps.back().color_space == SRGB)
    {
      Colormap &cmap = colormaps.back();
      cmap.color_space = LINEAR;
      for (size_t i = 0; i < cmap.colormap.size() / 4; ++i)
      {
        for (size_t j = 0; j < 3; ++j)
        {
          const float x = srgb_to_linear(cmap.colormap[i * 4 + j] / 255.f);
          cmap.colormap[i * 4 + j] = static_cast<uint8_t>(clamp(x * 255.f, 0.f, 255.f));
        }
      }
    }
  }

  void TransferFunctionWidget::draw_ui()
  {
    update_gpu_image();

    const ImGuiIO &io = ImGui::GetIO();

    ImGui::Text("Transfer Function");
    ImGui::TextWrapped(
        "Left click to add a point, right click remove. "
        "Left click + drag to move points.");

    if (ImGui::BeginCombo("Colormap", colormaps[selected_colormap].name.c_str()))
    {
      for (size_t i = 0; i < colormaps.size(); ++i)
      {
        if (ImGui::Selectable(colormaps[i].name.c_str(), selected_colormap == i))
        {
          selected_colormap = i;
          update_colormap();
        }
      }
      ImGui::EndCombo();
    }

    vec2f canvas_size = ImGui::GetContentRegionAvail();
    // Note: If you're not using OpenGL for rendering your UI, the setup for
    // displaying the colormap texture in the UI will need to be updated.
    size_t tmp = colormap_img;
    ImGui::Image(reinterpret_cast<void *>(tmp), ImVec2(canvas_size.x, 16));
    vec2f canvas_pos = ImGui::GetCursorScreenPos();
    canvas_size.y -= 20;

    const float point_radius = 10.f;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->PushClipRect(canvas_pos, canvas_pos + canvas_size);

    const vec2f view_scale(canvas_size.x, -canvas_size.y);
    const vec2f view_offset(canvas_pos.x, canvas_pos.y + canvas_size.y);

    draw_list->AddRect(canvas_pos, canvas_pos + canvas_size, ImColor(180, 180, 180, 255));

    ImGui::InvisibleButton("tfn_canvas", canvas_size);

    static bool clicked_on_item = false;
    if ((!io.MouseDown[0]) && (!io.MouseDown[1]))
    {
      clicked_on_item = false;
    }
    if (ImGui::IsItemHovered() || selected_point != (size_t)-1 && (io.MouseDown[0] || io.MouseDown[1]))
    {
      clicked_on_item = true;
    }

    ImVec2 bbmin = ImGui::GetItemRectMin();
    ImVec2 bbmax = ImGui::GetItemRectMax();
    ImVec2 clipped_mouse_pos = ImVec2(
        (std::min)((std::max)(io.MousePos.x, bbmin.x), bbmax.x),
        (std::min)((std::max)(io.MousePos.y, bbmin.y), bbmax.y));

    if (clicked_on_item)
    {
      vec2f mouse_pos = (vec2f(clipped_mouse_pos) - view_offset) / view_scale;
      mouse_pos.x = clamp(mouse_pos.x, 0.f, 1.f);
      mouse_pos.y = clamp(mouse_pos.y, 0.f, 1.f);

      if (io.MouseDown[0])
      {
        if (selected_point != (size_t)-1)
        {

          // Keep the first and last control points at the edges
          if (selected_point == 0)
          {
            alpha_control_pts[selected_point].x = 0.f;
          }
          else if (selected_point == alpha_control_pts.size() - 1)
          {
            alpha_control_pts[selected_point].x = 1.f;
          }
          else
          {
            if (mouse_pos.x - alpha_control_pts[selected_point - 1].x < alpha_points_min_distance || alpha_control_pts[selected_point + 1].x - mouse_pos.x < alpha_points_min_distance)
            {
              alpha_control_pts[selected_point].y = mouse_pos.y;
            }
            else
            {
              alpha_control_pts[selected_point] = mouse_pos;
            }
          }
        }
        else
        {
          if (io.MousePos.x - canvas_pos.x <= point_radius)
          {
            selected_point = 0;
          }
          else if (io.MousePos.x - canvas_pos.x >= canvas_size.x - point_radius)
          {
            selected_point = alpha_control_pts.size() - 1;
          }
          else
          {
            auto fnd =
                std::find_if(alpha_control_pts.begin(),
                             alpha_control_pts.end(),
                             [&](const vec2f &p)
                             {
                               const vec2f pt_pos = p * view_scale + view_offset;
                               float dist = (pt_pos - vec2f(clipped_mouse_pos)).length();
                               return dist <= point_radius;
                             });
            // No nearby point, we're adding a new one
            if (fnd == alpha_control_pts.end())
            {
              alpha_control_pts.push_back(mouse_pos);
              std::sort(alpha_control_pts.begin(),
                        alpha_control_pts.end(),
                        [](const vec2f &a, const vec2f &b)
                        { return a.x < b.x; });
              if (selected_point != 0 && selected_point != alpha_control_pts.size() - 1)
              {
                fnd = std::find_if(
                    alpha_control_pts.begin(), alpha_control_pts.end(), [&](const vec2f &p)
                    {
								const vec2f pt_pos = p * view_scale + view_offset;
								float dist = (pt_pos - vec2f(clipped_mouse_pos)).length();
								return dist <= point_radius; });
              }
            }
            selected_point = std::distance(alpha_control_pts.begin(), fnd);
          }
        }
        update_colormap();
      }
      else if (ImGui::IsMouseClicked(1))
      {
        selected_point = -1;
        // Find and remove the point
        auto fnd = std::find_if(
            alpha_control_pts.begin(), alpha_control_pts.end(), [&](const vec2f &p)
            {
						const vec2f pt_pos = p * view_scale + view_offset;
						float dist = (pt_pos - vec2f(clipped_mouse_pos)).length();
						return dist <= point_radius; });
        // We also want to prevent erasing the first and last points
        if (fnd != alpha_control_pts.end() && fnd != alpha_control_pts.begin() &&
            fnd != alpha_control_pts.end() - 1)
        {
          alpha_control_pts.erase(fnd);
        }
        update_colormap();
      }
      else
      {
        selected_point = -1;
      }
    }
    else
    {
      selected_point = -1;
    }

    // Draw the alpha control points, and build the points for the polyline
    // which connects them
    std::vector<ImVec2> polyline_pts;
    for (const auto &pt : alpha_control_pts)
    {
      const vec2f pt_pos = pt * view_scale + view_offset;
      polyline_pts.push_back(pt_pos);
      draw_list->AddCircleFilled(pt_pos, point_radius, 0xFFFFFFFF);
    }
    draw_list->AddPolyline(polyline_pts.data(), (int)polyline_pts.size(), 0xFFFFFFFF, false, 2.f);
    draw_list->PopClipRect();

    // Add Label tick marks
    int nbTicks = 5;
    vec2f tick_pos = ImGui::GetCursorScreenPos();
    tick_pos.y -= ImGui::GetStyle().ItemSpacing.y;
    vec2f tick_size = ImGui::GetContentRegionAvail();
    tick_size.y = 5;
    draw_list->PushClipRect(tick_pos, ImVec2(tick_pos.x + tick_size.x, tick_pos.y + tick_size.y));

    for (int i = 0; i < nbTicks; i++)
    {
      float percentage = float(i) / (nbTicks - 1);

      draw_list->AddLine(ImVec2(tick_pos.x + percentage * (tick_size.x - 1), tick_size.y),
                         ImVec2(tick_pos.x + percentage * (tick_size.x - 1), tick_pos.y + tick_size.y), ImColor(255, 255, 255, 255), 1);
    }
    draw_list->PopClipRect();

    // Add Label text
    const float ItemSpacing = ImGui::GetStyle().ItemSpacing.x;
    for (int i = 0; i < nbTicks; i++)
    {
      float percentage = float(i) / (nbTicks - 1);
      float val = (m_min_max_val[1] - m_min_max_val[0]) * percentage + m_min_max_val[0];
      std::stringstream text;
      text << std::fixed << std::setprecision(4) << val;
      if (i == 0)
      {
      }
      else if (i == nbTicks - 1)
      {
        ImGui::SameLine(ImGui::GetWindowWidth() - ItemSpacing - ImGui::CalcTextSize(text.str().c_str()).x);
      }
      else
      {
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
    colormap_changed = false;
    return current_colormap;
  }

  std::vector<float> TransferFunctionWidget::get_colormapf()
  {
    colormap_changed = false;
    std::vector<float> colormapf(current_colormap.size(), 0.f);
    for (size_t i = 0; i < current_colormap.size(); ++i)
    {
      colormapf[i] = current_colormap[i] / 255.f;
    }
    return colormapf;
  }

  void TransferFunctionWidget::setHistogram(const std::vector<float> &hist)
  {
    current_histogram = hist;
  }

  std::vector<float> &TransferFunctionWidget::getHistogram()
  {
    return current_histogram;
  }

  void TransferFunctionWidget::setMinMax(const float min, const float max)
  {
    m_min_max_val[0] = min;
    m_min_max_val[1] = max;
  }

  void TransferFunctionWidget::setBlendedHistogram(const std::vector<float> &hist1, const std::vector<float> &hist2, float alpha)
  {
    if (hist1.size() != hist2.size())
      return;

    current_histogram.clear();
    for (int i = 0; i < hist1.size(); i++)
      current_histogram.push_back(hist1[i] * alpha + hist2[i] * (1.0f - alpha));
  }

  void TransferFunctionWidget::get_colormapf(std::vector<float> &color, std::vector<float> &opacity)
  {
    colormap_changed = false;
    color.resize((current_colormap.size() / 4) * 3);
    opacity.resize(current_colormap.size() / 4);
    for (size_t i = 0; i < current_colormap.size() / 4; ++i)
    {
      color[i * 3] = current_colormap[i * 4] / 255.f;
      color[i * 3 + 1] = current_colormap[i * 4 + 1] / 255.f;
      color[i * 3 + 2] = current_colormap[i * 4 + 2] / 255.f;
      opacity[i] = current_colormap[i * 4 + 3] / 255.f;
    }
  }

  void TransferFunctionWidget::draw_legend(float legend_pos_x, float legend_pos_y, float legend_width, float legend_height)
  {
    bool show_legend = true;
    ImGui::SetNextWindowPos(ImVec2(legend_pos_x, legend_pos_y));
    ImGui::SetNextWindowSize(ImVec2(legend_width, legend_height));
    ImGui::Begin("##legend", &show_legend, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar);
    ImGuiIO &io = ImGui::GetIO();
    ImTextureID my_tex_id = io.Fonts->TexID;
    float texture_width = (float)io.Fonts->TexWidth;
    float texture_height = (float)io.Fonts->TexHeight;
    std::string max_str = std::to_string(m_min_max_val[1]);
    float offset = 0.008 * (texture_width * max_str.size());
    ImGui::Image(reinterpret_cast<void *>(colormap_img), ImVec2(legend_width, 16));
    ImGui::Text("%.0f", m_min_max_val[0]);
    ImGui::SameLine((legend_width / 2));
    ImGui::Text("%.0f", (m_min_max_val[0] + m_min_max_val[1]) / 2);
    ImGui::SameLine(ImGui::GetWindowWidth() - offset);
    ImGui::Text(" %.0f", m_min_max_val[1]);
    ImGui::End();
  }

  void TransferFunctionWidget::draw_histogram()
  {

    const ImGuiIO &io = ImGui::GetIO();

    ImGui::Text("Histogram");

    vec2f canvas_size = ImGui::GetContentRegionAvail();
    // Note: If you're not using OpenGL for rendering your UI, the setup for
    // displaying the colormap texture in the UI will need to be updated.
    // TODO(#45) ImGui::Image(reinterpret_cast<void*>(colormap_img), ImVec2(canvas_size.x, 16));
    vec2f canvas_pos = ImGui::GetCursorScreenPos();
    // canvas_size.y -= 250;
    canvas_size.y -= 20;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    draw_list->PushClipRect(canvas_pos, canvas_pos + canvas_size);

    const vec2f view_scale(canvas_size.x, -canvas_size.y);
    const vec2f view_offset(canvas_pos.x, canvas_pos.y + canvas_size.y);

    draw_list->AddRect(canvas_pos, canvas_pos + canvas_size, ImColor(180, 180, 180, 255));

    ImGui::InvisibleButton("hstg_canvas", canvas_size);

    // Draw the alpha control points, and build the points for the polyline
    // which connects them

    // Code to Draw histogram in the UI
    for (int i = 0; i < current_histogram.size(); i++)
    {
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

    if (colormap_img == (GLuint)-1)
    {
      glGenTextures(1, &colormap_img);
      glBindTexture(GL_TEXTURE_2D, colormap_img);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    if (gpu_image_stale)
    {
      gpu_image_stale = false;
      glBindTexture(GL_TEXTURE_2D, colormap_img);
      glTexImage2D(GL_TEXTURE_2D,
                   0,
                   GL_RGBA,
                   (GLsizei)current_colormap.size() / 4,
                   1,
                   0,
                   GL_RGBA,
                   GL_UNSIGNED_BYTE,
                   current_colormap.data());
    }
    glBindTexture(GL_TEXTURE_2D, prev_tex_2d);
  }

  void TransferFunctionWidget::update_colormap()
  {
    colormap_changed = true;
    gpu_image_stale = true;
    current_colormap = colormaps[selected_colormap].colormap;
    // We only change opacities for now, so go through and update the opacity
    // by blending between the neighboring control points
    auto a_it = alpha_control_pts.begin();
    const size_t npixels = current_colormap.size() / 4;

    for (size_t i = 0; i < npixels; ++i)
    {
      float x = static_cast<float>(i) / npixels;
      auto high = a_it + 1;
      if (x > high->x)
      {
        ++a_it;
        ++high;
      }
      float t = (x - a_it->x) / (high->x - a_it->x);
      float alpha = (1.f - t) * a_it->y + t * high->y;
      current_colormap[i * 4 + 3] = static_cast<uint8_t>(clamp(alpha * 255.f, 0.f, 255.f));
    }
  }

  void TransferFunctionWidget::set_quantiles(float min, float max)
  {
    m_quantiles[0] = min;
    m_quantiles[1] = max;
  }

  void TransferFunctionWidget::get_quantiles(float &min, float &max)
  {
    min = m_quantiles[0];
    max = m_quantiles[1];
  }

  void TransferFunctionWidget::load_embedded_preset(const uint8_t *buf,
                                                    size_t size,
                                                    const std::string &name)
  {
    int w, h, n;

    uint8_t *img_data = stbi_load_from_memory(buf, size, &w, &h, &n, 4);

    int out_w = 256;
    uint8_t *output_pixels = (unsigned char *)malloc(out_w * h * n);

    stbir_resize_uint8(img_data, w, h, 0, output_pixels, out_w, h, 0, n);
    auto img = std::vector<uint8_t>(output_pixels, output_pixels + out_w * 1 * 4);

    stbi_image_free(img_data);
    stbi_image_free(output_pixels);

    colormaps.emplace_back(name, img, SRGB);
    Colormap &cmap = colormaps.back();
    for (size_t i = 0; i < cmap.colormap.size() / 4; ++i)
    {
      for (size_t j = 0; j < 3; ++j)
      {
        const float x = srgb_to_linear(cmap.colormap[i * 4 + j] / 255.f);
        cmap.colormap[i * 4 + j] = static_cast<uint8_t>(clamp(x * 255.f, 0.f, 255.f));
      }
    }
  }

}
