
#include "VolumeVisualizationApp.h"

#include <cmath>
#include <cctype>
#include "Data.h"
#include "LoadDataAction.h"
#include <glm/gtc/type_ptr.inl>
#include <glm/gtc/matrix_transform.hpp>
#include "glm.h"

float noColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };



VolumeVisualizationApp::VolumeVisualizationApp(int argc, char** argv) : VRApp(argc, argv), m_grab{ false }, m_scale{ 1.0f }, width{ 10 }, height{ 10 }, m_texture_update{ false }, m_texture_loaded{ false }
{
	int argc_int = this->getLeftoverArgc();
	char** argv_int = this->getLeftoverArgv();

	if (argc_int < 2)
		std::cerr << "You need to provide a folder to load" << std::endl;
	
	for (int i = 0; i < 2; i++){
		volumes.push_back(new Data<unsigned short>());
		std::cerr << "Loading " << argv_int[1] << std::endl;
		LoadDataAction(argv_int[1], volumes.back()).run();

		position.push_back({ (float) i, 0, 0 });
	}
	for (int i = 2; i < argc_int; i++)
	{
		m_models_filenames.push_back(argv_int[i]);
	}

	//LoadDataAction("D:\\Test_images_for_Ben_Knorlein\\GPA_test_Images_E5\\out").run();
	//LoadDataAction("D:\\data\\Beth\\row6\\row6\\r06c03f04").run();

	m_texture_update = true;
	m_object_pose = glm::mat4(1.0f);

	light_pos[0] = 0.0;
	light_pos[1] = 4.0;
	light_pos[2] = 0.0;
	light_pos[3] = 1.0;
}

VolumeVisualizationApp::~VolumeVisualizationApp()
{
	for (int i = 0; i < volumes.size(); i++){
		if (volumes[i]->texture_id() != 0)
			glDeleteTextures(1, &volumes[i]->texture_id());
		delete volumes[i];
	}
}

void VolumeVisualizationApp::updateTexture()
{
	for (int i = 0; i < volumes.size(); i++){
		if (volumes[i]->texture_id() != 0)
			glDeleteTextures(1, &volumes[i]->texture_id());

		glGenTextures(1, &volumes[i]->texture_id());
		glBindTexture(GL_TEXTURE_3D, volumes[i]->texture_id());
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S,
			GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T,
			GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R,
			GL_CLAMP);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER,
			GL_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER,
			GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAX_LEVEL, 4);
		glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB,
			volumes[i]->volume()->get_width(),
			volumes[i]->volume()->get_height(),
			volumes[i]->volume()->get_depth(),
			0, GL_RGB, GL_UNSIGNED_SHORT, volumes[i]->volume()->get(0, 0, 0, 0));

		glGenerateMipmap(GL_TEXTURE_3D);

		m_texture_scale[0] = 1.0f / (volumes[i]->volume()->get_x_scale() * volumes[i]->volume()->get_width());
		m_texture_scale[1] = 1.0f / (volumes[i]->volume()->get_y_scale() * volumes[i]->volume()->get_height());
		m_texture_scale[2] = 1.0f / (volumes[i]->volume()->get_z_scale() * volumes[i]->volume()->get_depth());
	}

	m_texture_update = false;
	m_texture_loaded = true;
}

void VolumeVisualizationApp::onAnalogChange(const VRAnalogEvent &event) {
    // This routine is called for all Analog_Change events.  Check event->getName()
    // to see exactly which analog input has been changed, and then access the
    // new value with event->getValue().
	//std::cerr <<"onAnalogChange " << event.getName() << std::endl;
	if (event.getName() == "HTC_Controller_Right_Joystick0_Y")
	{
		if (event.getValue() < 0.0) {
			m_scale += 0.01;
		} else if(event.getValue() > 0.0)
		{
			m_scale -= 0.01;
		}

	}

	/*onAnalogChange HTC_Controller_Left_Joystick0_X
		onAnalogChange HTC_Controller_Left_Joystick0_Y
		onAnalogChange HTC_Controller_Left_Trigger1
		onAnalogChange HTC_Controller_Left_Trigger2
		onAnalogChange HTC_Controller_Right_Joystick0_X
		onAnalogChange HTC_Controller_Right_Joystick0_Y
		onAnalogChange HTC_Controller_Right_Trigger1
		onAnalogChange HTC_Controller_Right_Trigger2*/
}


void VolumeVisualizationApp::onButtonDown(const VRButtonEvent &event) {
    // This routine is called for all Button_Down events.  Check event->getName()
    // to see exactly which button has been pressed down.
	//std::cerr << "onButtonDown " << event.getName() << std::endl;
	if (event.getName() == "KbdEsc_Down")
	{
		exit(0);
	}
	if (event.getName() == "HTC_Controller_Right_Axis1Button_Down")
	{
		m_grab = true;
	}
}


void VolumeVisualizationApp::onButtonUp(const VRButtonEvent &event) {
    // This routine is called for all Button_Up events.  Check event->getName()
    // to see exactly which button has been released.
	//std::cerr << "onButtonUp " << event.getName() << std::endl;
	if (event.getName() == "HTC_Controller_Right_Axis1Button_Up")
	{
		m_grab = false;
	}
}


void VolumeVisualizationApp::onTrackerMove(const VRTrackerEvent &event) {
    // This routine is called for all Tracker_Move events.  Check event->getName()
    // to see exactly which tracker has moved, and then access the tracker's new
    // 4x4 transformation matrix with event->getTransform().

	//std::cerr << "onTrackerMove " << event.getName() << std::endl;
	//HTC_Controller_Left_Move + HTC_Controller_Right_Move
	if (event.getName() == "HTC_Controller_Right_Move") {
		glm::mat4 new_pose = glm::make_mat4(event.getTransform());
		if (m_grab) {
			// Update the paintingToRoom transform based upon the new transform
			// of the left hand relative to the last frame.
			m_object_pose = new_pose * glm::inverse(m_controller_pose) * m_object_pose;
		}
		m_controller_pose = new_pose;
	}
	else if (event.getName() == "HTC_HMD_1_Move") {
		m_headpose = glm::make_mat4(event.getTransform());;
		//m_headpose = glm::inverse(m_headpose);
		light_pos[0] = m_headpose[3][0];
		light_pos[1] = m_headpose[3][1];
		light_pos[2] = m_headpose[3][2];
	}
}


    
void VolumeVisualizationApp::onRenderGraphicsContext(const VRGraphicsState &renderState) {
    // This routine is called once per graphics context at the start of the
    // rendering process.  So, this is the place to initialize textures,
    // load models, or do other operations that you only want to do once per
    // frame when in stereo mode.
    
    if (renderState.isInitialRenderCall()) {
       #ifndef __APPLE__
            glewExperimental = GL_TRUE;
            GLenum err = glewInit();
            if (GLEW_OK != err) {
                std::cout << "Error initializing GLEW." << std::endl;
            }
        #endif        

		m_slice_render.initGL();

		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
		glLightfv(GL_LIGHT0, GL_SPECULAR, noColor);
		glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, true);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_NORMALIZE);
		for (std::string filename : m_models_filenames){
			GLMmodel* pmodel = glmReadOBJ((char*) filename.c_str());
			glmFacetNormals(pmodel);
			glmVertexNormals(pmodel, 90.0);
			glColor4f(1.0, 1.0, 1.0,1.0);
			m_displayLists.push_back(glmList(pmodel, GLM_SMOOTH));
			glmDelete(pmodel);
		}
        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0f);
        glDepthFunc(GL_LEQUAL);
        glClearColor(0.0, 0.0, 0.0, 1);

		
    }
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
}


void VolumeVisualizationApp::onRenderGraphicsScene(const VRGraphicsState &renderState) {
    // This routine is called once per eye.  This is the place to actually
    // draw the scene.
	if (m_texture_update)
		updateTexture();

	P = glm::make_mat4(renderState.getProjectionMatrix());
	MV = glm::make_mat4(renderState.getViewMatrix());
	
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(P));

	//glMatrixMode(GL_MODELVIEW);
	//glLoadMatrixf(glm::value_ptr(MV));

	//glPushMatrix();
	//glMultMatrixf(glm::value_ptr(m_controller_pose));

	//glBegin(GL_QUADS);
	//glColor3f(1.0, 0, 0);
	//glVertex3f(-0.5, 0.0, 0.0);
	//glVertex3f(-0.5, 0.0, -1.0);
	//glVertex3f(0.5, 0.0, -1.0);
	//glVertex3f(0.5, 0.0, 0.0);
	//glEnd();
	//glPopMatrix();

	MV = MV*m_object_pose;
	MV = glm::scale(MV, glm::vec3(m_scale, m_scale, m_scale));
	MV = glm::scale(MV, glm::vec3(m_texture_scale[0] / m_texture_scale[0], m_texture_scale[0] / m_texture_scale[1], m_texture_scale[0] / m_texture_scale[2]));

	if (!m_displayLists.empty()){
		m_to_volume = glm::translate(glm::mat4(1.0f), glm::vec3(-0.5f, -0.5f, -0.5f));
		m_to_volume = glm::scale(m_to_volume, glm::vec3(m_texture_scale[0], m_texture_scale[1], m_texture_scale[2]));
	}

	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(MV));

	if (!m_displayLists.empty())
	{
		//scale
		glPushMatrix();
		glMultMatrixf(glm::value_ptr(m_to_volume));
		for (int i = 0; i < m_displayLists[i];i++)
			glCallList(m_displayLists[i]);

		glPopMatrix();
	}

	if (m_texture_loaded){
		//check order
		std::vector<std::pair< float, int> > order;
		for (int i = 0; i < volumes.size(); i++){
			glm::mat4 MV2 = glm::translate(MV, glm::vec3(position[i].x, position[i].y, position[i].z));
			glm::vec4 center = MV2*glm::vec4(0, 0, 0, 1);
			float l = glm::length(center);
			order.push_back(std::make_pair(l , i));
		}
		std::sort(order.begin(), order.end());

		for (int i = order.size() - 1; i >=0; i--){
			glBindTexture(GL_TEXTURE_3D, volumes[order[i].second]->texture_id());
			glm::mat4 MV2 = glm::translate(MV, glm::vec3(position[order[i].second].x, position[order[i].second].y, position[order[i].second].z));
			m_slice_render.render(MV2, P);

			glBindTexture(GL_TEXTURE_3D, 0);
		}
	}

	glFlush();
}