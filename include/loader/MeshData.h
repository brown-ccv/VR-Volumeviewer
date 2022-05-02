#ifndef MESHDATA_H
#define MESHDATA_H

#include <string>

struct MeshData {
  unsigned int volume_id;
  std::string mesh_file_path;
  std::string texture_file_path;
};

#endif
