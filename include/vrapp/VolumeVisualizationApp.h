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


#include "VRMenuHandler.h"
#include "UIHelpers/transfer_function_multichannel_widget.h"
#include "UIHelpers/transfer_function_widget.h"
#include <api/MinVR.h>
#include "render/FrameBufferObject.h"
#include "render/Volume.h"
#include "interaction/LabelsManager.h"
#include "render/VolumeRaycastRenderer.h"
#include "render/DepthTexture.h"
#include <future>



using namespace MinVR;

#include <vector>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#define M_PI 3.14159265358979323846
#endif

#include "../render/VolumeSliceRenderer.h"
#include "../interaction/CreateMovieAction.h"


#include "ShaderProgram.h"

#include "VRVolumeApp.h"

class Model;
class Texture;
class UIView;
class VRVolumeApp;

class VolumeVisualizationApp : public VRApp {
public:

  /** The constructor passes argc, argv, and a MinVR config file on to VRApp.
   */
  VolumeVisualizationApp(int argc, char** argv);
  virtual ~VolumeVisualizationApp();



  /** USER INTERFACE CALLBACKS **/

  virtual void onCursorMove(const VRCursorEvent& state);

  virtual void onAnalogChange(const VRAnalogEvent& state);

  virtual void onButtonDown(const VRButtonEvent& state);

  virtual void onButtonUp(const VRButtonEvent& state);

  virtual void onTrackerMove(const VRTrackerEvent& state);

  virtual void onGenericEvent(const VRDataIndex& index);

  /** RENDERING CALLBACKS **/

  virtual void onRenderGraphicsScene(const VRGraphicsState& state);

  virtual void onRenderGraphicsContext(const VRGraphicsState& state);



private:

  int width;
  int height;


  GLfloat m_light_pos[4];


  unsigned int rendercount;




  std::chrono::steady_clock::time_point m_lastTime;



  VRVolumeApp* m_vrVolumeApp;

  int m_num_frames;

  const double fps_Limit = 1.0 / 60.0;
  std::chrono::steady_clock::time_point last_Update_Time ;  // number of seconds since the last loop
  
  float v1 = 0.0f;
  float v2 = 100.0f;

  //std::vector<glm::vec3> control_points 
  glm::vec3 pos1 = glm::vec3(1.0f, 0.0f, 10.0f);
  glm::vec3 pos2 = glm::vec3(50.0f, 150.0f, 50.0f);
  glm::vec3 pos3 = glm::vec3(-10.0f, 100.0f, 250.0f);
  glm::vec3 pos4 = glm::vec3(-10.0f, 180.0f, 290.0f);
};


#endif