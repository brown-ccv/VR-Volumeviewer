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
///\file LoadDescriptionAction.cpp
///\author Benjamin Knorlein
///\date 01/20/2021



#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../../include/loader/LoadDescriptionAction.h"
#include <iostream>
#include <fstream>

LoadDescriptionAction::LoadDescriptionAction(std::string file) :m_file(file)
{

}

std::istream& LoadDescriptionAction::safeGetline(std::istream& is, std::string& t)
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


std::vector <std::string> LoadDescriptionAction::run()
{
  std::vector <std::string> text;
  std::ifstream fin;
  fin.open(m_file);
  std::string line;
  while (!safeGetline(fin, line).eof())
  {
    text.push_back(line);
  }
  return text;
}
