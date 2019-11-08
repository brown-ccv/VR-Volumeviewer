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
///\file LoadGliderDataAction.cpp
///\author Benjamin Knorlein
///\date 6/25/2019

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "LoadGliderDataAction.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iostream>


LoadGliderDataAction::LoadGliderDataAction(std::string filename) :m_filename(filename)
{

}

std::istream& safeGetline(std::istream& is, std::string& t)
{
	t.clear();

	// The characters in the stream are read one-by-one using a std::streambuf.
	// That is faster than reading them one-by-one using the std::istream.
	// Code that uses streambuf this way must be guarded by a sentry object.
	// The sentry object performs various tasks,
	// such as thread synchronization and updating the stream state.

	std::istream::sentry se(is, true);
	std::streambuf* sb = is.rdbuf();

	for (;;)
	{
		int c = sb->sbumpc();
		switch (c)
		{
		case '\n':
			return is;
		case '\r':
			if (sb->sgetc() == '\n')
				sb->sbumpc();
			return is;
		case EOF:
			// Also handle the case when the last line has no line ending
			if (t.empty())
			{
				is.setstate(std::ios::eofbit);
			}
			return is;
		default:
			t += (char)c;
		}
	}
}

std::istream& comma(std::istream& in)
{
	if ((in >> std::ws).peek() != std::char_traits<char>::to_int_type(','))
	{
		in.setstate(std::ios_base::failbit);
	}
	return in.ignore();
}

Glider* LoadGliderDataAction::run()
{
	Glider * glider = new Glider();

	std::ifstream fin(m_filename.c_str());
	std::istringstream in;
	std::string line;
	int count = 0;
	bool good = true;
	safeGetline(fin, line);
	
	parseLabels(glider, line);

	while (!safeGetline(fin, line).eof() && good)
	{
		in.clear();
		in.str(line);
		std::vector<double> tmp;
		for (double value; in >> value; comma(in))
		{
			tmp.push_back(value);
		}

		//set positions (x, y, z)
		//for (int i = 0; i < 5; i++)
		glider->positions().push_back(_pt(tmp[0], tmp[2], 1.0 - (1.0 - tmp[1]) * 0.5));
		
		//set Date (month, day, hour, minute, second )
		for (int i = 0; i < 5; i++)
			glider->date(i).push_back(tmp[i + 3]);


		for (int i = 8; i < tmp.size(); i++){
			while (glider->values().size() <= i - 8)
			{
				glider->values().push_back(std::vector<float>());
			}
			glider->values()[i-8].push_back(tmp[i]);
		}

		line.clear();
		tmp.clear();
		count++;
		for (int i = 0; i < 2; i++)
		{
			if (safeGetline(fin, line).eof()){
				good = false;
				break;
			}
		}
	}
	fin.close();

	computeMinMax(glider);
	glider->parseLabels();
	return glider;
}

void LoadGliderDataAction::computeMinMax(Glider* glider)
{
	for (int i = 0; i < glider->values().size(); i++)
	{
		float _min = std::numeric_limits<float>::max();
		float _max = std::numeric_limits<float>::min();

		_min = *std::min_element(std::begin(glider->values()[i]), std::end(glider->values()[i]));
		_max = *std::max_element(std::begin(glider->values()[i]), std::end(glider->values()[i]));

		for (int j = 0; j < glider->values()[i].size(); j++)
		{
			if (glider->values()[i][j] == -9999999)
				continue;

			_min = std::min(_min,glider->values()[i][j]);
			_max = std::max(_max, glider->values()[i][j]);
		}

		glider->min_max().push_back(std::make_pair(_min,_max));
		std::cerr << glider->min_max().back().second << " " << glider->min_max().back().first << std::endl;
	}
}

void LoadGliderDataAction::parseLabels(Glider* glider, std::string line)
{
	
	std::istringstream in;
	in.clear();
	in.str(line);

	std::vector<std::string> tmp;
	while (in.good())
	{
		std::string substr;
		getline(in, substr, ',');
		tmp.push_back(substr);
	}

	for (int i = 8; i < tmp.size(); i++){
		glider->values_legend().push_back(tmp[i]);
		std::cerr << "Loading " << tmp[i] << std::endl;
	}
}
