// =============================================================================
// IrineCore — common/platform/interfaces/PlatformThread.h
// Cross-platform thread spawning, thread IDs, and CPU core discovery.
// =============================================================================
#pragma once

#include "PlatformTypes.h"
#include <cstdint>

namespace Irine {
namespace Platform {

uint32_t GetCurrentThreadId() noexcept;
uint32_t GetCurrentProcessId() noexcept;
uint32_t GetProcessorCount() noexcept;

} // namespace Platform
} // namespace Irine

#if !defined(_WIN32)

using _beginthreadex_proc_type = unsigned (__stdcall *)(void*);

uintptr_t _beginthreadex(
    void* security,
    unsigned stack_size,
    _beginthreadex_proc_type start_address,
    void* arglist,
    unsigned initflag,
    unsigned* thrdaddr) noexcept;

void _endthreadex(unsigned retval) noexcept;

DWORD  GetCurrentThreadId() noexcept;
DWORD  GetCurrentProcessId() noexcept;
HANDLE GetCurrentThread() noexcept;
BOOL   SetThreadPriority(HANDLE hThread, int nPriority) noexcept;
DWORD  ResumeThread(HANDLE hThread) noexcept;
DWORD  SuspendThread(HANDLE hThread) noexcept;

#ifndef CREATE_SUSPENDED
#define CREATE_SUSPENDED 0x00000004u
#define THREAD_PRIORITY_NORMAL 0
#define THREAD_PRIORITY_ABOVE_NORMAL 1
#define THREAD_PRIORITY_HIGHEST 2
#define THREAD_PRIORITY_BELOW_NORMAL (-1)
#endif

#endif // !_WIN32
