// =============================================================================
// IrineCore — common/platform/interfaces/PlatformSync.h
// Cross-platform mutex, critical section, event, and atomic operations.
// =============================================================================
#pragma once

#include "PlatformTypes.h"

#if !defined(_WIN32)

void  InitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection) noexcept;
BOOL  InitializeCriticalSectionAndSpinCount(LPCRITICAL_SECTION lpCriticalSection, DWORD dwSpinCount) noexcept;
void  DeleteCriticalSection(LPCRITICAL_SECTION lpCriticalSection) noexcept;
void  EnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection) noexcept;
BOOL  TryEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection) noexcept;
void  LeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection) noexcept;

HANDLE CreateEventA(void* lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCSTR lpName) noexcept;
HANDLE CreateEventW(void* lpEventAttributes, BOOL bManualReset, BOOL bInitialState, LPCWSTR lpName) noexcept;
#if defined(UNICODE) || defined(_UNICODE)
#define CreateEvent CreateEventW
#else
#define CreateEvent CreateEventA
#endif
BOOL  SetEvent(HANDLE hEvent) noexcept;
BOOL  ResetEvent(HANDLE hEvent) noexcept;
DWORD WaitForSingleObject(HANDLE hHandle, DWORD dwMilliseconds) noexcept;
BOOL  CloseHandle(HANDLE hObject) noexcept;

inline LONG InterlockedIncrement(volatile LONG* Addend) noexcept
{
    return __atomic_add_fetch(Addend, 1, __ATOMIC_SEQ_CST);
}
inline LONG InterlockedDecrement(volatile LONG* Addend) noexcept
{
    return __atomic_sub_fetch(Addend, 1, __ATOMIC_SEQ_CST);
}
inline LONG InterlockedExchange(volatile LONG* Target, LONG Value) noexcept
{
    return __atomic_exchange_n(Target, Value, __ATOMIC_SEQ_CST);
}
inline LONG InterlockedExchangeAdd(volatile LONG* Addend, LONG Value) noexcept
{
    return __atomic_fetch_add(Addend, Value, __ATOMIC_SEQ_CST);
}
inline LONG InterlockedCompareExchange(volatile LONG* Destination, LONG ExChange, LONG Comperand) noexcept
{
    LONG expected = Comperand;
    __atomic_compare_exchange_n(Destination, &expected, ExChange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}
inline LONGLONG InterlockedIncrement64(volatile LONGLONG* Addend) noexcept
{
    return __atomic_add_fetch(Addend, 1, __ATOMIC_SEQ_CST);
}
inline LONGLONG InterlockedDecrement64(volatile LONGLONG* Addend) noexcept
{
    return __atomic_sub_fetch(Addend, 1, __ATOMIC_SEQ_CST);
}
inline LONGLONG InterlockedExchange64(volatile LONGLONG* Target, LONGLONG Value) noexcept
{
    return __atomic_exchange_n(Target, Value, __ATOMIC_SEQ_CST);
}

#endif // !_WIN32
