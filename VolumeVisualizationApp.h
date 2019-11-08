#ifndef VOLUMEVISUALIZATIONAPP_H
#define VOLUMEVISUALIZATIONAPP_H

#include "GL/glew.h"
#include "VRMenuHandler.h"
#include "transferfunction/transfer_function_widget.h"
#include "imfilebrowser.h"
#include <api/MinVR.h>
#include "FrameBufferObject.h"
#include "Volume.h"
#include "VolumeRaycastRenderer.h"
#include "DepthTexture.h"
#include <future>
#include "ArcBall.h"

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

	void ui_callback();

	virtual void onCursorMove(const VRCursorEvent& state);
	
    virtual void onAnalogChange(const VRAnalogEvent &state);
    
    virtual void onButtonDown(const VRButtonEvent &state);
    
    virtual void onButtonUp(const VRButtonEvent &state);
    
    virtual void onTrackerMove(const VRTrackerEvent &state);
    
    /** RENDERING CALLBACKS **/
    
    virtual void onRenderGraphicsScene(const VRGraphicsState& state);
    
    virtual void onRenderGraphicsContext(const VRGraphicsState& state);

	void loadVolume(std::vector<std::string> vals, promise<Volume*>* promise);
	
private:
	void addLodadedTextures();
	void initTexture();

	int width;
	int height;

	glm::mat4 P;
	glm::mat4 MV;

	float m_scale;
	int m_slices;
	
	bool m_grab;
	bool m_clipping;
	bool m_animated;

	float m_speed;
	float m_frame;

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
	int m_rendermethod;
	int m_renderchannel;
	bool m_use_transferfunction;
	
	std::vector <VolumeRenderer*> m_renders;
	std::vector <DepthTexture*> m_depthTextures;
	unsigned int rendercount;

	VRMenuHandler* m_menu_handler;

	TransferFunctionWidget tfn_widget;
	ImGui::FileBrowser fileDialog;
	
	std::vector <promise<Volume*> * > promises;
	std::vector <future<Volume*>> futures;
	std::vector <std::thread *> threads;

	std::chrono::steady_clock::time_point m_lastTime;
	bool m_dynamic_slices;
	float m_fps;
	bool m_show_menu;

	//2D
	bool m_is2d;
	ArcBall m_trackball;
	
};


#endif