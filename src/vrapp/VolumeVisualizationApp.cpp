
#include "vrapp/VolumeVisualizationApp.h"

#include <cmath>
#include <cctype>
#include "loader/LoadDataAction.h"
#ifdef WITH_TEEM
#include "loader/LoadNrrdAction.h"
#endif
#include "interaction/HelperFunctions.h"

#include <glm/gtx/euler_angles.hpp>
#include "glm.h"

#include "GLMLoader.h"
#include "Texture.h"
#include "Model.h"

#include "loader/LoadDescriptionAction.h"
#include "render/FontHandler.h"

#include "UI/UIView.h"
#include "loader/VRDataLoader.h"

#include <cppfs/fs.h>
#include <cppfs/FilePath.h>
#include "render/FontHandler.h"

VolumeVisualizationApp::VolumeVisualizationApp(int argc, char **argv) : VRApp(argc, argv), m_vrVolumeApp(nullptr), m_num_frames(0)
{
  int argc_int = this->getLeftoverArgc();
  char **argv_int = this->getLeftoverArgv();

  m_vrVolumeApp = new VRVolumeApp();
  std::string current_Path = std::string(argv_int[0]);
  cppfs::FilePath p_filename(current_Path);
  std::string parent_Path = p_filename.directoryPath();
  m_vrVolumeApp->set_directory_path(parent_Path);
  FontHandler::setParentPath(parent_Path);

  if (argc_int >= 2)
  {
    for (int i = 1; i < argc_int; i++)
    {

      if (std::string(argv_int[i]) == std::string("use2DUI"))
      {
        // m_is2d = true;
        m_vrVolumeApp->set_is_2D(true);
      }
      if (std::string(argv_int[i]) == std::string("useHolo"))
      {
        // m_lookingGlass = true;
        // m_speed = 0.5;
        m_vrVolumeApp->set_looking_glass(true);
      }
      else if (std::string(argv_int[i]) == std::string("convert"))
      {
        // convert = true;
        m_vrVolumeApp->set_convert(true);
      }
      else if (helper::ends_with_string(std::string(argv_int[i]), ".txt"))
      {
        std::string fileName = argv_int[i];
        VRDataLoader::load_txt_file(*m_vrVolumeApp, fileName);
      }
      else if (helper::ends_with_string(std::string(argv_int[i]), ".nrrd"))
      {

        /*
          NRRD LOADING
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
          threads.push_back(ths);*/
        std::string nrrdFileName(argv_int[i]);
        m_vrVolumeApp->load_nrrd_file(nrrdFileName);
        // m_vrVolumeApp->initialize();
      }
    }
  }

  m_light_pos[0] = 0.0;
  m_light_pos[1] = 4.0;
  m_light_pos[2] = 0.0;
  m_light_pos[3] = 1.0;
}

VolumeVisualizationApp::~VolumeVisualizationApp()
{

  delete m_vrVolumeApp;
}

void VolumeVisualizationApp::onCursorMove(const VRCursorEvent &event)
{
  if (event.getName() == "Mouse_Move")
  {

    if (m_vrVolumeApp)
    {
      glm::vec2 pos2d(event.getPos()[0], event.getPos()[1]);
      m_vrVolumeApp->mouse_pos_event(pos2d);
    }
  }
}

#define count_Packages
#ifdef count_Packages
int last_received = 0;
#endif count_Packages

void VolumeVisualizationApp::onAnalogChange(const VRAnalogEvent &event)
{
  if (m_vrVolumeApp && m_vrVolumeApp->is_show_menu() && m_vrVolumeApp->is_ui_event())
  {
    if (event.getName() == "HTC_Controller_Right_TrackPad0_Y" || event.getName() == "HTC_Controller_1_TrackPad0_Y" || (event.getName() == "Wand_Joystick_Y_Update" && !(event.getValue() > -0.1 && event.getValue() < 0.1)))
      // m_menu_handler->setAnalogValue(event.getValue());
      m_vrVolumeApp->update_ui_events(event.getValue());
    if (event.getName() == "MouseWheel_Spin")
    {
      std::cerr << event.getValue() << std::endl;
      // m_menu_handler->setAnalogValue(event.getValue() * 10);
      m_vrVolumeApp->update_ui_events(event.getValue() * 10);
    }
  }
  else
  {
    if (event.getName() == "MouseWheel_Spin")
    {
      if (m_vrVolumeApp)
      {
        m_vrVolumeApp->update_track_ball_event(event.getValue() * 0.01);
      }
    }
  }

  if (event.getName() == "PhotonLoopFinished")
  {

    m_vrVolumeApp->update_3D_ui();
    m_vrVolumeApp->update_2D_ui();

    if (last_received + 1 != event.getValue())
    {
      std::cerr << "Problem with package , received " << event.getValue() << " expected " << last_received + 1 << std::endl;
    }
    last_received = event.getValue();
  }
}

void VolumeVisualizationApp::onButtonDown(const VRButtonEvent &event)
{
  if (m_vrVolumeApp && m_vrVolumeApp->is_ui_event())
  {
    if (event.getName() == "MouseBtnLeft_Down")
    {
      m_vrVolumeApp->button_events_ui_handle(0, 1);
    }
    else if (event.getName() == "MouseBtnRight_Down")
    {
      m_vrVolumeApp->button_events_ui_handle(1, 1);
    }
    else if (event.getName() == "MouseBtnMiddle_Down")
    {
      m_vrVolumeApp->button_events_ui_handle(2, 0);
    }

    if (m_vrVolumeApp)
    {
      std::string evName = event.getName();
      m_vrVolumeApp->set_character_state(evName, 1);
    }
  }
  else
  {
    if (event.getName() == "MouseBtnLeft_Down")
    {
      m_vrVolumeApp->button_event_trackBall_handle(0, 1);
    }
    else if (event.getName() == "MouseBtnRight_Down")
    {
      m_vrVolumeApp->button_event_trackBall_handle(1, 1);
    }
    else if (event.getName() == "MouseBtnMiddle_Down")
    {
      m_vrVolumeApp->button_event_trackBall_handle(2, 1);
    }
  }

  if (m_vrVolumeApp && m_vrVolumeApp->is_show_menu() && m_vrVolumeApp->is_ui_event())
  {
    if (event.getName() == "HTC_Controller_Right_Axis1Button_Down" || event.getName() == "HTC_Controller_1_Axis1Button_Down" || event.getName() == "B10_Down")
    {
      m_vrVolumeApp->button_events_ui_handle(0, 1);
    }
    else if (event.getName() == "HTC_Controller_Right_GripButton_Down" || event.getName() == "HTC_Controller_1_GripButton_Down" || event.getName() == "Wand_Right_Btn_Down")
    {
      m_vrVolumeApp->button_events_ui_handle(2, 1);
    }
    // else if (event.getName() == "HTC_Controller_Right_AButton_Down" || event.getName() == "HTC_Controller_1_AButton_Down")
    else if (event.getName() == "HTC_Controller_Right_Axis0Button_Down" || event.getName() == "HTC_Controller_1_Axis0Button_Down" || event.getName() == "B08_Down")
    {
      m_vrVolumeApp->button_events_ui_handle(1, 1);
    }
  }
  else
  {
    // This routine is called for all Button_Down events.  Check event->getName()
    // to see exactly which button has been pressed down.
    // std::cerr << "onButtonDown " << event.getName() << std::endl;
    if (event.getName() == "KbdEsc_Down")
    {
      exit(0);
    }
    else if (event.getName() == "HTC_Controller_Right_Axis1Button_Down" || event.getName() == "HTC_Controller_1_Axis1Button_Down" || event.getName() == "B10_Down")
    {
      // m_grab = true;
      // std::cerr << "Grab ON" << std::endl;
      if (m_vrVolumeApp)
      {
        m_vrVolumeApp->enable_grab(true);
      }
    }
    // else if (event.getName() == "HTC_Controller_Right_AButton_Down" || event.getName() == "HTC_Controller_1_AButton_Down")
    else if (event.getName() == "HTC_Controller_Right_Axis0Button_Down" || event.getName() == "HTC_Controller_1_Axis0Button_Down" || event.getName() == "Wand_Right_Btn_Down")
    {
      // m_clipping = true;
      if (m_vrVolumeApp)
      {
        m_vrVolumeApp->enable_clipping(true);
      }

      // std::cerr << "Clipping ON" << std::endl;
    }
    else if (event.getName() == "HTC_Controller_Right_GripButton_Down" || event.getName() == "HTC_Controller_1_GripButton_Down" || event.getName() == "B08_Down")
    {
      // m_show_menu = !m_show_menu;
      if (m_vrVolumeApp)
      {
        m_vrVolumeApp->enable_ui_menu();
      }
    }
  }
  if (!(m_vrVolumeApp && m_vrVolumeApp->is_show_menu() && m_vrVolumeApp->is_ui_event()))
  {
    if (event.getName() == "KbdW_Down")
    {
      m_vrVolumeApp->set_AWSD_keyBoard_event(W);
    }
    if (event.getName() == "KbdA_Down")
    {
      m_vrVolumeApp->set_AWSD_keyBoard_event(A);
    }
    if (event.getName() == "KbdS_Down")
    {
      m_vrVolumeApp->set_AWSD_keyBoard_event(S);
    }
    if (event.getName() == "KbdD_Down")
    {
      m_vrVolumeApp->set_AWSD_keyBoard_event(D);
    }
    if (event.getName() == "KbdQ_Down")
    {
      m_vrVolumeApp->set_AWSD_keyBoard_event(Q);
    }
    if (event.getName() == "KbdE_Down")
    {
      m_vrVolumeApp->set_AWSD_keyBoard_event(E);
    }
  }
}

void VolumeVisualizationApp::onButtonUp(const VRButtonEvent &event)
{
  if (m_vrVolumeApp && m_vrVolumeApp->is_ui_event())
  {
    if (event.getName() == "MouseBtnMiddle_ScrollUp")
    {
      m_vrVolumeApp->update_ui_events(10);
    }

    if (event.getName() == "MouseBtnMiddle_ScrollDown")
    {
      m_vrVolumeApp->update_ui_events(-10);
    }

    if (m_vrVolumeApp)
    {
      std::string evName = event.getName();
      m_vrVolumeApp->set_character_state(evName, 0);
    }
  }
  else
  {
    if (event.getName() == "MouseBtnMiddle_ScrollUp")
    {
      if (m_vrVolumeApp)
      {
        m_vrVolumeApp->button_event_trackBall_handle(2, 0);
      }
    }

    if (event.getName() == "MouseBtnMiddle_ScrollDown")
    {
      if (m_vrVolumeApp)
      {
        m_vrVolumeApp->button_event_trackBall_handle(2, 1);
      }
    }
  }

  if (event.getName() == "MouseBtnLeft_Up")
  {
    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->button_events_ui_handle(0, 0);
      m_vrVolumeApp->button_event_trackBall_handle(0, 0);
    }
  }
  else if (event.getName() == "MouseBtnRight_Up")
  {

    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->button_events_ui_handle(1, 0);
      m_vrVolumeApp->button_event_trackBall_handle(1, 0);
    }
  }
  else if (event.getName() == "MouseBtnMiddle_Up")
  {

    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->button_events_ui_handle(2, 0);
      m_vrVolumeApp->button_event_trackBall_handle(2, 0);
    }
  }

  if (m_vrVolumeApp && m_vrVolumeApp->is_show_menu())
  {
    if (event.getName() == "HTC_Controller_Right_Axis1Button_Up" || event.getName() == "HTC_Controller_1_Axis1Button_Up" || event.getName() == "B10_Up")
    {
      m_vrVolumeApp->button_events_ui_handle(0, 0);
    }
    else if (event.getName() == "HTC_Controller_Right_GripButton_Up" || event.getName() == "HTC_Controller_1_GripButton_Up" || event.getName() == "Wand_Right_Btn_Up")
    {
      m_vrVolumeApp->button_events_ui_handle(2, 0);
    }
    // else if (event.getName() == "HTC_Controller_Right_AButton_Down" || event.getName() == "HTC_Controller_1_AButton_Down")
    else if (event.getName() == "HTC_Controller_Right_Axis0Button_Up" || event.getName() == "HTC_Controller_1_Axis0Button_Up" || event.getName() == "B08_Up")
    {
      m_vrVolumeApp->button_events_ui_handle(1, 0);
    }
  }
  // This routine is called for all Button_Up events.  Check event->getName()
  // to see exactly which button has been released.
  // std::cerr << "onButtonUp " << event.getName() << std::endl;
  if (event.getName() == "HTC_Controller_Right_Axis1Button_Up" || event.getName() == "HTC_Controller_1_Axis1Button_Up" || event.getName() == "B10_Up")
  {

    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->enable_grab(false);
    }
  }
  // else if (event.getName() == "HTC_Controller_Right_AButton_Up" || event.getName() == "HTC_Controller_1_AButton_Up")
  else if (event.getName() == "HTC_Controller_Right_Axis0Button_Up" || event.getName() == "HTC_Controller_1_Axis0Button_Up" || event.getName() == "Wand_Right_Btn_Up")
  {

    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->enable_clipping(false);
    }
  }

  if (event.getName() == "KbdW_Up")
  {

    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->unset_AWSD_keyBoard_event(W);
    }
  }
  if (event.getName() == "KbdA_Up")
  {
    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->unset_AWSD_keyBoard_event(A);
    }
  }
  if (event.getName() == "KbdS_Up")
  {
    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->unset_AWSD_keyBoard_event(S);
    }
  }
  if (event.getName() == "KbdD_Up")
  {
    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->unset_AWSD_keyBoard_event(D);
    }
  }
  if (event.getName() == "KbdQ_Up")
  {
    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->unset_AWSD_keyBoard_event(Q);
    }
  }
  if (event.getName() == "KbdE_Up")
  {
    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->unset_AWSD_keyBoard_event(E);
    }
  }

  if (event.getName() == "KbdSpace_Up")
  {
    // m_renderVolume = !m_renderVolume;
    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->enable_render_volume();
    }
  }
}

void VolumeVisualizationApp::onTrackerMove(const VRTrackerEvent &event)
{

  if (event.getName() == "HTC_Controller_Right_Move" || event.getName() == "HTC_Controller_1_Move" || event.getName() == "Wand0_Move")
  {

    if (m_vrVolumeApp)
    {
      glm::mat4 new_pose = glm::make_mat4(event.getTransform());
      m_vrVolumeApp->update_UI_pose_controller(new_pose);
    }
  }

  // This routine is called for all Tracker_Move events.  Check event->getName()
  // to see exactly which tracker has moved, and then access the tracker's new
  // 4x4 transformation matrix with event->getTransform().
  if (event.getName() == "HTC_Controller_Right_Move" || event.getName() == "HTC_Controller_1_Move" || event.getName() == "Wand0_Move")
  {
    glm::mat4 new_pose = glm::make_mat4(event.getTransform());

    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->do_grab(new_pose);
    }
  }
  else if (event.getName() == "HTC_HMD_1_Move")
  {
    glm::mat4 headPose = glm::make_mat4(event.getTransform());
    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->update_head_pose(headPose);
    }
    // m_headpose = glm::inverse(m_headpose);
    m_light_pos[0] = headPose[3][0];
    m_light_pos[1] = headPose[3][1];
    m_light_pos[2] = headPose[3][2];
  }
}

void VolumeVisualizationApp::onGenericEvent(const VRDataIndex &index)
{
  if (index.getName() == "WindowClose")
  {
    shutdown();
  }

  if (index.getName() == "BufferSize")
  {
    const std::vector<float> *v = index.getValue("WindowSize");
    const float *f = &(v->front());
    glViewport(0, 0, GLint(f[0]), GLint(f[1]));
  }
}

void VolumeVisualizationApp::onRenderGraphicsContext(const VRGraphicsState &renderState)
{
  // This routine is called once per graphics context at the start of the
  // rendering process.  So, this is the place to initialize textures,
  // load models, or do other operations that you only want to do once per
  // frame when in stereo mode.
  m_num_frames++;

  std::chrono::steady_clock::time_point nowTime = std::chrono::steady_clock::now();
  std::chrono::duration<double> delta_time = std::chrono::duration<double>(nowTime - m_lastTime);
  float delta_time_to_milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(delta_time).count();
  float fps = 1000.0f / delta_time_to_milliseconds;

  if (m_vrVolumeApp)
  {
    m_vrVolumeApp->update_fps(fps);
    m_vrVolumeApp->update_dynamic_slices();
  }
  m_lastTime = nowTime;

  if (renderState.isInitialRenderCall())
  {

#ifndef __APPLE__
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
      std::cout << "Error initializing GLEW." << std::endl;
    }
#endif
    std::cout << "init vizapp " << std::endl;
    if (m_vrVolumeApp)
    {
      m_vrVolumeApp->initialize();
      m_vrVolumeApp->intialize_ui();
      m_vrVolumeApp->load_shaders();
    }
    std::cout << "init vizapp 2" << std::endl;
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, glm::value_ptr(m_vrVolumeApp->get_ambient()));
    glLightfv(GL_LIGHT0, GL_DIFFUSE, glm::value_ptr(m_vrVolumeApp->get_diffuse()));
    glLightfv(GL_LIGHT0, GL_SPECULAR, glm::value_ptr(m_vrVolumeApp->get_no_color()));
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, true);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_DEPTH_TEST);
    glClearDepth(1.0f);
    glDepthFunc(GL_LEQUAL);
    glClearColor(0.0, 0.0, 0.0, 1);
    std::cout << "init vizapp end" << std::endl;
    const float duration = 10.5f;
  }

  if (m_vrVolumeApp)
  {
    if (m_vrVolumeApp->pending_models_to_load())
    {
      m_vrVolumeApp->load_mesh_model();
    }

    m_vrVolumeApp->initialize_textures();
    m_vrVolumeApp->update_3D_ui();
    m_vrVolumeApp->update_trackBall_state();

    if (true)
    {
      m_vrVolumeApp->update_animation(delta_time.count());
      last_Update_Time = nowTime;
    }

    m_vrVolumeApp->set_render_count(0);
  }

  // std::cout << "init vizapp loop" << std::endl;
}

void VolumeVisualizationApp::onRenderGraphicsScene(const VRGraphicsState &renderState)
{
  // This routine is called once per eye.  This is the place to actually
  // draw the scene...

  if (m_vrVolumeApp)
  {
    m_vrVolumeApp->render(renderState);
  }
}
