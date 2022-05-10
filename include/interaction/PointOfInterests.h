#ifndef POINTOFINTEREST_H
#define POINTOFINTEREST_H

#include <glm/glm.hpp>
#include <string>

class PointOfInterest {

public :

  PointOfInterest():eye(glm::vec3(0.0f, 0.0f, 1.0f)), target(glm::vec3(0.0f, 0.0f, 0.0f)), up(glm::vec3(0.0f, 1.0f, 0.0f)), radius(1.f)
  {
  }
  
  PointOfInterest(glm::vec3 p_eye, glm::vec3 p_target, glm::vec3 p_up, float p_radius):eye(p_eye),target(p_target),up(p_up),radius(p_radius)
  {
    
  }

  PointOfInterest(const PointOfInterest& other)
  {
    eye = other.eye;
    radius = other.radius;
    target =other.target;
    up = other.up;
  };

  //The orbit camera is constrained to move around a centered point ("target"). 
  //The camera position is calculated as a linear combination between the center object 
  //and the radius of the virtual sphere that surrounds it. 
  //This is not equal to the view matrix eye position determined by the homogeneous coordinate (4th column)
  
  glm::vec3 get_camera_position()
  {
    eye = (glm::normalize(eye));
    return radius * eye + target;
  }

  glm::vec3 eye;
  glm::vec3 target;
  glm::vec3 up;
  float radius;
  std::string label;

  
};

#endif 