
import os
import pathlib 
from os.path import dirname, abspath
from pathlib import Path 
import shutil
import errno, os, stat


def handleRemoveReadonly(func, path, exc):
  excvalue = exc[1]
  if func in (os.rmdir, os.unlink, os.remove) and excvalue.errno == errno.EACCES:
      os.chmod(path, stat.S_IRWXU| stat.S_IRWXG| stat.S_IRWXO) # 0777
      func(path)
  else:
      raise

current_dir = pathlib.Path(__file__).parent.resolve()
parent_dir = dirname(dirname(abspath(__file__)))
print("working directory " + parent_dir)
folders_to_operate = ["CPPFSD","freetype","FTGL","glew","glfw","glm","MinVR","opencv","teem","vr-imgui","zlib","Choreograph"]
files_to_exclude = ["CMakeLists.txt","macros.cmake","minvr_patch_082021.patch","build_oscar.sh","build_oscar_release.sh"]
folders_to_exclude = ["cmake","clean","scripts","bat2exe"]

for filename in os.listdir(parent_dir):
    file_path=os.path.join(parent_dir, filename)
    if filename in folders_to_operate and os.path.isdir(file_path):
         # print(os.path.join(directory, filename))
        for sub_filename in os.listdir(file_path):
            sub_file_path=os.path.join(file_path, sub_filename)
            if  os.path.isdir(sub_file_path):
                 print("to delete dir"+sub_file_path) 
                 shutil.rmtree(sub_file_path , ignore_errors=False, onerror=handleRemoveReadonly)
            elif not sub_file_path.endswith('CMakeLists.txt'):
                 print("to delete file"+sub_file_path)
                 os.remove(sub_file_path)
        
    elif os.path.isdir(file_path) and filename not in folders_to_exclude:
        shutil.rmtree(file_path)
    else:
        if not os.path.isdir(file_path) and filename not in files_to_exclude:
             print("to delete file"+file_path)
             os.remove(file_path)


