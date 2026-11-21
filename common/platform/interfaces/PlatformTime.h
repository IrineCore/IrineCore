// =============================================================================
// IrineCore — common/platform/interfaces/PlatformTime.h
// Cross-platform monotonic clock, wall-clock time, and sleep abstractions.
// =============================================================================
#pragma once

#include "PlatformTypes.h"
#include <cstdint>

namespace Irine {
namespace Platform {

uint32_t GetTickCountMs() noexcept;
uint64_t GetTickCount64Ms() noexcept;
void     GetLocalSystemTime(SYSTEMTIME* outTime) noexcept;
void     SleepMs(uint32_t milliseconds) noexcept;

} // namespace Platform
} // namespace Irine

#if !defined(_WIN32)

DWORD     timeGetTime() noexcept;
DWORD     GetTickCount() noexcept;
ULONGLONG GetTickCount64() noexcept;
BOOL      QueryPerformanceCounter(LARGE_INTEGER* lpPerformanceCount) noexcept;
BOOL      QueryPerformanceFrequency(LARGE_INTEGER* lpFrequency) noexcept;
void      GetLocalTime(SYSTEMTIME* lpSystemTime) noexcept;
void      GetSystemTime(SYSTEMTIME* lpSystemTime) noexcept;
BOOL      SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, FILETIME* lpFileTime) noexcept;
void      Sleep(DWORD dwMilliseconds) noexcept;

#endif // !_WIN32
