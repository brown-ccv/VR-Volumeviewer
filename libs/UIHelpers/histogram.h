#pragma once
#include "GL/glew.h"
#include <cstdint>
#include <string>
#include <vector>
#include "imgui/imgui.h"


class Histogram {
  


	std::vector<float> current_histogram;
	float m_min_max_val[2];

public:

  
  
  
  Histogram();

    
   

	void setHistogram(const std::vector<float> &hist);

  std::vector<float>& getHistogram();

	void setMinMax(const float min, const float max);

	void setBlendedHistogram(const std::vector<float>& hist1, const std::vector<float>& hist2, float alpha);

  void draw_histogram();


private:


   
};

