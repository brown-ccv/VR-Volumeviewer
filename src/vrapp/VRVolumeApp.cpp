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

#include <fstream> 
#include <sstream> 
#include "GLMLoader.h"
#include <locale>


#include <glm/gtx/euler_angles.hpp>


VRVolumeApp::VRVolumeApp() :m_mesh_model(nullptr), m_clip_max{ 1.0f }, m_clip_min{ 0.0f }, m_clip_ypr{ 0.0f }, m_clip_pos{ 0.0 }, m_wasd_pressed(0),
m_lookingGlass(false), m_isInitailized(false), m_speed(0.01f), m_movieAction(nullptr), m_moviename("movie.mp4"), m_noColor(0.0f),
m_ambient(0.2f, 0.2f, 0.2f, 1.0f), m_diffuse(0.5f, 0.5f, 0.5f, 1.0f), m_ui_view(nullptr), m_animated(false), m_numVolumes(0), m_selectedVolume(0),
m_multiplier(1.0f), m_threshold(0.0f), m_frame(0.0f), m_use_multi_transfer(false), m_clipping(false), m_show_menu(true), 
m_window_properties(nullptr)
{
  m_renders.push_back(new VolumeSliceRenderer());
  m_renders.push_back(new VolumeRaycastRenderer());
}

VRVolumeApp::~VRVolumeApp()
{
  if (m_ui_view)
  {
    delete m_ui_view;
  }

  delete m_window_properties;
}


void VRVolumeApp::initialize()
{
  if (!m_isInitailized)
  {
    m_object_pose = glm::mat4(1.0f);
    initialize_GL();
    if (!m_ui_view)
    {
      m_ui_view = new  UIView(*this);
      m_ui_view->init_ui(m_is2d, m_lookingGlass);
    }
    m_window_properties = new Window_Properties();
    m_isInitailized = true;
    m_rendercount = 0;
  }

}

void VRVolumeApp::initialize_GL()
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



void VRVolumeApp::load_mesh_model()
{
  for (std::string filename : m_models_filenames) {

    m_mesh_model = GLMLoader::loadObjModel(filename);


    if (m_texture)
    {
      m_mesh_model->addTexture(m_texture);
    }


  }
  m_models_filenames.clear();
}

void VRVolumeApp::load_shaders()
{
  m_shader_file_path = "shaders";
  std::string vertexShaderFolderPath = m_shader_file_path + OS_SLASH + std::string("shader.vert");
  std::string fragmentShaderFolderPath = m_shader_file_path + OS_SLASH + std::string("shader.frag");
  m_simple_texture_shader.LoadShaders(vertexShaderFolderPath.c_str(), fragmentShaderFolderPath.c_str());
  m_simple_texture_shader.addUniform("p");
  m_simple_texture_shader.addUniform("mv");
}

void VRVolumeApp::initialize_textures()
{
  add_lodaded_textures();
  for (int i = 0; i < m_volumes.size(); i++) {

    std::vector< Volume* > vlm = m_volumes[i];
    for (int j = 0; j < vlm.size(); j++)
    {
      vlm[j]->initGL();
    }

  }
}

bool VRVolumeApp::pending_models_to_load()
{
  return m_models_filenames.size() > 0;
}

void VRVolumeApp::update_trackBall_state()
{
  if (m_wasd_pressed)
  {
    m_trackball.wasd_pressed(m_wasd_pressed);
  }

}

void VRVolumeApp::update_animation()
{
  if (m_ui_view)
  {
    if (m_volumes.size())
    {
      m_ui_view->update_animation(m_speed, m_volumes[m_selectedVolume].size() - 1);
    }
  }
}

void VRVolumeApp::run_movie()
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

void VRVolumeApp::set_render_count(unsigned int rendercount)
{
  m_rendercount = rendercount;
}

float VRVolumeApp::get_current_frame()
{
  return m_frame;
}

void VRVolumeApp::set_frame(float frame)
{
  m_frame = frame;
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

void VRVolumeApp::add_label(std::string& text, float x, float y, float z, float textPosZ, float size, int volume)
{
  m_labels.add(text, x, y, z, textPosZ, size, volume);
}

void VRVolumeApp::set_description(int descriptionHeight, std::string& descriptionFilename)
{
  m_descriptionHeight = descriptionHeight;
  m_descriptionFilename = descriptionFilename;
  m_description = LoadDescriptionAction(m_descriptionFilename).run();
  // std::cerr << m_description[0] << std::endl;
}

void VRVolumeApp::set_mesh(int volumeId, std::string& fileName, std::string& shaderFilePath)
{

  m_models_volumeID.push_back(volumeId - 1);
  m_models_filenames.push_back(fileName);
  m_models_MV.push_back(glm::mat4(1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1));
  m_shader_file_path = shaderFilePath;

}

void VRVolumeApp::set_texture(std::string& fileNamePath)
{
  std::cerr << "Load texture " << fileNamePath << std::endl;
  m_texture = new Texture(GL_TEXTURE_2D, fileNamePath);
}

void VRVolumeApp::init_num_volumes(int nVolumes)
{
  m_numVolumes = nVolumes;
  //m_data_labels.resize(m_numVolumes);
  m_volumes.resize(m_numVolumes);
  m_promises.resize(m_numVolumes);
  m_futures.resize(m_numVolumes);
  m_threads.resize(m_numVolumes);
  m_ui_view->update_ui(m_numVolumes);

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

std::vector <std::thread*>& VRVolumeApp::get_thread(int index)
{
  return  m_threads[index - 1];
}

void VRVolumeApp::init_volume_loading(int index, std::vector<std::string> vals)
{

  std::vector <std::thread*>& ths = m_threads[index - 1];
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
          m_ui_view->set_chracter(keyStr[0]);
        }
      }
      /*   if (std::isalpha(keyStr[0], loc) || isdigit(keyStr[0]))
         {
           if (m_ui_view)
           {
             m_ui_view->set_chracter(keyStr[0]);
           }
         }*/
    }
    else
    {
      if (state == 1)
      {
        if (keyStr == "Backspace" || keyStr == "Del")
        {
          m_ui_view->remove_character();
          //_keyBoardInputText->deleteCharacters(_keyBoardInputText->getTextSize() - 1, 1);
        }
        else if (keyStr == "Space")
        {
          m_ui_view->set_chracter(32);
          /*std::string space(" ");
          addTextToInputField(space);*/
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

std::vector< Volume* >& VRVolumeApp::get_volume(int volume)
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


void VRVolumeApp::load_nrrd_file(std::string& filename)
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
  ths.emplace_back(new std::thread(&VRVolumeApp::load_volume, this, vals, v2.back()));
  m_threads.push_back(ths);
  m_numVolumes = 1;
  set_num_volumes(m_numVolumes);
  if (m_ui_view)
  {
    m_ui_view->update_ui(m_numVolumes);
  }


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

  int window_w  =renderState.index().getValue("WindowWidth");
  int window_h  =renderState.index().getValue("WindowHeight");
  int framebuffer_w  =renderState.index().getValue("FramebufferWidth");
  int framebuffer_h  =renderState.index().getValue("FramebufferHeight");

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
      tmp = glm::scale(tmp, glm::vec3(m_ui_view->get_scale(), m_ui_view->get_scale(), m_ui_view->get_scale() * m_ui_view->get_z_scale()));
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

  if (m_clipping || m_ui_view->is_use_custom_clip_plane()) {
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

  for (auto ren : m_renders) {
    if (m_ui_view)
    {
      ren->setClipMinMax(m_ui_view->get_clip_min(), m_ui_view->get_clip_max());
    }
  }


  if (m_mesh_model)
  {
    m_simple_texture_shader.start();
    m_simple_texture_shader.setUniform("p", m_projection_mtrx);
    m_mesh_model->render(m_simple_texture_shader);
    m_simple_texture_shader.stop();
  }

  //render labels
  render_labels(renderState);

  /*if (m_ui_view && !m_is2d)
  {

    glm::mat4 viewMatrix = glm::make_mat4(renderState.getViewMatrix());
    m_ui_view->render_3D(viewMatrix);
  }*/


  m_depthTextures[m_rendercount]->copyDepthbuffer();
  (static_cast <VolumeRaycastRenderer*> (m_renders[1]))->setDepthTexture(m_depthTextures[m_rendercount]);

  render_volume(renderState);


  render_ui(renderState);

  /*if (m_ui_view && m_is2d)
  {
    m_ui_view->render_2D();
  }*/

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

void VRVolumeApp::render_labels(const MinVR::VRGraphicsState& renderState)
{
  //render labels
  m_labels.draw(m_models_MV, m_headpose, m_ui_view->get_z_scale());

  if (m_is2d && !m_description.empty())
    FontHandler::getInstance()->renderMultiLineTextBox2D(m_description, 50, 950, 200, m_descriptionHeight);
}

void VRVolumeApp::render_mesh(const MinVR::VRGraphicsState& renderState)
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
  if (m_mesh_model)
  {
    m_simple_texture_shader.start();
    m_simple_texture_shader.setUniform("p", m_projection_mtrx);
    m_mesh_model->render(m_simple_texture_shader);
    m_simple_texture_shader.stop();
  }
}

void VRVolumeApp::render_volume(const MinVR::VRGraphicsState& renderState)
{
  //render volumes
 // renderState->getProjectionMatrix();
  for (auto ren : m_renders) {
    ren->set_multiplier(m_ui_view->get_multiplier());
    ren->set_threshold(m_ui_view->get_threshold());
    ren->set_numSlices(m_ui_view->get_slices());
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
    std::vector<std::pair< float, int> > order;
    for (int i = 0; i < m_volumes.size(); i++) {
      glm::vec4 center = m_volumes[vol][i]->get_volume_mv() * glm::vec4(0, 0, 0, 1);
      float l = glm::length(center);
      order.push_back(std::make_pair(l, i));
    }
    std::sort(order.begin(), order.end());
    int renderMethod = m_ui_view->get_render_method();
    bool useTranferFunction = m_ui_view->is_use_transfer_function_enabled();

    for (int i = order.size() - 1; i >= 0; i--) {
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
  unsigned int active_volume = floor(m_frame);
  unsigned int active_volume2 = ceil(m_frame);
  int renderMethod = m_ui_view->get_render_method();
  bool useTranferFunction = m_ui_view->is_use_transfer_function_enabled();
  //bool useMultitransferFunction = m_ui_view->isUseMultiTransfer();

  if (m_ui_view && m_ui_view->is_transfer_function_enabled(tfn, vol))
  {
    if (active_volume < m_volumes[vol].size() && active_volume2 < m_volumes[vol].size() && m_volumes[vol][active_volume]->texture_initialized() && m_volumes[vol][active_volume2]->texture_initialized())
    {

      m_renders[renderMethod]->set_blending(true, m_frame - active_volume, m_volumes[vol][active_volume2]);

      if (m_ui_view->is_render_volume_enabled())
      {

        GLint colorMap = m_ui_view->get_transfer_function_colormap(tfn);
        GLint colorMapMult = m_ui_view->get_multitransfer_function_colormap(tfn);
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

        
        m_renders[renderMethod]->render(m_volumes[vol][active_volume], m_volumes[vol][active_volume]->get_volume_mv(), m_projection_mtrx, m_volumes[vol][active_volume]->get_volume_scale().x / m_volumes[vol][active_volume]->get_volume_scale().z,
          lut, m_ui_view->get_render_channel());
      }
    }
  }

}

void VRVolumeApp::render_ui(const MinVR::VRGraphicsState& renderState)
{
  //render menu	
  glm::mat4 mvMatrix = glm::make_mat4(renderState.getViewMatrix());

  if (m_ui_view && m_window_properties)
  { 
    m_window_properties->window_w = renderState.index().getValue("WindowWidth");
    m_window_properties->window_h = renderState.index().getValue("WindowHeight");
    m_window_properties->framebuffer_w = renderState.index().getValue("FramebufferWidth");
    m_window_properties->framebuffer_h = renderState.index().getValue("FramebufferHeight");
  

    if (!m_is2d)
    { 
      m_ui_view->render_3D(mvMatrix,*m_window_properties);
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
    m_frame += m_speed;
    if (m_frame > m_volumes[m_selectedVolume].size() - 1) m_frame = 0.0;
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
  //  m_ui_view->compute_new_histogram();
  }
}

void VRVolumeApp::clear_data()
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


bool VRVolumeApp::data_is_multi_channel()
{
  bool is_multi_channel = false;
  if (m_ui_view)
  {
    int renderchannel = m_ui_view->get_render_channel();
    for (int i = 0; i < m_volumes.size(); i++)
    {
      if (m_volumes.size() > 0 && m_volumes[i][0]->get_channels() > 1 &&
        (renderchannel == 0 || renderchannel == 5 || renderchannel == 6))
      {
        is_multi_channel |= true;
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
  m_trackball.mouse_move(mPos.x, mPos.y);
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
  m_trackball.mouse_scroll(value);
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
  m_trackball.mouse_pressed(button, state);
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
  if (m_grab) {
    // Update the paintingToRoom transform based upon the new transform
    // of the left hand relative to the last frame.
    m_object_pose = newPose * glm::inverse(m_controller_pose) * m_object_pose;
  }
  m_controller_pose = newPose;
}
