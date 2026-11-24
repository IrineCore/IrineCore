// =============================================================================
// IrineCore — common/platform/interfaces/PlatformTypes.h
// Cross-platform fundamental types & Win32/POSIX type compatibility layer.
// Enforces 2-byte WCHAR (UTF-16LE) across Windows and Linux (-fshort-wchar).
// =============================================================================
#pragma once

#include <cstdint>
#include <cstddef>
#include <cstring>

#if defined(_WIN32)

#ifndef WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#else
#include <winsock2.h>
#include <windows.h>
#endif

static_assert(sizeof(WCHAR) == 2, "WCHAR must be 2-byte UTF-16LE on Windows");

#else // Linux / POSIX

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <pthread.h>
#include <wchar.h>

// Calling convention macros (no-op on x86_64 Linux System V ABI)
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __fastcall
#define __fastcall
#endif
#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef APIENTRY
#define APIENTRY
#endif

// MSVC fixed-width integer aliases (macros so "unsigned __int64" is valid and distinct from "long")
#ifndef __int8
#define __int8  char
#endif
#ifndef __int16
#define __int16 short
#endif
#ifndef __int32
#define __int32 int
#endif
#ifndef __int64
#define __int64 long long
#endif
#ifndef _int8
#define _int8   char
#endif
#ifndef _int16
#define _int16  short
#endif
#ifndef _int32
#define _int32  int
#endif
#ifndef _int64
#define _int64  long long
#endif

using BYTE      = uint8_t;
using WORD      = uint16_t;
using DWORD     = uint32_t;
using QWORD     = uint64_t;
using SHORT     = int16_t;
using USHORT    = uint16_t;
using INT       = int32_t;
using UINT      = uint32_t;
using LONG      = int32_t;
using ULONG     = uint32_t;
using LONGLONG  = int64_t;
using ULONGLONG = uint64_t;
using INT8      = int8_t;
using UINT8     = uint8_t;
using INT16     = int16_t;
using UINT16    = uint16_t;
using INT32     = int32_t;
using UINT32    = uint32_t;
using INT64     = int64_t;
using UINT64    = uint64_t;
using DWORD_PTR = uintptr_t;
using ULONG_PTR = uintptr_t;
using LONG_PTR  = intptr_t;
using SIZE_T    = size_t;
using SSIZE_T   = ssize_t;
using FLOAT     = float;
using DOUBLE    = double;
using BOOL      = int32_t;
using BOOLEAN   = uint8_t;
using CHAR      = char;
using UCHAR     = unsigned char;
using WCHAR     = wchar_t;

static_assert(sizeof(WCHAR) == 2, "IrineCore on Linux requires -fshort-wchar so WCHAR is 2-byte UTF-16LE");

#if defined(UNICODE) || defined(_UNICODE)
using TCHAR = WCHAR;
#ifndef _T
#define _T(x) L##x
#endif
#else
using TCHAR = char;
#ifndef _T
#define _T(x) x
#endif
#endif

#ifndef TEXT
#define TEXT(x) _T(x)
#endif

using LPSTR     = char*;
using LPCSTR    = const char*;
using LPWSTR    = WCHAR*;
using LPCWSTR   = const WCHAR*;
using LPTSTR    = TCHAR*;
using LPCTSTR   = const TCHAR*;
using LPVOID    = void*;
using LPCVOID   = const void*;
using LPDWORD   = DWORD*;
using LPBYTE    = BYTE*;
using LPWORD    = WORD*;

using HANDLE    = void*;
using HMODULE   = void*;
using HINSTANCE = void*;
using HWND      = void*;
using HRESULT   = int32_t;
using errno_t   = int;

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef S_OK
#define S_OK    static_cast<HRESULT>(0)
#define E_FAIL  static_cast<HRESULT>(0x80004005L)
#define SUCCEEDED(hr) (static_cast<HRESULT>(hr) >= 0)
#define FAILED(hr)    (static_cast<HRESULT>(hr) < 0)
#endif

#define INVALID_HANDLE_VALUE reinterpret_cast<HANDLE>(static_cast<intptr_t>(-1))

#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#ifndef _MAX_PATH
#define _MAX_PATH 260
#endif
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#define _MAX_DIR   256
#define _MAX_FNAME 256
#define _MAX_EXT   256
#endif

#ifndef INFINITE
#define INFINITE 0xFFFFFFFFu
#define WAIT_OBJECT_0 0x00000000u
#define WAIT_TIMEOUT  0x00000102u
#define WAIT_FAILED   0xFFFFFFFFu
#endif

#ifndef CP_ACP
#define CP_ACP  0
#define CP_UTF8 65001
#endif

using SOCKET = int;
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR   (-1)
#endif
using SOCKADDR    = struct sockaddr;
using SOCKADDR_IN = struct sockaddr_in;
using IN_ADDR     = struct in_addr;
using LPHOSTENT   = struct hostent*;

struct WSABUF
{
    ULONG len;
    CHAR* buf;
};

struct OVERLAPPED
{
    ULONG_PTR Internal;
    ULONG_PTR InternalHigh;
    union
    {
        struct
        {
            DWORD Offset;
            DWORD OffsetHigh;
        };
        LPVOID Pointer;
    };
    HANDLE hEvent;
};
using LPOVERLAPPED = OVERLAPPED*;

union LARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        LONG  HighPart;
    };
    struct
    {
        DWORD LowPart;
        LONG  HighPart;
    } u;
    LONGLONG QuadPart;
};

union ULARGE_INTEGER
{
    struct
    {
        DWORD LowPart;
        DWORD HighPart;
    };
    struct
    {
        DWORD LowPart;
        DWORD HighPart;
    } u;
    ULONGLONG QuadPart;
};

struct FILETIME
{
    DWORD dwLowDateTime;
    DWORD dwHighDateTime;
};

struct SYSTEMTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
};

struct GUID
{
    uint32_t Data1;
    uint16_t Data2;
    uint16_t Data3;
    uint8_t  Data4[8];
};

struct CRITICAL_SECTION
{
    pthread_mutex_t mutex;
};
using LPCRITICAL_SECTION = CRITICAL_SECTION*;

#ifndef _countof
template <typename T, size_t N>
constexpr size_t _countof(const T (&)[N]) noexcept { return N; }
#endif

#ifndef ZeroMemory
#define ZeroMemory(Destination, Length) std::memset((Destination), 0, (Length))
#define CopyMemory(Destination, Source, Length) std::memcpy((Destination), (Source), (Length))
#define MoveMemory(Destination, Source, Length) std::memmove((Destination), (Source), (Length))
#define FillMemory(Destination, Length, Fill) std::memset((Destination), (Fill), (Length))
#endif

#ifndef MAKEWORD
#define MAKEWORD(a, b) (static_cast<WORD>(static_cast<BYTE>(static_cast<DWORD_PTR>(a) & 0xff)) | (static_cast<WORD>(static_cast<BYTE>(static_cast<DWORD_PTR>(b) & 0xff)) << 8))
#define MAKELONG(a, b) (static_cast<LONG>(static_cast<WORD>(static_cast<DWORD_PTR>(a) & 0xffff)) | (static_cast<LONG>(static_cast<WORD>(static_cast<DWORD_PTR>(b) & 0xffff)) << 16))
#define LOWORD(l)      (static_cast<WORD>(static_cast<DWORD_PTR>(l) & 0xffff))
#define HIWORD(l)      (static_cast<WORD>((static_cast<DWORD_PTR>(l) >> 16) & 0xffff))
#define LOBYTE(w)      (static_cast<BYTE>(static_cast<DWORD_PTR>(w) & 0xff))
#define HIBYTE(w)      (static_cast<BYTE>((static_cast<DWORD_PTR>(w) >> 8) & 0xff))
#endif

#ifndef closesocket
inline int closesocket(SOCKET s) { return ::close(s); }
#endif
inline int WSAGetLastError() { return errno; }
inline void WSASetLastError(int err) { errno = err; }

#endif // _WIN32
