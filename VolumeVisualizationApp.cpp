
#include "VolumeVisualizationApp.h"

#include <cmath>
#include <cctype>
#include "LoadDataAction.h"
#include <glm/gtc/type_ptr.inl>
#include <glm/gtc/matrix_transform.hpp>
#include "glm.h"

float noColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };

VolumeVisualizationApp::VolumeVisualizationApp(int argc, char** argv) : VRApp(argc, argv), m_grab{ false }
, m_scale{ 1.0f }, width{ 10 }, height{ 10 }, m_texture_loaded{ false }, m_multiplier{ 1.0f }, m_threshold{ 0.0 }, m_shader_modifiers{false}
, m_clipping{ false }, m_animated(false), m_framerepeat{ 60 }, m_framecounter{ 0 }, m_tune{ false }
{
	int argc_int = this->getLeftoverArgc();
	char** argv_int = this->getLeftoverArgv();

	if (argc_int < 2){
		std::cerr << "You need to provide a dataset to load" << std::endl;
		exit(EXIT_FAILURE);
	} else
	{
		std::cerr << "Loading " << argv_int[1] << std::endl;
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
				if (vals[0] == "threshold")
				{
					m_threshold = std::stof(vals[1]);
				}
				else if (vals[0] == "multiplier")
				{
					m_multiplier = std::stof(vals[1]);
				}
				else if (vals[0] == "animated")
				{
					m_animated = true;
				}
				else if (vals[0] == "mesh")
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

					float t_res[3];
					t_res[0] = stof(vals[2]);
					t_res[1] = stof(vals[3]);
					t_res[2] = stof(vals[4]);

					m_volumes.push_back(LoadDataAction(vals[1], &t_res[0]).run());
					m_volumes.back()->set_volume_position({ stof(vals[5]), stof(vals[6]), stof(vals[7]) });
					m_volumes.back()->set_volume_scale({ 0.0, 0.0, 0.0 });
					m_volumes.back()->set_volume_mv(glm::mat4());

					m_volumes.back()->set_transfer_function(new TransferFunction());

					if (vals.size() > 8)
					{
						if (vals[8] == "raycast")
						{
							std::cerr << "Set RenderMethod raycast" << std::endl;
							m_volumes.back()->set_render_type(RAYCAST_RENDERER);

						} 
						else if(vals[8] == "slice")
						{
							std::cerr << "Set RenderMethod slices" << std::endl;
							m_volumes.back()->set_render_type(SLICE_RENDERER);
						} 
						else
						{
							std::cerr << "Could not interpret rendermethod, use slice or raycast. Using raycast as default." << std::endl;
							m_volumes.back()->set_render_type(RAYCAST_RENDERER);
						}
					}
					else
					{
						std::cerr << "No RenderMethod defined, using raycast." << std::endl;
						m_volumes.back()->set_render_type(RAYCAST_RENDERER);
					}

					if (vals.size() > 9)
					{
						std::cerr << "Set render channel to " << vals[9] << std::endl;
						m_volumes.back()->set_render_channel(std::stoi(vals[9]));
					}
				}
			}
		}
	}
	std::cerr << " Done loading" << std::endl;
	inFile.close();

	m_object_pose = glm::mat4(1.0f);

	m_light_pos[0] = 0.0;
	m_light_pos[1] = 4.0;
	m_light_pos[2] = 0.0;
	m_light_pos[3] = 1.0;

	m_renders.push_back(new VolumeSliceRenderer());
	m_renders.push_back(new VolumeRaycastRenderer());
}

VolumeVisualizationApp::~VolumeVisualizationApp()
{
	for (int i = 0; i < m_volumes.size(); i++){
		delete m_volumes[i];
	}
	m_volumes.clear();
}

void VolumeVisualizationApp::initTexture()
{
	for (int i = 0; i < m_volumes.size(); i++){
		std::cerr << "Init Texture " << i << std::endl;
		m_volumes[i]->initGL();
	}

	m_texture_loaded = true;
	std::cerr << "End Update Textures" << std::endl;
}

void VolumeVisualizationApp::onAnalogChange(const VRAnalogEvent &event) {
    // This routine is called for all Analog_Change events.  Check event->getName()
    // to see exactly which analog input has been changed, and then access the
    // new value with event->getValue().
	//std::cerr <<"onAnalogChange " << event.getName() << std::endl;
	if (event.getName() == "HTC_Controller_Right_Joystick2_Y" || event.getName() == "HTC_Controller_1_Joystick2_Y" 
		|| event.getName() == "HTC_Controller_Right_TrackPad0_Y" || event.getName() == "HTC_Controller_1_TrackPad0_Y"
			)
	{
		if (event.getValue() > 0.5) {
			if (m_shader_modifiers)
			{
				if (m_tune){
					m_multiplier += 0.01;
					std::cerr << "multiplier " << m_multiplier << std::endl;
				}
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
				if (m_tune){
					m_multiplier -= 0.01;
					m_multiplier = (m_multiplier < 0) ? 0.0f : m_multiplier;
					std::cerr << "multiplier " << m_multiplier << std::endl;
				}
			}
			else{
				m_scale -= 0.01;
				m_scale = (m_scale < 0) ? 0.0f : m_scale;
				std::cerr << "scale " << m_scale << std::endl;
			}
		}
	}
	if (m_tune){
		if (event.getName() == "HTC_Controller_Right_Joystick2_X" || event.getName() == "HTC_Controller_1_Joystick2_X"
			|| event.getName() == "HTC_Controller_Right_TrackPad0_X" || event.getName() == "HTC_Controller_1_TrackPad0_X"
			)
		{
			if (event.getValue() > 0.5) {
				if (m_shader_modifiers)
				{
					m_threshold += 0.001;
					m_threshold = (m_threshold > 1.0f) ? 1.0f : m_threshold;
					std::cerr << "th0eshold " << m_threshold << std::endl;
				}
			}
			else if (event.getValue() < -0.5)
			{
				if (m_shader_modifiers)
				{
					m_threshold -= 0.001;
					m_threshold = (m_threshold < 0) ? 0.0f : m_threshold;
					std::cerr << "threshold " << m_threshold << std::endl;
				}
			}
		}
	}
}


void VolumeVisualizationApp::onButtonDown(const VRButtonEvent &event) {
    // This routine is called for all Button_Down events.  Check event->getName()
    // to see exactly which button has been pressed down.
	//std::cerr << "onButtonDown " << event.getName() << std::endl;
	if (event.getName() == "KbdEsc_Down")
	{
		exit(0);
	}
	else if (event.getName() == "KbdA_Down")
	{
		m_animated = !m_animated;
		if (m_animated)
			std::cerr << "Animation ON" << std::endl;
		else
			std::cerr << "Animation OFF" << std::endl;
	}
	else if (event.getName() == "KbdT_Down")
	{
		m_tune = !m_tune;
		if (m_tune)
			std::cerr << "Tuning ON" << std::endl;
		else
			std::cerr << "Tuning OFF" << std::endl;
	}
	else if (event.getName() == "HTC_Controller_Right_Axis1Button_Down" || event.getName() == "HTC_Controller_1_Axis1Button_Down")
	{
		m_grab = true;
		std::cerr << "Grab ON" << std::endl;		
	}
	else if (event.getName() == "HTC_Controller_Right_GripButton_Down" || event.getName() == "HTC_Controller_1_GripButton_Down")
	{
		m_shader_modifiers = true;
		std::cerr << "Modifier tuning ON" << std::endl;
	}
	//else if (event.getName() == "HTC_Controller_Right_AButton_Down" || event.getName() == "HTC_Controller_1_AButton_Down")
	else if (event.getName() == "HTC_Controller_Right_Axis0Button_Down" || event.getName() == "HTC_Controller_1_Axis0Button_Down")
	{
		m_clipping = true;
		std::cerr << "Clipping ON" << std::endl;
	}
}


void VolumeVisualizationApp::onButtonUp(const VRButtonEvent &event) {
    // This routine is called for all Button_Up events.  Check event->getName()
    // to see exactly which button has been released.
	//std::cerr << "onButtonUp " << event.getName() << std::endl;
	if (event.getName() == "HTC_Controller_Right_Axis1Button_Up" || event.getName() == "HTC_Controller_1_Axis1Button_Up")
	{
		m_grab = false;
		std::cerr << "Grab OFF" << std::endl;
	}
	else if (event.getName() == "HTC_Controller_Right_GripButton_Up" || event.getName() == "HTC_Controller_1_GripButton_Up")
	{
		m_shader_modifiers = false;
		std::cerr << "Modifier tuning OFF" << std::endl;
	}
	//else if (event.getName() == "HTC_Controller_Right_AButton_Up" || event.getName() == "HTC_Controller_1_AButton_Up")
	else if (event.getName() == "HTC_Controller_Right_Axis0Button_Up" || event.getName() == "HTC_Controller_1_Axis0Button_Up")
	{
		m_clipping = false;
		std::cerr << "Clipping OFF" << std::endl;
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
    if (renderState.isInitialRenderCall()) {
       #ifndef __APPLE__
            glewExperimental = GL_TRUE;
            GLenum err = glewInit();
            if (GLEW_OK != err) {
                std::cout << "Error initializing GLEW." << std::endl;
            }
        #endif        

		for (auto ren : m_renders)
			ren->initGL();
	
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

	if (!m_texture_loaded)
		initTexture();

	if (m_animated)
	{
		m_framecounter++;
	}
}


void VolumeVisualizationApp::onRenderGraphicsScene(const VRGraphicsState &renderState) {
    // This routine is called once per eye.  This is the place to actually
    // draw the scene.

	//setup projection
	P = glm::make_mat4(renderState.getProjectionMatrix());	
	MV = glm::make_mat4(renderState.getViewMatrix());
	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(P));

	//setup Modelview for volumes
	for (int i = 0; i < m_volumes.size(); i++){
		glm::mat4 tmp = MV;
		tmp = tmp* m_object_pose;
		tmp = glm::scale(tmp, glm::vec3(m_scale, m_scale, m_scale));
		if(!m_animated) 
			tmp = glm::translate(tmp, glm::vec3(m_volumes[i]->get_volume_position().x, m_volumes[i]->get_volume_position().y, m_volumes[i]->get_volume_position().z));
		m_volumes[i]->set_volume_mv(tmp);
	}

	//setup Modelview for meshes
	for (int i = 0; i < m_models_displayLists.size(); i++){
		m_models_MV[i] = m_volumes[m_models_volumeID[i]]->get_volume_mv();
		m_models_MV[i] = glm::translate(m_models_MV[i], glm::vec3(-0.5f, -0.5f, -0.5f));
	}

	//Set cuttingplane
	if (m_clipping){
		glm::mat4 clipPlane = inverse(m_controller_pose) * inverse(MV);
		for (auto ren : m_renders)
			ren->setClipping(true, &clipPlane);
	} else
	{
		for (auto ren : m_renders)
			ren->setClipping(false, nullptr);
	}

	//Render meshes
	for (int i = 0; i < m_models_displayLists.size(); i++){
		glMatrixMode(GL_MODELVIEW);
		glLoadMatrixf(glm::value_ptr(m_models_MV[i]));
		glCallList(m_models_displayLists[i]);
	}

	//render volumes
	if (m_texture_loaded){
		for (auto ren : m_renders){
			ren->set_multiplier(m_multiplier);
			ren->set_threshold(m_threshold);
		}
		
		if (m_animated)
		{
			unsigned int active_volume = (m_framecounter / m_framerepeat) % m_volumes.size();
			m_renders[m_volumes[active_volume]->render_type()]->render(m_volumes[active_volume], m_volumes[active_volume]->get_volume_mv(), P, m_volumes[active_volume]->get_volume_scale().x / m_volumes[active_volume]->get_volume_scale().z);
		}
		else{
			//check order
			std::vector<std::pair< float, int> > order;
			for (int i = 0; i < m_volumes.size(); i++){
				glm::vec4 center = m_volumes[i]->get_volume_mv() * glm::vec4(0, 0, 0, 1);
				float l = glm::length(center);
				order.push_back(std::make_pair(l, i));
			}
			std::sort(order.begin(), order.end());

			for (int i = order.size() - 1; i >= 0; i--){
				m_renders[m_volumes[order[i].second]->render_type()]->render(m_volumes[order[i].second], m_volumes[order[i].second]->get_volume_mv(), P, m_volumes[order[i].second]->get_volume_scale().x / m_volumes[order[i].second]->get_volume_scale().z);
			}
		}
	}

	glFlush();
}