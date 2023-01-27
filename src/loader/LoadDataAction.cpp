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
///\date 11/28/2017

#include "../../include/loader/LoadDataAction.h"
#include "../../include/interaction/HelperFunctions.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <fstream>
#include <algorithm>
#include "Texture.h"
#include <render/Volume2D.h>
#include <render/Volume3D.h>

LoadDataAction::LoadDataAction(std::string folder, float* res) : m_folder(folder), m_res(res)
{
}

void equalizeHistogram(std::vector<cv::Mat>& images, unsigned short min_value)
{
	if (images.empty())
	{
		return;
	}
	int total = 0;
	int n_bins = std::numeric_limits<unsigned short>::max() + 1;
	int max_val = std::numeric_limits<unsigned short>::max();

	// Compute histogram
	std::vector<long long> hist(n_bins, 0);

	for (auto& im : images)
	{
		int nRows = im.rows;
		int nCols = im.cols;

		if (im.isContinuous())
		{
			nCols *= nRows;
			nRows = 1;
		}

		int i, j;
		unsigned short* p;
		for (i = 0; i < nRows; ++i)
		{
			p = im.ptr<unsigned short>(i);
			for (j = 0; j < nCols; ++j)
			{
				if (p[j] >= min_value)
				{
					hist[p[j]]++;
					total++;
				}
			}
		}
	}
	std::cerr << "Total Pixel " << total << std::endl;

	// Find first non-zero bin
	int i = 1;
	while (!hist[i])
		++i;

	std::cerr << "Minimum is" << i << std::endl;

	// Compute scale
	float scale = (n_bins - 1.f) / (total - hist[i]);

	// Initialize lut
	std::vector<int> lut(n_bins, 0);
	i++;

	int sum = 0;
	for (; i < hist.size(); ++i)
	{
		sum += hist[i];
		// the value is saturated in range [0, max_val]
		lut[i] = std::max(0, std::min(int(round(sum * scale)), max_val));
	}

	for (auto& im : images)
	{
		int nRows = im.rows;
		int nCols = im.cols;

		if (im.isContinuous())
		{
			nCols *= nRows;
			nRows = 1;
		}

		int i, j;
		unsigned short* p;
		for (i = 0; i < nRows; ++i)
		{
			p = im.ptr<unsigned short>(i);
			for (j = 0; j < nCols; ++j)
			{
				if (p[j] >= min_value)
				{
					p[j] = lut[p[j]];
				}
				else
				{
					p[j] = 0;
				}
			}
		}
	}
}

Volume* LoadDataAction::run(bool convert)
{
	unsigned int channels, depth, w, h, d;
	float minval[2];
	std::vector<cv::Mat> images;
	time_t posix_time;

	if (helper::ends_with_string(m_folder, "png.desc"))
	{
		FILE* pFile;
		pFile = fopen(m_folder.c_str(), "r");
		fscanf(pFile, "%u,%u,%u,%f,%f\n'", &w, &h, &d, &minval[0], &minval[1]);
		fscanf(pFile, "%lld\n'", &posix_time);

		channels = 1;
		depth = CV_16U;
		helper::replace(m_folder, ".png.desc", ".png");
		std::vector<cv::Mat> images;

		Volume* volume = new Volume2D(w, h, d, m_res[0], m_res[1], m_res[2], 2, channels, m_folder);
		minval[0] = std::numeric_limits<unsigned int>::min();
		minval[1] = std::numeric_limits<unsigned int>::max();
		volume->setMinMax(minval[0], minval[1]);
		volume->setTime(posix_time);
		return volume;

	}
	else if (helper::ends_with_string(m_folder, "desc"))
	{

		FILE* pFile;
		pFile = fopen(m_folder.c_str(), "r");
		fscanf(pFile, "%u,%u,%u,%f,%f\n'", &w, &h, &d, &minval[0], &minval[1]);
		fscanf(pFile, "%lld\n'", &posix_time);

		channels = 1;
		depth = CV_32F;
		helper::replace(m_folder, ".desc", ".raw");
	}
	else
	{

		std::vector<std::string> filenames = readTiffs(m_folder);
		std::vector<cv::Mat> image_r;
		std::vector<cv::Mat> image_g;
		std::vector<cv::Mat> image_b;

		minval[0] = 0;

		for (auto name : filenames)
		{
			if (helper::contains_string(name, "ch1"))
			{
				std::cerr << "Load Image " << image_r.size() << " Channel 1 - " << name << std::endl;
				image_r.push_back(std::move(cv::imread(name, cv::IMREAD_ANYDEPTH | cv::IMREAD_GRAYSCALE)));
			}
			else if (helper::contains_string(name, "ch2"))
			{
				std::cerr << "Load Image " << image_g.size() << " Channel 2 - " << name << std::endl;
				image_g.push_back(std::move(cv::imread(name, cv::IMREAD_ANYDEPTH | cv::IMREAD_GRAYSCALE)));
			}
			else if (helper::contains_string(name, "ch3"))
			{
				std::cerr << "Load Image " << image_b.size() << " Channel 3 - " << name << std::endl;
				image_b.push_back(std::move(cv::imread(name, cv::IMREAD_ANYDEPTH | cv::IMREAD_GRAYSCALE)));
			}
			else
			{
				std::cerr << "Load Image " << images.size() << " RGB - " << name << std::endl;
				images.push_back(std::move(cv::imread(name, cv::IMREAD_ANYDEPTH | cv::IMREAD_ANYCOLOR)));
				cv::cvtColor(images.back(), images.back(), cv::COLOR_BGR2RGB);
			}
		}

		if (!image_r.empty() || !image_g.empty() || !image_b.empty())
		{
			mergeRGB(image_r, image_g, image_b, images);
		}

		channels = images[0].channels();
		depth = images[0].depth();
		switch (depth)
		{
		case CV_8U:
			minval[1] = 255.0f;
			break;
		case CV_16U:
			minval[1] = 65535.0f;
			break;
		case CV_32F:
			minval[1] = 1.0f;
			break;
		}
		w = images[0].cols;
		h = images[0].rows;
		d = images.size();
	}

	std::cerr << "Loading Volume size:  " << w << " , " << h << " , " << d << "Channels " << channels << std::endl;
	Volume* volume;
	switch (depth)
	{
	case CV_8U:
		volume = new Volume3D(w, h, d, m_res[0], m_res[1], m_res[2], 1, channels);
		uploadDataCV_8U(images, volume);
		break;
	case CV_16U:
		volume = new Volume3D(w, h, d, m_res[0], m_res[1], m_res[2], 2, channels);
		uploadDataCV_16U(images, volume);
		break;
	case CV_32F:
		volume = new Volume3D(w, h, d, m_res[0], m_res[1], m_res[2], 4, channels);
		uploadData_32F_raw(m_folder, volume);
		break;
	}
	volume->computeHistogram();
	volume->setMinMax(minval[0], minval[1]);
	volume->setTime(posix_time);
	return volume;
}

void LoadDataAction::mergeRGB(std::vector<cv::Mat>& image_r, std::vector<cv::Mat>& image_g, std::vector<cv::Mat>& image_b, std::vector<cv::Mat>& image)
{
	int d;
	cv::Mat B;
	if (!image_r.empty())
	{
		d = image_r.size();
		B = cv::Mat::zeros(image_r[0].rows, image_r[0].cols, image_r[0].depth());
	}
	else if (!image_g.empty())
	{
		d = image_g.size();
		B = cv::Mat::zeros(image_g[0].rows, image_g[0].cols, image_g[0].depth());
	}
	else if (!image_b.empty())
	{
		d = image_b.size();
		B = cv::Mat::zeros(image_b[0].rows, image_b[0].cols, image_b[0].depth());
	}

	for (int z = 0; z < d; z++)
	{
		std::vector<cv::Mat> array_to_merge;
		if (!image_r.empty())
		{
			array_to_merge.push_back(image_r[z]);
		}
		else
		{
			array_to_merge.push_back(B);
		}
		if (!image_g.empty())
		{
			array_to_merge.push_back(image_g[z]);
		}
		else
		{
			array_to_merge.push_back(B);
		}
		if (!image_b.empty())
		{
			array_to_merge.push_back(image_b[z]);
		}
		else
		{
			array_to_merge.push_back(B);
		}
		cv::Mat image_merged;
		cv::merge(array_to_merge, image_merged);
		image.push_back(image_merged);
	}
}

void LoadDataAction::uploadDataCV_8U(std::vector<cv::Mat> image, Volume* volume)
{

	unsigned char* ptr = reinterpret_cast<unsigned char*>(volume->get_data());
	// fill vol and points
	for (int z = 0; z < image.size(); z++)
	{
		if (image[z].channels() == 1)
		{
			cv::MatConstIterator_<uchar> it1 = image[z].begin<uchar>();
			cv::MatConstIterator_<uchar> it1_end = image[z].end<uchar>();
			for (; it1 != it1_end; ++it1)
			{
				*ptr++ = *it1;
			}
		}
		else if (image[z].channels() == 3)
		{
			cv::MatConstIterator_<cv::Vec3b> it1 = image[z].begin<cv::Vec3b>();
			cv::MatConstIterator_<cv::Vec3b> it1_end = image[z].end<cv::Vec3b>();
			for (; it1 != it1_end; ++it1)
			{
				*ptr++ = (*it1)[0];
				*ptr++ = (*it1)[1];
				*ptr++ = (*it1)[2];
			}
		}
		else if (image[z].channels() == 4)
		{
			cv::MatConstIterator_<cv::Vec4b> it1 = image[z].begin<cv::Vec4b>();
			cv::MatConstIterator_<cv::Vec4b> it1_end = image[z].end<cv::Vec4b>();
			for (; it1 != it1_end; ++it1)
			{
				*ptr++ = (*it1)[0];
				*ptr++ = (*it1)[1];
				*ptr++ = (*it1)[2];
				*ptr++ = (*it1)[3];
			}
		}
	}
}
void LoadDataAction::uploadDataCV_16U(std::vector<cv::Mat> image, Volume* volume)
{
	unsigned short* ptr = reinterpret_cast<unsigned short*>(volume->get_data());

	// fill vol and points
	for (int z = 0; z < image.size(); z++)
	{

		if (image[z].channels() == 1)
		{

			cv::MatConstIterator_<unsigned short> it1 = image[z].begin<unsigned short>();
			cv::MatConstIterator_<unsigned short> it1_end = image[z].end<unsigned short>();
			size_t num_pixels = 0;
			for (; it1 != it1_end; ++it1)
			{
				*ptr++ = *it1;
				num_pixels++;
			}

		}
		else if (image[z].channels() == 3)
		{
			cv::MatConstIterator_<cv::Vec3w> it1 = image[z].begin<cv::Vec3w>();
			cv::MatConstIterator_<cv::Vec3w> it1_end = image[z].end<cv::Vec3w>();
			for (; it1 != it1_end; ++it1)
			{
				*ptr++ = (*it1)[0];
				*ptr++ = (*it1)[1];
				*ptr++ = (*it1)[2];
			}
		}
		else if (image[z].channels() == 4)
		{
			cv::MatConstIterator_<cv::Vec4w> it1 = image[z].begin<cv::Vec4w>();
			cv::MatConstIterator_<cv::Vec4w> it1_end = image[z].end<cv::Vec4w>();
			for (; it1 != it1_end; ++it1)
			{
				*ptr++ = (*it1)[0];
				*ptr++ = (*it1)[1];
				*ptr++ = (*it1)[2];
				*ptr++ = (*it1)[3];
			}
		}
	}
}

void LoadDataAction::uploadData_32F_raw(std::string& filename, Volume* volume)
{
	void* data = reinterpret_cast<unsigned short*>(volume->get_data());

	FILE* file = fopen(filename.c_str(), "rb");

	// obtain file size:
	fseek(file, 0, SEEK_END);
	long lSize = ftell(file);
	rewind(file);
	if (lSize != volume->get_depth() * volume->get_height() * volume->get_width() * 4)
	{
		std::cerr << "Error datasize not equal - file" << lSize << " bytes - desc " << volume->get_depth() * volume->get_height() * volume->get_width() * 4 << " bytes" << std::endl;
	}

	fread(data, sizeof(float), lSize / 4, file);
	fclose(file);
}

std::vector<std::string> LoadDataAction::readTiffs(std::string foldername)
{
	std::vector<std::string> out_vector;
	DIR* dir;
	struct dirent* ent;

	if ((dir = opendir(foldername.c_str())) != NULL)
	{
		/* print all the files and directories within directory */
		while ((ent = readdir(dir)) != NULL)
		{
			if (helper::ends_with_string(ent->d_name, "tif") || helper::ends_with_string(ent->d_name, "tiff") || helper::ends_with_string(ent->d_name, "png"))
			{
				out_vector.push_back(foldername + OS_SLASH + ent->d_name);
			}
		}
		closedir(dir);
	}
	else
	{
		std::cerr << "Could not open folder" << std::endl;
	}
	std::sort(out_vector.begin(), out_vector.end());
	return out_vector;
}
