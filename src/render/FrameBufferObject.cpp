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
///\file FrameBufferObject.cpp
///\author Benjamin Knorlein
///\date 3/23/2019

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "render/FrameBufferObject.h"
#ifdef _WIN32
#include "GL/glew.h"
#include "GL/wglew.h"
#elif (!defined(__APPLE__))
#include "GL/glxew.h"
#endif

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <GL/gl.h>
#include <gl/GLU.h>
#define M_PI 3.14159265358979323846
#elif defined(__APPLE__)
#include <OpenGL/OpenGL.h>
#include <OpenGL/glu.h>
#else
#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#endif
#include <iostream>


FrameBufferObject::FrameBufferObject() :m_isInitialized{ false }
{
  create();
}

FrameBufferObject::~FrameBufferObject()
{
  if (m_isInitialized) {
    glDeleteRenderbuffers(1, &m_nDepthBufferId);
    glDeleteFramebuffers(1, &m_nRenderFramebufferId);
  }
}

void FrameBufferObject::bind(bool clipping_on)
{
  glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &m_pdrawFboId);
  glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &m_preadFboId);
  glGetIntegerv(GL_DRAW_BUFFER, &m_pDrawBuffer);
  glGetIntegerv(GL_READ_BUFFER, &m_pReadBuffer);
  glGetIntegerv(GL_DEPTH_FUNC, &m_pDepthTest);
  glGetFloatv(GL_DEPTH_CLEAR_VALUE, &m_pClearDepth);

  glBindFramebuffer(GL_FRAMEBUFFER, m_nRenderFramebufferId);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glViewport(0, 0, m_width, m_height);
  if (clipping_on) {
    glClearDepth(1.0f);
  }
  else
  {
    glClearDepth(0.0f);
  }
  glDepthFunc(GL_LESS);
  glClear(GL_DEPTH_BUFFER_BIT);

}

void FrameBufferObject::unbind()
{

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_pdrawFboId);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_preadFboId);

  glDrawBuffer(m_pDrawBuffer);
  glReadBuffer(m_pReadBuffer);
  glClearDepth(m_pClearDepth);
  glDepthFunc(m_pDepthTest);

}

void FrameBufferObject::create()
{
  if (!m_isInitialized) {
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    m_width = viewport[2];
    m_height = viewport[3];

    glGenTextures(1, &m_nDepthBufferId);
    glBindTexture(GL_TEXTURE_2D, m_nDepthBufferId);
    glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_LUMINANCE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);


    glGenFramebuffers(1, &m_nRenderFramebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, m_nRenderFramebufferId);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_nDepthBufferId, 0);

    //glGenTextures(1, &m_nResolveTextureId);
    //glBindTexture(GL_TEXTURE_2D, m_nResolveTextureId);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    //glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_nResolveTextureId, 0);

    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // check FBO status
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
      std::cerr << "Cannot create FrameBufferObject - error status" << status << std::endl;
    }

    glEnable(GL_DEPTH_TEST);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK);

    m_isInitialized = true;
  }
}
