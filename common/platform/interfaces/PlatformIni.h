// =============================================================================
// IrineCore — common/platform/interfaces/PlatformIni.h
// Cross-platform INI configuration parser supporting UTF-8 and UTF-16LE BOM.
// Provides GetPrivateProfileStringW / GetPrivateProfileIntW on Linux.
// =============================================================================
#pragma once

#include "PlatformTypes.h"

#if !defined(_WIN32)

DWORD GetPrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR  lpReturnedString,
    DWORD   nSize,
    LPCWSTR lpFileName) noexcept;

UINT GetPrivateProfileIntW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    INT     nDefault,
    LPCWSTR lpFileName) noexcept;

DWORD GetPrivateProfileStringA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    LPCSTR lpDefault,
    LPSTR  lpReturnedString,
    DWORD  nSize,
    LPCSTR lpFileName) noexcept;

UINT GetPrivateProfileIntA(
    LPCSTR lpAppName,
    LPCSTR lpKeyName,
    INT    nDefault,
    LPCSTR lpFileName) noexcept;

#endif // !_WIN32
