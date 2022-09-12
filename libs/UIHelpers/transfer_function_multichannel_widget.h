#ifndef TransferFunctionMultiChannelWidget_H_
#define TransferFunctionMultiChannelWidget_H_


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
#include <OpenGL/gl3.h>
#include <OpenGL/glext.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#endif



#include <cstdint>
#include <string>
#include <vector>
#include "imgui/imgui.h"
#include "transfer_function_widget.h"

class TransferFunctionMultiChannelWidget {
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

	std::vector<uint8_t> current_colormap;

	
	
	std::vector < size_t> selected_point = { (size_t)-1,(size_t)-1,(size_t)-1 };

	bool colormap_changed = true;
    GLuint colormap_img = -1;
	
	std::vector < std::vector<float> > current_histogram;
	
public:

	std::vector < std::vector<vec2f> > alpha_control_pts = { { vec2f(0.f), vec2f(1.f) }, { vec2f(0.f), vec2f(1.f) }, { vec2f(0.f), vec2f(1.f) } };

	TransferFunctionMultiChannelWidget();

    // Add a colormap preset. The image should be a 1D RGBA8 image
    void add_colormap(const tfnw::Colormap &map);

    // Add the transfer function UI into the currently active window
    void draw_ui();

    // Returns true if the colormap was updated since the last
    // call to draw_ui
    bool changed() const;

    // Get back the RGBA8 color data for the transfer function
    std::vector<uint8_t> get_colormap();

    // Get back the RGBA32F color data for the transfer function
    std::vector<float> get_colormapf();

	void setHistogram(const std::vector<float> &hist, int channel);

	void setBlendedHistogram(const std::vector<float>& hist1, const std::vector<float>& hist2, float alpha, int channel);
	
    // Get back the RGBA32F color data for the transfer function
    // as separate color and opacity vectors
    void get_colormapf(std::vector<float> &color, std::vector<float> &opacity);


		void draw_histogram();

	GLint get_colormap_gpu()
	{
		return colormap_img;
	}
	
private:
    void update_gpu_image();

    void update_colormap();
};

#endif