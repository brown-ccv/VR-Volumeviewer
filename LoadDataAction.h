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
///\file LoadDataAction.h
///\author Benjamin Knorlein
///\date 11/28/2017

#pragma once

#ifndef LOADDATAACTION_H
#define LOADDATAACTION_H
#include <string>
#include <vector>
#include "Data.h"

class LoadDataAction
	{
		
	public:
		LoadDataAction(std::string folder, Data<unsigned short> * data, float* res);
		void run();

		private:
			bool ends_with_string(std::string const& str, std::string const& what);
			bool contains_string(std::string const& str, std::string const& what);
			std::vector<std::string> readTiffs(std::string foldername);

			std::string m_folder;
			Data<unsigned short> * m_data;
			float* m_res;
	};

#endif // LOADDATAACTION_H
