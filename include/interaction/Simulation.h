#ifndef SIMULATION_H
#define SIMULATION_H

#include "choreograph/Choreograph.h"

#include <glm/glm.hpp>

#include "PointOfInterests.h"
#include <list>

enum ANIMATION_STATE
{
  STOP,
  PLAYING,
  PAUSE
};

/*
  SimulationState represents a frame in the simulation/animation
  where the camera is set on a postion P, pointing on direction of a target T,
  and min/max are set on a X-Y value.
  the animation change SimulationStates by interpolating between the mentioned values.
*/

#define SIMULATION_TIME_STEP 30.0

struct SimulationState
{
  SimulationState(){};

  SimulationState(const SimulationState &other)
  {
    poi = other.poi;
    max_clip = other.max_clip;
    min_clip = other.min_clip;
  }

  std::string time_label;
  PointOfInterest poi;
  glm::vec3 max_clip;
  glm::vec3 min_clip;
};

class VRVolumeApp;

class Simulation
{
public:
  Simulation(VRVolumeApp &controller_app, float time = 10.0f);

  void add_simulation_state(SimulationState &simulationState);

  void create_simulation_time_frames();
  void update_simulation();

  void set_animation_state();

  ANIMATION_STATE get_animation_state();
  std::string get_camera_animation_state();
  float get_simulation_duration();
  void set_simulation_duration(float duration);

  const std::list<SimulationState> &get_simulation_states();

  SimulationState &get_simulation_state_at(unsigned int index);

  SimulationState get_current_simulation_state();

  void remove_simulation_state(unsigned int index);

  float get_animation_duration();

private:
  void update_time_step();

  ch::Timeline m_timeline;
  float m_simulation_duration;

  std::list<SimulationState> m_simulation_states;

  std::vector<std::vector<glm::vec3>> m_vec3_sequences;
  std::vector<std::vector<float>> m_float_sequences;

  ch::Output<glm::vec3> m_target_animation;
  ch::Output<glm::vec3> m_eye_animation;
  ch::Output<glm::vec3> m_up_animation;
  ch::Output<float> m_radius_animation;
  ch::Output<glm::vec3> m_max_clip_animation;
  ch::Output<glm::vec3> m_min_clip_animation;

  bool m_is_animate_path;

  ANIMATION_STATE m_animation_state;
  std::string animation_button_label;

  VRVolumeApp &m_controller_app;
};

#endif