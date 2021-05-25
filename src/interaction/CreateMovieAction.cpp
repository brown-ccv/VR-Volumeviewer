//  ----------------------------------
//  Copyright © 2015, Brown University, Providence, RI.
//  
//  All Rights Reserved
//   
//  Use of the software is provided under the terms of the GNU General Public License version 3 
//  as published by the Free Software Foundation at http://www.gnu.org/licenses/gpl-3.0.html, provided 
//  that this copyright notice appear in all copies and that the name of Brown University not be used in 
//  advertising or publicity pertaining to the use or distribution of the software without specific written 
//  prior permission from Brown University.
//  
//  See license.txt for further information.
//  
//  BROWN UNIVERSITY DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE WHICH IS 
//  PROVIDED “AS IS”, INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS 
//  FOR ANY PARTICULAR PURPOSE.  IN NO EVENT SHALL BROWN UNIVERSITY BE LIABLE FOR ANY 
//  SPECIAL, INDIRECT OR CONSEQUENTIAL DAMAGES OR FOR ANY DAMAGES WHATSOEVER RESULTING 
//  FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR 
//  OTHER TORTIOUS ACTION, OR ANY OTHER LEGAL THEORY, ARISING OUT OF OR IN CONNECTION 
//  WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. 
//  ----------------------------------
//  
///\file LoadDataAction.cpp
///\author Benjamin Knorlein
///\date 12/16/2020

#pragma once

#include "../../include/interaction/CreateMovieAction.h"

#include <opencv2/highgui/highgui.hpp>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#include <iomanip> // std::setw 
#endif
#include <iostream>




CreateMovieAction::CreateMovieAction()
{
#ifndef _MSC_VER
  m_frame = 0;
#endif
}

CreateMovieAction::~CreateMovieAction()
{
  clear();
}

void CreateMovieAction::save(std::string filename) {
  if (m_frames.empty())
    return; m_frames[0].cols;

  cv::VideoWriter outputVideo;
  outputVideo.open(filename, outputVideo.fourcc('m', 'p', '4', 'v'), 30, cv::Size(m_frames[0].cols, m_frames[0].rows), true);
  if (outputVideo.isOpened())
  {
    for (auto& frame : m_frames) {
      cv::flip(frame, frame, 0);
      outputVideo << frame;
    }
    outputVideo.release();
  }
}

void CreateMovieAction::clear() {
  for (auto& frame : m_frames) {
    delete[] frame.data;
    frame.release();
  }
  m_frames.clear();
}

void CreateMovieAction::addFrame() {
  GLint dims[4] = { 0 };
  glGetIntegerv(GL_VIEWPORT, dims);
#ifdef _MSC_VER
  glReadBuffer(GL_FRONT);
#else
  glReadBuffer(GL_BACK);
#endif
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  unsigned char* pixels = new unsigned char[3 * dims[2] * dims[3]];
  glReadPixels(0, 0, dims[2], dims[3], GL_BGR_EXT, GL_UNSIGNED_BYTE, pixels);
#ifdef _MSC_VER
  m_frames.push_back(cv::Mat(dims[3], dims[2], CV_8UC3, pixels));
#else
  std::stringstream ss;
  ss << std::setw(10) << std::setfill('0') << m_frame;
  std::string s = ss.str();
  cv::flip(m_frames.back(), m_frames.back(), 0);
  cv::imwrite("movie//" + s + ".jpg", m_frames.back());
  m_frame++;
#endif



}
