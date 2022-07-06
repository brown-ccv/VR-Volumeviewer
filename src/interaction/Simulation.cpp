#include "../include/interaction/Simulation.h"
#include "../include/vrapp/VRVolumeApp.h"

Simulation::Simulation(VRVolumeApp &controller_app, float time) : m_simulation_duration(time), m_controller_app(controller_app),
                                                                  animation_button_label("ANIMATE"), m_animation_state(STOP)
{
}

void Simulation::add_simulation_state(SimulationState &simulationState)
{
  float time = m_simulation_states.size() * m_simulation_duration;

  int mins = (int)time / 60;
  int seconds = (int)time % 60;

  simulationState.time_label = std::to_string(mins) + ":" + std::to_string(seconds);
  m_simulation_states.push_back(simulationState);
}

void Simulation::create_simulation_time_frames()
{
  if (m_simulation_states.size() > 1)
  {
    SimulationState first_position = m_simulation_states.front();
    ch::Sequence<glm::vec3> sequence_eye(first_position.poi.eye);
    ch::Sequence<glm::vec3> sequence_target(first_position.poi.target);
    ch::Sequence<glm::vec3> sequence_up(first_position.poi.up);
    ch::Sequence<float> sequence_radius(first_position.poi.radius);
    ch::Sequence<glm::vec3> sequence_clip_max(first_position.max_clip);
    ch::Sequence<glm::vec3> sequence_clip_min(first_position.min_clip);

    for (auto iterator = std::next(m_simulation_states.begin()); iterator != m_simulation_states.end(); ++iterator)
    {
      sequence_eye.then<ch::RampTo>(iterator->poi.eye, m_simulation_duration);
      sequence_target.then<ch::RampTo>(iterator->poi.target, m_simulation_duration);
      sequence_up.then<ch::RampTo>(iterator->poi.up, m_simulation_duration);
      sequence_radius.then<ch::RampTo>(iterator->poi.radius, m_simulation_duration);
      sequence_clip_max.then<ch::RampTo>(iterator->max_clip, m_simulation_duration);
      sequence_clip_min.then<ch::RampTo>(iterator->min_clip, m_simulation_duration);
    }

    auto group = std::make_shared<ch::Timeline>();
    group->apply<glm::vec3>(&m_eye_animation, sequence_eye);
    group->apply<glm::vec3>(&m_target_animation, sequence_target);
    group->apply<glm::vec3>(&m_up_animation, sequence_up);
    group->apply<float>(&m_radius_animation, sequence_radius);
    group->apply<glm::vec3>(&m_max_clip_animation, sequence_clip_max);
    group->apply<glm::vec3>(&m_min_clip_animation, sequence_clip_min);

    m_timeline.addShared(group);
  }
}

void Simulation::update_simulation()
{
  if (m_animation_state != PAUSE)
  {
    update_time_step();
  }
  if (m_timeline.isFinished())
  {
    m_animation_state = STOP;
    animation_button_label = "Animate";
    m_controller_app.set_app_mode(MANUAL);
    m_timeline.resetTime();
#if (!defined(__APPLE__))
    if (m_controller_app.get_movie_state() == MOVIE_RECORD)
    {
      m_controller_app.stop_movie();
    }
#endif
  }
}

void Simulation::update_time_step()
{
  m_timeline.step(1.0 / SIMULATION_TIME_STEP);
}

void Simulation::set_animation_state()
{
  if (m_animation_state == STOP)
  {
    create_simulation_time_frames();
    m_controller_app.set_app_mode(SIMULATION);
    m_animation_state = PLAYING;
    animation_button_label = "PAUSE";
  }
  else if (m_animation_state == PLAYING)
  {
    if (!m_timeline.isFinished())
    {
      m_animation_state = PAUSE;
      animation_button_label = "ANIMATE";
    }
  }
  else if (m_animation_state == PAUSE)
  {
    m_animation_state = PLAYING;
    animation_button_label = "PAUSE";
  }

  m_is_animate_path = true;
}

ANIMATION_STATE Simulation::get_animation_state()
{
  switch (m_timeline.isFinished())
  {
  case true:
    return STOP;
  case false:
  {
    if (m_animation_state == STOP)
    {
      return PAUSE;
    }
    else
    {
      return PLAYING;
    }
  }
  default:
    return STOP;
  }
}

std::string Simulation::get_camera_animation_state()
{
  return animation_button_label;
}

float Simulation::get_simulation_duration()
{
  return m_simulation_duration;
}

void Simulation::set_simulation_duration(float duration)
{
  m_simulation_duration = duration;
}

const std::list<SimulationState> &Simulation::get_simulation_states()
{
  return m_simulation_states;
}

SimulationState &Simulation::get_simulation_state_at(unsigned int index)
{
  auto poi_iterator = m_simulation_states.begin();
  std::advance(poi_iterator, index);
  return *poi_iterator;
}

SimulationState Simulation::get_current_simulation_state()
{
  PointOfInterest poi(m_eye_animation.value(), m_target_animation.value(), m_up_animation.value(), m_radius_animation.value());

  SimulationState current_state;
  current_state.poi = poi;
  current_state.max_clip = m_max_clip_animation;
  current_state.min_clip = m_min_clip_animation;

  return current_state;
}

void Simulation::remove_simulation_state(unsigned int index)
{
  auto poi_iterator = m_simulation_states.begin();
  std::advance(poi_iterator, index);
  std::string time_label = poi_iterator->time_label;
  poi_iterator = m_simulation_states.erase(poi_iterator);
  for (poi_iterator; poi_iterator != m_simulation_states.end(); poi_iterator++)
  {
    std::string current_time_label = poi_iterator->time_label;
    poi_iterator->time_label = time_label;
    time_label = current_time_label;
  }
}

float Simulation::get_animation_duration()
{
  return m_simulation_duration;
}
