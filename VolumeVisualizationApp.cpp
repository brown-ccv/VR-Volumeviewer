
#include "VolumeVisualizationApp.h"

#include <cmath>
#include <cctype>
#include "LoadDataAction.h"
#ifdef WITH_TEEM
	#include "LoadNrrdAction.h"
#endif
#include "HelperFunctions.h"
#include <glm/gtc/type_ptr.inl>
#include <glm/gtc/matrix_transform.hpp>
#include "glm.h"

#include <filesystem>
namespace fs = std::filesystem;


float noColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
float diffuse[] = { 0.8f, 0.8f, 0.8f, 1.0f };

VolumeVisualizationApp::VolumeVisualizationApp(int argc, char** argv) : VRApp(argc, argv), m_grab{ false }
, m_scale{ 1.0f }, width{ 10 }, height{ 10 }, m_multiplier{ 1.0f }, m_threshold{ 0.0 }, m_is2d(false), m_menu_handler(NULL), m_lookingGlass{false}
, m_clipping{ false }, m_animated(false), m_speed{ 0.01 }, m_frame{ 0.0 }, m_slices(256), m_rendermethod{ 1 }, m_renderchannel{ 0 }, m_use_transferfunction{ false }, m_use_multi_transfer{ false }, m_dynamic_slices{ false }, m_show_menu{ true }, convert{ false }, m_stopped{false}
{
	int argc_int = this->getLeftoverArgc();
	char** argv_int = this->getLeftoverArgv();

	if (argc_int >= 2) {
		for (int i = 1; i < argc_int; i++) {
			if (std::string(argv_int[i]) == std::string("use2DUI"))
			{
				m_is2d = true;
			}
			if (std::string(argv_int[i]) == std::string("useHolo"))
			{
				m_lookingGlass = true;
				m_speed = 0.5;
			}
			else if (std::string(argv_int[i]) == std::string("convert")) {
				convert = true;
			}
			else if(helper::ends_with_string(std::string(argv_int[i]),".txt"))
			{
				loadTxtFile(std::string(argv_int[i]));
			}
			else if(helper::ends_with_string(std::string(argv_int[i]), ".nrrd")){
				std::vector<std::string> vals;
				vals.push_back(std::string(argv_int[i]));
				promises.push_back(new std::promise<Volume*>);
				futures.push_back(promises.back()->get_future());
				threads.push_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, promises.back()));
			}
		}
	}

	m_object_pose = glm::mat4(1.0f);

	m_light_pos[0] = 0.0;
	m_light_pos[1] = 4.0;
	m_light_pos[2] = 0.0;
	m_light_pos[3] = 1.0;

	m_renders.push_back(new VolumeSliceRenderer());
	m_renders.push_back(new VolumeRaycastRenderer());

	std::cerr << " Done loading" << std::endl;
	float fontsize = 2.0;
	if (m_is2d) {
		fontsize = 1.0;
	}
	if (m_lookingGlass) { 
		fontsize = 3.0;
	}


	m_menu_handler = new VRMenuHandler(m_is2d);
	VRMenu * menu = m_menu_handler->addNewMenu(std::bind(&VolumeVisualizationApp::ui_callback, this), 1024, 1024, 1, 1, fontsize);
	menu->setMenuPose(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 2, -1, 1));

	fileDialog.SetTitle("load data");
#ifdef WITH_NRRD
	fileDialog.SetTypeFilters({ ".txt", ".nrrd" });
#elseif
	fileDialog.SetTypeFilters({ ".txt"});
#endif
}

VolumeVisualizationApp::~VolumeVisualizationApp()
{
	for (int i = 0; i < m_volumes.size(); i++){
		delete m_volumes[i];
	}
	m_volumes.clear();
}

void VolumeVisualizationApp::loadTxtFile(std::string filename)
{
	std::ifstream inFile;
	inFile.open(filename);

	std::string line;

	fs::path p_filename(filename);

	while (getline(inFile, line)) {
		if (line[0] != '#') {
			std::vector<std::string> vals; // Create vector to hold our words
			std::stringstream ss(line);
			std::string buf;

			while (ss >> buf) {
				vals.push_back(buf);
			}
			if (vals.size() > 0) {
				if (vals[0] == "animated")
				{
					m_animated = true;
				}
				if (vals[0] == "threshold")
				{
					m_threshold = stof(vals[1]);
				}
				if (vals[0] == "mesh")
				{
					std::cerr << "Load Mesh " << vals[1] << std::endl;
					std::cerr << "for Volume " << vals[2] << std::endl;
					vals[1] = p_filename.parent_path().string() + OS_SLASH + vals[1];
					m_models_volumeID.push_back(stoi(vals[2]) - 1);
					m_models_filenames.push_back( vals[1]);
					m_models_MV.push_back(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
				}
				else if (vals[0] == "volume")
				{
					vals[1] = p_filename.parent_path().string() + OS_SLASH + vals[1];
					promises.push_back(new std::promise<Volume*>);
					futures.push_back(promises.back()->get_future());
					threads.push_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, promises.back()));
				}
			}
		}
	}
	inFile.close();
}

void VolumeVisualizationApp::loadVolume(std::vector<std::string> vals, std::promise<Volume*>* promise)
{
	if(vals.size() == 1 && helper::ends_with_string(vals[0],".nrrd"))
	{
#ifdef WITH_TEEM
		Volume* volume = LoadNrrdAction(vals[0]).run();
		volume->set_volume_position({ 0.0, 0.0, 0.0 });
		volume->set_volume_scale({ 0.0, 0.0, 0.0 });
		volume->set_volume_mv(glm::mat4());
		promise->set_value(volume);;
#endif
	}
	else {
		std::cerr << "Load volume " << vals[1] << std::endl;
		std::cerr << "Position " << vals[5] << " , " << vals[6] << " , " << vals[7] << std::endl;
		std::cerr << "Resolution " << vals[2] << " , " << vals[3] << " , " << vals[4] << std::endl;

		float t_res[3];
		t_res[0] = stof(vals[2]);
		t_res[1] = stof(vals[3]);
		t_res[2] = stof(vals[4]);

		Volume* volume = LoadDataAction(vals[1], &t_res[0]).run(convert);

		volume->set_volume_position({ stof(vals[5]), stof(vals[6]), stof(vals[7]) });
		volume->set_volume_scale({ 0.0, 0.0, 0.0 });
		volume->set_volume_mv(glm::mat4());

		if (vals.size() > 9)
		{
			std::cerr << "Set render channel to " << vals[9] << std::endl;
			volume->set_render_channel(std::stoi(vals[9]));
		}
		promise->set_value(volume);;
		std::cerr << "end load" << std::endl;
	}
}

void VolumeVisualizationApp::addLodadedTextures()
{
	bool allready = true;
	for (auto& f : futures)
	{
		allready = allready & f._Is_ready();
	}

	if(allready)
	{
		for (int i =0; i < futures.size(); i++)
		{
			m_volumes.push_back(futures[i].get());
			threads[i]->join();
			delete threads[i];
			delete promises[i];
		}
		threads.clear();
		promises.clear();
		futures.clear();
	}
}

void VolumeVisualizationApp::ui_callback()
{
	if(m_is2d){
		if (m_lookingGlass) {
			ImGui::SetNextWindowSize(ImVec2(1200, 1400), ImGuiCond_Once);
		}
		else {
			ImGui::SetNextWindowSize(ImVec2(500, 700), ImGuiCond_Once);
		}
		ImGui::SetNextWindowPos(ImVec2(40, 40), ImGuiCond_Once);
	}
	
	ImGui::Begin("Volumeviewer");
	
	// open file dialog when user clicks this button
	if (ImGui::Button("load file", ImVec2(ImGui::GetWindowSize().x * 0.5f - 1.5* ImGui::GetStyle().ItemSpacing.x, 0.0f)))
		fileDialog.Open();
	ImGui::SameLine();
	if (ImGui::Button("Clear all", ImVec2(ImGui::GetWindowSize().x  * 0.5f - 1.5 * ImGui::GetStyle().ItemSpacing.x, 0.0f)))
	{
		for (int i = 0; i < m_volumes.size(); i++) {
			delete m_volumes[i];
		}
		m_volumes.clear();
	}

	ImGui::SliderFloat("alpha multiplier", &m_multiplier, 0.0f, 1.0f, "%.3f");
	ImGui::SliderFloat("threshold", &m_threshold, 0.0f, 1.0f, "%.3f");
	ImGui::SliderFloat("scale", &m_scale, 0.0f, 5.0f, "%.3f");
	ImGui::SliderInt("Slices", &m_slices, 10, 1024, "%d");
	ImGui::Checkbox("automatic slice adjustment", &m_dynamic_slices);
	
	ImGui::SameLine(ImGui::GetWindowSize().x * 0.5f, 0);
	ImGui::Text("FPS = %f",m_fps);
	const char* items[] = { "sliced" , "raycast" };
	ImGui::Combo("RenderMethod",&m_rendermethod,items, IM_ARRAYSIZE(items));
	
	const char* items_channel[] = { "based on data" , "red", "green" , "blue", "alpha", "rgba", "rgba with alpha as max rgb" };
	ImGui::Combo("Render Channel", &m_renderchannel, items_channel, IM_ARRAYSIZE(items_channel));
	
	ImGui::Checkbox("use transferfunction", &m_use_transferfunction);
	if (m_use_transferfunction) {
		if (m_volumes.size() > 0 && m_volumes[0]->get_channels() > 1 && (m_renderchannel == 0 || m_renderchannel == 5 || m_renderchannel == 6)){
			for (int i = 0; i < 3; i++) {
				if (m_animated)
				{
					unsigned int active_volume = floor(m_frame);
					unsigned int active_volume2 = ceil(m_frame);
					double alpha = m_frame - active_volume;
					if (active_volume < m_volumes.size() && active_volume2 < m_volumes.size())
						tfn_widget_multi.setBlendedHistogram(m_volumes[active_volume]->getTransferfunction(i), m_volumes[active_volume2]->getTransferfunction(i), alpha, i);
				}
				else {
					tfn_widget_multi.setHistogram(m_volumes[0]->getTransferfunction(i), i);
				}
			}
			m_use_multi_transfer = true;
			tfn_widget_multi.draw_ui();
		}
		else
		{
			if (m_animated)
			{
				unsigned int active_volume = floor(m_frame);
				unsigned int active_volume2 = ceil(m_frame);
				double alpha = m_frame - active_volume;
				tfn_widget.setMinMax(m_volumes[active_volume]->getMin() * alpha + m_volumes[active_volume2]->getMin() * (1.0 - alpha),
					m_volumes[active_volume]->getMax() * alpha + m_volumes[active_volume2]->getMax() * (1.0 - alpha));
				if (active_volume < m_volumes.size() && active_volume2 < m_volumes.size())
					tfn_widget.setBlendedHistogram(m_volumes[active_volume]->getTransferfunction(0), m_volumes[active_volume2]->getTransferfunction(0), alpha);
			}else if(m_volumes.size() > 0) {
				tfn_widget.setHistogram(m_volumes[0]->getTransferfunction(0));
				tfn_widget.setMinMax(m_volumes[0]->getMin(), m_volumes[0]->getMax());
			}
			m_use_multi_transfer = false;
			tfn_widget.draw_ui();
		}
	}

	if (m_animated) {
		ImGui::Text("Timestep");
		ImGui::SameLine();
		ImGui::SetNextItemWidth(-100 - ImGui::GetStyle().ItemSpacing.x);
		float m_frame_display = m_frame + 1;
		ImGui::SliderFloat("", &m_frame_display,1, m_volumes.size());
		m_frame = m_frame_display - 1;
		ImGui::SameLine();

		std::string text = m_stopped ? "Play" : "Stop";
		if (ImGui::Button(text.c_str(),ImVec2(100,0))) {
			m_stopped = !m_stopped;
		}
	}

	//file loading
	fileDialog.Display();

	if (fileDialog.HasSelected())
	{
		if (helper::ends_with_string(fileDialog.GetSelected().string(), ".txt"))
		{
#ifdef WITH_TEEM
			loadTxtFile(fileDialog.GetSelected().string());
#endif
		}
		else if (helper::ends_with_string(fileDialog.GetSelected().string(), ".nrrd")) {
			std::vector<std::string> vals;
			vals.push_back(fileDialog.GetSelected().string());	
			promises.push_back(new std::promise<Volume*>);
			futures.push_back(promises.back()->get_future());
			threads.push_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, promises.back()));
		}
		fileDialog.ClearSelected();
	}
	
	ImGui::End();
}

void VolumeVisualizationApp::initTexture()
{
	addLodadedTextures();
	for (int i = 0; i < m_volumes.size(); i++){
		m_volumes[i]->initGL();
	}

}

void VolumeVisualizationApp::onCursorMove(const VRCursorEvent& event)
{
	if (event.getName() == "Mouse_Move" && m_menu_handler != NULL)
	{
		m_trackball.mouseMove(event.getPos()[0], event.getPos()[1]);
		m_menu_handler->setCursorPos(event.getPos()[0], event.getPos()[1]);
	}
}

#define count_Packages 
#ifdef count_Packages
	int last_received = 0;
#endif count_Packages

void VolumeVisualizationApp::onAnalogChange(const VRAnalogEvent &event) {
	if (m_show_menu && m_menu_handler != NULL && m_menu_handler->windowIsActive()) {
		if (event.getName() == "HTC_Controller_Right_TrackPad0_Y" || event.getName() == "HTC_Controller_1_TrackPad0_Y"
			|| (event.getName() == "Wand_Joystick_Y_Update" && !(event.getValue() > -0.1 && event.getValue() < 0.1) ))
				
			m_menu_handler->setAnalogValue(event.getValue());
	}

	if (event.getName() == "PhotonLoopFinished") {
		if (!m_is2d)
			m_menu_handler->renderToTexture();
		else
			m_menu_handler->drawMenu();

		if (last_received + 1 != event.getValue()) {
			std::cerr << "Problem with package , received " << event.getValue() << " expected "  << last_received + 1 << std::endl;
		}
		last_received = event.getValue();
	}
}


void VolumeVisualizationApp::onButtonDown(const VRButtonEvent &event) {
	if (m_menu_handler != NULL && m_menu_handler->windowIsActive()) {
		if (event.getName() == "MouseBtnLeft_Down")
		{
			m_menu_handler->setButtonClick(0, 1);
		}
		else if (event.getName() == "MouseBtnRight_Down")
		{
			m_menu_handler->setButtonClick(1, 1);
		}
	}else
	{
		if (event.getName() == "MouseBtnLeft_Down")
		{
			m_trackball.mouse_pressed(0, true);
		}
		else if (event.getName() == "MouseBtnRight_Down")
		{
			m_trackball.mouse_pressed(1, true);
		}
	}

	if (m_show_menu && m_menu_handler != NULL && m_menu_handler->windowIsActive()) {
		if (event.getName() == "HTC_Controller_Right_Axis1Button_Down" || event.getName() == "HTC_Controller_1_Axis1Button_Down" || event.getName() == "B10_Down")
		{
			//left click
			m_menu_handler->setButtonClick(0, 1);
		}
		else if (event.getName() == "HTC_Controller_Right_GripButton_Down" || event.getName() == "HTC_Controller_1_GripButton_Down" || event.getName() == "Wand_Right_Btn_Down")
		{
			//middle click
			m_menu_handler->setButtonClick(2, 1);
		}
		//else if (event.getName() == "HTC_Controller_Right_AButton_Down" || event.getName() == "HTC_Controller_1_AButton_Down")
		else if (event.getName() == "HTC_Controller_Right_Axis0Button_Down" || event.getName() == "HTC_Controller_1_Axis0Button_Down" || event.getName() == "B08_Down" )
		{
			//right click
			m_menu_handler->setButtonClick(1, 1);
		}
	}
	else {
		// This routine is called for all Button_Down events.  Check event->getName()
		// to see exactly which button has been pressed down.
		//std::cerr << "onButtonDown " << event.getName() << std::endl;
		if (event.getName() == "KbdEsc_Down")
		{
			exit(0);
		}
		else if (event.getName() == "HTC_Controller_Right_Axis1Button_Down" || event.getName() == "HTC_Controller_1_Axis1Button_Down" || event.getName() == "B10_Down")
		{
			m_grab = true;
			//std::cerr << "Grab ON" << std::endl;
		}
		//else if (event.getName() == "HTC_Controller_Right_AButton_Down" || event.getName() == "HTC_Controller_1_AButton_Down")
		else if (event.getName() == "HTC_Controller_Right_Axis0Button_Down" || event.getName() == "HTC_Controller_1_Axis0Button_Down" || event.getName() == "Wand_Right_Btn_Down")
		{
			m_clipping = true;
			//std::cerr << "Clipping ON" << std::endl;
		}
		else if (event.getName() == "HTC_Controller_Right_GripButton_Down" || event.getName() == "HTC_Controller_1_GripButton_Down" || event.getName() == "B08_Down")
		{
			m_show_menu = !m_show_menu;
		}
	}
}


void VolumeVisualizationApp::onButtonUp(const VRButtonEvent &event) {
	if (m_menu_handler != NULL && m_menu_handler->windowIsActive()) {
		if (event.getName() == "MouseBtnMiddle_ScrollUp")
		{
			m_menu_handler->setAnalogValue(10);
		}

		if (event.getName() == "MouseBtnMiddle_ScrollDown")
		{
			m_menu_handler->setAnalogValue(-10);
		}
	}
	else
	{
		if (event.getName() == "MouseBtnMiddle_ScrollUp")
		{
			m_trackball.mouse_pressed(2, false);
		}

		if (event.getName() == "MouseBtnMiddle_ScrollDown")
		{
			m_trackball.mouse_pressed(2, true);
		}
	}
	
	if (event.getName() == "MouseBtnLeft_Up")
	{
		if (m_menu_handler != NULL) m_menu_handler->setButtonClick(0, 0);
		m_trackball.mouse_pressed(0, false);
	}
	else if (event.getName() == "MouseBtnRight_Up")
	{
		m_trackball.mouse_pressed(1, false);
		if (m_menu_handler != NULL) m_menu_handler->setButtonClick(1, 0);
	}
	

	if (m_show_menu && m_menu_handler != NULL) {
		if (event.getName() == "HTC_Controller_Right_Axis1Button_Up" || event.getName() == "HTC_Controller_1_Axis1Button_Up" || event.getName() == "B10_Up")
		{
			//left click
			m_menu_handler->setButtonClick(0, 0);

		}
		else if (event.getName() == "HTC_Controller_Right_GripButton_Up" || event.getName() == "HTC_Controller_1_GripButton_Up" || event.getName() == "Wand_Right_Btn_Up")
		{
			//middle click
			m_menu_handler->setButtonClick(2, 0);
		}
		//else if (event.getName() == "HTC_Controller_Right_AButton_Down" || event.getName() == "HTC_Controller_1_AButton_Down")
		else if (event.getName() == "HTC_Controller_Right_Axis0Button_Up" || event.getName() == "HTC_Controller_1_Axis0Button_Up" || event.getName() == "B08_Up")
		{
			//right click
			m_menu_handler->setButtonClick(1, 0);
		}
	}
	// This routine is called for all Button_Up events.  Check event->getName()
	// to see exactly which button has been released.
	//std::cerr << "onButtonUp " << event.getName() << std::endl;
	if (event.getName() == "HTC_Controller_Right_Axis1Button_Up" || event.getName() == "HTC_Controller_1_Axis1Button_Up" || event.getName() == "B10_Up")
	{
		m_grab = false;
		//std::cerr << "Grab OFF" << std::endl;
	}
	//else if (event.getName() == "HTC_Controller_Right_AButton_Up" || event.getName() == "HTC_Controller_1_AButton_Up")
	else if (event.getName() == "HTC_Controller_Right_Axis0Button_Up" || event.getName() == "HTC_Controller_1_Axis0Button_Up" || event.getName() == "Wand_Right_Btn_Up")
	{
		m_clipping = false;
		//std::cerr << "Clipping OFF" << std::endl;
	}
	
}


void VolumeVisualizationApp::onTrackerMove(const VRTrackerEvent &event) {
	if (m_show_menu && m_menu_handler != NULL) {
		if (event.getName() == "HTC_Controller_Right_Move" || event.getName() == "HTC_Controller_1_Move" || event.getName() == "Wand0_Move") {
			m_menu_handler->setControllerPose(glm::make_mat4(event.getTransform()));
		}
	}


	// This routine is called for all Tracker_Move events.  Check event->getName()
    // to see exactly which tracker has moved, and then access the tracker's new
    // 4x4 transformation matrix with event->getTransform().
	if (event.getName() == "HTC_Controller_Right_Move" || event.getName() == "HTC_Controller_1_Move" || event.getName() == "Wand0_Move") {
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
	
	std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
	std::chrono::duration<double> time_span = std::chrono::duration<double>(nowTime - m_lastTime);
	m_fps = 1000.0f / std::chrono::duration_cast<std::chrono::milliseconds>(time_span).count();
	m_lastTime = nowTime;

	if (m_dynamic_slices) {
		if (m_fps < 25)
			{
				m_slices -= 5;
				m_slices = (m_slices < 10) ? 10 : m_slices;
			}
			else if (m_fps > 60)
			{
				m_slices += 5;
				m_slices = (m_slices > 1024) ? 1024 : m_slices;
			}
	}
	
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
        glEnable(GL_DEPTH_TEST);
        glClearDepth(1.0f);
        glDepthFunc(GL_LEQUAL);
        glClearColor(0.0, 0.0, 0.0, 1);
    }
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, m_light_pos);

	for (std::string filename : m_models_filenames) {
		std::cerr << "Generate DisplayList " << filename << std::endl;
		GLMmodel* pmodel = glmReadOBJ((char*)filename.c_str());
		glmFacetNormals(pmodel);
		glmVertexNormals(pmodel, 90.0);
		glColor4f(1.0, 1.0, 1.0, 1.0);
		m_models_displayLists.push_back(glmList(pmodel, GLM_SMOOTH));
		glmDelete(pmodel);
	}
	m_models_filenames.clear();
	
	initTexture();

	if (m_animated && ! m_stopped)
	{
		m_frame+=m_speed;
		if (m_frame >= m_volumes.size() - 1) m_frame = 0.0 ;
	}
	rendercount = 0;

	if (m_show_menu) m_menu_handler->renderToTexture();
}

void VolumeVisualizationApp::onRenderGraphicsScene(const VRGraphicsState &renderState) {
    // This routine is called once per eye.  This is the place to actually
    // draw the scene...
	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
	if (renderState.isInitialRenderCall())
	{
		m_depthTextures.push_back(new DepthTexture);
	}

	//setup projection
	P = glm::make_mat4(renderState.getProjectionMatrix());
	MV = glm::make_mat4(renderState.getViewMatrix());

	//overwrite MV for 2D viewing
	if(m_is2d)
		MV = MV * m_trackball.getViewmatrix();
	
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
		if (m_volumes.size() > m_models_volumeID[i]) {
			m_models_MV[i] = m_volumes[m_models_volumeID[i]]->get_volume_mv();
			m_models_MV[i] = glm::translate(m_models_MV[i], glm::vec3(-0.5f, -0.5f, -0.5f * m_volumes[m_models_volumeID[i]]->get_volume_scale().x / m_volumes[m_models_volumeID[i]]->get_volume_scale().z));
			m_models_MV[i] = glm::scale(m_models_MV[i], glm::vec3(m_volumes[m_models_volumeID[i]]->get_volume_scale().x, m_volumes[m_models_volumeID[i]]->get_volume_scale().y, m_volumes[m_models_volumeID[i]]->get_volume_scale().x));
		}
	}

	//Set cuttingplane
	if (m_clipping){
		glm::mat4 clipPlane = glm::inverse(m_controller_pose) * glm::inverse(MV);
		for (auto ren : m_renders)
			ren->setClipping(true, &clipPlane);
	} else
	{
		for (auto ren : m_renders)
			ren->setClipping(false, nullptr);
	}
	
	//Render meshes
	for (int i = 0; i < m_models_displayLists.size(); i++){
		if (m_volumes.size() > m_models_volumeID[i]) {
			glMatrixMode(GL_MODELVIEW);
			glLoadMatrixf(glm::value_ptr(m_models_MV[i]));
			glColor3f(1.0f, 1.0f, 1.0f);
			glCallList(m_models_displayLists[i]);
		}
	}
	
	//render menu	
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(glm::value_ptr(glm::make_mat4(renderState.getViewMatrix())));
	if (m_show_menu && !m_is2d) 
		m_menu_handler->drawMenu();
	
	m_depthTextures[rendercount]->copyDepthbuffer();
	(static_cast <VolumeRaycastRenderer*> (m_renders[1]))->setDepthTexture(m_depthTextures[rendercount]);

	//render volumes
	for (auto ren : m_renders){
		ren->set_multiplier(m_multiplier);
		ren->set_threshold(m_threshold);
		ren->set_numSlices(m_slices);
		ren->useMultichannelColormap(m_use_multi_transfer);
	}
	
	if (m_animated)
	{
		unsigned int active_volume = floor(m_frame);
		unsigned int active_volume2 = ceil(m_frame);
		
		if (active_volume < m_volumes.size() && active_volume2 < m_volumes.size() && m_volumes[active_volume]->texture_initialized() && m_volumes[active_volume2]->texture_initialized()) {
			m_renders[m_rendermethod]->set_blending(true, m_frame - active_volume, m_volumes[active_volume2]);
			
			m_renders[m_rendermethod]->render(m_volumes[active_volume], m_volumes[active_volume]->get_volume_mv(), P, m_volumes[active_volume]->get_volume_scale().x / m_volumes[active_volume]->get_volume_scale().z,
				m_use_transferfunction ? (m_use_multi_transfer) ? tfn_widget_multi.get_colormap_gpu() : tfn_widget.get_colormap_gpu() : -1, m_renderchannel);
		}
	}
	else {
		//check order
		std::vector<std::pair< float, int> > order;
		for (int i = 0; i < m_volumes.size(); i++) {
			glm::vec4 center = m_volumes[i]->get_volume_mv() * glm::vec4(0, 0, 0, 1);
			float l = glm::length(center);
			order.push_back(std::make_pair(l, i));
		}
		std::sort(order.begin(), order.end());

		for (int i = order.size() - 1; i >= 0; i--) {
			if (m_volumes[order[i].second]->texture_initialized())
				m_renders[m_rendermethod]->render(m_volumes[order[i].second], m_volumes[order[i].second]->get_volume_mv(), P, m_volumes[order[i].second]->get_volume_scale().x / m_volumes[order[i].second]->get_volume_scale().z,
					m_use_transferfunction ? (m_use_multi_transfer) ? tfn_widget_multi.get_colormap_gpu() : tfn_widget.get_colormap_gpu() : -1, m_renderchannel);
		}
	}

	if (m_is2d)
		m_menu_handler->drawMenu();
	
	glFlush();

	rendercount++;
	
}