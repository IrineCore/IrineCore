#pragma once
#include "windows.h"
#include <unistd.h>
#include <fcntl.h>
#ifndef _A_NORMAL
#define _A_NORMAL 0x00
#define _A_RDONLY 0x01
#define _A_HIDDEN 0x02
#define _A_SYSTEM 0x04
#define _A_SUBDIR 0x10
#define _A_ARCH   0x20
#endif
struct _finddata_t {
    unsigned    attrib;
    time_t      time_create;
    time_t      time_access;
    time_t      time_write;
    uint32_t    size;
    char        name[260];
};
struct _finddata64i32_t {
    unsigned    attrib;
    int64_t     time_create;
    int64_t     time_access;
    int64_t     time_write;
    uint32_t    size;
    char        name[260];
};
inline intptr_t _findfirst(const char* filespec, _finddata_t* fileinfo) {
    WIN32_FIND_DATAA fd{};
    HANDLE h = FindFirstFileA(filespec, &fd);
    if (h == INVALID_HANDLE_VALUE) return -1;
    if (fileinfo) {
        std::memset(fileinfo, 0, sizeof(*fileinfo));
        fileinfo->attrib = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? _A_SUBDIR : _A_NORMAL;
        fileinfo->size = fd.nFileSizeLow;
        strncpy_s(fileinfo->name, sizeof(fileinfo->name), fd.cFileName, _TRUNCATE);
    }
    return reinterpret_cast<intptr_t>(h);
}
inline int _findnext(intptr_t handle, _finddata_t* fileinfo) {
    WIN32_FIND_DATAA fd{};
    if (!FindNextFileA(reinterpret_cast<HANDLE>(handle), &fd)) return -1;
    if (fileinfo) {
        std::memset(fileinfo, 0, sizeof(*fileinfo));
        fileinfo->attrib = (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) ? _A_SUBDIR : _A_NORMAL;
        fileinfo->size = fd.nFileSizeLow;
        strncpy_s(fileinfo->name, sizeof(fileinfo->name), fd.cFileName, _TRUNCATE);
    }
    return 0;
}
inline int _findclose(intptr_t handle) {
    return FindClose(reinterpret_cast<HANDLE>(handle)) ? 0 : -1;
}
