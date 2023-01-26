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
///\file DepthTexture.cpp
///\author Benjamin Knorlein
///\date 6/14/2019

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "render/DepthTexture.h"
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


DepthTexture::DepthTexture(int window_width, int window_height, int framebuffer_width, int framebuffer_height) : m_isInitialized{ false }
{
	create(window_width, window_height, framebuffer_width, framebuffer_height);
}

DepthTexture::~DepthTexture()
{
	if (m_isInitialized)
	{
		glDeleteTextures(1, &m_depth_texture);
	}
}

void DepthTexture::copyDepthbuffer()
{
	glBindTexture(GL_TEXTURE_2D, m_depth_texture);
	glReadBuffer(m_pDrawBuffer);

	// TODO: Does not work for multisample framebuffer
	glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, m_framebuffer_width, m_framebuffer_height, 0);
	//glCheckError();
	glBindTexture(GL_TEXTURE_2D, 0);
	//glCheckError();
	glReadBuffer(m_pReadBuffer);
	//glCheckError();

}

void DepthTexture::create(int window_width, int window_height, int framebuffer_width, int framebuffer_height)
{
	if (!m_isInitialized)
	{
		glGetIntegerv(GL_DRAW_BUFFER, &m_pDrawBuffer);
		glGetIntegerv(GL_READ_BUFFER, &m_pReadBuffer);

		// GLint viewport[4];
		// glGetIntegerv(GL_VIEWPORT, viewport);
		// m_width = viewport[2];
		// m_height = viewport[3];
		m_width = window_width;
		m_height = window_height;
		m_framebuffer_width = framebuffer_width;
		m_framebuffer_height = framebuffer_height;

		m_display_scale_x = m_framebuffer_width / window_width;
		m_display_scale_y = m_framebuffer_height / window_height;

		glGenTextures(1, &m_depth_texture);
		glBindTexture(GL_TEXTURE_2D, m_depth_texture);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, m_width, m_height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		glBindTexture(GL_TEXTURE_2D, 0);

		m_isInitialized = true;
	}
}
