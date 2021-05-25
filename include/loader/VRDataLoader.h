#ifndef VRDATALOADER_H
#define VRDATALOADER_H

#include <string>
class VRVolumeApp;

class VRDataLoader
{
public:


  static VRDataLoader* get_instance();

  static void load_txt_file(VRVolumeApp& vrVolumeApp, std::string& filename);

private:

  VRDataLoader();

  static VRDataLoader* m_instance;
};

#endif

