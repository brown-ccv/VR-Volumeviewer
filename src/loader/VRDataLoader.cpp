#include "loader/VRDataLoader.h"
#include <fstream>


//#include <filesystem>
#include <cppfs/fs.h>
#include <cppfs/FilePath.h>

#include <sstream>  
#include "vrapp/VRVolumeApp.h"

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#define OS_SLASH_LOCAL "\\"
#include "../../external/msvc/dirent.h"
#else
#define OS_SLASH_LOCAL "//"
#include <dirent.h>
#endif



//VRDataLoader* VRDataLoader::m_instance = nullptr;

VRDataLoader::VRDataLoader()
{

}

//VRDataLoader* VRDataLoader::get_instance()
//{
//  if (!m_instance)
//  {
//    m_instance = new VRDataLoader;
//  }
//  return m_instance;
//}VRDataLoader* VRDataLoader::get_instance()
//{
//  if (!m_instance)
//  {
//    m_instance = new VRDataLoader;
//  }
//  return m_instance;
//}


void VRDataLoader::load_txt_file( VRVolumeApp& vrVolumeApp,  std::string& filename)
{
  
  std::ifstream inFile;
  inFile.open(filename);

  std::string line;

  //std::filesystem::path p_filename(filename);
  cppfs::FilePath p_filename(filename);

  while (getline(inFile, line)) {
    if (line[0] != '#') {
      std::vector<std::string> vals; // Create vector to hold our words
      std::stringstream ss(line);
      std::string buf;

      while (ss >> buf) {
        vals.push_back(buf);
      }
      if (vals.size() > 0) {
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
          std::cerr << "for Volume " << vals[7] << std::endl;
          std::string label = p_filename.directoryPath() + OS_SLASH_LOCAL +  vals[1];
          vrVolumeApp.add_label(label, stof(vals[2]), stof(vals[3]), stof(vals[4]), stof(vals[5]), stof(vals[6]), stoi(vals[7]) - 1);
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
          //	std::cerr << "Load Mesh " << vals[1] << std::endl;
          //	std::cerr << "for Volume " << vals[2] << std::endl;

          //std::string fullPath = p_filename.parent_path().string() + OS_SLASH_LOCAL + vals[1];
          std::string fullPath = p_filename.directoryPath() + OS_SLASH_LOCAL + vals[1];
          std::cerr << "Load Mesh " << fullPath << std::endl;
          std::string shaderFilePath = p_filename.directoryPath() + OS_SLASH_LOCAL + "shaders";

          vrVolumeApp.set_mesh(stoi(vals[2]), fullPath, shaderFilePath);

        }
        if (tag == "texture")
        {

          //std::cerr << "for Volume " << vals[2] << std::endl;
          std::string fullPath = p_filename.directoryPath() + OS_SLASH_LOCAL + vals[1];
          vrVolumeApp.set_texture(fullPath);


        }
        if (tag == "numVolumes")
        {

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


          vals[1] = p_filename.directoryPath() + OS_SLASH_LOCAL + vals[1];



          std::vector<std::promise<Volume*>*>& v = vrVolumeApp.get_promise(volumeIndex);
          std::promise<Volume*>* pm = new std::promise<Volume*>();

          v.push_back(pm);

          std::vector<std::future<Volume*>>* fut;
          if (!vrVolumeApp.get_future(volumeIndex))
          {
            vrVolumeApp.set_future(volumeIndex, new std::vector<std::future<Volume*>>);
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