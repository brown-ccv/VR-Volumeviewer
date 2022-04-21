#ifndef VRDATALOADER_H
#define VRDATALOADER_H

#include <string>
class VRVolumeApp;

class VRDataLoader
{
public:
  VRDataLoader();

  static void load_txt_file(VRVolumeApp &vrVolumeApp, std::string &filename);
};

#endif
