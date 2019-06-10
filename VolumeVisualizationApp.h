#ifndef VOLUMEVISUALIZATIONAPP_H
#define VOLUMEVISUALIZATIONAPP_H

#include "GL/glew.h"

#include <api/MinVR.h>
#include "FrameBufferObject.h"
#include "Volume.h"
#include "VolumeRaycastRenderer.h"
using namespace MinVR;

#include <vector>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#define M_PI 3.14159265358979323846
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include "VolumeSliceRenderer.h"



class VolumeVisualizationApp : public VRApp {
public:
    
    /** The constructor passes argc, argv, and a MinVR config file on to VRApp.
     */
    VolumeVisualizationApp(int argc, char** argv);
    virtual ~VolumeVisualizationApp();
 
    /** USER INTERFACE CALLBACKS **/
    
    virtual void onAnalogChange(const VRAnalogEvent &state);
    
    virtual void onButtonDown(const VRButtonEvent &state);
    
    virtual void onButtonUp(const VRButtonEvent &state);
    
    virtual void onTrackerMove(const VRTrackerEvent &state);
    
    /** RENDERING CALLBACKS **/
    
    virtual void onRenderGraphicsScene(const VRGraphicsState& state);
    
    virtual void onRenderGraphicsContext(const VRGraphicsState& state);
      
private:
	void updateTexture();

	bool m_texture_loaded;

	int width;
	int height;

	glm::mat4 P;
	glm::mat4 MV;

	float m_scale;

	bool m_grab;
	bool m_shader_modifiers;
	bool m_clipping;
	bool m_animated;
	bool m_tune;

	unsigned int m_framerepeat;
	unsigned int m_framecounter;

	glm::mat4 m_controller_pose;
	glm::mat4 m_object_pose;
	glm::mat4 m_to_volume;
	glm::mat4 m_headpose;

	std::vector <std::string> m_models_filenames;
	std::vector <unsigned int> m_models_displayLists;
	std::vector<pt> m_models_position;
	std::vector<int> m_models_volumeID;
	std::vector<glm::mat4> m_models_MV;

	GLfloat m_light_pos[4];
	
	std::vector< Volume * > m_volumes;
	
	float m_multiplier;
	float m_threshold;

	std::vector <VolumeRenderer*> m_renders;
};


#endif