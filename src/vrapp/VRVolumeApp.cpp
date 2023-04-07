#include "vrapp/VRVolumeApp.h"
#include "render/VolumeSliceRenderer.h"
#include "render/VolumeRaycastRenderer.h"
#include "render/Volume.h"
#include "render/VolumeRenderer.h"
#include "loader/LoadDescriptionAction.h"
#include "loader/LoadNrrdAction.h"
#include "loader/LoadDataAction.h"
#include "UI/UIView.h"
#include "interaction/HelperFunctions.h"

#include "interaction/CreateMovieAction.h"
#include "interaction/Simulation.h"
#include "interaction/LabelsManager.h"
#include "render/Mesh.h"

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



#include <fstream>
#include <sstream>
#include "GLMLoader.h"
#include <locale>
#include <algorithm>

#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include "Model.h"
#include "Texture.h"

VRVolumeApp::VRVolumeApp() : m_clip_max{ 1.0f }, m_clip_min{ 0.0f }, m_clip_ypr{ 0.0f }, m_clip_pos{ 0.0 }, m_wasd_pressed(0),
m_lookingGlass(false), m_isInitailized(false), m_speed(0.01f), m_movieAction(nullptr), m_moviename("movie.mp4"), m_noColor(0.0f),
m_ambient(0.2f, 0.2f, 0.2f, 1.0f), m_diffuse(0.5f, 0.5f, 0.5f, 1.0f), m_ui_view(nullptr), m_animated(false), m_numVolumes(0), m_selectedVolume(0),
m_multiplier(1.0f), m_threshold(0.0f), m_frame_step(0.0f), m_use_multi_transfer(false), m_clipping(false), m_show_menu(true),
m_window_properties(nullptr), m_volume_animation_scale_factor(1.0f), m_current_movie_state(MOVIE_STOP), m_app_mode(MANUAL), m_end_load(false),
m_volumes_global_min_value(std::numeric_limits<float>::max()), m_volumes_global_max_value(std::numeric_limits<float>::min()), m_is2d(false), m_grab(false)
{
	m_renders.push_back(new VolumeSliceRenderer());
	m_renders.push_back(new VolumeRaycastRenderer(*this));
}

VRVolumeApp::~VRVolumeApp()
{
	for (int i = 0; i < m_volumes.size(); i++)
	{
		std::vector<Volume*> v = m_volumes[i];
		for (int j = 0; j < v.size(); j++)
		{
			delete v[j];
		}
	}
	m_volumes.clear();

	for (Mesh* mesh : m_mesh_models)
	{
		delete mesh;
	}

	delete m_window_properties;

	delete m_simulation;

	delete m_label_manager;
}

void VRVolumeApp::initialize()
{

	if (!m_isInitailized)
	{
		m_object_pose = glm::mat4(1.0f);
		initialize_GL();

		if (!m_ui_view)
		{
			std::cout << "initialize UI " << std::endl;
			m_ui_view = std::make_unique<UIView>(*this);
			m_ui_view->init_ui(m_is2d, m_lookingGlass);
		}
		m_window_properties = new Window_Properties();
		m_simulation = new Simulation(*this);
		m_label_manager = new LabelsManager(*this, m_line_shader, m_simple_texture_shader);
		m_label_manager->set_parent_directory(get_directory_path());

		m_isInitailized = true;
		m_rendercount = 0;
		std::cout << "initialize end" << std::endl;
	}
}

void VRVolumeApp::initialize_GL()
{
	for (auto ren : m_renders)
	{
		ren->initGL();
	}

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glLightfv(GL_LIGHT0, GL_AMBIENT, glm::value_ptr(m_ambient));
	glLightfv(GL_LIGHT0, GL_DIFFUSE, glm::value_ptr(m_diffuse));
	glLightfv(GL_LIGHT0, GL_SPECULAR, glm::value_ptr(m_noColor));
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, true);
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_NORMALIZE);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0f);
	glDepthFunc(GL_LEQUAL);
	glClearColor(0.0, 0.0, 0.0, 1);
}

void VRVolumeApp::load_mesh_models()
{
	for (MeshData mesh_data : m_mesh_models_data)
	{

		Model* mesh_model = GLMLoader::loadObjModel(mesh_data.mesh_file_path);
		Texture* texture = new Texture(GL_TEXTURE_2D, mesh_data.texture_file_path);
		mesh_model->setTexture(texture);
		Mesh* mesh = new Mesh(mesh_model, mesh_data.volume_id);
		get_mesh_models().push_back(mesh);
	}
	m_mesh_models_data.clear();
}

void VRVolumeApp::load_shaders()
{

	m_shader_file_path = get_directory_path() + OS_SLASH + "shaders";
	std::string vertexShaderFolderPath = m_shader_file_path + OS_SLASH + std::string("shader.vert");
	std::string fragmentShaderFolderPath = m_shader_file_path + OS_SLASH + std::string("shader.frag");
	m_simple_texture_shader.LoadShaders(vertexShaderFolderPath.c_str(), fragmentShaderFolderPath.c_str());
	m_simple_texture_shader.addUniform("projection_matrix");
	m_simple_texture_shader.addUniform("model_view_matrix");

	std::string linesVertexShaderFolderPath = m_shader_file_path + OS_SLASH + std::string("lines_shader.vert");
	std::string linesFragmentShaderFolderPath = m_shader_file_path + OS_SLASH + std::string("lines_shader.frag");
	m_line_shader.LoadShaders(linesVertexShaderFolderPath.c_str(), linesFragmentShaderFolderPath.c_str());
	m_line_shader.addUniform("projection_matrix");
	m_line_shader.addUniform("model_view_matrix");
}

void VRVolumeApp::initialize_textures()
{
	add_lodaded_textures();

	for (int i = 0; i < m_volumes.size(); i++)
	{
		std::vector<Volume*> vlm = m_volumes[i];
		for (int j = 0; j < vlm.size(); j++)
		{
			vlm[j]->initGL();
		}
	}
}

bool VRVolumeApp::pending_models_to_load()
{
	return m_mesh_models_data.size() > 0;
}

void VRVolumeApp::update_trackBall_state()
{
	if (m_wasd_pressed)
	{
		get_trackball_camera().wasd_pressed(m_wasd_pressed);
	}
}

void VRVolumeApp::update_animation(float fps)
{
	if (m_ui_view)
	{
		if (m_volumes.size())
		{
			m_ui_view->update_animation(m_speed, m_volumes[m_selectedVolume].size() - 1);
		}
	}
}

#if (!defined(__APPLE__))
void VRVolumeApp::run_movie(bool is_animation)
{

	if (m_movieAction)
		delete m_movieAction;

	m_movieAction = new CreateMovieAction();
	m_frame_step = 0;
	if (is_animation)
	{
		m_show_menu = false;
	}
	m_current_movie_state = MOVIE_RECORD;
}

void VRVolumeApp::stop_movie()
{
	m_current_movie_state = MOVIE_STOP;
#ifdef _MSC_VER
	m_movieAction->save(m_moviename);
	m_ui_view->set_show_movie_saved_pop_up(true);
#endif
	delete m_movieAction;
	m_movieAction = nullptr;
	m_show_menu = true;
}

#endif

void VRVolumeApp::set_render_count(unsigned int rendercount)
{
	m_rendercount = rendercount;
}

float VRVolumeApp::get_current_frame()
{
	return m_frame_step;
}

void VRVolumeApp::set_frame(float frame)
{
	m_frame_step = frame;
}

glm::vec4& VRVolumeApp::get_no_color()
{
	return m_noColor;
}

glm::vec4& VRVolumeApp::get_ambient()
{
	return m_ambient;
}

glm::vec4& VRVolumeApp::get_diffuse()
{
	return m_diffuse;
}

void VRVolumeApp::set_multi_transfer(bool multiTransfer)
{
	m_use_multi_transfer = multiTransfer;
}

bool VRVolumeApp::is_multi_transfer()
{
	return m_use_multi_transfer;
}

bool VRVolumeApp::is_ui_event()
{
	if (m_ui_view && m_ui_view->is_ui_window_active())
	{
		return true;
	}
	return false;
}

bool VRVolumeApp::is_show_menu()
{
	return m_show_menu;
}

void VRVolumeApp::set_is_animated(bool animated)
{
	m_animated = animated;
	m_ui_view->set_is_animated(m_animated);
}

void VRVolumeApp::set_threshold(float threshold)
{
	m_threshold = threshold;
}

void VRVolumeApp::add_label(std::string& text, float x, float y, float z, float textPosZ, float size, float offset, int volume)
{
	if (m_label_manager)
	{
		m_label_manager->add(text, x, y, z, textPosZ, size, offset, volume);
	}
}

void VRVolumeApp::set_description(int descriptionHeight, std::string& descriptionFilename)
{
	m_descriptionHeight = descriptionHeight;
	m_descriptionFilename = descriptionFilename;
	m_description = LoadDescriptionAction(m_descriptionFilename).run();
}

void VRVolumeApp::set_mesh(int volume_id, std::string& mesh_file_path, std::string& texture_file_path)
{
	MeshData mesh_data = { (unsigned int)volume_id - 1, mesh_file_path, texture_file_path };
	m_mesh_models_data.push_back(mesh_data);
	m_models_volumeID.push_back(volume_id - 1);
	m_models_MV.push_back(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, -2, 1));
}

void VRVolumeApp::init_num_volumes(int nVolumes)
{

	m_numVolumes = nVolumes;
	m_volumes.resize(m_numVolumes);
	m_promises.resize(m_numVolumes);
	m_futures.resize(m_numVolumes);
	m_threads.resize(m_numVolumes);

	if (m_ui_view)
	{
		m_ui_view->init_ui(m_is2d, m_lookingGlass);
		m_ui_view->update_ui(m_numVolumes);
	}
}

void VRVolumeApp::add_data_label(std::string& label)
{
	m_ui_view->add_data_label(label);
}

std::vector<std::promise<Volume*>*>& VRVolumeApp::get_promise(int index)
{
	return m_promises[index - 1];
}

std::vector<std::future<Volume*>>* VRVolumeApp::get_future(int index)
{
	return m_futures[index - 1];
}

void VRVolumeApp::set_future(int index, std::vector<std::future<Volume*>>* futures)
{
	m_futures[index - 1] = futures;
}

std::vector<std::thread*>& VRVolumeApp::get_thread(int index)
{
	return m_threads[index - 1];
}

void VRVolumeApp::init_volume_loading(int index, std::vector<std::string> vals)
{

	std::vector<std::thread*>& ths = m_threads[index - 1];
	std::vector<std::promise<Volume*>*> v2 = m_promises[index - 1];
	ths.emplace_back(new std::thread(&VRVolumeApp::load_volume, this, vals, v2.back()));
}

void VRVolumeApp::set_character_state(std::string& eventName, int state)
{
	size_t pos = eventName.find("Kbd");
	size_t pos_ = eventName.find("_");

	if (pos != std::string::npos)
	{
		std::locale loc;
		std::string keyStr = eventName.substr(3, pos_ - 3);
		if (keyStr.size() == 1)
		{
			int keyCode = keyStr[0];
			ImGuiIO& io = ImGui::GetIO();
			if (state == 1)
			{
				io.KeysDown[keyCode] = true;
			}
			if (state == 0)
			{
				io.KeysDown[keyCode] = false;
			}

			if (state == 1)
			{
				if (m_ui_view)
				{
					m_ui_view->add_character(keyStr[0]);
				}
			}
		}
		else
		{
			if (state == 1)
			{
				if (keyStr == "Backspace" || keyStr == "Del")
				{
					m_ui_view->remove_character();
				}
				else if (keyStr == "Space")
				{
					m_ui_view->add_character(32);
				}
			}
		}
	}
}

void VRVolumeApp::set_directory_path(std::string& dir_path)
{
	m_directiort_path = dir_path;
}

std::string& VRVolumeApp::get_directory_path()
{
	return m_directiort_path;
}

void VRVolumeApp::set_loaded_file(std::string& dir_path)
{
	m_current_file_loaded = dir_path;
}

std::string& VRVolumeApp::get_loaded_file()
{
	return m_current_file_loaded;
}

std::vector<Volume*>& VRVolumeApp::get_volume(int volume)
{
	return m_volumes[volume];
}

void VRVolumeApp::intialize_ui()
{
	if (m_ui_view)
	{
		m_ui_view->init_ui(m_is2d, m_lookingGlass);
	}
}

void VRVolumeApp::load_volume(std::vector<std::string> vals, std::promise<Volume*>* promise)
{
	if (helper::ends_with_string(vals[1], ".nrrd"))
	{

		Volume* volume = LoadNrrdAction(vals[1]).run();
		volume->set_volume_position({ 0.0, 0.0, 0.0 });
		volume->set_volume_scale({ 1.0, 1.0,1.0 });
		volume->set_volume_mv(glm::mat4());
		volume->set_render_channel(1);
		promise->set_value(volume);
		

	}
	else
	{
		std::cerr << "Load volume " << vals[1] << std::endl;
		std::cerr << "Position " << vals[5] << " , " << vals[6] << " , " << vals[7] << std::endl;
		std::cerr << "Spacing " << vals[2] << " , " << vals[3] << " , " << vals[4] << std::endl;

		float t_res[3];
		t_res[0] = stof(vals[2]);
		t_res[1] = stof(vals[3]);
		t_res[2] = stof(vals[4]);

		Volume* volume = LoadDataAction(vals[1], &t_res[0]).run(m_convert);

		volume->set_volume_position({ stof(vals[5]), stof(vals[6]), stof(vals[7]) });
		volume->set_volume_scale({ 1.0, 1.0, 1.0 });
		volume->set_volume_mv(glm::mat4());

		if (vals.size() > 9)
		{
			std::cerr << "Set render channel to " << vals[9] << std::endl;
			volume->set_render_channel(std::stoi(vals[9]));
		}
		promise->set_value(volume);

		std::cerr << "end load" << std::endl;
	}
}

void VRVolumeApp::load_nrrd_file(std::string& filename)
{
	std::vector<std::string> vals;
	vals.push_back(filename);

	std::vector<std::thread*> ths;
	ths.emplace_back(new std::thread(&VRVolumeApp::load_volume, this, vals, nullptr));
	m_threads.push_back(ths);
	m_numVolumes = 1;
	set_num_volumes(m_numVolumes);
	if (m_ui_view)
	{
		m_ui_view->update_ui(m_numVolumes);
	}
}

void VRVolumeApp::render(const MinVR::VRGraphicsState& render_state)
{
	m_trackball.set_app_mode(m_app_mode);
	if (m_app_mode == SIMULATION)
	{
		if (m_simulation)
		{
			m_simulation->update_simulation();
			SimulationState sim_state = m_simulation->get_current_simulation_state();
			m_trackball.update_sim_poi(sim_state.poi);
			m_ui_view->set_clip_max(sim_state.max_clip);
			m_ui_view->set_clip_min(sim_state.min_clip);
		}
	}

	if (m_is2d)
	{
		m_headpose = glm::make_mat4(render_state.getViewMatrix());
		m_headpose = glm::inverse(m_headpose);
	}

	glClearColor(0, 0, 0, 1);
	glClearDepth(1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (render_state.isInitialRenderCall())
	{
		int window_w = render_state.index().getValue("WindowWidth");
		int window_h = render_state.index().getValue("WindowHeight");
		int framebuffer_w = render_state.index().getValue("FramebufferWidth");
		int framebuffer_h = render_state.index().getValue("FramebufferHeight");
		m_depthTextures.push_back(new DepthTexture(window_w, window_h, framebuffer_w, framebuffer_h));
	}

	m_projection_mtrx = glm::make_mat4(render_state.getProjectionMatrix());
	m_view_matrix = glm::make_mat4(render_state.getViewMatrix());
	// overwrite MV for 2D viewing
	if (m_is2d)
		m_view_matrix = m_trackball.get_view_matrix();

	glm::mat4 general_model_matrix = glm::scale(glm::mat4(1.0f), glm::vec3(m_ui_view->get_scale(), m_ui_view->get_scale(), m_ui_view->get_scale()));

	// setup Modelview for volumes
	for (int i = 0; i < m_volumes.size(); i++)
	{
		for (int j = 0; j < m_volumes[i].size(); j++)
		{
			//glm::mat4 volume_matrix = m_view_matrix * m_object_pose * glm::scale(general_model_matrix, glm::vec3(1.0, 1.0, m_ui_view->get_z_scale()));
			glm::mat4 volume_matrix = m_view_matrix * m_object_pose * glm::scale(general_model_matrix, glm::vec3(1.0, 1.0, m_volumes[i][j]->get_volume_scale().x / m_volumes[i][j]->get_volume_scale().z));
			
			if (!m_animated)
			{
				volume_matrix = glm::translate(volume_matrix, glm::vec3(m_volumes[i][j]->get_volume_position().x, m_volumes[i][j]->get_volume_position().y, m_volumes[i][j]->get_volume_position().z));
			}
			m_volumes[i][j]->set_volume_mv(volume_matrix);
		}
	}

	// setup Modelview for meshes
	for (Mesh* mesh : m_mesh_models)
	{
		if (!m_volumes.empty())
		{
			int i = 0;
			Volume* volume = m_volumes[mesh->get_volume_id()][0];
			glm::mat4 volume_mv = volume->get_volume_mv();
			float px = volume->get_volume_scale().x;
			float py = volume->get_volume_scale().y;
			float pz = volume->get_volume_scale().z;

			/*
			* TO DO #60 : Fix parent child relationship to not affect the mesh with z-scale
			 glm::vec3 volume_position = volume_mv[3];
			 glm::mat4 mesh_model_matrix =  glm::translate(general_model_view, volume_position);
			 volume_mv = glm::translate(general_model_view, volume_position);
			 glm::mat4 mesh_model_matrix = glm::translate(volume_mv, glm::vec3(-0.5f, -0.5f, -0.5f * px / pz));
			 mesh_model_matrix = glm::scale(mesh_model_matrix, glm::vec3(px, py, px));
			**/

			//volume_mv = glm::rotate(volume_mv, glm::radians(180.0f), glm::vec3(1.0f,0.f,0.f));
			/*volume_mv = glm::translate(volume_mv, glm::vec3(-0.5f, -0.5f, -0.5f * m_volumes[0][m_models_volumeID[i]]->get_volume_scale().x / (m_volumes[0][m_models_volumeID[i]]->get_volume_scale().z)));
			volume_mv = glm::scale(volume_mv, glm::vec3(m_volumes[0][m_models_volumeID[i]]->get_volume_scale().x, m_volumes[0][m_models_volumeID[i]]->get_volume_scale().y, m_volumes[0][m_models_volumeID[i]]->get_volume_scale().x));


			mesh->get_model().setMVMatrix(volume_mv);*/

			glm::mat4 mesh_model_matrix = glm::translate(volume_mv, glm::vec3(-0.5f, -0.5f, -0.5f * px / pz));
			mesh_model_matrix = glm::scale(mesh_model_matrix, glm::vec3(px, py, px));
			mesh->get_model().setMVMatrix(mesh_model_matrix); 
		}
	}

	if (m_clipping || m_ui_view->is_use_custom_clip_plane())
	{
		glm::mat4 clipPlane = glm::inverse(m_controller_pose) * glm::inverse(m_view_matrix);

		if (m_use_custom_clip_plane)
		{
			clipPlane = glm::eulerAngleYXZ(m_clip_ypr.x, m_clip_ypr.y, m_clip_ypr.z);
			clipPlane = glm::translate(clipPlane, m_clip_pos);
			clipPlane = clipPlane * glm::inverse(m_view_matrix);
		}

		for (auto ren : m_renders)
			ren->set_clipping_plane(true, &clipPlane);
	}
	else
	{
		for (auto ren : m_renders)
			ren->set_clipping_plane(false, nullptr);
	}

	for (auto ren : m_renders)
	{
		if (m_ui_view)
		{
			ren->set_clip_min_max(m_ui_view->get_clip_min(), m_ui_view->get_clip_max());
		}
	}

	//render mesh/terrain
	if (m_ui_view->get_render_mesh())
	{
		for (Mesh* mesh : m_mesh_models)
		{
			m_simple_texture_shader.start();
			m_simple_texture_shader.setUniform("projection_matrix", m_projection_mtrx);
			mesh->get_model().render(m_simple_texture_shader);
			m_simple_texture_shader.stop();
		}
	}
	

	// render labels
	if (m_ui_view->get_render_labels())
	{
		render_labels(render_state);
	}
	

	m_depthTextures[m_rendercount]->copyDepthbuffer();
	
	(static_cast<VolumeRaycastRenderer*>(m_renders[1]))->setDepthTexture(m_depthTextures[m_rendercount]);

	// drawTime
	if (m_is2d && m_animated)
	{
		unsigned int active_volume = floor(m_frame_step);
		unsigned int active_volume2 = ceil(m_frame_step);
		if (active_volume < m_volumes[0].size() && active_volume2 < m_volumes[0].size() && m_volumes[0][active_volume]->texture_initialized() && m_volumes[0][active_volume2]->texture_initialized())
		{
			float alpha = m_frame_step - active_volume;
			time_t time = m_volumes[0][active_volume]->getTime() * (1 - alpha) + m_volumes[0][active_volume2]->getTime() * alpha;
			m_ui_view->set_volume_time_info(time);
		}
	}

	render_volume(render_state);

	render_ui(render_state);

	glFlush();

	m_rendercount++;

	if (m_movieAction)
	{
#ifndef _MSC_VER
		glFinish();
#endif
		std::cerr << "Add Frame" << std::endl;
		m_movieAction->addFrame();
	}
}

void VRVolumeApp::render_labels(const MinVR::VRGraphicsState& renderState)
{
	// render labels
	if (m_label_manager)
	{
		m_label_manager->draw_labels(m_projection_mtrx, m_headpose, m_ui_view->get_z_scale());
	}
}

void VRVolumeApp::render_volume(const MinVR::VRGraphicsState& renderState)
{
	for (auto ren : m_renders)
	{
		ren->set_multiplier(m_ui_view->get_multiplier());
		ren->set_threshold(m_ui_view->get_threshold());
		ren->set_num_slices(m_ui_view->get_slices());
		ren->useMultichannelColormap(m_use_multi_transfer);
	}

	for (int tfn = 0; tfn < m_ui_view->get_num_transfer_functions(); tfn++)
	{

		for (int vol = 0; vol < m_numVolumes; vol++)
		{
			m_animated ? animated_render(tfn, vol) : normal_render_volume(tfn, vol);
		}
	}
}

void VRVolumeApp::normal_render_volume(int tfn, int vol)
{
	if (m_ui_view && m_ui_view->is_transfer_function_enabled(tfn, vol))
	{
		std::vector<std::pair<float, int>> order;
		for (int i = 0; i < m_volumes.size(); i++)
		{
			glm::vec4 center = m_volumes[vol][i]->get_volume_mv() * glm::vec4(0, 0, 0, 1);
			float l = glm::length(center);
			order.push_back(std::make_pair(l, i));
		}
		std::sort(order.begin(), order.end());
		int renderMethod = m_ui_view->get_render_method();
		bool useTranferFunction = m_ui_view->is_use_transfer_function_enabled();

		for (int i = order.size() - 1; i >= 0; i--)
		{
			if (m_volumes[vol][order[i].second]->texture_initialized())
			{
				if (m_ui_view->is_render_volume_enabled())
				{

					GLint colorMap = m_ui_view->get_transfer_function_colormap(tfn);
					GLint colorMapMult = m_ui_view->get_multitransfer_function_colormap(tfn);
					m_renders[renderMethod]->render(m_volumes[vol][order[i].second], m_volumes[vol][order[i].second]->get_volume_mv(), m_projection_mtrx, m_volumes[vol][order[i].second]->get_volume_scale().x / m_volumes[vol][order[i].second]->get_volume_scale().z,
						useTranferFunction ? (m_use_multi_transfer) ? colorMapMult : colorMap : -1, m_ui_view->get_render_channel());
				}
			}
		}
	}
}

void VRVolumeApp::animated_render(int tfn, int vol)
{
	/*
	  A sequence of volumes in the same data set can be animated. Each of the volumes are considered a key-frame.
	  m_frame_step is a float send by the UI as a measurement of the distance between the current (active_volume) and th next (next_active_volume) key-frame.
	*/

	unsigned int active_volume = floor(m_frame_step);
	unsigned int next_active_volume = ceil(m_frame_step);

	int render_method = m_ui_view->get_render_method();
	bool use_tranferFunction = m_ui_view->is_use_transfer_function_enabled();

	size_t max_animation_length = 0;
	float volume_min = std::numeric_limits<float>::max();
	float volume_max = std::numeric_limits<float>::min();
	bool dirty = false;
	if (m_ui_view && m_ui_view->is_transfer_function_enabled(tfn, vol))
	{
		dirty = true;
		max_animation_length = std::max(max_animation_length, m_volumes[vol].size());
		volume_min = std::min(volume_min, m_volumes[vol][active_volume]->getMin());
		volume_max = std::max(volume_max, m_volumes[vol][active_volume]->getMax());

		if (active_volume < m_volumes[vol].size() && next_active_volume < m_volumes[vol].size() && m_volumes[vol][active_volume]->texture_initialized() && m_volumes[vol][next_active_volume]->texture_initialized())
		{
			// use the m_frame_step to create a blend model between the current and the next volume
			m_renders[render_method]->set_blending(true, m_frame_step - active_volume, m_volumes[vol][next_active_volume]);

			if (m_ui_view->is_render_volume_enabled())
			{

				GLint lut = -1;
				if (use_tranferFunction)
				{
					if (m_use_multi_transfer)
					{
						lut = m_ui_view->get_multitransfer_function_colormap(tfn);
					}
					else
					{
						lut = m_ui_view->get_transfer_function_colormap(tfn);
					}
				}

				m_renders[render_method]->render(m_volumes[vol][active_volume], m_volumes[vol][active_volume]->get_volume_mv(), m_projection_mtrx, m_volumes[vol][active_volume]->get_volume_scale().x / m_volumes[vol][active_volume]->get_volume_scale().z,
					lut, m_ui_view->get_render_channel());
			}
		}
	}
	m_ui_view->set_animation_length(max_animation_length);

	if (dirty)
	{
		m_volumes_global_min_value = std::min(m_volumes_global_min_value, volume_min);
		m_volumes_global_max_value = std::max(m_volumes_global_max_value, volume_max);
		m_ui_view->set_transfer_function_min_max(m_volumes_global_min_value, m_volumes_global_max_value);
	}
}

void VRVolumeApp::render_ui(const MinVR::VRGraphicsState& renderState)
{
	// render menu
	if (m_ui_view && m_window_properties)
	{
		m_window_properties->window_w = renderState.index().getValue("WindowWidth");
		m_window_properties->window_h = renderState.index().getValue("WindowHeight");
		m_window_properties->framebuffer_w = renderState.index().getValue("FramebufferWidth");
		m_window_properties->framebuffer_h = renderState.index().getValue("FramebufferHeight");
		glm::mat4 projectionMatrix = glm::make_mat4(renderState.getProjectionMatrix());
		glm::mat4 viewMatrix = glm::make_mat4(renderState.getViewMatrix());

		if (!m_is2d)
		{
			glm::mat4 mvMatrix = glm::make_mat4(renderState.getViewMatrix());
			m_ui_view->render_3D(projectionMatrix, viewMatrix, *m_window_properties);
		}
		else
		{
			m_ui_view->render_2D(*m_window_properties);
		}
	}
}

void VRVolumeApp::update_frame_state()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, m_light_pos);
	if (m_animated && !m_ui_view->is_stopped())
	{
		m_frame_step += (m_speed * m_volume_animation_scale_factor);
		if (m_frame_step > m_volumes[m_selectedVolume].size() - 1)
		{
			m_frame_step = 0.0;
		}
	}
	m_rendercount = 0;
}

void VRVolumeApp::update_3D_ui()
{
	if (m_show_menu && m_ui_view)
	{
		if (!m_is2d)
		{
			m_ui_view->update_3D_ui_frame();
		}
	}
}

void VRVolumeApp::update_2D_ui()
{
	if (m_show_menu && m_ui_view)
	{
		if (m_is2d && m_window_properties)
		{
			m_ui_view->render_2D(*m_window_properties);
		}
	}
}

void VRVolumeApp::add_lodaded_textures()
{
	bool allready = true;
	for (auto& fut : m_futures)
	{
		std::vector<std::future<Volume*>>* _ft = fut;
		for (auto& f : *_ft)
		{
#ifdef _MSC_VER
			allready = allready & f._Is_ready();
#else
			allready = allready & (f.wait_for(std::chrono::seconds(0)) == std::future_status::ready);
#endif
		}
	}

	if (allready)
	{
		for (int i = 0; i < m_futures.size(); i++)
		{
			std::vector<std::future<Volume*>>* _ft = m_futures[i];
			int counter = 0;
			for (auto& value : *_ft)
			{

				Volume* vlm = value.get();
				m_volumes[i].push_back(vlm);
				m_threads[i][counter]->join();
				delete m_threads[i][counter];
				delete m_promises[i][counter];
				counter++;
			}
		}

		m_threads.clear();
		m_promises.clear();
		m_futures.clear();
	}
}

void VRVolumeApp::clear_data()
{
	for (int i = 0; i < m_volumes.size(); i++)
	{
		std::vector<Volume*> v = m_volumes[i];
		for (int j = 0; j < v.size(); j++)
		{
			delete v[i];
		}
	}
	m_volumes.clear();
	m_numVolumes = 0;
	m_description.clear();
	if (m_label_manager)
	{
		m_label_manager->clear();
	}

	m_mesh_models_data.clear();
	m_models_displayLists.clear();
	m_models_position.clear();
	m_models_volumeID.clear();
	m_models_MV.clear();
}

bool VRVolumeApp::data_is_multi_channel()
{
	bool is_multi_channel = false;
	if (m_ui_view)
	{
		int renderchannel = m_ui_view->get_render_channel();
		for (int i = 0; i < m_volumes.size(); i++)
		{
			// renderchannel[0] = "based on data"
			// renderchannel[5] =  "rgba"
			// renderchannel[6] = "rgba with alpha as max rgb";
			if (!m_volumes[i].empty())
			{
				if (m_volumes.size() > 0 && m_volumes[i][0]->get_channels() > 1 &&
					(renderchannel == 0 || renderchannel == 5 || renderchannel == 6))
				{
					is_multi_channel |= true;
				}
			}
			
		}
	}

	return is_multi_channel;
}

void VRVolumeApp::get_min_max(const float frame, float& min, float& max)
{
	unsigned int active_volume = floor(frame);
	unsigned int active_volume2 = ceil(frame);
	double alpha = frame - active_volume;
	min = m_volumes[m_selectedVolume][active_volume]->getMin() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMin() * (1.0 - alpha);
	max = m_volumes[m_selectedVolume][active_volume]->getMax() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMax() * (1.0 - alpha);
}

int VRVolumeApp::get_num_volumes()
{
	return m_numVolumes;
}

void VRVolumeApp::set_is_2D(bool is2d)
{
	m_is2d = is2d;
}

void VRVolumeApp::set_looking_glass(bool islookingGlass)
{
	m_lookingGlass = islookingGlass;
}

void VRVolumeApp::set_convert(bool covert)
{
	m_convert = covert;
}

void VRVolumeApp::set_num_volumes(int nVolumes)
{
	m_numVolumes = nVolumes;
}

void VRVolumeApp::mouse_pos_event(glm::vec2& mPos)
{
	get_trackball_camera().mouse_move(mPos.x, mPos.y);
	if (m_ui_view)
	{
		m_ui_view->set_cursor_pos(mPos);
	}
}

void VRVolumeApp::update_ui_events(float value)
{
	if (m_show_menu && m_ui_view)
	{
		m_ui_view->set_analog_value(value);
	}
}

void VRVolumeApp::update_track_ball_event(float value)
{
	get_trackball_camera().mouse_scroll(value);
}

void VRVolumeApp::button_events_ui_handle(int button, int state)
{
	if (m_ui_view)
	{
		m_ui_view->set_button_click(button, state);
	}
}

void VRVolumeApp::button_event_trackBall_handle(int button, int state)
{
	get_trackball_camera().mouse_pressed(button, state);
}

void VRVolumeApp::enable_grab(bool grab)
{
	m_grab = grab;
}

void VRVolumeApp::enable_clipping(bool clipping)
{
	m_clipping = clipping;
}

void VRVolumeApp::enable_ui_menu()
{
	m_show_menu = !m_show_menu;
}

void VRVolumeApp::set_AWSD_keyBoard_event(int key)
{
	m_wasd_pressed = m_wasd_pressed | key;
}

void VRVolumeApp::unset_AWSD_keyBoard_event(int key)
{
	m_wasd_pressed = m_wasd_pressed & ~key;
}

void VRVolumeApp::enable_render_volume()
{
	if (m_ui_view)
	{
		m_ui_view->set_enable_render_volume();
	}
}

void VRVolumeApp::update_UI_pose_controller(glm::mat4& newPose)
{
	if (m_ui_view)
	{
		m_ui_view->set_controller_pose(newPose);
	}
}

void VRVolumeApp::update_head_pose(glm::mat4& newPose)
{
	if (m_is2d)
	{
		m_headpose = newPose;
	}
}

void VRVolumeApp::update_fps(float fps)
{
	m_fps = fps;
}

float VRVolumeApp::get_fps()
{
	return m_fps;
}

void VRVolumeApp::update_dynamic_slices()
{
	if (m_ui_view && m_ui_view->is_dynamic_slices())
	{
		m_ui_view->update_slices(m_fps);
	}
}

void VRVolumeApp::do_grab(glm::mat4& newPose)
{
	if (m_grab)
	{
		// Update the paintingToRoom transform based upon the new transform
		// of the left hand relative to the last frame.
		m_object_pose = newPose * glm::inverse(m_controller_pose) * m_object_pose;
	}
	m_controller_pose = newPose;
}

ArcBallCamera& VRVolumeApp::get_trackball_camera()
{
	return m_trackball;
}

void VRVolumeApp::set_volume_animation_scale_factor(float scale)
{
	m_volume_animation_scale_factor = scale;
}

Simulation& VRVolumeApp::get_simulation()
{
	return *m_simulation;
}

void VRVolumeApp::set_clip_min(glm::vec3 clip_min)
{
	m_ui_view->set_clip_min(clip_min);
}

void VRVolumeApp::set_clip_max(glm::vec3 clip_max)
{
	m_ui_view->set_clip_max(clip_max);
}

std::string VRVolumeApp::get_movie_state_label()
{
	if (m_current_movie_state == STOP)
	{
		return "Write Movie";
	}
	else
	{
		return "STOP RECORD";
	}
}

MOVIESTATE VRVolumeApp::get_movie_state()
{
	return m_current_movie_state;
}

void VRVolumeApp::set_app_mode(APPMODE mode)
{
	m_app_mode = mode;
}

std::vector<std::vector<Volume*>>& VRVolumeApp::get_volumes()
{
	return m_volumes;
}

std::vector<Mesh*>& VRVolumeApp::get_mesh_models()
{
	return m_mesh_models;
}
