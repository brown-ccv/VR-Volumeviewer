#pragma once
#include "VRMenuHandler.h"
#include "imfilebrowser.h"
#include "transferfunction/transfer_function_multichannel_widget.h"
#include "transferfunction/transfer_function_widget.h"

class VRVolumeApp;
class CreateMovieAction;

class UIView
{
public:
  UIView(VRVolumeApp& controllerApp);
  void draw_ui_callback();
  void init_ui( bool is2D, bool lookingGlass);
  void update_ui(int numVolumes );
  void render_2D();
  void render_3D(glm::mat4& space_matrix);
  void update_3D_ui_frame();

  void set_cursor_pos(glm::vec2&);
  void set_analog_value(float);

  int get_num_transfer_functions();
  bool is_transfer_function_enabled(int, int);
  int get_render_method();
  bool is_render_volume_enabled();
  bool is_use_transfer_function_enabled();

  GLint get_transfer_function_colormap(int trnsF);
  GLint get_multitransfer_function_colormap(int trnsF);

  int get_render_channel();
  void set_button_click(int, int);

  void set_enable_render_volume();

  void set_controller_pose(glm::mat4& pose);

  void set_dynamic_slices(bool);
  bool is_dynamic_slices();

  void update_slices(float fps);

  bool is_animated();

  bool is_stopped();

  void update_animation(float speed, int numFrames );

  void add_data_label(std::string& dataLabel);

  void clear_data_labels();

  float get_z_scale();

  float get_scale();

  float get_slices();

  float get_threshold();

  float get_multiplier();

  bool is_ui_window_active();

  bool is_use_custom_clip_plane();

  glm::vec3 get_clip_min();
  glm::vec3 get_clip_max();

private:


  VRVolumeApp& m_controller_app;
  VRMenuHandler* m_menu_handler;
  ImGui::FileBrowser fileDialog;
  
  float m_multiplier;
  float m_threshold;
  int m_rendermethod;
  int m_renderchannel;
  bool m_use_transferfunction;

  float m_z_scale;
  float m_scale;
  int m_slices;
  bool m_dynamic_slices;
  
  bool m_show_menu;
  bool m_renderVolume;
  std::vector<TransferFunctionMultiChannelWidget> tfn_widget_multi;
  std::vector<TransferFunctionWidget> tfn_widget;
  std::vector<std::vector<bool>> m_selectedTrFn;

  std::vector<std::string> m_dataLabels;
  int m_selectedTrnFnc;

  bool m_animated;
  float m_ui_frame_controller;
  float m_stopped;

  glm::vec3 m_clip_min;
  glm::vec3 m_clip_max;

  bool m_useCustomClipPlane;
  glm::vec3 m_clip_ypr;
  glm::vec3 m_clip_pos;

  

  bool m_initialized;
};

