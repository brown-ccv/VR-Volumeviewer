#pragma once
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
#include <cstdint>
#include <string>
#include <vector>
#include "imgui.h"
#include "Vec2.h"

namespace tfnw
{

    enum class ColorSpace
    {
        LINEAR,
        SRGB
    };

    struct Colormap
    {
        std::string name;
        // An RGBA8 1D image
        std::vector<uint8_t> colormap;
        ColorSpace color_space;

        Colormap(const std::string &name,
                 const std::vector<uint8_t> &img,
                 const ColorSpace color_space);
    };

    class TransferFunctionWidget
    {

        std::vector<Colormap> colormaps;

        std::vector<uint8_t> current_colormap;

        size_t selected_point = -1;

        bool clicked_on_item = false;
        bool gpu_image_stale = true;
        bool colormap_changed = true;
        GLuint colormap_img = -1;

    public:
        TransferFunctionWidget();

        std::vector<vec2f> alpha_control_pts = {vec2f(0.f), vec2f(1.f)};

        size_t selected_colormap = 0;

        // Add a colormap preset. The image should be a 1D RGBA8 image, if the image
        // is provided in sRGBA colorspace it will be linearized
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

        void draw_legend();

        void draw_legend(float legend_pos_x, float legend_pos_y, float legend_width, float legend_height);

        void update_colormap();

        void set_quantiles(float min, float max);

        void get_quantiles(float &min, float &max);

        std::vector<float> current_histogram;

        float m_min_max_val[2];

        void setHistogram(const std::vector<float> &hist);

        std::vector<float> &getHistogram();

        void setMinMax(const float min, const float max);

    private:
        void update_gpu_image();

        void load_embedded_preset(const uint8_t *buf, size_t size, const std::string &name);

        float m_quantiles[2];
    };
}
