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
///\file VolumeRenderer.h
///\author Benjamin Knorlein
///\date 5/24/2019

#pragma once

#ifndef VOLUMERENDER_H
#define VOLUMERENDER_H

#include "FrameBufferObject.h"
#include "Volume.h"

class VolumeRenderer
{
public:
	VolumeRenderer(){};
	virtual ~VolumeRenderer(){};

	virtual void initGL() = 0;
	virtual void render(Volume *volume, const glm::mat4 &MV, glm::mat4 &P, float z_scale, GLint colorma, int renderChannel) = 0;

	virtual void set_threshold(float threshold) = 0;
	virtual void set_multiplier(float multiplier) = 0;
	virtual void set_numSlices(int slices) = 0;

	virtual void set_blending(bool useBlending, float alpha, Volume *volume) = 0;
	virtual void useMultichannelColormap(bool useMulti) = 0;

	virtual void setClipMinMax(glm::vec3 min_clip, glm::vec3 max_clip) = 0;

	void setClipping(bool isClipping, glm::mat4 *clipPlane)
	{
		m_clipping = isClipping;
		if (m_clipping)
			m_clipPlane = *clipPlane;
	};

protected:
	bool m_clipping;
	glm::mat4 m_clipPlane;
};

#endif // VOLUMERENDER_H
