#pragma once
#include "windows.h"
#include <unistd.h>
inline char* _getcwd(char* buf, int maxlen) { return ::getcwd(buf, static_cast<size_t>(maxlen)); }
inline int   _chdir(const char* path) { return path ? ::chdir(Irine::Platform::NormalizePath(path).c_str()) : -1; }
inline int   _rmdir(const char* path) { return path ? ::rmdir(Irine::Platform::NormalizePath(path).c_str()) : -1; }
