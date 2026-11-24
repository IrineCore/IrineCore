#pragma once
#include "windows.h"
#ifndef COINIT_MULTITHREADED
#define COINIT_MULTITHREADED 0x0
#define COINIT_APARTMENTTHREADED 0x2
#endif
inline HRESULT CoInitialize(LPVOID) noexcept { return S_OK; }
inline HRESULT CoInitializeEx(LPVOID, DWORD) noexcept { return S_OK; }
inline void    CoUninitialize() noexcept {}
inline HRESULT CoCreateGuid(GUID* pguid) noexcept {
    if (!pguid) return E_FAIL;
    unsigned int r[4];
    for (int i = 0; i < 4; ++i) rand_s(&r[i]);
    std::memcpy(pguid, r, sizeof(GUID));
    return S_OK;
}
