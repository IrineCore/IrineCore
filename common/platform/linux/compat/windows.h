// =============================================================================
// IrineCore — common/platform/linux/compat/windows.h
// Linux compatibility umbrella routing legacy <windows.h> includes to
// IrineCore's Platform Abstraction Layer (common/platform/Platform.h).
// Strictly invisible on Windows/MSVC builds.
// =============================================================================
#pragma once

#if defined(_WIN32)
#error "Linux compat/windows.h must never be included on Windows targets."
#endif

#include "../../Platform.h"

// Tell unixODBC <sqltypes.h> that Windows types are already defined and SQLWCHAR == wchar_t (2-byte UTF-16LE under -fshort-wchar)
#ifndef ALREADY_HAVE_WINDOWS_TYPE
#define ALREADY_HAVE_WINDOWS_TYPE 1
#endif
#ifndef SQL_WCHART_CONVERT
#define SQL_WCHART_CONVERT 1
#endif
#ifndef SQL_API
#define SQL_API
#endif
using SCHAR    = signed char;
using SQLSCHAR = signed char;
using SDWORD   = int;
using UDWORD   = unsigned int;
using SWORD    = signed short;
using UWORD    = unsigned short;
using SLONG    = signed long;
using SSHORT   = signed short;
using SDOUBLE  = double;
using LDOUBLE  = double;
using SFLOAT   = float;
using PTR      = void*;
using RETCODE  = signed short;
using SQLHWND  = void*;

#include <cstdint>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <cmath>
#include <climits>
#include <cerrno>
#include <cwchar>
#include <cwctype>
#include <type_traits>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <dlfcn.h>

// -----------------------------------------------------------------------------
// Additional Win32 primitive types & calling convention qualifiers
// -----------------------------------------------------------------------------
#ifndef CONST
#define CONST const
#endif
#ifndef VOID
#define VOID void
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef OPTIONAL
#define OPTIONAL
#endif
#ifndef FAR
#define FAR
#endif
#ifndef NEAR
#define NEAR
#endif
#ifndef PASCAL
#define PASCAL
#endif
#ifndef NTAPI
#define NTAPI
#endif
#ifndef STDMETHODCALLTYPE
#define STDMETHODCALLTYPE
#endif
#ifndef DECLSPEC_NOVTABLE
#define DECLSPEC_NOVTABLE
#endif
#ifndef FORCEINLINE
#define FORCEINLINE inline __attribute__((always_inline))
#endif

using UINT_PTR    = uintptr_t;
using INT_PTR     = intptr_t;
using WPARAM      = UINT_PTR;
using LPARAM      = LONG_PTR;
using LRESULT     = LONG_PTR;
using ATOM        = WORD;
using COLORREF    = DWORD;
using LCID        = DWORD;
using LANGID      = WORD;
using ACCESS_MASK = DWORD;
using REGSAM      = ACCESS_MASK;
using HKEY        = void*;
using PHKEY       = HKEY*;
using HDC         = void*;
using HBRUSH      = void*;
using HPEN        = void*;
using HFONT       = void*;
using HICON       = void*;
using HCURSOR     = void*;
using HMENU       = void*;
using HBITMAP     = void*;
using HGLOBAL     = void*;
using HLOCAL      = void*;
using HRSRC       = void*;
using HINTERNET   = void*;
using SC_HANDLE   = void*;
using PVOID       = void*;
using PBYTE       = BYTE*;
using PWORD       = WORD*;
using PDWORD      = DWORD*;
using PULONG      = ULONG*;
using PLONG       = LONG*;
using PCHAR       = CHAR*;
using PUCHAR      = UCHAR*;
using PWCHAR      = WCHAR*;
using PSTR        = CHAR*;
using PCSTR       = const CHAR*;
using PWSTR       = WCHAR*;
using PCWSTR      = const WCHAR*;
using PTSTR       = TCHAR*;
using PCTSTR      = const TCHAR*;
using LPUINT      = UINT*;
using PUINT       = UINT*;
using LPINT       = INT*;
using PINT        = INT*;
using LPLONG      = LONG*;
using LPBOOL      = BOOL*;
using PULONG_PTR  = ULONG_PTR*;
using LPSOCKADDR  = SOCKADDR*;
using LPHANDLE    = HANDLE*;
using PHOSTENT    = LPHOSTENT;
using byte        = unsigned char;
using LPFILETIME   = FILETIME*;
using LPSYSTEMTIME = SYSTEMTIME*;
using PLARGE_INTEGER  = LARGE_INTEGER*;
using PULARGE_INTEGER = ULARGE_INTEGER*;
using LPWSAOVERLAPPED = OVERLAPPED*;
using WSAOVERLAPPED   = OVERLAPPED;
using LPWSABUF        = WSABUF*;
using WSAEVENT        = HANDLE;
using FARPROC         = void*;
using INT64_PTR       = int64_t;
using DWORD64         = uint64_t;
using ULONG32         = uint32_t;
using ULONG64         = uint64_t;
using LONG64          = int64_t;
using HKL             = void*;
struct IBindStatusCallback
{
    virtual ~IBindStatusCallback() = default;
    virtual HRESULT OnProgress(ULONG, ULONG, ULONG, LPCWSTR) { return S_OK; }
};
using LPBINDSTATUSCALLBACK = IBindStatusCallback*;
#ifndef BINDSTATUS_DOWNLOADINGDATA
#define BINDSTATUS_DOWNLOADINGDATA 5u
#define BINDSTATUS_ENDDOWNLOADDATA 6u
#endif
using __time64_t      = int64_t;
using __time32_t      = int32_t;
using LINGER          = struct linger;
using LPTHREAD_START_ROUTINE = DWORD (*)(LPVOID);
using PTHREAD_START_ROUTINE  = LPTHREAD_START_ROUTINE;

#ifndef MAKEINTRESOURCEA
#define MAKEINTRESOURCEA(i) ((LPSTR)((ULONG_PTR)((WORD)(i))))
#define MAKEINTRESOURCEW(i) ((LPWSTR)((ULONG_PTR)((WORD)(i))))
#endif
#ifndef MAKELANGID
#define MAKELANGID(p, s) ((((WORD)(s)) << 10) | (WORD)(p))
#define LANG_NEUTRAL 0x00
#define SUBLANG_DEFAULT 0x01
#endif
#ifndef SORT_DEFAULT
#define SORT_DEFAULT 0x0
#endif
#ifndef MAKELCID
#define MAKELCID(lgid, srtid) ((DWORD)((((DWORD)((WORD)(srtid))) << 16) | ((DWORD)((WORD)(lgid)))))
#endif
#ifndef LOCALE_IDEFAULTANSICODEPAGE
#define LOCALE_IDEFAULTANSICODEPAGE 0x00001004u
#define LOCALE_SABBREVLANGNAME      0x00000003u
#endif
#ifndef LANG_SYSTEM_DEFAULT
#define LANG_SYSTEM_DEFAULT 0x0800u
#endif
#ifndef MAKELONGLONG
#define MAKELONGLONG(lo, hi) ((static_cast<uint64_t>(static_cast<uint32_t>(hi)) << 32) | static_cast<uint32_t>(lo))
#endif

// Common Win32 / WinSock constants
#ifndef ERROR_SUCCESS
#define ERROR_SUCCESS           0L
#define NO_ERROR                0L
#define ERROR_FILE_NOT_FOUND    2L
#define ERROR_PATH_NOT_FOUND    3L
#define ERROR_ACCESS_DENIED     5L
#define ERROR_INVALID_HANDLE    6L
#define ERROR_NOT_ENOUGH_MEMORY 8L
#define ERROR_NO_MORE_FILES     18L
#define ERROR_SHARING_VIOLATION 32L
#define ERROR_NETNAME_DELETED   64L
#define ERROR_FILE_EXISTS       80L
#define ERROR_INVALID_PARAMETER 87L
#define ERROR_INSUFFICIENT_BUFFER 122L
#define ERROR_ALREADY_EXISTS    183L
#define ERROR_MORE_DATA         234L
#define ERROR_NOT_FOUND         1168L
#define WAIT_ABANDONED          0x00000080u
#define ERROR_IO_PENDING        997L
#define WSA_IO_PENDING          ERROR_IO_PENDING
#define WSAEWOULDBLOCK          EWOULDBLOCK
#define WSAEINPROGRESS          EINPROGRESS
#define WSAEALREADY             EALREADY
#define WSAENOTSOCK             ENOTSOCK
#define WSAEMSGSIZE             EMSGSIZE
#define WSAEADDRINUSE           EADDRINUSE
#define WSAEADDRNOTAVAIL        EADDRNOTAVAIL
#define WSAENETDOWN             ENETDOWN
#define WSAENETUNREACH          ENETUNREACH
#define WSAECONNABORTED         ECONNABORTED
#define WSAECONNRESET           ECONNRESET
#define WSAENOBUFS              ENOBUFS
#define WSAEISCONN              EISCONN
#define WSAENOTCONN             ENOTCONN
#define WSAETIMEDOUT            ETIMEDOUT
#define WSAECONNREFUSED         ECONNREFUSED
#define WSAEINTR                EINTR
#define SD_RECEIVE              SHUT_RD
#define SD_SEND                 SHUT_WR
#define SD_BOTH                 SHUT_RDWR
#ifndef FIONBIO
#define FIONBIO                 0x5421
#endif
#ifndef INADDR_NONE
#define INADDR_NONE             ((in_addr_t)0xFFFFFFFF)
#endif
#endif

// File access / creation / attribute constants
#ifndef GENERIC_READ
#define GENERIC_READ            0x80000000u
#define GENERIC_WRITE           0x40000000u
#define GENERIC_EXECUTE         0x20000000u
#define GENERIC_ALL             0x10000000u
#define FILE_SHARE_READ         0x00000001u
#define FILE_SHARE_WRITE        0x00000002u
#define FILE_SHARE_DELETE       0x00000004u
#define CREATE_NEW              1u
#define CREATE_ALWAYS           2u
#define OPEN_EXISTING           3u
#define OPEN_ALWAYS             4u
#define TRUNCATE_EXISTING       5u
#define FILE_ATTRIBUTE_READONLY   0x00000001u
#define FILE_ATTRIBUTE_HIDDEN     0x00000002u
#define FILE_ATTRIBUTE_SYSTEM     0x00000004u
#define FILE_ATTRIBUTE_DIRECTORY  0x00000010u
#define FILE_ATTRIBUTE_ARCHIVE    0x00000020u
#define FILE_ATTRIBUTE_NORMAL     0x00000080u
#define FILE_FLAG_OVERLAPPED      0x40000000u
#define FILE_FLAG_SEQUENTIAL_SCAN 0x08000000u
#define INVALID_FILE_ATTRIBUTES   ((DWORD)-1)
#define INVALID_SET_FILE_POINTER  ((DWORD)-1)
#define INVALID_FILE_SIZE         ((DWORD)-1)
#define FILE_BEGIN              0u
#define FILE_CURRENT            1u
#define FILE_END                2u
#define PAGE_READONLY           0x02u
#define PAGE_READWRITE          0x04u
#define PAGE_WRITECOPY          0x08u
#define FILE_MAP_COPY           0x0001u
#define FILE_MAP_WRITE          0x0002u
#define FILE_MAP_READ           0x0004u
#define FILE_MAP_ALL_ACCESS     0x001Fu
#define FILE_READ_ATTRIBUTES    0x0080u
#define COPY_FILE_FAIL_IF_EXISTS 0x00000001u
#define VER_PLATFORM_WIN32_WINDOWS 1u
#define VER_PLATFORM_WIN32_NT      2u
#define STARTF_USESTDHANDLES       0x00000100u
#define NORMAL_PRIORITY_CLASS      0x00000020u
#define CREATE_NO_WINDOW           0x08000000u
#endif

#ifndef EXCEPTION_ACCESS_VIOLATION
#define EXCEPTION_ACCESS_VIOLATION 0xC0000005u
#define EXCEPTION_NONCONTINUABLE   0x1u
#endif
#ifndef EXCEPTION_STACK_OVERFLOW
#define EXCEPTION_STACK_OVERFLOW   0xC00000FDu
#endif
#ifndef IMAGE_FILE_MACHINE_AMD64
#define IMAGE_FILE_MACHINE_I386    0x014Cu
#define IMAGE_FILE_MACHINE_IA64    0x0200u
#define IMAGE_FILE_MACHINE_AMD64   0x8664u
#endif
#ifndef MB_SERVICE_NOTIFICATION
#define MB_SERVICE_NOTIFICATION    0x00200000u
#endif
#ifndef HKEY_LOCAL_MACHINE
#define HKEY_CLASSES_ROOT          ((HKEY)(ULONG_PTR)0x80000000u)
#define HKEY_CURRENT_USER          ((HKEY)(ULONG_PTR)0x80000001u)
#define HKEY_LOCAL_MACHINE         ((HKEY)(ULONG_PTR)0x80000002u)
#define HKEY_USERS                 ((HKEY)(ULONG_PTR)0x80000003u)
#define REG_NONE                   0u
#define REG_SZ                     1u
#define REG_EXPAND_SZ              2u
#define REG_BINARY                 3u
#define REG_DWORD                  4u
#define REG_OPTION_NON_VOLATILE    0x00000000u
#define REG_CREATED_NEW_KEY        0x00000001u
#define REG_OPENED_EXISTING_KEY    0x00000002u
#define KEY_QUERY_VALUE            0x0001u
#define KEY_SET_VALUE              0x0002u
#define KEY_CREATE_SUB_KEY         0x0004u
#define KEY_ENUMERATE_SUB_KEYS     0x0008u
#define KEY_NOTIFY                 0x0010u
#define KEY_CREATE_LINK            0x0020u
#define KEY_READ                   0x20019u
#define KEY_WRITE                  0x20006u
#define KEY_ALL_ACCESS             0xF003Fu
#endif

// MessageBox / Console / Misc flags
#ifndef MB_OK
#define MB_OK               0x00000000u
#define MB_OKCANCEL         0x00000001u
#define MB_YESNO            0x00000004u
#define MB_ICONHAND         0x00000010u
#define MB_ICONERROR        MB_ICONHAND
#define MB_ICONSTOP         MB_ICONHAND
#define MB_ICONQUESTION     0x00000020u
#define MB_ICONEXCLAMATION  0x00000030u
#define MB_ICONWARNING      MB_ICONEXCLAMATION
#define MB_ICONASTERISK     0x00000040u
#define MB_ICONINFORMATION  MB_ICONASTERISK
#define IDOK                1
#define IDCANCEL            2
#define IDYES               6
#define IDNO                7
#define SW_HIDE             0
#define SW_SHOWNORMAL       1
#define SW_SHOW             5
#define STD_INPUT_HANDLE    ((DWORD)-10)
#define STD_OUTPUT_HANDLE   ((DWORD)-11)
#define STD_ERROR_HANDLE    ((DWORD)-12)
#define FOREGROUND_BLUE      0x0001
#define FOREGROUND_GREEN     0x0002
#define FOREGROUND_RED       0x0004
#define FOREGROUND_INTENSITY 0x0008
#define BACKGROUND_BLUE      0x0010
#define BACKGROUND_GREEN     0x0020
#define BACKGROUND_RED       0x0040
#define BACKGROUND_INTENSITY 0x0080
#define FORMAT_MESSAGE_ALLOCATE_BUFFER 0x00000100u
#define FORMAT_MESSAGE_IGNORE_INSERTS  0x00000200u
#define FORMAT_MESSAGE_FROM_SYSTEM     0x00001000u
#define THREAD_PRIORITY_LOWEST         (-2)
#define THREAD_PRIORITY_TIME_CRITICAL  15
#define STILL_ACTIVE                   259u
#define MAXIMUM_WAIT_OBJECTS           64u
#define WSA_FLAG_OVERLAPPED            0x01u
#ifndef SOMAXCONN
#define SOMAXCONN                      128
#endif
#endif

// -----------------------------------------------------------------------------
// Win32 / WinSock structures
// -----------------------------------------------------------------------------
struct POINT { LONG x; LONG y; };
struct SIZE  { LONG cx; LONG cy; };
struct RECT  { LONG left; LONG top; LONG right; LONG bottom; };

struct SECURITY_ATTRIBUTES
{
    DWORD  nLength;
    LPVOID lpSecurityDescriptor;
    BOOL   bInheritHandle;
};
using LPSECURITY_ATTRIBUTES = SECURITY_ATTRIBUTES*;

struct SYSTEM_INFO
{
    WORD      wProcessorArchitecture;
    WORD      wReserved;
    DWORD     dwPageSize;
    LPVOID    lpMinimumApplicationAddress;
    LPVOID    lpMaximumApplicationAddress;
    DWORD_PTR dwActiveProcessorMask;
    DWORD     dwNumberOfProcessors;
    DWORD     dwProcessorType;
    DWORD     dwAllocationGranularity;
    WORD      wProcessorLevel;
    WORD      wProcessorRevision;
};
using LPSYSTEM_INFO = SYSTEM_INFO*;

struct MEMORYSTATUS
{
    DWORD  dwLength;
    DWORD  dwMemoryLoad;
    SIZE_T dwTotalPhys;
    SIZE_T dwAvailPhys;
    SIZE_T dwTotalPageFile;
    SIZE_T dwAvailPageFile;
    SIZE_T dwTotalVirtual;
    SIZE_T dwAvailVirtual;
};
using LPMEMORYSTATUS = MEMORYSTATUS*;

struct MEMORYSTATUSEX
{
    DWORD     dwLength;
    DWORD     dwMemoryLoad;
    DWORD64   ullTotalPhys;
    DWORD64   ullAvailPhys;
    DWORD64   ullTotalPageFile;
    DWORD64   ullAvailPageFile;
    DWORD64   ullTotalVirtual;
    DWORD64   ullAvailVirtual;
    DWORD64   ullAvailExtendedVirtual;
};
using LPMEMORYSTATUSEX = MEMORYSTATUSEX*;

struct WIN32_FIND_DATAA
{
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    CHAR     cFileName[MAX_PATH];
    CHAR     cAlternateFileName[14];
};
using LPWIN32_FIND_DATAA = WIN32_FIND_DATAA*;

struct WIN32_FIND_DATAW
{
    DWORD    dwFileAttributes;
    FILETIME ftCreationTime;
    FILETIME ftLastAccessTime;
    FILETIME ftLastWriteTime;
    DWORD    nFileSizeHigh;
    DWORD    nFileSizeLow;
    DWORD    dwReserved0;
    DWORD    dwReserved1;
    WCHAR    cFileName[MAX_PATH];
    WCHAR    cAlternateFileName[14];
};
using LPWIN32_FIND_DATAW = WIN32_FIND_DATAW*;

#if defined(UNICODE) || defined(_UNICODE)
using WIN32_FIND_DATA   = WIN32_FIND_DATAW;
using LPWIN32_FIND_DATA = LPWIN32_FIND_DATAW;
#else
using WIN32_FIND_DATA   = WIN32_FIND_DATAA;
using LPWIN32_FIND_DATA = LPWIN32_FIND_DATAA;
#endif

struct WSADATA
{
    WORD wVersion;
    WORD wHighVersion;
    char szDescription[257];
    char szSystemStatus[129];
    unsigned short iMaxSockets;
    unsigned short iMaxUdpDg;
    char* lpVendorInfo;
};
using LPWSADATA = WSADATA*;

struct WSANETWORKEVENTS
{
    long lNetworkEvents;
    int  iErrorCode[10];
};
using LPWSANETWORKEVENTS = WSANETWORKEVENTS*;

struct COORD { SHORT X; SHORT Y; };
struct SMALL_RECT { SHORT Left; SHORT Top; SHORT Right; SHORT Bottom; };
struct CONSOLE_SCREEN_BUFFER_INFO
{
    COORD dwSize;
    COORD dwCursorPosition;
    WORD  wAttributes;
    SMALL_RECT srWindow;
    COORD dwMaximumWindowSize;
};

struct EXCEPTION_RECORD
{
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    EXCEPTION_RECORD* ExceptionRecord;
    PVOID ExceptionAddress;
    DWORD NumberParameters;
    ULONG_PTR ExceptionInformation[15];
};
struct CONTEXT { DWORD ContextFlags; };
struct EXCEPTION_POINTERS
{
    EXCEPTION_RECORD* ExceptionRecord;
    CONTEXT* ContextRecord;
};
using _EXCEPTION_POINTERS  = EXCEPTION_POINTERS;
using LPEXCEPTION_POINTERS = EXCEPTION_POINTERS*;
using PEXCEPTION_POINTERS  = EXCEPTION_POINTERS*;

struct OSVERSIONINFOA
{
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR  szCSDVersion[128];
};
struct OSVERSIONINFOW
{
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    WCHAR szCSDVersion[128];
};
struct OSVERSIONINFOEXA : OSVERSIONINFOA
{
    WORD wServicePackMajor;
    WORD wServicePackMinor;
    WORD wSuiteMask;
    BYTE wProductType;
    BYTE wReserved;
};
struct OSVERSIONINFOEXW : OSVERSIONINFOW
{
    WORD wServicePackMajor;
    WORD wServicePackMinor;
    WORD wSuiteMask;
    BYTE wProductType;
    BYTE wReserved;
};
struct VS_FIXEDFILEINFO
{
    DWORD dwSignature;
    DWORD dwStrucVersion;
    DWORD dwFileVersionMS;
    DWORD dwFileVersionLS;
    DWORD dwProductVersionMS;
    DWORD dwProductVersionLS;
    DWORD dwFileFlagsMask;
    DWORD dwFileFlags;
    DWORD dwFileOS;
    DWORD dwFileType;
    DWORD dwFileSubtype;
    DWORD dwFileDateMS;
    DWORD dwFileDateLS;
};
struct STARTUPINFOA
{
    DWORD  cb;
    LPSTR  lpReserved;
    LPSTR  lpDesktop;
    LPSTR  lpTitle;
    DWORD  dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
    WORD   wShowWindow, cbReserved2;
    LPBYTE lpReserved2;
    HANDLE hStdInput, hStdOutput, hStdError;
};
struct STARTUPINFOW
{
    DWORD  cb;
    LPWSTR lpReserved;
    LPWSTR lpDesktop;
    LPWSTR lpTitle;
    DWORD  dwX, dwY, dwXSize, dwYSize, dwXCountChars, dwYCountChars, dwFillAttribute, dwFlags;
    WORD   wShowWindow, cbReserved2;
    LPBYTE lpReserved2;
    HANDLE hStdInput, hStdOutput, hStdError;
};
struct PROCESS_INFORMATION
{
    HANDLE hProcess;
    HANDLE hThread;
    DWORD  dwProcessId;
    DWORD  dwThreadId;
};
using LPPROCESS_INFORMATION = PROCESS_INFORMATION*;
#if defined(UNICODE) || defined(_UNICODE)
using OSVERSIONINFO   = OSVERSIONINFOW;
using OSVERSIONINFOEX = OSVERSIONINFOEXW;
using STARTUPINFO     = STARTUPINFOW;
#else
using OSVERSIONINFO   = OSVERSIONINFOA;
using OSVERSIONINFOEX = OSVERSIONINFOEXA;
using STARTUPINFO     = STARTUPINFOA;
#endif

// -----------------------------------------------------------------------------
// Win32 / POSIX compatibility functions (implemented in LinuxPosixCompat.cpp)
// -----------------------------------------------------------------------------
DWORD  GetLastError() noexcept;
void   SetLastError(DWORD dwErrCode) noexcept;
void   GetSystemInfo(LPSYSTEM_INFO lpSystemInfo) noexcept;
void   GlobalMemoryStatus(LPMEMORYSTATUS lpBuffer) noexcept;
BOOL   GlobalMemoryStatusEx(LPMEMORYSTATUSEX lpBuffer) noexcept;
BOOL   FileTimeToSystemTime(const FILETIME* lpFileTime, LPSYSTEMTIME lpSystemTime) noexcept;
BOOL   SystemTimeToFileTime(const SYSTEMTIME* lpSystemTime, LPFILETIME lpFileTime) noexcept;
BOOL   FileTimeToLocalFileTime(const FILETIME* lpFileTime, LPFILETIME lpLocalFileTime) noexcept;
LONG   CompareFileTime(const FILETIME* lpFileTime1, const FILETIME* lpFileTime2) noexcept;
inline PVOID SecureZeroMemory(PVOID ptr, SIZE_T cnt) noexcept { if (ptr && cnt) { volatile unsigned char* p = static_cast<volatile unsigned char*>(ptr); while (cnt--) *p++ = 0; } return ptr; }
int    wsprintfA(LPSTR lpOut, LPCSTR lpFmt, ...) noexcept;
int    wsprintfW(LPWSTR lpOut, LPCWSTR lpFmt, ...) noexcept;
char*  _strrev(char* str) noexcept;
int    _wsplitpath(const WCHAR* path, WCHAR* drive, WCHAR* dir, WCHAR* fname, WCHAR* ext) noexcept;
inline HKL GetKeyboardLayout(DWORD) noexcept { return nullptr; }
int    GetLocaleInfoA(LCID Locale, DWORD LCType, LPSTR lpLCData, int cchData) noexcept;
int    GetLocaleInfoW(LCID Locale, DWORD LCType, LPWSTR lpLCData, int cchData) noexcept;
LONG   RegOpenKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) noexcept;
LONG   RegOpenKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD ulOptions, REGSAM samDesired, PHKEY phkResult) noexcept;
LONG   RegCreateKeyExA(HKEY hKey, LPCSTR lpSubKey, DWORD Reserved, LPSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition) noexcept;
LONG   RegCreateKeyExW(HKEY hKey, LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition) noexcept;
LONG   RegQueryValueExA(HKEY hKey, LPCSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) noexcept;
LONG   RegQueryValueExW(HKEY hKey, LPCWSTR lpValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData) noexcept;
LONG   RegSetValueExA(HKEY hKey, LPCSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData) noexcept;
LONG   RegSetValueExW(HKEY hKey, LPCWSTR lpValueName, DWORD Reserved, DWORD dwType, const BYTE* lpData, DWORD cbData) noexcept;
LONG   RegEnumKeyExA(HKEY hKey, DWORD dwIndex, LPSTR lpName, LPDWORD lpcchName, LPDWORD lpReserved, LPSTR lpClass, LPDWORD lpcchClass, LPFILETIME lpftLastWriteTime) noexcept;
LONG   RegEnumKeyExW(HKEY hKey, DWORD dwIndex, LPWSTR lpName, LPDWORD lpcchName, LPDWORD lpReserved, LPWSTR lpClass, LPDWORD lpcchClass, LPFILETIME lpftLastWriteTime) noexcept;
LONG   RegDeleteKeyA(HKEY hKey, LPCSTR lpSubKey) noexcept;
LONG   RegDeleteKeyW(HKEY hKey, LPCWSTR lpSubKey) noexcept;
LONG   RegCloseKey(HKEY hKey) noexcept;
BOOL   CopyFileExA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, void* lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags) noexcept;
BOOL   CopyFileExW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, void* lpProgressRoutine, LPVOID lpData, LPBOOL pbCancel, DWORD dwCopyFlags) noexcept;
BOOL   LockFile(HANDLE hFile, DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh, DWORD nNumberOfBytesToLockLow, DWORD nNumberOfBytesToLockHigh) noexcept;
BOOL   UnlockFile(HANDLE hFile, DWORD dwFileOffsetLow, DWORD dwFileOffsetHigh, DWORD nNumberOfBytesToUnlockLow, DWORD nNumberOfBytesToUnlockHigh) noexcept;
BOOL   GetFileTime(HANDLE hFile, LPFILETIME lpCreationTime, LPFILETIME lpLastAccessTime, LPFILETIME lpLastWriteTime) noexcept;
inline DWORD GetFileVersionInfoSizeA(LPCSTR, LPDWORD) noexcept { return 0; }
inline DWORD GetFileVersionInfoSizeW(LPCWSTR, LPDWORD) noexcept { return 0; }
inline BOOL  GetFileVersionInfoA(LPCSTR, DWORD, DWORD, LPVOID) noexcept { return FALSE; }
inline BOOL  GetFileVersionInfoW(LPCWSTR, DWORD, DWORD, LPVOID) noexcept { return FALSE; }
inline BOOL  VerQueryValueA(LPCVOID, LPCSTR, LPVOID*, PUINT) noexcept { return FALSE; }
inline BOOL  VerQueryValueW(LPCVOID, LPCWSTR, LPVOID*, PUINT) noexcept { return FALSE; }
UINT   GetTempFileNameA(LPCSTR lpPathName, LPCSTR lpPrefixString, UINT uUnique, LPSTR lpTempFileName) noexcept;
UINT   GetTempFileNameW(LPCWSTR lpPathName, LPCWSTR lpPrefixString, UINT uUnique, LPWSTR lpTempFileName) noexcept;
BOOL   CreatePipe(HANDLE* hReadPipe, HANDLE* hWritePipe, LPSECURITY_ATTRIBUTES lpPipeAttributes, DWORD nSize) noexcept;
BOOL   CreateProcessA(LPCSTR lpApplicationName, LPSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCSTR lpCurrentDirectory, STARTUPINFOA* lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) noexcept;
BOOL   CreateProcessW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes, LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory, STARTUPINFOW* lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation) noexcept;
BOOL   GetDiskFreeSpaceA(LPCSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters) noexcept;
BOOL   GetDiskFreeSpaceW(LPCWSTR lpRootPathName, LPDWORD lpSectorsPerCluster, LPDWORD lpBytesPerSector, LPDWORD lpNumberOfFreeClusters, LPDWORD lpTotalNumberOfClusters) noexcept;

HANDLE CreateThread(LPSECURITY_ATTRIBUTES lpThreadAttributes, SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreationFlags, LPDWORD lpThreadId) noexcept;
BOOL   TerminateThread(HANDLE hThread, DWORD dwExitCode) noexcept;
BOOL   GetExitCodeThread(HANDLE hThread, LPDWORD lpExitCode) noexcept;
DWORD_PTR SetThreadAffinityMask(HANDLE hThread, DWORD_PTR dwThreadAffinityMask) noexcept;
inline BOOL SetThreadPriorityBoost(HANDLE, BOOL) noexcept { return TRUE; }
inline int  _isnan(double x) noexcept { return std::isnan(x) ? 1 : 0; }
inline int  _finite(double x) noexcept { return std::isfinite(x) ? 1 : 0; }
DWORD  WaitForMultipleObjects(DWORD nCount, const HANDLE* lpHandles, BOOL bWaitAll, DWORD dwMilliseconds) noexcept;
HANDLE CreateMutexA(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCSTR lpName) noexcept;
HANDLE CreateMutexW(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCWSTR lpName) noexcept;
BOOL   ReleaseMutex(HANDLE hMutex) noexcept;
HANDLE CreateSemaphoreA(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCSTR lpName) noexcept;
HANDLE CreateSemaphoreW(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCWSTR lpName) noexcept;
BOOL   ReleaseSemaphore(HANDLE hSemaphore, LONG lReleaseCount, LPLONG lpPreviousCount) noexcept;

HANDLE CreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) noexcept;
HANDLE CreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) noexcept;
BOOL   ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPOVERLAPPED lpOverlapped) noexcept;
BOOL   WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nNumberOfBytesToWrite, LPDWORD lpNumberOfBytesWritten, LPOVERLAPPED lpOverlapped) noexcept;
DWORD  GetFileSize(HANDLE hFile, LPDWORD lpFileSizeHigh) noexcept;
BOOL   GetFileSizeEx(HANDLE hFile, PLARGE_INTEGER lpFileSize) noexcept;
DWORD  SetFilePointer(HANDLE hFile, LONG lDistanceToMove, PLONG lpDistanceToMoveHigh, DWORD dwMoveMethod) noexcept;
BOOL   FlushFileBuffers(HANDLE hFile) noexcept;
BOOL   SetEndOfFile(HANDLE hFile) noexcept;
DWORD  GetFileAttributesA(LPCSTR lpFileName) noexcept;
DWORD  GetFileAttributesW(LPCWSTR lpFileName) noexcept;
BOOL   SetFileAttributesA(LPCSTR lpFileName, DWORD dwFileAttributes) noexcept;
BOOL   SetFileAttributesW(LPCWSTR lpFileName, DWORD dwFileAttributes) noexcept;
BOOL   MoveFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName) noexcept;
BOOL   MoveFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName) noexcept;
BOOL   RemoveDirectoryA(LPCSTR lpPathName) noexcept;
BOOL   RemoveDirectoryW(LPCWSTR lpPathName) noexcept;
HANDLE FindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) noexcept;
HANDLE FindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATAW lpFindFileData) noexcept;
BOOL   FindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData) noexcept;
BOOL   FindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData) noexcept;
BOOL   FindClose(HANDLE hFindFile) noexcept;

HANDLE CreateFileMappingA(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCSTR lpName) noexcept;
HANDLE CreateFileMappingW(HANDLE hFile, LPSECURITY_ATTRIBUTES lpAttributes, DWORD flProtect, DWORD dwMaximumSizeHigh, DWORD dwMaximumSizeLow, LPCWSTR lpName) noexcept;
LPVOID MapViewOfFile(HANDLE hFileMappingObject, DWORD dwDesiredAccess, DWORD dwFileOffsetHigh, DWORD dwFileOffsetLow, SIZE_T dwNumberOfBytesToMap) noexcept;
BOOL   UnmapViewOfFile(LPCVOID lpBaseAddress) noexcept;
inline BOOL FlushViewOfFile(LPCVOID, SIZE_T) noexcept { return TRUE; }
DWORD  GetFullPathNameA(LPCSTR lpFileName, DWORD nBufferLength, LPSTR lpBuffer, LPSTR* lpFilePart) noexcept;
DWORD  GetFullPathNameW(LPCWSTR lpFileName, DWORD nBufferLength, LPWSTR lpBuffer, LPWSTR* lpFilePart) noexcept;

// Win32 Heap APIs mapped to standard allocator on Linux
enum HEAP_INFORMATION_CLASS { HeapCompatibilityInformation = 0 };
inline HANDLE GetProcessHeap() noexcept { return reinterpret_cast<HANDLE>(static_cast<uintptr_t>(1)); }
inline HANDLE HeapCreate(DWORD, SIZE_T, SIZE_T) noexcept { return reinterpret_cast<HANDLE>(static_cast<uintptr_t>(1)); }
inline BOOL   HeapDestroy(HANDLE) noexcept { return TRUE; }
inline LPVOID HeapAlloc(HANDLE, DWORD dwFlags, SIZE_T dwBytes) noexcept { return (dwFlags & 0x08) ? std::calloc(1, dwBytes ? dwBytes : 1) : std::malloc(dwBytes ? dwBytes : 1); }
inline BOOL   HeapFree(HANDLE, DWORD, LPVOID lpMem) noexcept { std::free(lpMem); return TRUE; }
inline BOOL   HeapSetInformation(HANDLE, HEAP_INFORMATION_CLASS, PVOID, SIZE_T) noexcept { return TRUE; }
#ifndef HEAP_ZERO_MEMORY
#define HEAP_ZERO_MEMORY 0x00000008u
#endif

// CRT safe printf / locale / exception helpers
#ifndef _cdecl
#define _cdecl
#endif
#ifndef _stdcall
#define _stdcall
#endif
#ifndef _fastcall
#define _fastcall
#endif
#ifndef _thiscall
#define _thiscall
#endif
#ifndef _OUT_TO_DEFAULT
#define _OUT_TO_DEFAULT 0
#define _OUT_TO_STDERR  1
#define _OUT_TO_MSGBOX  2
#define _REPORT_ERRMODE 3
#endif
#ifndef VK_CANCEL
#define VK_CANCEL  0x03
#define VK_BACK    0x08
#define VK_TAB     0x09
#define VK_RETURN  0x0D
#define VK_SHIFT   0x10
#define VK_CONTROL 0x11
#define VK_MENU    0x12
#define VK_PAUSE   0x13
#define VK_CAPITAL 0x14
#define VK_ESCAPE  0x1B
#define VK_SPACE   0x20
#define VK_PRIOR   0x21
#define VK_NEXT    0x22
#define VK_END     0x23
#define VK_HOME    0x24
#define VK_LEFT    0x25
#define VK_UP      0x26
#define VK_RIGHT   0x27
#define VK_DOWN    0x28
#define VK_INSERT  0x2D
#define VK_DELETE  0x2E
#endif
inline void ExitProcess(UINT uExitCode) noexcept { std::exit(static_cast<int>(uExitCode)); }
inline void _fpreset() noexcept {}
inline int  _set_error_mode(int) noexcept { return 0; }
inline WCHAR* _wsetlocale(int, const WCHAR* locale) noexcept { return const_cast<WCHAR*>(locale); }
#ifndef fwprintf_s
#define fwprintf_s fwprintf
#define fprintf_s  fprintf
#define wprintf_s  wprintf
#define printf_s   printf
#endif
inline DWORD GetCurrentProcessorNumber() noexcept { return 0; }
inline void  RaiseException(DWORD, DWORD, DWORD, const ULONG_PTR*) noexcept {}
using LPTOP_LEVEL_EXCEPTION_FILTER = LONG (*)(EXCEPTION_POINTERS*);
inline LPTOP_LEVEL_EXCEPTION_FILTER SetUnhandledExceptionFilter(LPTOP_LEVEL_EXCEPTION_FILTER) noexcept { return nullptr; }
using _invalid_parameter_handler = void (*)(const wchar_t*, const wchar_t*, const wchar_t*, unsigned int, uintptr_t);
using _purecall_handler = void (*)();
inline _invalid_parameter_handler _set_invalid_parameter_handler(_invalid_parameter_handler) noexcept { return nullptr; }
inline _purecall_handler _set_purecall_handler(_purecall_handler) noexcept { return nullptr; }
inline DWORD GetExceptionCode() noexcept { return 0; }
inline EXCEPTION_POINTERS* GetExceptionInformation() noexcept { return nullptr; }

BOOL   WritePrivateProfileStringA(LPCSTR lpAppName, LPCSTR lpKeyName, LPCSTR lpString, LPCSTR lpFileName) noexcept;
BOOL   WritePrivateProfileStringW(LPCWSTR lpAppName, LPCWSTR lpKeyName, LPCWSTR lpString, LPCWSTR lpFileName) noexcept;

HMODULE LoadLibraryA(LPCSTR lpLibFileName) noexcept;
HMODULE LoadLibraryW(LPCWSTR lpLibFileName) noexcept;
FARPROC GetProcAddress(HMODULE hModule, LPCSTR lpProcName) noexcept;
BOOL    FreeLibrary(HMODULE hLibModule) noexcept;
HMODULE GetModuleHandleA(LPCSTR lpModuleName) noexcept;
HMODULE GetModuleHandleW(LPCWSTR lpModuleName) noexcept;

void   OutputDebugStringA(LPCSTR lpOutputString) noexcept;
void   OutputDebugStringW(LPCWSTR lpOutputString) noexcept;
int    MessageBoxA(HWND hWnd, LPCSTR lpText, LPCSTR lpCaption, UINT uType) noexcept;
int    MessageBoxW(HWND hWnd, LPCWSTR lpText, LPCWSTR lpCaption, UINT uType) noexcept;
DWORD  FormatMessageA(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPSTR lpBuffer, DWORD nSize, va_list* Arguments) noexcept;
DWORD  FormatMessageW(DWORD dwFlags, LPCVOID lpSource, DWORD dwMessageId, DWORD dwLanguageId, LPWSTR lpBuffer, DWORD nSize, va_list* Arguments) noexcept;
BOOL   GetComputerNameA(LPSTR lpBuffer, LPDWORD nSize) noexcept;
BOOL   GetComputerNameW(LPWSTR lpBuffer, LPDWORD nSize) noexcept;
BOOL   SetConsoleTitleA(LPCSTR lpConsoleTitle) noexcept;
BOOL   SetConsoleTitleW(LPCWSTR lpConsoleTitle) noexcept;
HANDLE GetStdHandle(DWORD nStdHandle) noexcept;
BOOL   SetConsoleTextAttribute(HANDLE hConsoleOutput, WORD wAttributes) noexcept;
BOOL   GetConsoleScreenBufferInfo(HANDLE hConsoleOutput, CONSOLE_SCREEN_BUFFER_INFO* lpConsoleScreenBufferInfo) noexcept;
BOOL   IsDebuggerPresent() noexcept;
void   DebugBreak() noexcept;
HANDLE GetCurrentProcess() noexcept;
BOOL   GetVersionExA(OSVERSIONINFOA* lpVersionInfo) noexcept;
BOOL   GetVersionExW(OSVERSIONINFOW* lpVersionInfo) noexcept;
HLOCAL LocalFree(HLOCAL hMem) noexcept;
LPVOID VirtualAlloc(LPVOID lpAddress, SIZE_T dwSize, DWORD flAllocationType, DWORD flProtect) noexcept;
BOOL   VirtualFree(LPVOID lpAddress, SIZE_T dwSize, DWORD dwFreeType) noexcept;

// WinSock / IOCP shims
int    WSAStartup(WORD wVersionRequested, LPWSADATA lpWSAData) noexcept;
int    WSACleanup() noexcept;
int    ioctlsocket(SOCKET s, long cmd, unsigned long* argp) noexcept;
SOCKET WSASocketA(int af, int type, int protocol, void* lpProtocolInfo, unsigned int g, DWORD dwFlags) noexcept;
SOCKET WSASocketW(int af, int type, int protocol, void* lpProtocolInfo, unsigned int g, DWORD dwFlags) noexcept;
int    WSASend(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesSent, DWORD dwFlags, LPWSAOVERLAPPED lpOverlapped, void* lpCompletionRoutine) noexcept;
int    WSARecv(SOCKET s, LPWSABUF lpBuffers, DWORD dwBufferCount, LPDWORD lpNumberOfBytesRecvd, LPDWORD lpFlags, LPWSAOVERLAPPED lpOverlapped, void* lpCompletionRoutine) noexcept;
HANDLE CreateIoCompletionPort(HANDLE FileHandle, HANDLE ExistingCompletionPort, ULONG_PTR CompletionKey, DWORD NumberOfConcurrentThreads) noexcept;
BOOL   GetQueuedCompletionStatus(HANDLE CompletionPort, LPDWORD lpNumberOfBytesTransferred, ULONG_PTR* lpCompletionKey, LPOVERLAPPED* lpOverlapped, DWORD dwMilliseconds) noexcept;
BOOL   PostQueuedCompletionStatus(HANDLE CompletionPort, DWORD dwNumberOfBytesTransferred, ULONG_PTR dwCompletionKey, LPOVERLAPPED lpOverlapped) noexcept;

// Extra Interlocked / CRT helpers
inline PVOID InterlockedExchangePointer(PVOID volatile* Target, PVOID Value) noexcept
{
    return __atomic_exchange_n(Target, Value, __ATOMIC_SEQ_CST);
}
inline PVOID InterlockedCompareExchangePointer(PVOID volatile* Destination, PVOID Exchange, PVOID Comperand) noexcept
{
    PVOID expected = Comperand;
    __atomic_compare_exchange_n(Destination, &expected, Exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}
inline LONGLONG InterlockedExchangeAdd64(volatile LONGLONG* Addend, LONGLONG Value) noexcept
{
    return __atomic_fetch_add(Addend, Value, __ATOMIC_SEQ_CST);
}
inline LONGLONG InterlockedCompareExchange64(volatile LONGLONG* Destination, LONGLONG Exchange, LONGLONG Comperand) noexcept
{
    LONGLONG expected = Comperand;
    __atomic_compare_exchange_n(Destination, &expected, Exchange, false, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST);
    return expected;
}

// CRT path / file / random helpers
int    _splitpath(const char* path, char* drive, char* dir, char* fname, char* ext) noexcept;
errno_t _splitpath_s(const char* path, char* drive, size_t driveLen, char* dir, size_t dirLen, char* fname, size_t fnameLen, char* ext, size_t extLen) noexcept;
errno_t _wsplitpath_s(const WCHAR* path, WCHAR* drive, size_t driveLen, WCHAR* dir, size_t dirLen, WCHAR* fname, size_t fnameLen, WCHAR* ext, size_t extLen) noexcept;
void   _makepath(char* path, const char* drive, const char* dir, const char* fname, const char* ext) noexcept;
char*  _fullpath(char* absPath, const char* relPath, size_t maxLength) noexcept;
char*  _i64toa(int64_t val, char* buf, int radix) noexcept;
char*  _ui64toa(uint64_t val, char* buf, int radix) noexcept;
errno_t _itoa_s(int val, char* buf, size_t size, int radix) noexcept;
errno_t _itow_s(int val, WCHAR* buf, size_t size, int radix) noexcept;
errno_t _i64toa_s(int64_t val, char* buf, size_t size, int radix) noexcept;
errno_t _i64tow_s(int64_t val, WCHAR* buf, size_t size, int radix) noexcept;
errno_t _ui64toa_s(uint64_t val, char* buf, size_t size, int radix) noexcept;
errno_t _ui64tow_s(uint64_t val, WCHAR* buf, size_t size, int radix) noexcept;
errno_t _strlwr_s(char* str, size_t size) noexcept;
errno_t _strupr_s(char* str, size_t size) noexcept;
errno_t _wcslwr_s(WCHAR* str, size_t size) noexcept;
errno_t _wcsupr_s(WCHAR* str, size_t size) noexcept;
FILE*  _wfopen(const WCHAR* filename, const WCHAR* mode) noexcept;
errno_t rand_s(unsigned int* randomValue) noexcept;
int    _vsnprintf(char* buffer, size_t count, const char* format, va_list argptr) noexcept;
int    _snprintf(char* buffer, size_t count, const char* format, ...) noexcept;
int    _vscprintf(const char* format, va_list argptr) noexcept;
int    _vscwprintf(const WCHAR* format, va_list argptr) noexcept;
int    _scprintf(const char* format, ...) noexcept;
int    _scwprintf(const WCHAR* format, ...) noexcept;
int    _strtoi64(const char* str, char** endptr, int base) noexcept;
int64_t _strtoi64_ll(const char* str, char** endptr, int base) noexcept;
uint64_t _strtoui64(const char* str, char** endptr, int base) noexcept;
int64_t _wcstoi64(const WCHAR* str, WCHAR** endptr, int base) noexcept;
uint64_t _wcstoui64(const WCHAR* str, WCHAR** endptr, int base) noexcept;
int    _filelength(int fd) noexcept;
int64_t _filelengthi64(int fd) noexcept;
int    _fileno(FILE* stream) noexcept;
int    _chsize(int fd, long size) noexcept;
int    _stat64(const char* path, struct stat* buffer) noexcept;
int    _wstat(const WCHAR* path, struct stat* buffer) noexcept;
int    _kbhit() noexcept;
int    _getch() noexcept;
int    _putch(int c) noexcept;

#ifndef strlwr
inline char* strlwr(char* s) { return _strlwr(s); }
#endif
#ifndef strupr
inline char* strupr(char* s) { return _strupr(s); }
#endif
#ifndef itoa
inline char* itoa(int v, char* b, int r) { return _itoa(v, b, r); }
#endif
#ifndef _fseeki64
inline int _fseeki64(FILE* fp, int64_t offset, int origin) { return fseeko(fp, (off_t)offset, origin); }
#endif
#ifndef _ftelli64
inline int64_t _ftelli64(FILE* fp) { return (int64_t)ftello(fp); }
#endif
#ifndef _time64
inline __time64_t _time64(__time64_t* t) { time_t now = time(nullptr); if (t) *t = (__time64_t)now; return (__time64_t)now; }
#endif
#ifndef _localtime64_s
inline errno_t _localtime64_s(struct tm* outTm, const __time64_t* t) { time_t tt = t ? (time_t)(*t) : 0; return localtime_s(outTm, &tt); }
#endif
#ifndef _localtime32_s
inline errno_t _localtime32_s(struct tm* outTm, const __time32_t* t) { time_t tt = t ? (time_t)(*t) : 0; return localtime_s(outTm, &tt); }
#endif
#ifndef _gmtime32_s
inline errno_t _gmtime32_s(struct tm* outTm, const __time32_t* t) { if (!outTm || !t) return EINVAL; time_t tt = (time_t)(*t); return gmtime_r(&tt, outTm) ? 0 : EINVAL; }
#endif
#ifndef _localtime64
inline struct tm* _localtime64(const __time64_t* t) { static thread_local struct tm s_tm; _localtime64_s(&s_tm, t); return &s_tm; }
#endif
#ifndef _mktime64
inline __time64_t _mktime64(struct tm* tmPtr) { return (__time64_t)mktime(tmPtr); }
#endif
#ifndef _mktime32
inline __time32_t _mktime32(struct tm* tmPtr) { return (__time32_t)mktime(tmPtr); }
#endif
#ifndef _mkgmtime64
inline __time64_t _mkgmtime64(struct tm* tmPtr) { return (__time64_t)timegm(tmPtr); }
#endif
#ifndef _mkgmtime32
inline __time32_t _mkgmtime32(struct tm* tmPtr) { return (__time32_t)timegm(tmPtr); }
#endif
#ifndef _mkgmtime
inline time_t _mkgmtime(struct tm* tmPtr) { return timegm(tmPtr); }
#endif
#ifndef _gmtime64_s
inline errno_t _gmtime64_s(struct tm* outTm, const __time64_t* t) { if (!outTm || !t) return EINVAL; time_t tt = (time_t)(*t); return gmtime_r(&tt, outTm) ? 0 : EINVAL; }
#endif
#ifndef _gmtime64
inline struct tm* _gmtime64(const __time64_t* t) { static thread_local struct tm s_tm; _gmtime64_s(&s_tm, t); return &s_tm; }
#endif
#ifndef gmtime_s
inline errno_t gmtime_s(struct tm* outTm, const time_t* t) { if (!outTm || !t) return EINVAL; return gmtime_r(t, outTm) ? 0 : EINVAL; }
#endif
#ifndef sscanf_s
#define sscanf_s sscanf
#endif
#ifndef swscanf_s
#define swscanf_s swscanf
#endif
#ifndef vsprintf_s
inline int vsprintf_s(char* buf, size_t sz, const char* fmt, va_list ap) { return _vsnprintf_s(buf, sz, _TRUNCATE, fmt, ap); }
template <size_t N> inline int vsprintf_s(char (&buf)[N], const char* fmt, va_list ap) { return _vsnprintf_s(buf, N, _TRUNCATE, fmt, ap); }
#endif
#ifndef vswprintf_s
inline int vswprintf_s(WCHAR* buf, size_t sz, const WCHAR* fmt, va_list ap) { return _vsnwprintf_s(buf, sz, _TRUNCATE, fmt, ap); }
template <size_t N> inline int vswprintf_s(WCHAR (&buf)[N], const WCHAR* fmt, va_list ap) { return _vsnwprintf_s(buf, N, _TRUNCATE, fmt, ap); }
#endif
template <size_t N> inline errno_t _strlwr_s(char (&s)[N]) { return _strlwr_s(s, N); }
template <size_t N> inline errno_t _strupr_s(char (&s)[N]) { return _strupr_s(s, N); }
template <size_t N> inline errno_t _wcslwr_s(WCHAR (&s)[N]) { return _wcslwr_s(s, N); }
template <size_t N> inline errno_t _wcsupr_s(WCHAR (&s)[N]) { return _wcsupr_s(s, N); }
template <size_t N> inline errno_t _itoa_s(int v, char (&b)[N], int r) { return _itoa_s(v, b, N, r); }
template <size_t N> inline errno_t _itow_s(int v, WCHAR (&b)[N], int r) { return _itow_s(v, b, N, r); }
template <size_t N> inline errno_t _i64toa_s(int64_t v, char (&b)[N], int r) { return _i64toa_s(v, b, N, r); }
template <size_t N> inline errno_t _i64tow_s(int64_t v, WCHAR (&b)[N], int r) { return _i64tow_s(v, b, N, r); }
template <size_t N> inline errno_t _ui64toa_s(uint64_t v, char (&b)[N], int r) { return _ui64toa_s(v, b, N, r); }
template <size_t N> inline errno_t _ui64tow_s(uint64_t v, WCHAR (&b)[N], int r) { return _ui64tow_s(v, b, N, r); }
template <size_t N>
inline int _snprintf_s(char (&buffer)[N], size_t count, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = _vsnprintf_s(buffer, N, count, format, args);
    va_end(args);
    return r;
}
template <size_t N>
inline int _snwprintf_s(WCHAR (&buffer)[N], size_t count, const WCHAR* format, ...)
{
    va_list args;
    va_start(args, format);
    int r = _vsnwprintf_s(buffer, N, count, format, args);
    va_end(args);
    return r;
}

// ANSI / Unicode Win32 API mappings
#if defined(UNICODE) || defined(_UNICODE)
#define CreateMutex            CreateMutexW
#define CreateSemaphore        CreateSemaphoreW
#define CreateFile             CreateFileW
#define GetFileAttributes      GetFileAttributesW
#define SetFileAttributes      SetFileAttributesW
#define DeleteFile             DeleteFileW
#define CopyFile               CopyFileW
#define MoveFile               MoveFileW
#define CreateDirectory        CreateDirectoryW
#define RemoveDirectory        RemoveDirectoryW
#define GetCurrentDirectory    GetCurrentDirectoryW
#define SetCurrentDirectory    SetCurrentDirectoryW
#define GetModuleFileName      GetModuleFileNameW
#define GetFullPathName        GetFullPathNameW
#define FindFirstFile          FindFirstFileW
#define FindNextFile           FindNextFileW
#define CreateFileMapping      CreateFileMappingW
#define GetPrivateProfileString GetPrivateProfileStringW
#define GetPrivateProfileInt   GetPrivateProfileIntW
#define WritePrivateProfileString WritePrivateProfileStringW
#define LoadLibrary            LoadLibraryW
#define GetModuleHandle        GetModuleHandleW
#define OutputDebugString      OutputDebugStringW
#define MessageBox             MessageBoxW
#define FormatMessage          FormatMessageW
#define GetComputerName        GetComputerNameW
#define SetConsoleTitle        SetConsoleTitleW
#define GetVersionEx           GetVersionExW
#define GetLocaleInfo          GetLocaleInfoW
#define RegOpenKeyEx           RegOpenKeyExW
#define RegCreateKeyEx         RegCreateKeyExW
#define RegQueryValueEx        RegQueryValueExW
#define RegSetValueEx          RegSetValueExW
#define RegEnumKeyEx           RegEnumKeyExW
#define RegDeleteKey           RegDeleteKeyW
#define CopyFileEx             CopyFileExW
#define GetFileVersionInfoSize GetFileVersionInfoSizeW
#define GetFileVersionInfo     GetFileVersionInfoW
#define VerQueryValue          VerQueryValueW
#define GetTempFileName        GetTempFileNameW
#define CreateProcess          CreateProcessW
#define GetDiskFreeSpace       GetDiskFreeSpaceW
#define wsprintf               wsprintfW
#define WSASocket              WSASocketW
#define lstrlen                wcslen
#define lstrcpy                wcscpy
#define lstrcmp                wcscmp
#define lstrcmpi               _wcsicmp
#else
#define CreateMutex            CreateMutexA
#define CreateSemaphore        CreateSemaphoreA
#define CreateFile             CreateFileA
#define GetFileAttributes      GetFileAttributesA
#define SetFileAttributes      SetFileAttributesA
#define DeleteFile             DeleteFileA
#define CopyFile               CopyFileA
#define MoveFile               MoveFileA
#define CreateDirectory        CreateDirectoryA
#define RemoveDirectory        RemoveDirectoryA
#define GetCurrentDirectory    GetCurrentDirectoryA
#define SetCurrentDirectory    SetCurrentDirectoryA
#define GetModuleFileName      GetModuleFileNameA
#define GetFullPathName        GetFullPathNameA
#define FindFirstFile          FindFirstFileA
#define FindNextFile           FindNextFileA
#define CreateFileMapping      CreateFileMappingA
#define GetPrivateProfileString GetPrivateProfileStringA
#define GetPrivateProfileInt   GetPrivateProfileIntA
#define WritePrivateProfileString WritePrivateProfileStringA
#define LoadLibrary            LoadLibraryA
#define GetModuleHandle        GetModuleHandleA
#define OutputDebugString      OutputDebugStringA
#define MessageBox             MessageBoxA
#define FormatMessage          FormatMessageA
#define GetComputerName        GetComputerNameA
#define SetConsoleTitle        SetConsoleTitleA
#define GetVersionEx           GetVersionExA
#define GetLocaleInfo          GetLocaleInfoA
#define RegOpenKeyEx           RegOpenKeyExA
#define RegCreateKeyEx         RegCreateKeyExA
#define RegQueryValueEx        RegQueryValueExA
#define RegSetValueEx          RegSetValueExA
#define RegEnumKeyEx           RegEnumKeyExA
#define RegDeleteKey           RegDeleteKeyA
#define CopyFileEx             CopyFileExA
#define GetFileVersionInfoSize GetFileVersionInfoSizeA
#define GetFileVersionInfo     GetFileVersionInfoA
#define VerQueryValue          VerQueryValueA
#define GetTempFileName        GetTempFileNameA
#define CreateProcess          CreateProcessA
#define GetDiskFreeSpace       GetDiskFreeSpaceA
#define wsprintf               wsprintfA
#define WSASocket              WSASocketA
#define lstrlen                strlen
#define lstrcpy                strcpy
#define lstrcmp                strcmp
#define lstrcmpi               _stricmp
#endif

// Non-macro Win32 min/max compatibility templates (never collide with std::min/std::max or numeric_limits::max())
#ifndef NOMINMAX
template <typename T, typename U>
inline auto min(const T& a, const U& b) -> typename std::decay<decltype((a < b) ? a : b)>::type
{
    return (a < b) ? a : b;
}
template <typename T, typename U>
inline auto max(const T& a, const U& b) -> typename std::decay<decltype((a > b) ? a : b)>::type
{
    return (a > b) ? a : b;
}
#endif
