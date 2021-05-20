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
  void drawUICB();
  void initUI( bool is2D, bool lookingGlass);
  void updateUI(int numVolumes );
  void render2D();
  void render3D(glm::mat4& space_matrix);
  void update3DUIFrame();

  void setCursorPos(glm::vec2&);
  void setAnalogValue(float);

  int getNumTransferFunctions();
  bool isTransferFunctionEnabled(int, int);
  int getRenderMethod();
  bool isRenderVolumeEnabled();
  bool isUseTransferFunctionEnabled();

  GLint getTransferFunctionColormap(int trnsF);
  GLint getMTransferFunctionColormap(int trnsF);

  int getRenderChannel();
  void setButtonClick(int, int);

  void setEnableRenderVolume();

  void setControllerPose(glm::mat4& pose);

  void setDynamicSlices(bool);
  bool isDynamicSlices();

  void updateSlices(float fps);

  bool isAnimated();

  bool isStopped();

  void updateAnimation(float speed, int numFrames );

  void addDataLabel(std::string& dataLabel);

  void clearDataLabels();

private:


  VRVolumeApp& m_controller_app;
  VRMenuHandler* m_menu_handler;
  ImGui::FileBrowser fileDialog;
  
  float m_multiplier;
  float m_threshold;
  int m_rendermethod;
  int m_renderchannel;
  bool m_use_transferfunction;
  bool m_use_multi_transfer;
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
  float m_frame;
  float m_stopped;

  glm::vec3 m_clip_min;
  glm::vec3 m_clip_max;

  bool m_useCustomClipPlane;
  glm::vec3 m_clip_ypr;
  glm::vec3 m_clip_pos;

  

  bool m_initialized;
};

