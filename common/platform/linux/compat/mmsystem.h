#pragma once
#include "windows.h"
inline UINT timeBeginPeriod(UINT) noexcept { return 0; }
inline UINT timeEndPeriod(UINT) noexcept { return 0; }
