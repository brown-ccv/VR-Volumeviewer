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
#include "render/FontHandler.h"
#include "interaction/CreateMovieAction.h"

#include "GL/glew.h"
#include <filesystem>
#include <fstream> 
#include <sstream> 
#include "GLMLoader.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#define OS_SLASH_LOCAL "\\"
#include "../../external/msvc/dirent.h"
#else
#define OS_SLASH_LOCAL "//"
#include <dirent.h>
#endif

#include <glm/gtx/euler_angles.hpp>


VRVolumeApp::VRVolumeApp():m_mesh_model(nullptr), m_clip_max{ 1.0f }, m_clip_min{ 0.0f }, m_clip_ypr{ 0.0f }, m_clip_pos{ 0.0 }, m_wasd_pressed(0),
m_lookingGlass( false ), m_isInitailized(false), m_speed(0.01f), m_movieAction( nullptr ), m_moviename( "movie.mp4" ), m_noColor(0.0f),
m_ambient(0.2f, 0.2f, 0.2f, 1.0f), m_diffuse(0.5f, 0.5f, 0.5f, 1.0f), m_ui_view(nullptr), m_animated(false), m_numVolumes(0), m_selectedVolume(0),
m_multiplier( 1.0f ), m_threshold( 0.0f ),m_frame( 0.0f ), m_use_multi_transfer( false )
{
  m_renders.push_back(new VolumeSliceRenderer());
  m_renders.push_back(new VolumeRaycastRenderer());
}




void VRVolumeApp::initialize()
{
  if (!m_isInitailized)
  {
    m_object_pose = glm::mat4(1.0f);
    initializeGL();
    if (!m_ui_view)
    {
      m_ui_view = new  UIView(*this);
      m_ui_view->initUI(m_is2d, m_lookingGlass);
    }
    m_isInitailized = true;
    m_rendercount = 0;
  }

}

void VRVolumeApp::initializeGL()
{
  for (auto ren : m_renders)
    ren->initGL();

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



void VRVolumeApp::loadMeshModel()
{
	for (std::string filename : m_models_filenames) {

		m_mesh_model = GLMLoader::loadObjModel(filename);


		if (m_texture)
		{
			m_mesh_model->addTexture(m_texture);
		}
		
		/*  glmFacetNormals(pmodel);
			glmVertexNormals(pmodel, 90.0);
		 glColor4f(1.0, 1.0, 1.0, 1.0);
			m_models_displayLists.push_back(glmList(pmodel, GLM_SMOOTH));
			glmDelete(pmodel);*/
	}
	m_models_filenames.clear();
}

void VRVolumeApp::loadShaders()
{
  m_shader_file_path = "shaders";
  std::string vertexShaderFolderPath = m_shader_file_path + OS_SLASH + std::string("shader.vert");
  std::string fragmentShaderFolderPath = m_shader_file_path + OS_SLASH + std::string("shader.frag");
  m_simple_texture_shader.LoadShaders(vertexShaderFolderPath.c_str(), fragmentShaderFolderPath.c_str());
  // simpleTextureShader.addUniform("m");
   //simpleTextureShader.addUniform("v");
  m_simple_texture_shader.addUniform("p");
  m_simple_texture_shader.addUniform("mv");
}

void VRVolumeApp::initializeTextures()
{
  addLodadedTextures();
  for (int i = 0; i < m_volumes.size(); i++) {

    std::vector< Volume* > vlm = m_volumes[i];
    for (int j = 0; j < vlm.size(); j++)
    {
      vlm[j]->initGL();
    }

  }
}

bool VRVolumeApp::pendingModelsToLoad()
{
  return m_models_filenames.size() > 0;
}

void VRVolumeApp::updateTrackBallState()
{
  if (m_wasd_pressed)
  {
    m_trackball.wasd_pressed(m_wasd_pressed);
  }
   
}

void VRVolumeApp::updateAnimation()
{
  if (m_ui_view)
  {
    if (m_volumes.size())
    {
      m_ui_view->updateAnimation(m_speed, m_volumes[m_selectedVolume].size() - 1);
    }
  }
}

void VRVolumeApp::runMovie()
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

void VRVolumeApp::setRendercount(unsigned int rendercount)
{
  m_rendercount = rendercount;
}

float VRVolumeApp::getCurrentFrame()
{
  return m_frame;
}

void VRVolumeApp::setFrame(float frame)
{
  m_frame = frame;
}

glm::vec4& VRVolumeApp::getNoColor()
{
  return m_noColor;
}

glm::vec4& VRVolumeApp::getAmbient()
{
  return m_ambient;
}

glm::vec4& VRVolumeApp::getDiffuse()
{
  return m_diffuse;
}

void VRVolumeApp::setMultiTransfer(bool multiTransfer)
{
  m_use_multi_transfer = multiTransfer;
}

bool VRVolumeApp::isMultiTransfer()
{
  return m_use_multi_transfer;
}

bool VRVolumeApp::isUIEvent()
{
  if (m_ui_view->isUIWindowActive())
  {
    return true;
  }
  return false;
}

void VRVolumeApp::intializeUI()
{
  m_ui_view->initUI( m_is2d, m_lookingGlass);
}

void VRVolumeApp::loadTxtFile(std::string& filename)
{
	std::ifstream inFile;
	inFile.open(filename);

	std::string line;

	std::filesystem::path p_filename(filename);

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
					m_descriptionFilename = p_filename.parent_path().string() + OS_SLASH_LOCAL + vals[1];
					m_description = LoadDescriptionAction(m_descriptionFilename).run();
					std::cerr << m_description[0] << std::endl;
				}
				if (tag == "mesh")
				{
					//	std::cerr << "Load Mesh " << vals[1] << std::endl;
					//	std::cerr << "for Volume " << vals[2] << std::endl;
					vals[1] = p_filename.parent_path().string() + OS_SLASH_LOCAL + vals[1];
					std::cerr << "Load Mesh " << vals[1] << std::endl;
					m_models_volumeID.push_back(stoi(vals[2]) - 1);
					m_models_filenames.push_back(vals[1]);
					m_models_MV.push_back(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
					m_shader_file_path = p_filename.parent_path().string() + OS_SLASH_LOCAL + "shaders";

				}
				if (tag == "texture")
				{

					//std::cerr << "for Volume " << vals[2] << std::endl;
					m_texture_filePath = p_filename.parent_path().string() + OS_SLASH_LOCAL + vals[1];
					std::cerr << "Load texture " << m_texture_filePath << std::endl;
					m_texture = new Texture(GL_TEXTURE_2D, m_texture_filePath);

				}
				if (tag == "numVolumes")
				{
					m_numVolumes = std::stoi(vals[1]);
					//m_data_labels.resize(m_numVolumes);
					for (int i = 0; i < m_numVolumes; i++)
					{
						m_ui_view->addDataLabel(vals[i + 2]);
					}
					m_volumes.resize(m_numVolumes);
					m_promises.resize(m_numVolumes);
					m_futures.resize(m_numVolumes);
					m_threads.resize(m_numVolumes);
          m_ui_view->updateUI(m_numVolumes);

					/*tfn_widget_multi.resize(1);
					tfn_widget.resize(1);
					selectedTrFn.resize(1);

					*/
					
					
				}
				else if (tag.rfind("volume") == 0)
				{
					char str[3];
					int i;

					std::string strVolumeIndex = tag.substr(6);
					size_t volumeIndex = std::stoi(strVolumeIndex);


					vals[1] = p_filename.parent_path().string() + OS_SLASH_LOCAL + vals[1];



					std::vector<std::promise<Volume*>*>& v = m_promises[volumeIndex - 1];
					std::promise<Volume*>* pm = new std::promise<Volume*>();

					v.push_back(pm);

					//promises[volumeIndex-1]=v;

					std::vector<std::future<Volume*>>* fut;
					if (!m_futures[volumeIndex - 1])
					{
						m_futures[volumeIndex - 1] = new std::vector<std::future<Volume*>>;

					}
					fut = m_futures[volumeIndex - 1];

					fut->push_back(pm->get_future());
					m_futures[volumeIndex - 1] = fut;

					std::vector <std::thread*>& ths = m_threads[volumeIndex - 1];
					std::vector<std::promise<Volume*>*> v2 = m_promises[volumeIndex - 1];
					ths.emplace_back(new std::thread(&VRVolumeApp::loadVolume, this, vals, v2.back()));

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

void VRVolumeApp::loadVolume(std::vector<std::string> vals, std::promise<Volume*>* promise)
{
  if (vals.size() == 1 && helper::ends_with_string(vals[0], ".nrrd"))
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

    Volume* volume = LoadDataAction(vals[1], &t_res[0]).run(m_convert);

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


void VRVolumeApp::loadNrrdFile(std::string& filename)
{
  std::vector<std::string> vals;
  vals.push_back(filename);
  std::vector<std::promise<Volume*>*> v;
  std::promise<Volume*>* pm = new std::promise<Volume*>();

  v.push_back(pm);
  m_promises.push_back(v);

  std::vector<std::future<Volume*>>* fut = new std::vector<std::future<Volume*>>;
  fut->push_back(pm->get_future());
  m_futures.push_back(fut);

  std::vector <std::thread*> ths;
  std::vector<std::promise<Volume*>*> v2 = m_promises.back();
  ths.emplace_back(new std::thread(&VRVolumeApp::loadVolume, this, vals, v2.back()));
  m_threads.push_back(ths);
  m_numVolumes = 1;
  setNumVolumes(m_numVolumes);
  m_ui_view->updateUI(m_numVolumes);
  
  }

void VRVolumeApp::render(const MinVR::VRGraphicsState& renderState)
{
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
  m_projection_mtrx = glm::make_mat4(renderState.getProjectionMatrix());
  m_model_view = glm::make_mat4(renderState.getViewMatrix());

  //overwrite MV for 2D viewing
  if (m_is2d)
    m_model_view = m_trackball.getViewmatrix();

  glMatrixMode(GL_PROJECTION);
  glLoadMatrixf(glm::value_ptr(m_projection_mtrx));

  //setup Modelview for volumes
  for (int i = 0; i < m_volumes.size(); i++) {
    for (int j = 0; j < m_volumes[i].size(); j++) {
      glm::mat4 tmp = m_model_view;
      tmp = tmp * m_object_pose;
      tmp = glm::scale(tmp, glm::vec3(m_ui_view->getScale(), m_ui_view->getScale(), m_ui_view->getScale() * m_ui_view->getZScale()));
      if (!m_animated)
        tmp = glm::translate(tmp, glm::vec3(m_volumes[i][j]->get_volume_position().x, m_volumes[i][j]->get_volume_position().y, m_volumes[i][j]->get_volume_position().z));
      m_volumes[i][j]->set_volume_mv(tmp);
    }
  }

  //setup Modelview for meshes
  if (m_mesh_model)
  {
    int i = 0;
    if (m_volumes.size() > m_models_volumeID[i]) {
      glm::mat4 volume_mv = m_volumes[0][m_models_volumeID[i]]->get_volume_mv();
      volume_mv = glm::translate(volume_mv, glm::vec3(-0.5f, -0.5f, -0.5f * m_volumes[0][m_models_volumeID[i]]->get_volume_scale().x / (m_volumes[0][m_models_volumeID[i]]->get_volume_scale().z)));
      volume_mv = glm::scale(volume_mv, glm::vec3(m_volumes[0][m_models_volumeID[i]]->get_volume_scale().x, m_volumes[0][m_models_volumeID[i]]->get_volume_scale().y, m_volumes[0][m_models_volumeID[i]]->get_volume_scale().x));
      
      m_mesh_model->setMVMatrix(volume_mv);
      m_models_MV[i] = volume_mv;

      /*Original transformations of the model - DO NOT REMOVE YET*/
      //mesh_model->setPosition(glm::vec3(-0.5f, -0.5f, 
      //	-0.5f * m_volumes[m_models_volumeID[i]]->get_volume_scale().x / (m_volumes[m_models_volumeID[i]]->get_volume_scale().z)));
      //mesh_model->setScale(glm::vec3(m_volumes[m_models_volumeID[i]]->get_volume_scale().x, m_volumes[m_models_volumeID[i]]->get_volume_scale().y, m_volumes[m_models_volumeID[i]]->get_volume_scale().x));
    }
  }

  /* SOriginal transformations of the volumes - DO NOT REMOVE YET */
  //for (int i = 0; i < m_models_displayLists.size(); i++) {
  //	if (m_volumes.size() > m_models_volumeID[i]) {
  //		m_models_MV[i] = m_volumes[m_models_volumeID[i]]->get_volume_mv();
  //		m_models_MV[i] = glm::translate(m_models_MV[i], glm::vec3(-0.5f, -0.5f, -0.5f * m_volumes[m_models_volumeID[i]]->get_volume_scale().x / (m_volumes[m_models_volumeID[i]]->get_volume_scale().z)));
  //		m_models_MV[i] = glm::scale(m_models_MV[i], glm::vec3(m_volumes[m_models_volumeID[i]]->get_volume_scale().x, m_volumes[m_models_volumeID[i]]->get_volume_scale().y, m_volumes[m_models_volumeID[i]]->get_volume_scale().x));
  //		//m_models_MV[i] = glm::scale(m_models_MV[i], glm::vec3(10,10,10));
  //	}
  //}

  if (m_clipping || m_use_custom_clip_plane) {
    glm::mat4 clipPlane = glm::inverse(m_controller_pose) * glm::inverse(m_model_view);

    if (m_use_custom_clip_plane) {
      clipPlane = glm::eulerAngleYXZ(m_clip_ypr.x, m_clip_ypr.y, m_clip_ypr.z);
      clipPlane = glm::translate(clipPlane, m_clip_pos);
      clipPlane = clipPlane * glm::inverse(m_model_view);
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

  if (m_mesh_model)
  {
    m_simple_texture_shader.start();
    m_simple_texture_shader.setUniform("p", m_projection_mtrx);
    m_mesh_model->render(m_simple_texture_shader);
    m_simple_texture_shader.stop();
  }

  //render labels
 
  if (m_ui_view && !m_is2d)
  {
    renderLabels(renderState);
    glm::mat4 viewMatrix = glm::make_mat4(renderState.getViewMatrix());
    m_ui_view->render3D(viewMatrix);
  }
  

  m_depthTextures[m_rendercount]->copyDepthbuffer();
  (static_cast <VolumeRaycastRenderer*> (m_renders[1]))->setDepthTexture(m_depthTextures[m_rendercount]);

  renderVolume(renderState);

  if (m_ui_view && m_is2d)
  {
    m_ui_view->render2D();
  }

  //drawTime
  if (m_is2d && m_animated) {
    unsigned int active_volume = floor(m_frame);
    unsigned int active_volume2 = ceil(m_frame);
    if (active_volume < m_volumes[0].size() && active_volume2 < m_volumes[0].size() && m_volumes[0][active_volume]->texture_initialized() && m_volumes[0][active_volume2]->texture_initialized()) {
      float alpha = m_frame - active_volume;
      time_t time = m_volumes[0][active_volume]->getTime() * (1 - alpha) + m_volumes[0][active_volume2]->getTime() * alpha;
      FontHandler::getInstance()->drawClock(time);
    }
  }

  glFlush();

  m_rendercount++;



  if (m_movieAction) {
#ifndef _MSC_VER
    glFinish();
#endif
    std::cerr << "Add Frame" << std::endl;
    m_movieAction->addFrame();
    if (m_frame > m_volumes[m_selectedVolume].size() - 1 - m_speed) {
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

void VRVolumeApp::renderLabels(const MinVR::VRGraphicsState& renderState)
{
  //render labels
  m_labels.draw(m_models_MV, m_headpose, m_ui_view->getZScale());

  if (m_is2d && !m_description.empty())
    FontHandler::getInstance()->renderMultiLineTextBox2D(m_description, 50, 950, 200, m_descriptionHeight);
}

void VRVolumeApp::renderMesh(const MinVR::VRGraphicsState& renderState)
{
  /*Original render Mesh using Fixed pipeline -  DO NOT REMOVE YET*/
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

  if (m_mesh_model)
  {
    m_simple_texture_shader.start();
    m_simple_texture_shader.setUniform("p", m_projection_mtrx);
    m_mesh_model->render(m_simple_texture_shader);
    m_simple_texture_shader.stop();
  }
}

void VRVolumeApp::renderVolume(const MinVR::VRGraphicsState& renderState)
{
  //render volumes
  for (auto ren : m_renders) {
    ren->set_multiplier(m_ui_view->getMultiplier());
    ren->set_threshold(m_ui_view->getThreshold());
    ren->set_numSlices(m_ui_view->getSlices());
    ren->useMultichannelColormap(m_use_multi_transfer);
  }

  

    for (int tfn = 0; tfn < m_ui_view->getNumTransferFunctions(); tfn++)
    {

      for (int vol = 0; vol < m_numVolumes; vol++)
      {
        m_animated ? animatedRender(tfn, vol) : normalRenderVolume(tfn, vol);
     
      }
    }
  
  
}

void VRVolumeApp::normalRenderVolume(int tfn, int vol)
{
  if (m_ui_view->isTransferFunctionEnabled(tfn, vol))
  {
    std::vector<std::pair< float, int> > order;
    for (int i = 0; i < m_volumes.size(); i++) {
      glm::vec4 center = m_volumes[vol][i]->get_volume_mv() * glm::vec4(0, 0, 0, 1);
      float l = glm::length(center);
      order.push_back(std::make_pair(l, i));
    }
    std::sort(order.begin(), order.end());
    int renderMethod = m_ui_view->getRenderMethod();
    bool useTranferFunction = m_ui_view->isUseTransferFunctionEnabled();

    for (int i = order.size() - 1; i >= 0; i--) {
      if (m_volumes[vol][order[i].second]->texture_initialized())
      {
        if (m_ui_view->isRenderVolumeEnabled())
        {

          GLint colorMap = m_ui_view->getTransferFunctionColormap(tfn);
          GLint colorMapMult = m_ui_view->getMTransferFunctionColormap(tfn);
          m_renders[renderMethod]->render(m_volumes[vol][order[i].second], m_volumes[vol][order[i].second]->get_volume_mv(), m_projection_mtrx, m_volumes[vol][order[i].second]->get_volume_scale().x / m_volumes[vol][order[i].second]->get_volume_scale().z,
            useTranferFunction ? (m_use_multi_transfer) ? colorMapMult : colorMap : -1, m_ui_view->getRenderChannel());
        }

      }
    }
  }
}

void VRVolumeApp::animatedRender(int tfn, int vol)
{
  unsigned int active_volume = floor(m_frame);
  unsigned int active_volume2 = ceil(m_frame);
  int renderMethod = m_ui_view->getRenderMethod();
  bool useTranferFunction = m_ui_view->isUseTransferFunctionEnabled();
  //bool useMultitransferFunction = m_ui_view->isUseMultiTransfer();

  if (m_ui_view->isTransferFunctionEnabled(tfn, vol))
  {
    if (active_volume < m_volumes[vol].size() && active_volume2 < m_volumes[vol].size() && m_volumes[vol][active_volume]->texture_initialized() && m_volumes[vol][active_volume2]->texture_initialized())
    {
    
      m_renders[renderMethod]->set_blending(true, m_frame - active_volume, m_volumes[vol][active_volume2]);

      if (m_ui_view->isRenderVolumeEnabled())
      {
        
        GLint colorMap = m_ui_view->getTransferFunctionColormap(tfn);
        GLint colorMapMult = m_ui_view->getMTransferFunctionColormap(tfn);
        GLint lut = -1;
        if (useTranferFunction)
        {
          if (m_use_multi_transfer)
          {
            lut = colorMapMult;
          }
          else
          {
            lut = colorMap;
          }
        }

        std::cout << "render" << std::endl ;
        m_renders[renderMethod]->render(m_volumes[vol][active_volume], m_volumes[vol][active_volume]->get_volume_mv(), m_projection_mtrx, m_volumes[vol][active_volume]->get_volume_scale().x / m_volumes[vol][active_volume]->get_volume_scale().z,
          lut, m_ui_view->getRenderChannel());
      }
    }
  }

}

void VRVolumeApp::renderUI(const MinVR::VRGraphicsState& renderState)
{
  //render menu	
  glm::mat4 mvMatrix = glm::make_mat4(renderState.getViewMatrix());
  if (!m_is2d)
  {
    m_ui_view->render3D(mvMatrix);
  }
  else
  {
    m_ui_view->render2D();
  }


  m_depthTextures[m_rendercount]->copyDepthbuffer();
  (static_cast <VolumeRaycastRenderer*> (m_renders[1]))->setDepthTexture(m_depthTextures[m_rendercount]);
}


void VRVolumeApp::updateFrameState()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glLightfv(GL_LIGHT0, GL_POSITION, m_light_pos);
  if (m_animated && !m_stopped)
  {
    m_frame += m_speed;
    if (m_frame > m_volumes[m_selectedVolume].size() - 1) m_frame = 0.0;
  }
	m_rendercount = 0;
}

void VRVolumeApp::update3DUI()
{
  if (m_show_menu && m_ui_view)
  {
    if (!m_is2d)
    {
      m_ui_view->update3DUIFrame();
    }
  }

}




void VRVolumeApp::update2DUI()
{
  if (m_show_menu && m_ui_view)
  {
    if (m_is2d)
    {
      m_ui_view->render2D();
    }
  }
}

void VRVolumeApp::addLodadedTextures()
{
  bool allready = true;
  for (auto& fut : m_futures)
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

  if (allready)
  {
    for (int i = 0; i < m_futures.size(); i++)
    {
      std::vector <std::future<Volume*>>* _ft = m_futures[i];
      int counter = 0;
      for (auto& value : *_ft)
        //for (int j = 0; j < _ft->size(); j++)
      {
        //Volume* vlm = *_ft[j].get();
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

void VRVolumeApp::clearData()
{
  for (int i = 0; i < m_volumes.size(); i++) {
    std::vector< Volume* > v = m_volumes[i];
    for (int j = 0; j < v.size(); j++)
    {
      delete v[i];
    }

  }
  m_volumes.clear();
  m_description.clear();
  m_labels.clear();

  m_models_filenames.clear();
  m_models_displayLists.clear();
  m_models_position.clear();
  m_models_volumeID.clear();
  m_models_MV.clear();
}


bool VRVolumeApp::dataIsMultiChannel()
{
  bool is_multi_channel = false;
  int renderchannel = m_ui_view->getRenderChannel();
  for (int i = 0; i < m_volumes.size(); i++)
  {
    if (m_volumes.size() > 0 && m_volumes[i][0]->get_channels() > 1 &&
      (renderchannel == 0 || renderchannel == 5 || renderchannel == 6))
    {
      is_multi_channel |= true;
    }
  }

  return is_multi_channel;
}

void VRVolumeApp::getMinMax(const float frame, float& min, float& max)
{
  unsigned int active_volume = floor(frame);
  unsigned int active_volume2 = ceil(frame);
  double alpha = frame - active_volume;
  min = m_volumes[m_selectedVolume][active_volume]->getMin() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMin() * (1.0 - alpha);
  max = m_volumes[m_selectedVolume][active_volume]->getMax() * alpha + m_volumes[m_selectedVolume][active_volume2]->getMax() * (1.0 - alpha);
}

int VRVolumeApp::getNumVolumes()
{
  return m_numVolumes;
}

void VRVolumeApp::setIs2D(bool is2d)
{
  m_is2d = is2d;
}

void VRVolumeApp::setLookingGlass(bool islookingGlass)
{
  m_lookingGlass = islookingGlass;
}

void VRVolumeApp::setConvert(bool covert)
{
   m_convert = covert;
}

void VRVolumeApp::setNumVolumes(int nVolumes)
{
  m_numVolumes = nVolumes;
}

void VRVolumeApp::loadTextFile(std::string& filename)
{

}

void VRVolumeApp::mousePosEvent(glm::vec2& mPos)
{
  m_trackball.mouse_move(mPos.x, mPos.y);
  if (m_ui_view)
  {
    m_ui_view->setCursorPos(mPos);
  }
  
  
}

void VRVolumeApp::updateUIEvents(float value)
{
  if (m_show_menu && m_ui_view)
  {
    m_ui_view->setAnalogValue(value);
  }

}

void VRVolumeApp::updateTrackBallEvent(float value)
{
  m_trackball.mouse_scroll(value);
}

void VRVolumeApp::buttonEventsUIHandle(int button, int state)
{
  if (m_ui_view)
  {
    m_ui_view->setButtonClick(button, state);
  }
  
}

void VRVolumeApp::buttonEventTrackBallHandle(int button, int state)
{
  m_trackball.mouse_pressed(button, state);
}

void VRVolumeApp::enableGrab(bool grab)
{
  m_grab = grab;
}

void VRVolumeApp::enableClipping(bool clipping)
{
  m_clipping = clipping;
}

void VRVolumeApp::enableUIMenu()
{
  m_show_menu = !m_show_menu;
}

void VRVolumeApp::setAWSDKeyBoardEvent(int key)
{
  m_wasd_pressed = m_wasd_pressed | key;
}

void VRVolumeApp::unsetAWSDKeyBoardEvent(int key)
{
  m_wasd_pressed = m_wasd_pressed & ~key;
}

void VRVolumeApp::enableRenderVolume()
{
  if (m_ui_view)
  {
    m_ui_view->setEnableRenderVolume();
  }
}

void VRVolumeApp::updateUIPoseController(glm::mat4& newPose)
{
  if (m_ui_view)
  {
    m_ui_view->setControllerPose(newPose);
  }
}

void VRVolumeApp::updateHeadPose(glm::mat4& newPose)
{
  if (m_is2d)
  {
    m_headpose = newPose;
  }
  
}

void VRVolumeApp::updateFps(float fps)
{
  m_fps = fps;
}

float VRVolumeApp::getFps()
{
  return m_fps;
}

void VRVolumeApp::updateDynamicSlices()
{
  if (m_ui_view && m_ui_view->isDynamicSlices()) 
  {
    m_ui_view->updateSlices(m_fps);
  }
}

void VRVolumeApp::doGrab(glm::mat4& newPose)
{
  if (m_grab) {
    // Update the paintingToRoom transform based upon the new transform
    // of the left hand relative to the last frame.
    m_object_pose = newPose * glm::inverse(m_controller_pose) * m_object_pose;
  }
  m_controller_pose = newPose;
}
