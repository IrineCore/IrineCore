#pragma once
#include "windows.h"
struct PROCESS_MEMORY_COUNTERS {
    DWORD  cb;
    DWORD  PageFaultCount;
    SIZE_T PeakWorkingSetSize;
    SIZE_T WorkingSetSize;
    SIZE_T QuotaPeakPagedPoolUsage;
    SIZE_T QuotaPagedPoolUsage;
    SIZE_T QuotaPeakNonPagedPoolUsage;
    SIZE_T QuotaNonPagedPoolUsage;
    SIZE_T PagefileUsage;
    SIZE_T PeakPagefileUsage;
};
using PPROCESS_MEMORY_COUNTERS = PROCESS_MEMORY_COUNTERS*;
inline BOOL GetProcessMemoryInfo(HANDLE, PPROCESS_MEMORY_COUNTERS ppsmemCounters, DWORD cb) noexcept {
    if (ppsmemCounters && cb >= sizeof(PROCESS_MEMORY_COUNTERS)) std::memset(ppsmemCounters, 0, sizeof(*ppsmemCounters));
    return TRUE;
}
