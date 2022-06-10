#ifndef UIVIEW_H
#define UIVIEW_H

#include "VRMenuHandler.h"
#if (!defined(__APPLE__))
#include "imfilebrowser.h"
#endif
#include "ImGuiFileBrowser.h"
#include "UIHelpers/transfer_function_multichannel_widget.h"
#include "UIHelpers/transfer_function_widget.h"
#include "UIHelpers/histogram.h"
#include <stdint.h>
#include <fstream>

class VRVolumeApp;
class CreateMovieAction;
#define INPUT_TEXT_SIZE 200
#define MAX_COLUMS 50

enum SAVE_MODAL
{
  SAVE_NONE,
  SAVE_SESSION,
  SAVE_TRANSFER_FUNCTION
};
enum LOAD_MODAL
{
  LOAD_NONE,
  LOAD_SESSION,
  LOAD_TRFR_FNC
};
enum BUTTON_ACTION
{
  NONE,
  ADD,
  EDIT,
  REMOVE
};

struct Window_Properties
{

  int window_w = 0;
  int window_h = 0;
  int framebuffer_w = 0;
  int framebuffer_h = 0;

  bool operator==(Window_Properties &other)
  {
    if (other.window_w == window_w &&
        other.window_h == window_h &&
        other.framebuffer_w == framebuffer_w &&
        other.framebuffer_h == framebuffer_h)
    {
      return true;
    }
    return false;
  }
};

class UIView
{
public:
  UIView(VRVolumeApp &controllerApp);
  ~UIView();

  void draw_ui_callback();
  void init_ui(bool is2D, bool lookingGlass);
  void update_ui(int numVolumes);
  void render_2D(Window_Properties &window_properties);
  void render_3D(glm::mat4 &space_matrix, Window_Properties &window_properties);
  void update_3D_ui_frame();

  void set_cursor_pos(glm::vec2 &);
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

  void set_controller_pose(glm::mat4 &pose);

  void set_dynamic_slices(bool);
  bool is_dynamic_slices();

  void update_slices(float fps);

  bool is_animated();

  void set_is_animated(bool animated);

  bool is_stopped();

  void update_animation(float speed, int numFrames);

  void add_data_label(std::string &dataLabel);

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

  void set_clip_min(glm::vec3 clip_min);
  void set_clip_max(glm::vec3 clip_max);

  void add_character(char c);

  void remove_character();

  void compute_new_histogram_view();

  void set_animation_length(int num_frames);

  void get_quantiles(int row);

  void set_volume_time_info(time_t time);

  void draw_transfer_function_legend();

  void set_transfer_function_min_max(float min, float max);

  bool get_show_movie_saved_pop_up() const { return m_show_movie_saved_pop_up; }

  void set_show_movie_saved_pop_up(bool val) { m_show_movie_saved_pop_up = val; }

private:
  struct MyTransFerFunctions
  {
    int ID;
    std::string Name;
    std::vector<bool> volumes;
  };

  void open_save_modal_dialog(std::string &id, bool &window_state,
                              std::function<void(std::ofstream &)> save_function, std::string &extension);

  void add_transfer_function();

  void save_transfer_functions(std::ofstream &saveFile);

  void save_user_session(std::ofstream &saveFile);

  void load_transfer_functions(std::ifstream &loadPath);

  void load_user_session(std::string filePath);

  void save_simulation_states(std::ofstream &loadPath, int num_poi);

  void load_camera_poi(std::ifstream &loadPath, int num_poi);

  void read_file_line(std::string &line, std::vector<std::string> &values);

  void load_ocean_color_maps();

  void adjust_transfer_function_to_histogram();

  VRVolumeApp &m_controller_app;
  VRMenuHandler *m_menu_handler;
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
  unsigned int m_num_animation_frames;
  float m_stopped;

  glm::vec3 m_clip_min;
  glm::vec3 m_clip_max;

  bool m_useCustomClipPlane;
  glm::vec3 m_clip_ypr;
  glm::vec3 m_clip_pos;

  int m_trnfnc_table_selection;

  int m_camera_poi_table_selection;

  bool m_initialized;

  bool m_transfer_function_options_window;

  bool m_save_transfer_function_open;

  bool m_save_session_dialog_open;

  SAVE_MODAL m_current_save_modal;

  LOAD_MODAL m_current_load_modal;

  std::string m_file_extension_filter;

  std::string m_copy_trnfnct_name;

  std::string m_copy_camera_name;

  bool m_camera_name_window_open;

  BUTTON_ACTION m_camera_button_action;

  std::string m_save_file_name;

  unsigned int m_trnfnct_counter;

  bool m_non_trns_functions_selected_modal;

  bool m_ui_background;

  bool m_column_selected[MAX_COLUMS];

  unsigned int m_column_selection_state;

  bool m_compute_new_histogram;

  Histogram m_histogram;

  vec2f m_histogram_point_1;
  vec2f m_histogram_point_2;
  float m_histogram_quantiles[2];

  std::vector<std::string> m_ocean_color_maps_names;

  std::string m_color_map_directory;

  float m_animation_step;
  std::string m_string_animation_duration;

  bool m_camera_animation_duration_open;

  std::vector<float> m_clip_maxs;
  std::vector<float> m_clip_mins;

  bool m_show_clock;
  float m_clock_pos_x;
  float m_clock_pos_y;
  float m_clock_width;
  float m_clock_height;

  float m_legend_pos_y;

  std::string m_time_info;
  std::string m_day_info;

  std::string m_months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

  int m_simulation_state_selection;
  bool m_time_frame_edited;

  bool m_show_movie_saved_pop_up;
};

#endif
