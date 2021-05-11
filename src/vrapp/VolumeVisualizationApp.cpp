
#include "../../include/vrapp/VolumeVisualizationApp.h"

#include <cmath>
#include <cctype>
#include "../../include/loader/LoadDataAction.h"
#ifdef WITH_TEEM
	#include "../../include/loader/LoadNrrdAction.h"
#endif
#include "../../include/interaction/HelperFunctions.h"
//#include <glm/gtc/type_ptr.inl>
//#include <glm/gtc/matrix_transform.hpp>
//#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include "../../libs/glm.h"

#include "GLMLoader.h"
#include "Texture.h"
#include "Model.h"

#include "../../include/loader/LoadDescriptionAction.h"
#include "../../include/render/FontHandler.h"
#include <filesystem>



namespace fs = std::filesystem;


float noColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
float ambient[] = { 0.2f, 0.2f, 0.2f, 1.0f };
float diffuse[] = { 0.5f, 0.5f, 0.5f, 1.0f };

VolumeVisualizationApp::VolumeVisualizationApp(int argc, char** argv) : VRApp(argc, argv), m_grab{ false }
, m_scale{ 1.0f }, width{ 10 }, height{ 10 }, m_multiplier{ 1.0f }, m_threshold{ 0.0 }, m_is2d(true), m_menu_handler(NULL), m_lookingGlass{false}
, m_clipping{ false }, m_animated(false), m_speed{ 0.01 }, m_frame{ 0.0 }, m_slices(256), m_rendermethod{ 1 }, m_renderchannel{ 0 }
, m_use_transferfunction{ false }, m_use_multi_transfer{ false }, m_dynamic_slices{ false }, m_show_menu{ true }, convert{ false }
, m_stopped{ false }, m_z_scale{ 0.16 }, m_clip_max{ 1.0 }, m_clip_min{ 0.0 }, m_clip_ypr{ 0.0 }, m_clip_pos{ 0.0 }, m_useCustomClipPlane{false}
, m_wasd_pressed{ 0 }, m_useCameraCenterRotations{ false }, m_movieAction{ nullptr }, m_moviename{"movie.mp4"}
, mesh_model(nullptr), m_renderVolume(true), m_numVolumes(0), m_selectedVolume(0), m_selectedTrnFnc(0)
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
				std::vector<std::promise<Volume*>*> v;
				std::promise<Volume*>* pm = new std::promise<Volume*>();
				
				v.push_back(pm);
				promises.push_back(v);
				
				std::vector<std::future<Volume*>>* fut = new std::vector<std::future<Volume*>>;
				fut->push_back(pm->get_future());
				futures.push_back(fut);
				
				std::vector <std::thread*> ths;
				std::vector<std::promise<Volume*>*> v2= promises.back();
				ths.emplace_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, v2.back()));
				threads.push_back(ths);
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
		std::vector< Volume* > v = m_volumes[i];
		for (int j = 0; j < v.size(); j++)
		{
			delete v[i];
		}
		
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
				std::string tag = vals[0];
				if (tag == "animated")
				{
					m_animated = true;
				}
				if (tag == "threshold")
				{
					m_threshold = stof(vals[1]);
				}
				if (tag == "label")
				{
					std::cerr << "add Label " << vals[1] << std::endl;
					std::cerr << "at position " << vals[2] << " , " << vals[3] << " , " << vals[4] << std::endl;
					std::cerr << "text at position " << vals[2] << " , " << vals[3] << " , " << vals[5] << std::endl;
					std::cerr << "text Size " << vals[6] << std::endl;
					std::cerr << "for Volume " << vals[7] << std::endl;
					m_labels.add(vals[1], stof(vals[2]), stof(vals[3]), stof(vals[4]), stof(vals[5]), stof(vals[6]), stoi(vals[7]) - 1);
				}
				if (tag == "desc")
				{
					std::cerr << "Load Description " << vals[1] << std::endl;
					std::cerr << "with size " << vals[2] << std::endl;
					m_descriptionHeight = stoi(vals[2]);
					m_descriptionFilename = p_filename.parent_path().string() + OS_SLASH + vals[1];
					m_description = LoadDescriptionAction(m_descriptionFilename).run();
					std::cerr << m_description[0] << std::endl;
				}
				if (tag == "mesh")
				{
				//	std::cerr << "Load Mesh " << vals[1] << std::endl;
				//	std::cerr << "for Volume " << vals[2] << std::endl;
					vals[1] = p_filename.parent_path().string() + OS_SLASH + vals[1];
					std::cerr << "Load Mesh " <<  vals[1] << std::endl;
					m_models_volumeID.push_back(stoi(vals[2]) - 1);
					m_models_filenames.push_back(vals[1]);
					m_models_MV.push_back(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
					shaderFilePath = p_filename.parent_path().string() + OS_SLASH + "shaders";

				}
				if (tag == "texture")
				{
          
					//std::cerr << "for Volume " << vals[2] << std::endl;
					textureFilePath = p_filename.parent_path().string() + OS_SLASH + vals[1];
					std::cerr << "Load texture " << textureFilePath << std::endl;
					m_texture = new Texture(GL_TEXTURE_2D, textureFilePath);

				}
				if (tag == "numVolumes")
				{
					m_numVolumes = std::stoi(vals[1]);
					dataLabels.resize(m_numVolumes);
					for (int i = 0; i < m_numVolumes;i++)
					{
						dataLabels[i] = vals[i + 2];
					}
					m_volumes.resize(m_numVolumes);
					promises.resize(m_numVolumes);
					futures.resize(m_numVolumes);
					threads.resize(m_numVolumes);
					tfn_widget_multi.resize(1);
					tfn_widget.resize(1);
					selectedTrFn.resize(1);
					selectedTrFn[0].resize(m_numVolumes);
					for (int i = 0; i < m_numVolumes; i++)
					{
						selectedTrFn[0][i] = false;
					}
				}
				else if (tag.rfind("volume") == 0)
				{
					char str[3];
          int i;

					std::string strVolumeIndex = tag.substr(6);
					size_t volumeIndex = std::stoi(strVolumeIndex);

					
					vals[1] = p_filename.parent_path().string() + OS_SLASH + vals[1];

					
					
          std::vector<std::promise<Volume*>*>& v = promises[volumeIndex - 1];
          std::promise<Volume*>* pm = new std::promise<Volume*>();

          v.push_back(pm);

          //promises[volumeIndex-1]=v;

					std::vector<std::future<Volume*>>* fut;
					if (!futures[volumeIndex-1])
					{
						futures[volumeIndex -1 ] = new std::vector<std::future<Volume*>>;
						
					}
					fut = futures[volumeIndex - 1];
					
          fut->push_back(pm->get_future());
          futures[volumeIndex - 1] = fut;

          std::vector <std::thread*>& ths = threads[volumeIndex - 1];
          std::vector<std::promise<Volume*>*> v2 = promises[volumeIndex - 1];
          ths.emplace_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, v2.back()));

          //threads[volumeIndex - 1] = ths;


					/*promises[volumeIndex -1].push_back(new std::promise<Volume*>);
					futures[volumeIndex - 1]->push_back(promises[i].back()->get_future());
					threads[volumeIndex - 1].push_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, promises[i].back()));*/
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
	for (auto& fut : futures)
	{
    std::vector <std::future<Volume*>>* _ft = fut;
    for (auto& f : *_ft)
    {
#ifdef _MSC_VER
      allready = allready & f._Is_ready();
#else
      allready = allready & (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
#endif
  }

	}

	if(allready)
	{
		for (int i =0; i < futures.size(); i++)
		{
			std::vector <std::future<Volume*>>* _ft = futures[i];
			int counter = 0;
			for (auto& value : *_ft)
			//for (int j = 0; j < _ft->size(); j++)
			{
				//Volume* vlm = *_ft[j].get();
				Volume* vlm = value.get();
				m_volumes[i].push_back(vlm);
				threads[i][counter]->join();
        delete threads[i][counter];
        delete promises[i][counter];
				counter++;
			}
			
			
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
	ImGui::BeginTabBar("##tabs");

	if (ImGui::BeginTabItem("General")) {
		// open file dialog when user clicks this button
		if (ImGui::Button("load file", ImVec2(ImGui::GetWindowSize().x * 0.5f - 1.5* ImGui::GetStyle().ItemSpacing.x, 0.0f)))
			fileDialog.Open();
		ImGui::SameLine();
		if (ImGui::Button("Clear all", ImVec2(ImGui::GetWindowSize().x  * 0.5f - 1.5 * ImGui::GetStyle().ItemSpacing.x, 0.0f)))
		{
      for (int i = 0; i < m_volumes.size(); i++) {
        std::vector< Volume* > v = m_volumes[i];
        for (int j = 0; j < v.size(); j++)
        {
          delete v[i];
        }

      }
      m_volumes.clear();
			m_volumes.clear();
			m_description.clear();
			m_labels.clear();

			m_models_filenames.clear();
			m_models_displayLists.clear();
			m_models_position.clear();
			m_models_volumeID.clear();
			m_models_MV.clear();
		}

		ImGui::SliderFloat("alpha multiplier", &m_multiplier, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("threshold", &m_threshold, 0.0f, 1.0f, "%.3f");
		ImGui::SliderFloat("scale", &m_scale, 0.001f, 5.0f, "%.3f");
		ImGui::SliderFloat("z - scale", &m_z_scale, 0.001f, 5.0f, "%.3f");
		ImGui::SliderInt("Slices", &m_slices, 10, 1024, "%d");
		ImGui::Checkbox("automatic slice adjustment", &m_dynamic_slices);

		ImGui::SameLine(ImGui::GetWindowSize().x * 0.5f, 0);
		ImGui::Text("FPS = %f", m_fps);
		const char* items[] = { "sliced" , "raycast" };
		ImGui::Combo("RenderMethod", &m_rendermethod, items, IM_ARRAYSIZE(items));

		const char* items_channel[] = { "based on data" , "red", "green" , "blue", "alpha", "rgba", "rgba with alpha as max rgb" };
		ImGui::Combo("Render Channel", &m_renderchannel, items_channel, IM_ARRAYSIZE(items_channel));

		ImGui::Checkbox("Render Volume data", &m_renderVolume);

    if (ImGui::SmallButton("New")) {
			tfn_widget.push_back(TransferFunctionWidget());
			tfn_widget_multi.push_back(TransferFunctionMultiChannelWidget());
			int index = selectedTrFn.size();
			selectedTrFn.push_back(std::vector<bool>(m_numVolumes));
			for (int i = 0; i < m_numVolumes;i++)
			{
				selectedTrFn[index][i] = false;
			}
    };
		ImGui::SameLine();
    if (ImGui::SmallButton("Remove")) {

      
    };
		

		if (m_numVolumes > 0)
		{
      ImGui::BeginTable("##Transfer Function Editor", 3);
      ImGui::TableSetupColumn("Name");
      for (int column = 0; column < m_numVolumes; column++)
      {
        ImGui::TableSetupColumn(dataLabels[column].c_str());
      }
      ImGui::TableHeadersRow();
      
			for (int row = 0; row < tfn_widget.size(); row++)
      {
				ImGui::TableNextRow();
        for (int col = 0; col < m_numVolumes+1; col++)
        {
					ImGui::TableSetColumnIndex(col);
					if (col == 0)
					{
            char buf[32];
            sprintf(buf, "TF%d", row);
            if (ImGui::SmallButton(buf)) {
              std::cout << buf << std::endl; 
							m_selectedTrnFnc = row;
            };

					}
					else 
					{
						char buf[32];
						sprintf(buf, "##On%d%d", col, row);
						bool b = selectedTrFn[row][col - 1];
						ImGui::Checkbox(buf, &b);
						selectedTrFn[row][col - 1] = b;
					}

        }
			}
			
			ImGui::EndTable();
		}
		
		
		//const char* v_items[10] ;
		//for (int i = 0; i < dataLabels.size();i++)
		//{
		//	v_items[i] = dataLabels[i].c_str();
		//}

		//static const char* current_item = NULL;
		//ImGui::BeginCombo("##custom combo", current_item);
		//for (int n = 0; n < dataLabels.size(); n++)
		//{
  //    bool is_selected = (current_item == dataLabels[n].c_str());
  //    if (ImGui::Selectable(dataLabels[n].c_str(), is_selected))
		//		m_selectedVolume =n;
  //    if (is_selected)
  //      ImGui::SetItemDefaultFocus();
		//}
		//ImGui::EndCombo();

  /*	ImGui::Combo("data type", &m_selectedVolume,
      [](void* vec, int idx, const char** out_text) {
        std::vector<std::string>* vector = reinterpret_cast<std::vector<std::string>*>(vec);
          if (idx < 0 || idx >= vector->size())return false;
        *out_text = vector->at(idx).c_str();
        return true;
      }, reinterpret_cast<void*>(&dataLabels), dataLabels.size());*/
		
		
		//ImGui::Combo("Volume data", &m_selectedVolume, v_items, IM_ARRAYSIZE(v_items));

		ImGui::Checkbox("use transferfunction", &m_use_transferfunction);
		if (m_use_transferfunction) {

			bool is_multi_channel = false;
			for (int i = 0; i < m_volumes.size(); i++)
			{
				if (m_volumes.size() > 0 && m_volumes[i][0]->get_channels() > 1 &&
					(m_renderchannel == 0 || m_renderchannel == 5 || m_renderchannel == 6))
				{
					is_multi_channel |= true;
				}
			}
						 if (is_multi_channel)
						 {
							 for (int i = 0; i < 3; i++) {
								 if (m_animated)
								 {
									 /*	unsigned int active_volume = floor(m_frame);
										 unsigned int active_volume2 = ceil(m_frame);
										 double alpha = m_frame - active_volume;
										 if (active_volume < m_volumes[m_selectedVolume].size() && active_volume2 < m_volumes[m_selectedVolume].size())
										 {
											 tfn_widget_multi[m_selectedVolume].setBlendedHistogram(
												 m_volumes[m_selectedVolume][active_volume]->getTransferfunction(i),
												 m_volumes[m_selectedVolume][active_volume2]->getTransferfunction(i), alpha, i);
										 }*/

								 }
								 else {
									 /*		tfn_widget_multi[m_selectedVolume].setHistogram(m_volumes[m_selectedVolume][0]->getTransferfunction(i), i);*/
								 }
							 }
							 m_use_multi_transfer = true;
							 tfn_widget_multi[m_selectedTrnFnc].draw_ui();
						 }
						 else
						 {
							 if (m_animated)
							 {
								 /*	unsigned int active_volume = floor(m_frame);
									 unsigned int active_volume2 = ceil(m_frame);
									 double alpha = m_frame - active_volume;
									 tfn_widget[m_selectedVolume].setMinMax(m_volumes[m_selectedVolume][active_volume]->getMin() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMin() * (1.0 - alpha),
										 m_volumes[m_selectedVolume][active_volume]->getMax() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMax() * (1.0 - alpha));
									 if (active_volume < m_volumes[m_selectedVolume].size() && active_volume2 < m_volumes[m_selectedVolume].size())
										 tfn_widget[m_selectedVolume].setBlendedHistogram(m_volumes[m_selectedVolume][active_volume]->getTransferfunction(0), m_volumes[m_selectedVolume][active_volume2]->getTransferfunction(0), alpha);*/
							 }
							 else if (m_volumes.size() > 0) {
								 /*tfn_widget[m_selectedVolume].setHistogram(m_volumes[m_selectedVolume][0]->getTransferfunction(0));
								 tfn_widget[m_selectedVolume].setMinMax(m_volumes[m_selectedVolume][0]->getMin(), m_volumes[m_selectedVolume][0]->getMax());*/
							 }
							 m_use_multi_transfer = false;
							 tfn_widget[m_selectedTrnFnc].draw_ui();
						 }
			
       }

		

		if (m_animated) {
			ImGui::Text("Timestep");
			ImGui::SameLine();
			ImGui::SetNextItemWidth(-100 - ImGui::GetStyle().ItemSpacing.x);
			float frame_tmp = m_frame + 1;
			ImGui::SliderFloat("##Timestep", &frame_tmp, 1, m_volumes[m_selectedVolume].size());
			m_frame = frame_tmp - 1;
			ImGui::SameLine();

			std::string text = m_stopped ? "Play" : "Stop";
			if (ImGui::Button(text.c_str(), ImVec2(100, 0))) {
				m_stopped = !m_stopped;
			}


			if (ImGui::Button("Write Movie"))
			{
#ifndef _MSC_VER
				fs::create_directory("movie");
#endif
				if (m_movieAction)
					delete m_movieAction;

				m_movieAction = new CreateMovieAction();
				m_frame = 0;
				m_show_menu = false;
			}
		}

		ImGui::Checkbox("Camera centric rotations", &m_useCameraCenterRotations);
		m_trackball.setCameraCenterRotation(m_useCameraCenterRotations);
		ImGui::EndTabItem();	
	}
	
	if (ImGui::BeginTabItem("Clipping")) {

		ImGui::Text("Axis aligned clip");
		glm::vec2 bound = { m_clip_min.x * 100 ,m_clip_max.x * 100 };
		ImGui::DragFloatRange2("X", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
		m_clip_min.x = bound.x / 100; 
		m_clip_max.x = bound.y / 100;

		bound = { m_clip_min.y * 100 ,m_clip_max.y * 100 };
		ImGui::DragFloatRange2("Y", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
		m_clip_min.y = bound.x / 100;
		m_clip_max.y = bound.y / 100;

		bound = { m_clip_min.z * 100 ,m_clip_max.z * 100 };
		ImGui::DragFloatRange2("Z", &bound.x, &bound.y, 0.1f, 0.0f, 100.0f, "Min: %.1f %%", "Max: %.1f %%");
		m_clip_min.z = bound.x / 100;
		m_clip_max.z = bound.y / 100;

		if (ImGui::Button("Reset")) {
			m_clip_min = glm::vec3(0.0f);
			m_clip_max = glm::vec3(1.0f);
		}

		ImGui::Checkbox("Custom Clipping plane", &m_useCustomClipPlane);
		if (m_useCustomClipPlane) {
			ImGui::SliderAngle("Pitch", &m_clip_ypr.y, -90, 90);
			ImGui::SliderAngle("Roll", &m_clip_ypr.z, -180, 180);

			ImGui::SliderFloat("Position X", &m_clip_pos.x, -0.5, 0.5);
			ImGui::SliderFloat("Position y", &m_clip_pos.y, -0.5, 0.5);
			ImGui::SliderFloat("Position z", &m_clip_pos.z, -0.5, 0.5);
			if (ImGui::Button("Reset##Reset2")) {
				m_clip_ypr = glm::vec3(0.0f);
				m_clip_pos = glm::vec3(0.0f);
			}
		}

		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();
	//file loading
	fileDialog.Display();

	if (fileDialog.HasSelected())
	{
		if (helper::ends_with_string(fileDialog.GetSelected().string(), ".txt"))
		{
			loadTxtFile(fileDialog.GetSelected().string());
		}
#ifdef WITH_TEEM
		else if (helper::ends_with_string(fileDialog.GetSelected().string(), ".nrrd")) {
			std::vector<std::string> vals;
		/*	vals.push_back(fileDialog.GetSelected().string());	
			promises.push_back(new std::promise<Volume*>);
			futures.push_back(promises.back()->get_future());
			threads.push_back(new std::thread(&VolumeVisualizationApp::loadVolume, this, vals, promises.back()));*/
		}
#endif
		fileDialog.ClearSelected();
	}
	
	ImGui::End();
}

void VolumeVisualizationApp::initTexture()
{
	addLodadedTextures();
	for (int i = 0; i < m_volumes.size(); i++){

		std::vector< Volume* > vlm = m_volumes[i];
		for (int j = 0; j < vlm.size(); j++)
		{
			vlm[j]->initGL();
		}
		
	}

}

void VolumeVisualizationApp::onCursorMove(const VRCursorEvent& event)
{
	if (event.getName() == "Mouse_Move" && m_menu_handler != NULL)
	{
		m_trackball.mouse_move(event.getPos()[0], event.getPos()[1]);
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
			|| (event.getName() == "Wand_Joystick_Y_Update" && !(event.getValue() > -0.1 && event.getValue() < 0.1)))
			m_menu_handler->setAnalogValue(event.getValue());

		if (event.getName() == "MouseWheel_Spin") {
			std::cerr << event.getValue() << std::endl;
			m_menu_handler->setAnalogValue(event.getValue() * 10);
		}
	}
	else {
		if (event.getName() == "MouseWheel_Spin") {
			m_trackball.mouse_scroll(event.getValue() * 0.01);
		}
		
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
		else if (event.getName() == "MouseBtnMiddle_Down") {
			m_menu_handler->setButtonClick(2, 0);
		}
	}
	else
	{
		if (event.getName() == "MouseBtnLeft_Down")
		{
			m_trackball.mouse_pressed(0, true);
		}
		else if (event.getName() == "MouseBtnRight_Down")
		{
			m_trackball.mouse_pressed(1, true);
		}
		else if (event.getName() == "MouseBtnMiddle_Down") {
			m_trackball.mouse_pressed(2, true);
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
	if (!(m_show_menu && m_menu_handler != NULL && m_menu_handler->windowIsActive())) {
		if (event.getName() == "KbdW_Down") {
			m_wasd_pressed = m_wasd_pressed | W;
		}
		if (event.getName() == "KbdA_Down") {
			m_wasd_pressed = m_wasd_pressed | A;
		}
		if (event.getName() == "KbdS_Down") {
			m_wasd_pressed = m_wasd_pressed | S;
		}
		if (event.getName() == "KbdD_Down") {
			m_wasd_pressed = m_wasd_pressed | D;
		}
		if (event.getName() == "KbdQ_Down") {
			m_wasd_pressed = m_wasd_pressed | Q;
		}
		if (event.getName() == "KbdE_Down") {
			m_wasd_pressed = m_wasd_pressed | E;
		}

    if (event.getName() == "KbdE_Down") {
      m_wasd_pressed = m_wasd_pressed | E;
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
	else if (event.getName() == "MouseBtnMiddle_Up") {
		m_trackball.mouse_pressed(2, false);
		if (m_menu_handler != NULL) m_menu_handler->setButtonClick(2, 0);
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

	if (event.getName() == "KbdW_Up") {
		m_wasd_pressed = m_wasd_pressed & ~W;
	}
	if (event.getName() == "KbdA_Up") {
		m_wasd_pressed = m_wasd_pressed & ~A;
	}
	if (event.getName() == "KbdS_Up") {
		m_wasd_pressed = m_wasd_pressed & ~S;
	}
	if (event.getName() == "KbdD_Up") {
		m_wasd_pressed = m_wasd_pressed & ~D;
	}
	if (event.getName() == "KbdQ_Up") {
		m_wasd_pressed = m_wasd_pressed & ~Q;
	}
	if (event.getName() == "KbdE_Up") {
		m_wasd_pressed = m_wasd_pressed & ~E;
	}

  if (event.getName() == "KbdSpace_Up") {
    
			m_renderVolume = !m_renderVolume;
    
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
	

	
		mesh_model = GLMLoader::loadObjModel(filename);
		
	
		if (m_texture)
		{
			mesh_model->addTexture(m_texture);
		}
		std::string vertexShaderFolderPath = shaderFilePath + OS_SLASH+std::string("shader.vert");
		std::string fragmentShaderFolderPath = shaderFilePath + OS_SLASH +std::string("shader.frag");
		simpleTextureShader.LoadShaders(vertexShaderFolderPath.c_str(), fragmentShaderFolderPath.c_str());
   // simpleTextureShader.addUniform("m");
    //simpleTextureShader.addUniform("v");
    simpleTextureShader.addUniform("p");
		simpleTextureShader.addUniform("mv");
  /*  glmFacetNormals(pmodel);
    glmVertexNormals(pmodel, 90.0);
   glColor4f(1.0, 1.0, 1.0, 1.0);
    m_models_displayLists.push_back(glmList(pmodel, GLM_SMOOTH));
    glmDelete(pmodel);*/
	}
	m_models_filenames.clear();
	
	initTexture();

	if (m_animated && ! m_stopped)
	{
		m_frame+=m_speed;
		if (m_frame > m_volumes[m_selectedVolume].size() - 1) m_frame = 0.0 ;
	}
	rendercount = 0;

	if (m_show_menu) m_menu_handler->renderToTexture();
	if(m_wasd_pressed)
		m_trackball.wasd_pressed(m_wasd_pressed);
}





void VolumeVisualizationApp::clearData()
{
  for (int i = 0; i < m_volumes.size(); i++) {
    std::vector< Volume* > v = m_volumes[i];
    for (int j = 0; j < v.size(); j++)
    {
      delete v[i];
    }

  }
  m_volumes.clear();
  m_volumes.clear();
  m_description.clear();
  m_labels.clear();

  m_models_filenames.clear();
  m_models_displayLists.clear();
  m_models_position.clear();
  m_models_volumeID.clear();
  m_models_MV.clear();
}

int VolumeVisualizationApp::numVolumes()
{
	return m_numVolumes;
}

bool VolumeVisualizationApp::dataIsMultiChannel()
{
  bool is_multi_channel = false;
  for (int i = 0; i < m_volumes.size(); i++)
  {
    if (m_volumes.size() > 0 && m_volumes[i][0]->get_channels() > 1 &&
      (m_renderchannel == 0 || m_renderchannel == 5 || m_renderchannel == 6))
    {
      is_multi_channel |= true;
    }
  }

	return is_multi_channel;
}

void VolumeVisualizationApp::getMinMax(const float frame, float& min, float& max)
{
  unsigned int active_volume = floor(frame);
  unsigned int active_volume2 = ceil(frame);
	double alpha = frame - active_volume;
	min = m_volumes[m_selectedVolume][active_volume]->getMin() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMin() * (1.0 - alpha);
	max = m_volumes[m_selectedVolume][active_volume]->getMax() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMax() * (1.0 - alpha);
}

void VolumeVisualizationApp::onRenderGraphicsScene(const VRGraphicsState& renderState) {
	// This routine is called once per eye.  This is the place to actually
	// draw the scene...
	if (m_is2d) {
		m_headpose = glm::make_mat4(renderState.getViewMatrix());
		m_headpose = glm::inverse(m_headpose);
	}


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
	if (m_is2d)
		MV = m_trackball.getViewmatrix();

	glMatrixMode(GL_PROJECTION);
	glLoadMatrixf(glm::value_ptr(P));

	//setup Modelview for volumes
	for (int i = 0; i < m_volumes.size(); i++) {
		for (int j = 0; j < m_volumes[i].size(); j++) {
      glm::mat4 tmp = MV;
      tmp = tmp * m_object_pose;
      tmp = glm::scale(tmp, glm::vec3(m_scale, m_scale, m_scale * m_z_scale));
      if (!m_animated)
        tmp = glm::translate(tmp, glm::vec3(m_volumes[i][j]->get_volume_position().x, m_volumes[i][j]->get_volume_position().y, m_volumes[i][j]->get_volume_position().z));
      m_volumes[i][j]->set_volume_mv(tmp);
		}
	
	}

	//setup Modelview for meshes
	if (mesh_model)
	{
		int i = 0;
		if (m_volumes.size() > m_models_volumeID[i]) {
			glm::mat4 volume_mv = m_volumes[0][m_models_volumeID[i]]->get_volume_mv();
			volume_mv = glm::translate(volume_mv, glm::vec3(-0.5f, -0.5f, -0.5f * m_volumes[0][m_models_volumeID[i]]->get_volume_scale().x / (m_volumes[0][m_models_volumeID[i]]->get_volume_scale().z)));
			volume_mv = glm::scale(volume_mv, glm::vec3(m_volumes[0][m_models_volumeID[i]]->get_volume_scale().x, m_volumes[0][m_models_volumeID[i]]->get_volume_scale().y, m_volumes[0][m_models_volumeID[i]]->get_volume_scale().x));
			//volume_mv = glm::scale(m_models_MV[i], glm::vec3(5,5,5));
			mesh_model->setMVMatrix(volume_mv);
			m_models_MV[i] = volume_mv;
			//mesh_model->setPosition(glm::vec3(-0.5f, -0.5f, 
			//	-0.5f * m_volumes[m_models_volumeID[i]]->get_volume_scale().x / (m_volumes[m_models_volumeID[i]]->get_volume_scale().z)));
			//mesh_model->setScale(glm::vec3(m_volumes[m_models_volumeID[i]]->get_volume_scale().x, m_volumes[m_models_volumeID[i]]->get_volume_scale().y, m_volumes[m_models_volumeID[i]]->get_volume_scale().x));
		}

	}

	//for (int i = 0; i < m_models_displayLists.size(); i++) {
	//	if (m_volumes.size() > m_models_volumeID[i]) {
	//		m_models_MV[i] = m_volumes[m_models_volumeID[i]]->get_volume_mv();
	//		m_models_MV[i] = glm::translate(m_models_MV[i], glm::vec3(-0.5f, -0.5f, -0.5f * m_volumes[m_models_volumeID[i]]->get_volume_scale().x / (m_volumes[m_models_volumeID[i]]->get_volume_scale().z)));
	//		m_models_MV[i] = glm::scale(m_models_MV[i], glm::vec3(m_volumes[m_models_volumeID[i]]->get_volume_scale().x, m_volumes[m_models_volumeID[i]]->get_volume_scale().y, m_volumes[m_models_volumeID[i]]->get_volume_scale().x));
	//		//m_models_MV[i] = glm::scale(m_models_MV[i], glm::vec3(10,10,10));
	//	}
	//}

	//Set cuttingplane
	if (m_clipping || m_useCustomClipPlane) {
		glm::mat4 clipPlane = glm::inverse(m_controller_pose) * glm::inverse(MV);

		if (m_useCustomClipPlane) {
			clipPlane = glm::eulerAngleYXZ(m_clip_ypr.x, m_clip_ypr.y, m_clip_ypr.z);
			clipPlane = glm::translate(clipPlane, m_clip_pos);
			clipPlane = clipPlane * glm::inverse(MV);
		}

		for (auto ren : m_renders)
			ren->setClipping(true, &clipPlane);
	}
	else
	{
		for (auto ren : m_renders)
			ren->setClipping(false, nullptr);
	}

	for (auto ren : m_renders)
		ren->setClipMinMax(m_clip_min, m_clip_max);

	//Render meshes
	/*for (int i = 0; i < m_models_displayLists.size(); i++) {
		if (m_volumes.size() > m_models_volumeID[i]) {
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadMatrixf(glm::value_ptr(m_models_MV[i]));
			glColor3f(1.0f, 1.0f, 1.0f);
			glCallList(m_models_displayLists[i]);
			glPopMatrix();
		}
	}*/

  /*unsigned int errorCode = 0;
  while ((errorCode = glGetError()) != GL_NO_ERROR) {
    std::cout << errorCode;
  }*/

	if (mesh_model)
	{

		/* unsigned int errorCode = 0;
			 while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::cout << errorCode;
      }*/
   
    simpleTextureShader.start();
		simpleTextureShader.setUniform("p", P);
    mesh_model->render(simpleTextureShader);
    simpleTextureShader.stop();
    /* while ((errorCode = glGetError()) != GL_NO_ERROR) {
       std::cout << errorCode;
     }*/
    
	}
	
	//render labels
	m_labels.draw(m_models_MV,m_headpose, m_z_scale);

	if(m_is2d && !m_description.empty())
		FontHandler::getInstance()->renderMultiLineTextBox2D(m_description, 50, 950, 200, m_descriptionHeight);

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

			for (int tfn = 0; tfn < tfn_widget.size(); tfn++)
			{

				for (int vol = 0; vol < m_numVolumes; vol++)
				{
					if (selectedTrFn[tfn][vol])
					{
            if (active_volume < m_volumes[vol].size() && active_volume2 < m_volumes[vol].size() && m_volumes[vol][active_volume]->texture_initialized() && m_volumes[vol][active_volume2]->texture_initialized()) {
              m_renders[m_rendermethod]->set_blending(true, m_frame - active_volume, m_volumes[vol][active_volume2]);

              if (m_renderVolume)
              {
                m_renders[m_rendermethod]->render(m_volumes[vol][active_volume], m_volumes[vol][active_volume]->get_volume_mv(), P, m_volumes[vol][active_volume]->get_volume_scale().x / m_volumes[vol][active_volume]->get_volume_scale().z,
                  m_use_transferfunction ? (m_use_multi_transfer) ? tfn_widget_multi[tfn].get_colormap_gpu() : tfn_widget[tfn].get_colormap_gpu() : -1, m_renderchannel);
              }
            }
					}
				}
			}

     
		
	}
	else {
		//check order
		for (int tfn = 0; tfn < tfn_widget.size(); tfn++)
		{

			for (int vol = 0; vol < m_numVolumes; vol++)
			{
				if (selectedTrFn[tfn][vol])
				{
          std::vector<std::pair< float, int> > order;
          for (int i = 0; i < m_volumes.size(); i++) {
            glm::vec4 center = m_volumes[vol][i]->get_volume_mv() * glm::vec4(0, 0, 0, 1);
            float l = glm::length(center);
            order.push_back(std::make_pair(l, i));
          }
          std::sort(order.begin(), order.end());

          for (int i = order.size() - 1; i >= 0; i--) {
            if (m_volumes[vol][order[i].second]->texture_initialized())
            {
              if (m_renderVolume)
              {
                m_renders[m_rendermethod]->render(m_volumes[vol][order[i].second], m_volumes[vol][order[i].second]->get_volume_mv(), P, m_volumes[vol][order[i].second]->get_volume_scale().x / m_volumes[vol][order[i].second]->get_volume_scale().z,
                  m_use_transferfunction ? (m_use_multi_transfer) ? tfn_widget_multi[tfn].get_colormap_gpu() : tfn_widget[tfn].get_colormap_gpu() : -1, m_renderchannel);
              }

            }
				}
			}
		}
	
				
		}
	}

	//draw UI
	if (m_show_menu && m_is2d)
		m_menu_handler->drawMenu();

	//draw Transferfunction
	if (m_is2d && m_use_transferfunction) {
		tfn_widget[m_selectedTrnFnc].drawLegend();
	}

	//drawTime
	if (m_is2d && m_animated) {
		unsigned int active_volume = floor(m_frame);
		unsigned int active_volume2 = ceil(m_frame);
		if (active_volume < m_volumes[0].size() && active_volume2 < m_volumes[0].size() && m_volumes[0][active_volume]->texture_initialized() && m_volumes[0][active_volume2]->texture_initialized()){
			float alpha = m_frame - active_volume;
			time_t time = m_volumes[0][active_volume]->getTime() * (1 - alpha) + m_volumes[0][active_volume2]->getTime() * alpha;
			FontHandler::getInstance()->drawClock(time);
		}
	}

	glFlush();

	rendercount++;
	
	if (m_movieAction) {
#ifndef _MSC_VER
		glFinish();
#endif
		std::cerr << "Add Frame" << std::endl;
		m_movieAction->addFrame();
		if (m_frame >  m_volumes[m_selectedVolume].size() - 1 - m_speed) {
			std::cerr << "Save Movie" << std::endl;
#ifdef _MSC_VER
			m_movieAction->save(m_moviename);
#endif
			delete m_movieAction;
			m_movieAction = nullptr;
			m_show_menu = true;
		}
	}
}
