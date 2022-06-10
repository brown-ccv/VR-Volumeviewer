
#include <assert.h>

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#define OS_SLASH "\\"
#include "../../external/msvc/dirent.h"
#else
#define OS_SLASH "//"
#include <dirent.h>
#endif