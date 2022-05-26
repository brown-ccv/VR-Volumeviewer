#include "loader/VRDataLoader.h"
#include <fstream>

//#include <filesystem>
#include <cppfs/fs.h>
#include <cppfs/FilePath.h>

#include <sstream>
#include "vrapp/VRVolumeApp.h"

#include "common/common.h"

VRDataLoader::VRDataLoader()
{
}

void VRDataLoader::load_txt_file(VRVolumeApp &vrVolumeApp, std::string &filename)
{

  std::ifstream inFile;
  inFile.open(filename);

  std::string line;

  cppfs::FilePath p_filename(filename);

  while (getline(inFile, line))
  {
    if (line[0] != '#')
    {
      std::vector<std::string> vals; // Create vector to hold our words
      std::stringstream ss(line);
      std::string buf;

      while (ss >> buf)
      {
        vals.push_back(buf);
      }
      if (vals.size() > 0)
      {
        std::string tag = vals[0];
        if (tag == "animated")
        {
          vrVolumeApp.set_is_animated(true);
        }
        if (tag == "threshold")
        {
          vrVolumeApp.set_threshold(stof(vals[1]));
        }
        if (tag == "label")
        {
          std::cerr << "add Label " << vals[1] << std::endl;
          std::cerr << "at position " << vals[2] << " , " << vals[3] << " , " << vals[4] << std::endl;
          std::cerr << "text at position " << vals[2] << " , " << vals[3] << " , " << vals[5] << std::endl;
          std::cerr << "text Size " << vals[6] << std::endl;
          std::cerr << "text offset " << vals[7] << std::endl;
          std::cerr << "for Volume " << vals[8] << std::endl;
          std::string label = p_filename.directoryPath() + OS_SLASH + vals[1];
          vrVolumeApp.add_label(label, stof(vals[2]), stof(vals[3]), stof(vals[4]), stof(vals[5]), stof(vals[6]), stof(vals[7]), stoi(vals[8]) - 1);
        }
        if (tag == "desc")
        {
          std::cerr << "Load Description " << vals[1] << std::endl;
          std::cerr << "with size " << vals[2] << std::endl;
          int descHeight = stoi(vals[2]);
          std::string fileName = vals[1];
          vrVolumeApp.set_description(descHeight, fileName);
        }
        if (tag == "mesh")
        {
          assert(vals.size() == 4 && "Check mesh initialization parameters in the volume configuration file");
          std::string mesh_file_full_path = p_filename.directoryPath() + OS_SLASH + vals[1];
          std::string texture_file_full_path = p_filename.directoryPath() + OS_SLASH + vals[2];
          vrVolumeApp.set_mesh(stoi(vals[3]), mesh_file_full_path, texture_file_full_path);
        }
        if (tag == "numVolumes")
        {
          std::cerr << "numVolumes " << vals[1] << std::endl;
          int numVolumes = std::stoi(vals[1]);
          vrVolumeApp.init_num_volumes(numVolumes);
          for (int i = 0; i < numVolumes; i++)
          {
            vrVolumeApp.add_data_label(vals[i + 2]);
          }
        }
        else if (tag.rfind("volume") == 0)
        {
          char str[3];
          int i;

          std::string strVolumeIndex = tag.substr(6);
          size_t volumeIndex = std::stoi(strVolumeIndex);
          vals[1] = p_filename.directoryPath() + OS_SLASH + vals[1];

          std::vector<std::promise<Volume *> *> &v = vrVolumeApp.get_promise(volumeIndex);
          std::promise<Volume *> *pm = new std::promise<Volume *>();
          v.push_back(pm);

          std::vector<std::future<Volume *>> *fut;
          if (!vrVolumeApp.get_future(volumeIndex))
          {
            vrVolumeApp.set_future(volumeIndex, new std::vector<std::future<Volume *>>);
          }

          fut = vrVolumeApp.get_future(volumeIndex);

          fut->push_back(pm->get_future());

          vrVolumeApp.set_future(volumeIndex, fut);

          vrVolumeApp.init_volume_loading(volumeIndex, vals);
        }
      }
    }
  }
  inFile.close();
  vrVolumeApp.set_loaded_file(filename);
}