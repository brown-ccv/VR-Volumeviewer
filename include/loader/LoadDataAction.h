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

#ifndef LOADDATAACTION_H
#define LOADDATAACTION_H
#include <string>
#include <vector>
#include "render/Volume.h"
#include <opencv2/core/mat.hpp>
#include "common/common.h"

class LoadDataAction
{

public:
  LoadDataAction(std::string folder, float *res);
  Volume *run(bool convert = false);

private:
  std::vector<std::string> readTiffs(std::string foldername);

  std::string m_folder;
  float *m_res;

  void mergeRGB(std::vector<cv::Mat> &image_r, std::vector<cv::Mat> &image_g, std::vector<cv::Mat> &image_b, std::vector<cv::Mat> &image);
  static void uploadDataCV_8U(std::vector<cv::Mat> image, Volume *volume);
  static void uploadDataCV_16U(std::vector<cv::Mat> image, Volume *volume);
  static void uploadData_32F_raw(std::string filename, Volume *volume);
};

#endif // LOADDATAACTION_H
