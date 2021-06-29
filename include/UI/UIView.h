#ifndef UIVIEW_H
#define UIVIEW_H

#include "VRMenuHandler.h"
#if (!defined(__APPLE__))
#include "imfilebrowser.h"
#endif
#include "ImGuiFileBrowser.h"
#include "transferfunction/transfer_function_multichannel_widget.h"
#include "transferfunction/transfer_function_widget.h"

#include <fstream>

class VRVolumeApp;
class CreateMovieAction;
#define INPUT_TEXT_SIZE 200
#define MAX_COLUMS 50

enum SAVE_MODAL {SAVE_NONE, SAVE_SESSION, SAVE_TRFR_FNC};
enum LOAD_MODAL {LOAD_NONE, LOAD_SESSION, LOAD_TRFR_FNC };

class UIView
{
public:
  UIView(VRVolumeApp& controllerApp);
  ~UIView();
  

  void draw_ui_callback();
  void init_ui(bool is2D, bool lookingGlass);
  void update_ui(int numVolumes);
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

  void update_animation(float speed, int numFrames);

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

  void set_chracter(char c);

  void remove_character();

private:

  struct MyTransFerFunctions
  {
    int         ID;
    std::string Name;
    std::vector<bool> volumes;
  };

  void open_save_modal_dialog(std::string& id, bool& window_state, 
    std::function<void(std::ofstream&)> save_function,  std::string& extension);

  void add_trans_function();

  void save_trans_functions(std::ofstream& saveFile);

  void save_user_session(std::ofstream& saveFile);

  void load_trans_functions(std::ifstream& loadPath);

  void load_user_session(std::string filePath);

  VRVolumeApp& m_controller_app;
  VRMenuHandler* m_menu_handler;
  imgui_addons::ImGuiFileBrowser fileDialog;
  bool m_file_dialog_open;
  bool m_file_dialog_save_dir;
  std::string m_dir_to_save;
  imgui_addons::ImGuiFileBrowser fileDialogLoadTrnsFnc;
  imgui_addons::ImGuiFileBrowser saveDialogLoadTrnsFnc;
  bool m_file_load_trnsf;

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
  std::vector<MyTransFerFunctions> m_tfns;
  std::vector<std::vector<bool>> m_selected_volume_TrFn;

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

  int m_table_selection;

  bool m_initialized;

  bool m_trn_fct_name_open;

  bool m_save_trnfct_open;  

  bool m_save_session_dialog_open;

  SAVE_MODAL m_current_save_modal;

  LOAD_MODAL m_current_load_modal;

  std::string m_file_extension_filter;

  std::string m_copy_trnfnct_name;

  std::string m_save_file_name;

  unsigned int m_trnfnct_counter;

  bool m_non_trns_functions_selected_modal;

  bool m_ui_background;

  
  
  int m_column_selected = 0;
  
};

#endif