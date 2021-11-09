#ifndef TransferFunctionWidget_H_
#define TransferFunctionWidget_H_


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
#include "Vec2.h"

struct Colormap {
    std::string name;
    // An RGBA8 1D image
    std::vector<uint8_t> colormap;

    Colormap(const std::string &name, const std::vector<uint8_t> &img);
};

class TransferFunctionWidget {
  

    std::vector<Colormap> colormaps;
    
    std::vector<uint8_t> current_colormap;

   
    size_t selected_point = -1;

    bool colormap_changed = true;
    GLuint colormap_img = -1;

	std::vector<float> current_histogram;
	float m_min_max_val[2];

public:

  
  size_t selected_colormap = 0;
  std::vector<vec2f> alpha_control_pts = { vec2f(0.f), vec2f(1.f) };

    TransferFunctionWidget();

    // Add a colormap preset. The image should be a 1D RGBA8 image
    void add_colormap(const Colormap &map);

    // Add the transfer function UI into the currently active window
    void draw_ui();

    // Returns true if the colormap was updated since the last
    // call to draw_ui
    bool changed() const;

    // Get back the RGBA8 color data for the transfer function
    std::vector<uint8_t> get_colormap();

    // Get back the RGBA32F color data for the transfer function
    std::vector<float> get_colormapf();

	void setHistogram(const std::vector<float> &hist);

  std::vector<float>& getHistogram();

	void setMinMax(const float min, const float max);

	void setBlendedHistogram(const std::vector<float>& hist1, const std::vector<float>& hist2, float alpha);

    // Get back the RGBA32F color data for the transfer function
    // as separate color and opacity vectors
    void get_colormapf(std::vector<float> &color, std::vector<float> &opacity);

	GLint get_colormap_gpu()
	{
		return colormap_img;
	}

  void set_colormap_gpu(GLint colormap)
  {
    colormap_img = colormap;
  }
	
	void drawLegend();

  void draw_histogram();

  void update_colormap();

  void set_Quantiles(float min, float max);

  void get_Quantiles(float& min, float& max);

private:
    void update_gpu_image();

    

    void load_embedded_preset(const uint8_t *buf, size_t size, const std::string &name);
    
    float m_quantiles[2];
    
};


#endif