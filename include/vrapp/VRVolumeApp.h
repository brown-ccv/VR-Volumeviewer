#pragma once
#include <string>
#include <vector>
#include <future>
#include <api/MinVR.h>
#include "interaction/Labels.h"
#include "interaction/ArcBall.h"
#include "render/Volume.h"

#include "Model.h"


class UIView;
class CreateMovieAction;
class VolumeRenderer;
class DepthTexture;

class VRVolumeApp
{
public:

  VRVolumeApp();
  
  
  void render(const MinVR::VRGraphicsState& renderState);

  void initialize();

  void loadTxtFile(std::string& filename);

  void loadVolume(std::vector<std::string> vals, std::promise<Volume*>* promise);

  void loadNrrdFile(std::string& filename);

  void updateFrameState();

  void update3DUI();
  void update2DUI();

  virtual void clearData() ;
  virtual int getNumVolumes() ;
  virtual bool dataIsMultiChannel() ;
  virtual void getMinMax(const float frame,float & min, float& max) ;

  

  void setIs2D(bool);
  void setLookingGlass(bool);
  void setConvert(bool);
  void setNumVolumes(int);

  virtual void loadTextFile(std::string& filename);

  void mousePosEvent(glm::vec2& mPos);
  void updateUIEvents(float value);
  void updateTrackBallEvent(float value);
  void buttonEventsUIHandle(int button, int state);
  void buttonEventTrackBallHandle(int button, int state);
  
  void enableGrab(bool );
  void enableClipping(bool);
  
  void enableUIMenu( );

  void setAWSDKeyBoardEvent(int key);
  void unsetAWSDKeyBoardEvent(int key);

  void enableRenderVolume();

  void updateUIPoseController(glm::mat4& newPose);
  void updateHeadPose(glm::mat4& newPose);
  
  void updateFps(float fps);
  float getFps();

  void updateDynamicSlices();

  void doGrab(glm::mat4& pose);
  
  void intializeUI();

  void loadMeshModel();

  void loadShaders();

  void initializeTextures();

  bool pendingModelsToLoad();

  void updateTrackBallState();

  void updateAnimation();

  void runMovie();

  void setRendercount(unsigned int);

  glm::vec4& getNoColor();
  glm::vec4& getAmbient();
  glm::vec4& getDiffuse();

protected:

  glm::vec4 m_noColor;// (0.0f, 0.0f, 0.0f, 0.0f);
  glm::vec4 m_ambient;// [] = { 0.2f, 0.2f, 0.2f, 1.0f };
  glm::vec4 m_diffuse; //= { 0.5f, 0.5f, 0.5f, 1.0f };


  void addLodadedTextures();

  virtual void initializeGL();
 
 

  virtual void renderLabels(const MinVR::VRGraphicsState& renderState);
  virtual void renderMesh(const MinVR::VRGraphicsState& renderState);
  virtual void renderVolume(const MinVR::VRGraphicsState& renderState);
  virtual void renderUI(const MinVR::VRGraphicsState& renderState);

  virtual void animatedRender(int,int);
  virtual void normalRenderVolume(int, int);

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
  float m_stopped;
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


  ArcBall m_trackball;
  bool m_lookingGlass;

  glm::mat4 m_object_pose;
  float m_scale;
  float m_z_scale;

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
};

