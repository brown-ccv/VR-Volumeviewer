#ifndef VOLUMEVISUALIZATIONAPP_H
#define VOLUMEVISUALIZATIONAPP_H

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

#include <api/MinVR.h>
#include "render/FrameBufferObject.h"

#include <vector>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#define M_PI 3.14159265358979323846
#endif
#include <chrono>

class VRVolumeApp;

class VolumeVisualizationApp : public MinVR::VRApp
{
public:
  /** The constructor passes argc, argv, and a MinVR config file on to VRApp.
   */
  VolumeVisualizationApp(int argc, char **argv);
  virtual ~VolumeVisualizationApp();

  /** USER INTERFACE CALLBACKS **/

  virtual void onCursorMove(const MinVR::VRCursorEvent &state);

  virtual void onAnalogChange(const MinVR::VRAnalogEvent &state);

  virtual void onButtonDown(const MinVR::VRButtonEvent &state);

  virtual void onButtonUp(const MinVR::VRButtonEvent &state);

  virtual void onTrackerMove(const MinVR::VRTrackerEvent &state);

  virtual void onGenericEvent(const MinVR::VRDataIndex &index);

  /** RENDERING CALLBACKS **/

  virtual void onRenderGraphicsScene(const MinVR::VRGraphicsState &state);

  virtual void onRenderGraphicsContext(const MinVR::VRGraphicsState &state);

private:
  int width;
  int height;

  GLfloat m_light_pos[4];

  unsigned int rendercount;

  std::chrono::steady_clock::time_point m_lastTime;

  std::unique_ptr<VRVolumeApp> m_vrVolumeApp;

  int m_num_frames;

  const double fps_limit = 1.0 / 60.0;
  std::chrono::steady_clock::time_point last_Update_Time; // number of seconds since the last loop
};

#endif