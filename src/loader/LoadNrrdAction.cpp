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
///\file LoadNrrdAction.cpp
///\author Benjamin Knorlein
///\date 11/14/2019


#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "../../include/loader/LoadNrrdAction.h"
#include <iostream>
#include <teem/nrrd.h>
#include <teem/biff.h>
#include <render/Volume3D.h>

LoadNrrdAction::LoadNrrdAction(std::string& file) :m_file(file)
{

}

Volume* LoadNrrdAction::run()
{
  int dimension[3];
  float spacing[3];
  int channels;


  std::cerr << "Loading Volume  " << m_file << std::endl;

  Nrrd* nrrd = nrrdNew();
  if (nrrdLoad(nrrd, m_file.c_str(), NULL)) {
    char* err = biffGetDone(NRRD);
    std::cerr << "NRRD : trouble reading " << m_file << " " << err << std::endl;
    free(err);
    return nullptr;
  }

  if (nrrd->dim > 4)
  {
    std::cerr << "NRRD :  nrrd input can only handle data with dimension 4 or less." << std::endl;
    nrrdNuke(nrrd);
    return nullptr;
  }
  else if (nrrd->spaceDim > 3) {
    std::cerr << "NRRD :  nrrd input can only handle data with space dimension 3 or less." << std::endl;
    nrrdNuke(nrrd);
    return nullptr;
  }

  //single channel
  if (nrrd->dim == 3)
  {
    channels = 1;
    dimension[0] = nrrd->axis[0].size;
    dimension[1] = nrrd->axis[1].size;
    dimension[2] = nrrd->axis[2].size;
    spacing[0] = nrrd->axis[0].spaceDirection[0];
    spacing[1] = nrrd->axis[1].spaceDirection[1];
    spacing[2] = nrrd->axis[2].spaceDirection[2];

  }
  //multi channel image
  else if (nrrd->dim == 4)
  {
    channels = nrrd->axis[0].size;
    dimension[0] = nrrd->axis[1].size;
    dimension[1] = nrrd->axis[2].size;
    dimension[2] = nrrd->axis[3].size;
    spacing[0] = nrrd->axis[1].spaceDirection[0];
    spacing[1] = nrrd->axis[2].spaceDirection[1];
    spacing[2] = nrrd->axis[3].spaceDirection[2];
    if (channels != 1 && channels != 3 && channels != 4)
    {
      std::cerr << "NRRD : only 1,3 and 4 channels supported : current " << channels << std::endl;
      nrrdNuke(nrrd);
      return nullptr;
    }
  }
  else
  {
    std::cerr << "NRRD : only volumetric data supported" << std::endl;
    nrrdNuke(nrrd);
    return nullptr;
  }


  int datasize = -1;
  switch (nrrd->type)
  {
  case nrrdTypeUChar:  datasize = 1; break;
  case nrrdTypeUShort: datasize = 2; break;
  case nrrdTypeFloat:  datasize = 4; break;
  default: break;
  }

  if (datasize < 0)
  {
    std::cerr << "NRRD : datatype not supported" << std::endl;
    nrrdNuke(nrrd);
    return nullptr;
  }

  std::cerr << "Loading Volume size:  " << dimension[0] << " , " << dimension[1] << " , " << dimension[2]
    << " spacing:  " << spacing[0] << " , " << spacing[1] << " , " << spacing[2] << " Channels " << channels << std::endl;

  Volume* volume = new Volume3D(dimension[0], dimension[1], dimension[2], spacing[0], spacing[1], spacing[2], datasize, channels);
  unsigned short* volume_data = reinterpret_cast <unsigned short*> (volume->get_datau16());

  size_t nElements = nrrdElementNumber(nrrd);
  size_t nElementSize = nrrdElementSize(nrrd);
  memcpy(volume_data, nrrd->data,
    nrrdElementNumber(nrrd) * nrrdElementSize(nrrd));

  nrrdNuke(nrrd);
  volume->computeHistogram();
  return volume;
}
