#ifndef VRVOLUMEAPP_H
#define VRVOLUMEAPP_H

#include <string>
#include <vector>
#include <future>
#include <api/MinVR.h>
#include "interaction/Labels.h"
#include "interaction/ArcBallCamera.h"
#include "render/Volume.h"

#include "Model.h"


class UIView;
class CreateMovieAction;
class VolumeRenderer;
class DepthTexture;
class Window_Properties;

class VRVolumeApp
{
public:

  VRVolumeApp();
  ~VRVolumeApp();



  void render(const MinVR::VRGraphicsState& renderState);

  void initialize();

  void load_volume(std::vector<std::string> vals, std::promise<Volume*>* promise);

  void load_nrrd_file(std::string& filename);

  void update_frame_state();

  void update_3D_ui();
  void update_2D_ui();

  void clear_data();
  int get_num_volumes();
  bool data_is_multi_channel();
  void get_min_max(const float frame, float& min, float& max);



  void set_is_2D(bool);
  void set_looking_glass(bool);
  void set_convert(bool);
  void set_num_volumes(int);

  void mouse_pos_event(glm::vec2& mPos);
  void update_ui_events(float value);
  void update_track_ball_event(float value);
  void button_events_ui_handle(int button, int state);
  void button_event_trackBall_handle(int button, int state);

  void enable_grab(bool);
  void enable_clipping(bool);

  void enable_ui_menu();

  void set_AWSD_keyBoard_event(int key);
  void unset_AWSD_keyBoard_event(int key);

  void enable_render_volume();

  void update_UI_pose_controller(glm::mat4& newPose);
  void update_head_pose(glm::mat4& newPose);

  void update_fps(float fps);
  float get_fps();

  void update_dynamic_slices();

  void do_grab(glm::mat4& pose);

  void intialize_ui();

  void load_mesh_model();

  void load_shaders();

  void initialize_textures();

  bool pending_models_to_load();

  void update_trackBall_state();

  void update_animation(float fps);

#if (!defined(__APPLE__))
 void run_movie();
#endif
 

  void set_render_count(unsigned int);

  float get_current_frame();

  void set_frame(float frame);

  glm::vec4& get_no_color();
  glm::vec4& get_ambient();
  glm::vec4& get_diffuse();

  void set_multi_transfer(bool);
  bool is_multi_transfer();

  bool is_ui_event();

  bool is_show_menu();

  void set_is_animated(bool);

  void set_threshold(float);

  void add_label(std::string& text, float x, float y, float z, float textPosZ, float size, int volume);

  void set_description(int, std::string& text);

  void set_mesh(int volumeId, std::string& fileName, std::string& shaderFilePath);

  void set_texture(std::string& fileNamePath);

  void init_num_volumes(int);

  void add_data_label(std::string&);

  std::vector<std::promise<Volume*>*>& get_promise(int index);

  std::vector<std::future<Volume*>>* get_future(int index);

  void set_future(int index, std::vector<std::future<Volume*>>*);

  std::vector <std::thread*>& get_thread(int index);

  void init_volume_loading(int index, std::vector<std::string>);

  void set_character_state(std::string& key, int state);
  
  void set_directory_path(std::string& dir_path);

  std::string& get_directory_path();

  void set_loaded_file(std::string& dir_path);

  std::string& get_loaded_file();

  std::vector< Volume* >& get_volume(int volume);

  ArcBallCamera& get_trackball_camera();

  void set_animation_speed(float time);

protected:

  glm::vec4 m_noColor;// (0.0f, 0.0f, 0.0f, 0.0f);
  glm::vec4 m_ambient;// [] = { 0.2f, 0.2f, 0.2f, 1.0f };
  glm::vec4 m_diffuse; //= { 0.5f, 0.5f, 0.5f, 1.0f };


  void add_lodaded_textures();

  void initialize_GL();



  void render_labels(const MinVR::VRGraphicsState& renderState);
  void render_mesh(const MinVR::VRGraphicsState& renderState);
  void render_volume(const MinVR::VRGraphicsState& renderState);
  void render_ui(const MinVR::VRGraphicsState& renderState);

  void animated_render(int, int);
  void normal_render_volume(int, int);

  

  std::vector < std::vector< Volume* >> m_volumes;
  std::vector<std::string> m_description;
  Labels m_labels;
  std::vector <std::string> m_models_filenames;
  std::vector <unsigned int> m_models_displayLists;
  std::vector<pt> m_models_position;
  std::vector<int> m_models_volumeID;
  std::vector<glm::mat4> m_models_MV;

  GLfloat m_light_pos[4];


  std::vector <VolumeRenderer*> m_renders;
  std::vector <DepthTexture*> m_depthTextures;

  std::vector< std::vector<std::promise<Volume*>*>> m_promises;
  std::vector< std::vector <std::future<Volume*>>*> m_futures;
  std::vector< std::vector <std::thread*>> m_threads;

  Model* m_mesh_model;
  ShaderProgram m_simple_texture_shader;
  std::string m_shader_file_path;
  std::string m_texture_filePath;
  Texture* m_texture;

  bool m_isInitailized;
  bool m_animated;
  float m_threshold;
  int m_descriptionHeight;
  float m_animation_speed;
  
  float m_frame;
  float m_speed;
  unsigned int m_rendercount;
  int m_selectedVolume;

  std::string m_descriptionFilename;

  size_t m_numVolumes;
  std::vector<std::string> m_data_labels;


  UIView* m_ui_view;


  bool m_convert;
  bool m_is2d;
  glm::mat4 m_headpose;


  glm::mat4 m_projection_mtrx;
  glm::mat4 m_model_view;


  ArcBallCamera m_trackball;
  bool m_lookingGlass;

  glm::mat4 m_object_pose;


  Model* mesh_model;

  bool m_clipping;
  bool m_use_custom_clip_plane;
  glm::mat4 m_controller_pose;

  glm::vec3 m_clip_ypr;
  glm::vec3 m_clip_pos;
  glm::vec3 m_clip_min;
  glm::vec3 m_clip_max;


  float m_multiplier;
  int m_slices;

  bool m_use_multi_transfer;
  bool m_show_menu;



  /*Input Events*/
  bool m_grab;
  int m_wasd_pressed;


  float m_fps;

  CreateMovieAction* m_movieAction;
  std::string m_moviename;

   glm::vec2 m_window_size ;
   glm::vec2 m_frame_buffer_size;
  std::string m_directiort_path;

  std::string m_current_file_loaded;

  Window_Properties* m_window_properties;
};
#endif
