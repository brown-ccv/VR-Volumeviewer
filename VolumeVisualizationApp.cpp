
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

VolumeVisualizationApp::VolumeVisualizationApp(int argc, char** argv) : VRApp(argc, argv), m_grab{ false }
, m_scale{ 1.0f }, width{ 10 }, height{ 10 }, m_texture_update{ false }, m_texture_loaded{ false }, m_multiplier{ 1.0f }, m_threshold{ 0.0 }, m_shader_modifiers{false}, m_clipping{false}
{
	int argc_int = this->getLeftoverArgc();
	char** argv_int = this->getLeftoverArgv();

	if (argc_int < 2){
		std::cerr << "You need to provide a dataset to load" << std::endl;
		exit(EXIT_FAILURE);
	}

	std::ifstream inFile;
	inFile.open(argv_int[1]);

	std::string line;

	while (getline(inFile, line)) {
		if (line[0] != '#'){
			std::vector<std::string> vals; // Create vector to hold our words
			std::stringstream ss(line);
			std::string buf;

			while (ss >> buf){
				vals.push_back(buf);
			}
			if (vals.size() > 0){
				if (vals[0] == "mesh")
				{
					std::cerr << "Load Mesh " << vals[1] << std::endl;
					std::cerr << "for Volume " << vals[2] << std::endl;
					m_models_volumeID.push_back(stoi(vals[2]) - 1);
					m_models_filenames.push_back(vals[1]);
					m_models_MV.push_back(glm::mat4());
				}
				else if (vals[0] == "volume")
				{
					std::cerr << "Load volume " << vals[1] << std::endl;
					std::cerr << "Position " << vals[5] << " , " << vals[6] << " , " << vals[7] << std::endl;
					std::cerr << "Resolution " << vals[2] << " , " << vals[3] << " , " << vals[4] << std::endl;

					m_volumes.push_back(new Data<unsigned short>());
					float t_res[3];
					t_res[0] = stof(vals[2]);
					t_res[1] = stof(vals[3]);
					t_res[2] = stof(vals[4]);
					LoadDataAction(vals[1], m_volumes.back(), &t_res[0]).run();
					m_volume_position.push_back({ stof(vals[5]), stof(vals[6]), stof(vals[7]) });
					m_volume_scale.push_back({ 0.0, 0.0, 0.0 });
					m_volume_MV.push_back(glm::mat4());
				}
			}
		}
	}
	std::cerr << " Done loading" << std::endl;
	inFile.close();

	//LoadDataAction("D:\\Test_images_for_Ben_Knorlein\\GPA_test_Images_E5\\out").run();
	//LoadDataAction("D:\\data\\Beth\\row6\\row6\\r06c03f04").run();

	m_texture_update = true;
	m_object_pose = glm::mat4(1.0f);

	m_light_pos[0] = 0.0;
	m_light_pos[1] = 4.0;
	m_light_pos[2] = 0.0;
	m_light_pos[3] = 1.0;
}

VolumeVisualizationApp::~VolumeVisualizationApp()
{
	for (int i = 0; i < m_volumes.size(); i++){
		if (m_volumes[i]->texture_id() != 0)
			glDeleteTextures(1, &m_volumes[i]->texture_id());
		delete m_volumes[i];
	}
}

void VolumeVisualizationApp::updateTexture()
{
	for (int i = 0; i < m_volumes.size(); i++){
		std::cerr << "Update Texture " << i << std::endl;
		if (m_volumes[i]->texture_id() != 0)
			glDeleteTextures(1, &m_volumes[i]->texture_id());

		glGenTextures(1, &m_volumes[i]->texture_id());
		glBindTexture(GL_TEXTURE_3D, m_volumes[i]->texture_id());
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
			m_volumes[i]->volume()->get_width(),
			m_volumes[i]->volume()->get_height(),
			m_volumes[i]->volume()->get_depth(),
			0, GL_RGB, GL_UNSIGNED_SHORT, m_volumes[i]->volume()->get(0, 0, 0, 0));

		glGenerateMipmap(GL_TEXTURE_3D);

		m_volume_scale[i].x = 1.0f / (m_volumes[i]->volume()->get_x_scale() * m_volumes[i]->volume()->get_width());
		m_volume_scale[i].y = 1.0f / (m_volumes[i]->volume()->get_y_scale() * m_volumes[i]->volume()->get_height());
		m_volume_scale[i].z = 1.0f / (m_volumes[i]->volume()->get_z_scale() * m_volumes[i]->volume()->get_depth());
		//std::cerr << m_volume_scale[i].x << " , " << m_volume_scale[i].y << " , " << m_volume_scale[i].z << std::endl;
	}

	m_texture_update = false;
	m_texture_loaded = true;
	std::cerr << "End Update Textures" << std::endl;
}

void VolumeVisualizationApp::onAnalogChange(const VRAnalogEvent &event) {
    // This routine is called for all Analog_Change events.  Check event->getName()
    // to see exactly which analog input has been changed, and then access the
    // new value with event->getValue().
	//std::cerr <<"onAnalogChange " << event.getName() << std::endl;
	if (event.getName() == "HTC_Controller_Right_Joystick0_Y" || event.getName() == "HTC_Controller_1_Joystick0_Y" || 
		event.getName() == "HTC_Controller_Right_TrackPad0_Y" || event.getName() == "HTC_Controller_1_TrackPad0_Y")
	{
		if (event.getValue() > 0.5) {
			if (m_shader_modifiers)
			{
				m_multiplier += 0.01;
				std::cerr << "multiplier " << m_multiplier << std::endl;
			}
			else{
				m_scale += 0.01;
				std::cerr << "scale " << m_scale << std::endl;
			}
		} 
		else if(event.getValue() < -0.5)
		{
			if (m_shader_modifiers)
			{
				m_multiplier -= 0.01;
				m_multiplier = (m_multiplier < 0) ? 0.0f : m_multiplier;
				std::cerr << "multiplier " << m_multiplier << std::endl;
			}
			else{
				m_scale -= 0.01;
				m_scale = (m_scale < 0) ? 0.0f : m_scale;
				std::cerr << "scale " << m_scale << std::endl;
			}
		}
	}
	if (event.getName() == "HTC_Controller_Right_Joystick0_X" || event.getName() == "HTC_Controller_1_Joystick0_X" ||
		event.getName() == "HTC_Controller_Right_TrackPad0_X" || event.getName() == "HTC_Controller_1_TrackPad0_X")
	{
		if (event.getValue() > 0.5) {
			if (m_shader_modifiers)
			{
				m_threshold += 0.01;
				m_threshold = (m_threshold >1.0f) ? 1.0f : m_threshold;
				std::cerr << "threshold " << m_threshold << std::endl;
			}
		}
		else if (event.getValue() < -0.5)
		{
			if (m_shader_modifiers)
			{
				m_threshold -= 0.01;
				m_threshold = (m_threshold < 0) ? 0.0f : m_threshold;
				std::cerr << "threshold " << m_threshold << std::endl;
			}
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
	if (event.getName() == "HTC_Controller_Right_Axis1Button_Down" || event.getName() == "HTC_Controller_1_Axis1Button_Down")
	{
		m_grab = true;
	}
	if (event.getName() == "HTC_Controller_Right_GripButton_Down" || event.getName() == "HTC_Controller_1_GripButton_Down")
	{
		m_shader_modifiers = true;
		std::cerr << "multiplier mod on" << std::endl;
	}
	if (event.getName() == "HTC_Controller_Right_AButton_Down" || event.getName() == "HTC_Controller_1_AButton_Down")
	{
		m_clipping = true;
	}
}


void VolumeVisualizationApp::onButtonUp(const VRButtonEvent &event) {
    // This routine is called for all Button_Up events.  Check event->getName()
    // to see exactly which button has been released.
	std::cerr << "onButtonUp " << event.getName() << std::endl;
	if (event.getName() == "HTC_Controller_Right_Axis1Button_Up" || event.getName() == "HTC_Controller_1_Axis1Button_Up")
	{
		m_grab = false;
	}
	if (event.getName() == "HTC_Controller_Right_GripButton_Up" || event.getName() == "HTC_Controller_1_GripButton_Up")
	{
		m_shader_modifiers = false;
		std::cerr << "multiplier mod" << std::endl;
	}
	if (event.getName() == "HTC_Controller_Right_AButton_Up" || event.getName() == "HTC_Controller_1_AButton_Up")
	{
		m_clipping = false;
	}

}


void VolumeVisualizationApp::onTrackerMove(const VRTrackerEvent &event) {
    // This routine is called for all Tracker_Move events.  Check event->getName()
    // to see exactly which tracker has moved, and then access the tracker's new
    // 4x4 transformation matrix with event->getTransform().

	//std::cerr << "onTrackerMove " << event.getName() << std::endl;
	//HTC_Controller_Left_Move + HTC_Controller_Right_Move
	if (event.getName() == "HTC_Controller_Right_Move" || event.getName() == "HTC_Controller_1_Move") {
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
		m_light_pos[0] = m_headpose[3][0];
		m_light_pos[1] = m_headpose[3][1];
		m_light_pos[2] = m_headpose[3][2];
	}
}


    
void VolumeVisualizationApp::onRenderGraphicsContext(const VRGraphicsState &renderState) {
    // This routine is called once per graphics context at the start of the
    // rendering process.  So, this is the place to initialize textures,
    // load models, or do other operations that you only want to do once per
    // frame when in stereo mode.
	m_rendercount = 0;
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
			std::cerr << "Generate DisplayList " << filename  << std::endl;
			GLMmodel* pmodel = glmReadOBJ((char*) filename.c_str());
			glmFacetNormals(pmodel);
			glmVertexNormals(pmodel, 90.0);
			glColor4f(1.0, 1.0, 1.0,1.0);
			m_models_displayLists.push_back(glmList(pmodel, GLM_SMOOTH));
			glmDelete(pmodel);
		}
        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0f);
        glDepthFunc(GL_LEQUAL);
        glClearColor(0.0, 0.0, 0.0, 1);
    }
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, m_light_pos);
}


void VolumeVisualizationApp::onRenderGraphicsScene(const VRGraphicsState &renderState) {
    // This routine is called once per eye.  This is the place to actually
    // draw the scene.
	if (m_texture_update)
		updateTexture();

	if (renderState.isInitialRenderCall())
	{
		m_framebuffers.push_back(new FrameBufferObject());
	}

	//setup projection
	P = glm::make_mat4(renderState.getProjectionMatrix());	
	MV = glm::make_mat4(renderState.getViewMatrix());
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(P));

	//setup Modelview for volumes
	for (int i = 0; i < m_volumes.size(); i++){
		m_volume_MV[i] = MV;
		m_volume_MV[i] = m_volume_MV[i] * m_object_pose;
		m_volume_MV[i] = glm::scale(m_volume_MV[i], glm::vec3(m_scale, m_scale, m_scale));
		m_volume_MV[i] = glm::scale(m_volume_MV[i], glm::vec3(m_volume_scale[i].x / m_volume_scale[i].x, m_volume_scale[i].x / m_volume_scale[i].y, m_volume_scale[i].x / m_volume_scale[i].z));
		m_volume_MV[i] = glm::translate(m_volume_MV[i], glm::vec3(m_volume_position[i].x, m_volume_position[i].y, m_volume_position[i].z));
	}

	//setup Modelview for meshes
	for (int i = 0; i < m_models_displayLists.size(); i++){
		m_models_MV[i] = m_volume_MV[m_models_volumeID[i]];
		m_models_MV[i] = glm::translate(m_models_MV[i], glm::vec3(-0.5f, -0.5f, -0.5f));
		m_models_MV[i] = glm::scale(m_models_MV[i], glm::vec3(m_volume_scale[m_models_volumeID[i]].x, m_volume_scale[m_models_volumeID[i]].y, m_volume_scale[m_models_volumeID[i]].z));
	}

	//Render cuttingplane
	m_framebuffers[m_rendercount]->bind(m_clipping);
	if (m_clipping){
		glMatrixMode(GL_PROJECTION);
		glLoadMatrixf(glm::value_ptr(P));

		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(MV));

		glPushMatrix();
		glMultMatrixf(glm::value_ptr(m_controller_pose));

		glBegin(GL_QUADS);
		//glColor3f(1.0, 0, 0);
		glVertex3f(-100.0, 0.0, 100.0);
		glVertex3f(-100.0, 0.0, -100.0);
		glVertex3f(100.0, 0.0, -100.0);
		glVertex3f(100.0, 0.0, 100.0);
		glEnd();
		glPopMatrix();
	}
	m_framebuffers[m_rendercount]->unbind();

	//Render meshes
	for (int i = 0; i < m_models_displayLists.size(); i++){
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(m_models_MV[i]));
		glCallList(m_models_displayLists[i]);
	}

	//render volumes
	if (m_texture_loaded){
		m_slice_render.set_multiplier(m_multiplier);
		m_slice_render.set_threshold(m_threshold);
		//check order
		std::vector<std::pair< float, int> > order;
		for (int i = 0; i < m_volumes.size(); i++){
			glm::vec4 center = m_volume_MV[i] *glm::vec4(0, 0, 0, 1);
			float l = glm::length(center);
			order.push_back(std::make_pair(l , i));
		}
		std::sort(order.begin(), order.end());

		for (int i = order.size() - 1; i >=0; i--){
			glDepthMask(GL_FALSE);
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_3D, m_volumes[order[i].second]->texture_id());

			glActiveTexture(GL_TEXTURE0 + 1);
			glBindTexture(GL_TEXTURE_2D, m_framebuffers[m_rendercount]->depth_texture());

			m_slice_render.set_viewport(m_framebuffers[m_rendercount]->width(), m_framebuffers[m_rendercount]->height());
			m_slice_render.render(m_volume_MV[order[i].second], P);

			glBindTexture(GL_TEXTURE_2D, 0);

			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_3D, 0);

			glDepthMask(GL_TRUE);
		}
	}

	glFlush();
	m_rendercount++;
}