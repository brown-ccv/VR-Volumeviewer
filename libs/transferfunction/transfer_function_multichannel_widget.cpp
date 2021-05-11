#include "transfer_function_multichannel_widget.h"
#include <algorithm>
#include <cmath>
#include <iostream>

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

TransferFunctionMultiChannelWidget::vec2f::vec2f(float c) : x(c), y(c) {}

TransferFunctionMultiChannelWidget::vec2f::vec2f(float x, float y) : x(x), y(y) {}

TransferFunctionMultiChannelWidget::vec2f::vec2f(const ImVec2 &v) : x(v.x), y(v.y) {}

float TransferFunctionMultiChannelWidget::vec2f::length() const
{
    return std::sqrt(x * x + y * y);
}

TransferFunctionMultiChannelWidget::vec2f TransferFunctionMultiChannelWidget::vec2f::operator+(
    const TransferFunctionMultiChannelWidget::vec2f &b) const
{
    return vec2f(x + b.x, y + b.y);
}

TransferFunctionMultiChannelWidget::vec2f TransferFunctionMultiChannelWidget::vec2f::operator-(
    const TransferFunctionMultiChannelWidget::vec2f &b) const
{
    return vec2f(x - b.x, y - b.y);
}

TransferFunctionMultiChannelWidget::vec2f TransferFunctionMultiChannelWidget::vec2f::operator/(
    const TransferFunctionMultiChannelWidget::vec2f &b) const
{
    return vec2f(x / b.x, y / b.y);
}

TransferFunctionMultiChannelWidget::vec2f TransferFunctionMultiChannelWidget::vec2f::operator*(
    const TransferFunctionMultiChannelWidget::vec2f &b) const
{
    return vec2f(x * b.x, y * b.y);
}

TransferFunctionMultiChannelWidget::vec2f::operator ImVec2() const
{
    return ImVec2(x, y);
}

TransferFunctionMultiChannelWidget::TransferFunctionMultiChannelWidget()
{
	current_colormap = std::vector<uint8_t>(256, 0);
	
    // Initialize the colormap alpha channel w/ a linear ramp
    update_colormap();

	current_histogram = std::vector < std::vector<float> >(3, std::vector<float>());
	//for (int i = 0; i < 255; i++)
	//	current_histogram.push_back(static_cast <float> (rand()) / static_cast <float> (RAND_MAX));
}

void TransferFunctionMultiChannelWidget::draw_ui()
{
    update_gpu_image();
    colormap_changed = false;

    const ImGuiIO &io = ImGui::GetIO();

    ImGui::Text("Transfer Function");
    ImGui::TextWrapped(
        "Left click to add a point, right click remove. "
        "Left click + drag to move points.");

    vec2f canvas_size = ImGui::GetContentRegionAvail();
	canvas_size.y -= 80;
	canvas_size.y = canvas_size.y / 3;
	vec2f canvas_pos = ImGui::GetCursorScreenPos();
	
	const float point_radius = 20.f;
	ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Note: If you're not using OpenGL for rendering your UI, the setup for
    // displaying the colormap texture in the UI will need to be updated.
	for (int i = 0; i < 3; i++) {
		vec2f canvas_pos = ImGui::GetCursorScreenPos();
		
		draw_list->PushClipRect(canvas_pos, canvas_pos + canvas_size);

		const vec2f view_scale(canvas_size.x, -canvas_size.y);
		const vec2f view_offset(canvas_pos.x, canvas_pos.y + canvas_size.y);
		ImColor color;
		if (i == 0)
			color = ImColor(180, 120, 120, 255);
		else if (i == 1)
			color = ImColor(120, 180, 120, 255);
		else
			color = ImColor(120, 120, 180, 255);
		
		draw_list->AddRect(canvas_pos-vec2f(0,-1), canvas_pos + canvas_size , color);
		ImGui::InvisibleButton("tfn_canvas", canvas_size);
		if (ImGui::IsItemHovered() || selected_point[i] != (size_t)-1) {

			if (io.MouseDown[0]) {
				if (selected_point[i] != (size_t)-1) {
					vec2f mouse_pos = (vec2f(io.MousePos) - view_offset) / view_scale;

					mouse_pos.x = clamp(mouse_pos.x, 0.f, 1.f);
					mouse_pos.y = clamp(mouse_pos.y, 0.f, 1.f);
					
					alpha_control_pts[i][selected_point[i]] = mouse_pos;

					// Keep the first and last control points at the edges
					if (selected_point[i] == 0) {
						alpha_control_pts[i][selected_point[i]].x = 0.f;
					}
					else if (selected_point[i] == alpha_control_pts[i].size() - 1) {
						alpha_control_pts[i][selected_point[i]].x = 1.f;
					}
					//make sure the other points do not cross
					else
					{
						alpha_control_pts[i][selected_point[i]].x = (alpha_control_pts[i][selected_point[i]].x < alpha_control_pts[i][selected_point[i] - 1].x)
							? alpha_control_pts[i][selected_point[i] - 1].x + 0.01 : alpha_control_pts[i][selected_point[i]].x;
						alpha_control_pts[i][selected_point[i]].x = (alpha_control_pts[i][selected_point[i]].x > alpha_control_pts[i][selected_point[i] + 1].x)
							? alpha_control_pts[i][selected_point[i] + 1].x - 0.01 : alpha_control_pts[i][selected_point[i]].x;
					}
				}
				else if(selected_point[0] == (size_t)-1 && selected_point[1] == (size_t)-1 && selected_point[2] == (size_t)-1 &&
					io.MousePos.x >= canvas_pos.x && io.MousePos.x <= canvas_pos.x + canvas_size.x
					&& io.MousePos.y >= canvas_pos.y && io.MousePos.y <= canvas_pos.y + canvas_size.y) {
					// See if we're selecting a point or adding one
					if (io.MousePos.x - canvas_pos.x <= point_radius) {
						selected_point[i] = 0;
					}
					else if (io.MousePos.x - canvas_pos.x >= canvas_size.x - point_radius) {
						selected_point[i] = alpha_control_pts[i].size() - 1;
					}
					else {
						auto fnd = std::find_if(
							alpha_control_pts[i].begin(), alpha_control_pts[i].end(), [&](const vec2f& p) {
								const vec2f pt_pos = p * view_scale + view_offset;
								float dist = (pt_pos - vec2f(io.MousePos)).length();
								return dist <= point_radius;
							});
						// No nearby point, we're adding a new one
						if (fnd == alpha_control_pts[i].end()) {
							vec2f mouse_pos = (vec2f(io.MousePos) - view_offset) / view_scale;

							mouse_pos.x = clamp(mouse_pos.x, 0.f, 1.f);
							mouse_pos.y = clamp(mouse_pos.y, 0.f, 1.f);

							alpha_control_pts[i].push_back(mouse_pos);
							// Keep alpha control points ordered by x coordinate, update
							// selected point index to match
							std::sort(alpha_control_pts[i].begin(),
								alpha_control_pts[i].end(),
								[](const vec2f& a, const vec2f& b) { return a.x < b.x; });
							if (selected_point[i] != 0 && selected_point[i] != alpha_control_pts[i].size() - 1) {
								fnd = std::find_if(
									alpha_control_pts[i].begin(), alpha_control_pts[i].end(), [&](const vec2f& p) {
										const vec2f pt_pos = p * view_scale + view_offset;
										float dist = (pt_pos - vec2f(io.MousePos)).length();
										return dist <= point_radius;
									});
							}
						}
						selected_point[i] = std::distance(alpha_control_pts[i].begin(), fnd);
					}
				}
				update_colormap();
			}
			else if (ImGui::IsMouseClicked(1)) {
				selected_point[i] = -1;
				// Find and remove the point
				auto fnd = std::find_if(
					alpha_control_pts[i].begin(), alpha_control_pts[i].end(), [&](const vec2f& p) {
						const vec2f pt_pos = p * view_scale + view_offset;
						float dist = (pt_pos - vec2f(io.MousePos)).length();
						return dist <= point_radius;
					});
				// We also want to prevent erasing the first and last points
				if (fnd != alpha_control_pts[i].end() && fnd != alpha_control_pts[i].begin() &&
					fnd != alpha_control_pts[i].end() - 1) {
					alpha_control_pts[i].erase(fnd);
				}
				update_colormap();
			}
			else {
				selected_point[i] = -1;
			}
			

		}
		if (i == 0)
			color = ImColor(255, 120, 120, 255);
		else if (i == 1)
			color = ImColor(120, 255, 120, 255);
		else
			color = ImColor(120, 120, 255, 255);

		// Draw the alpha control points, and build the points for the polyline
		// which connects them
		std::vector<ImVec2> polyline_pts;
		for (const auto& pt : alpha_control_pts[i]) {
			const vec2f pt_pos = pt * view_scale + view_offset;
			polyline_pts.push_back(pt_pos);
			draw_list->AddCircleFilled(pt_pos, point_radius, color);
		}


  //Code to Draw histogram in the UI
	/*	
	for (int k = 0; k < current_histogram[i].size(); k++) {
			vec2f lp = vec2f(((float)k) / current_histogram[i].size(), 0.0f);
			vec2f hp = vec2f(((float)k + 1.0f) / current_histogram[i].size(), current_histogram[i][k]);
			draw_list->AddRectFilled(lp * view_scale + view_offset, hp * view_scale + view_offset, 0x77777777);
		}*/

		draw_list->AddPolyline(polyline_pts.data(), polyline_pts.size(), color, false, 2.f);
		draw_list->PopClipRect();

		ImGui::Dummy(ImVec2(0.0f, 20.0f));
	}
}

bool TransferFunctionMultiChannelWidget::changed() const
{
    return colormap_changed;
}

std::vector<uint8_t> TransferFunctionMultiChannelWidget::get_colormap()
{
    return current_colormap;
}

std::vector<float> TransferFunctionMultiChannelWidget::get_colormapf()
{
    std::vector<float> colormapf(current_colormap.size(), 0.f);
    for (size_t i = 0; i < current_colormap.size(); ++i) {
        colormapf[i] = current_colormap[i] / 255.f;
    }
    return colormapf;
}

void TransferFunctionMultiChannelWidget::setHistogram(const std::vector<float> &hist, int channel)
{
	current_histogram[channel] = hist;
}

void TransferFunctionMultiChannelWidget::setBlendedHistogram(const std::vector<float>& hist1,
	const std::vector<float>& hist2, float alpha, int channel)
{
	if (hist1.size() != hist2.size())
		return;

	for (int c = 0; c < 3; c++) {
		current_histogram[c].clear();
		for (int i = 0; i < hist1.size(); i++)
			current_histogram[c].push_back(hist1[i] * alpha + hist2[i] * (1.0f - alpha));
	}
}


void TransferFunctionMultiChannelWidget::get_colormapf(std::vector<float> &color, std::vector<float> &opacity)
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

void TransferFunctionMultiChannelWidget::update_gpu_image()
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

void TransferFunctionMultiChannelWidget::update_colormap()
{
	colormap_changed = true;
    // We only change opacities for now, so go through and update the opacity
    // by blending between the neighboring control points
	for (int c = 0; c < 3; c++) {
		auto a_it = alpha_control_pts[c].begin();
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
			current_colormap[i * 4 + c] = static_cast<uint8_t>(clamp(alpha * 255.f, 0.f, 255.f));
		}
	}
	
	const size_t npixels = current_colormap.size() / 4;
	for (size_t i = 0; i < npixels; ++i)
	{
		current_colormap[i * 4 + 3] = static_cast<uint8_t>(clamp(((float) i)/ npixels * 255.f, 0.f, 255.f));
	}
}
