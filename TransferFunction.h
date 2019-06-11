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
///\file Trans ferFunction.h
///\author Benjamin Knorlein
///\date 6/11/2019

#pragma once

#ifndef TRANSFERFUNCTION_H
#define TRANSFERFUNCTION_H


class TransferFunction
{
public:
	TransferFunction();
	TransferFunction(float* data, unsigned int dataLength);
	~TransferFunction();

	void initGL();

	unsigned& texture_id()
	{
		return m_texture_id;
	}

	void set_texture_id(const unsigned texture_id)
	{
		m_texture_id = texture_id;
	}

private:
	void computeJetFunction();

	unsigned int m_texture_id;
	float* m_data;
	unsigned int m_dataLength;
};

#endif // TRANSFERFUNCTION_H
