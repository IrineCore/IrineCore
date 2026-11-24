#pragma once
#include "windows.h"
#include <cstdarg>

inline HRESULT StringCchCopyA(LPSTR pszDest, size_t cchDest, LPCSTR pszSrc) noexcept {
    if (!pszDest || cchDest == 0 || !pszSrc) return E_FAIL;
    strncpy_s(pszDest, cchDest, pszSrc, _TRUNCATE);
    return S_OK;
}
inline HRESULT StringCchCopyW(LPWSTR pszDest, size_t cchDest, LPCWSTR pszSrc) noexcept {
    if (!pszDest || cchDest == 0 || !pszSrc) return E_FAIL;
    wcsncpy_s(pszDest, cchDest, pszSrc, _TRUNCATE);
    return S_OK;
}
inline HRESULT StringCchCatA(LPSTR pszDest, size_t cchDest, LPCSTR pszSrc) noexcept {
    if (!pszDest || cchDest == 0 || !pszSrc) return E_FAIL;
    strncat_s(pszDest, cchDest, pszSrc, _TRUNCATE);
    return S_OK;
}
inline HRESULT StringCchCatW(LPWSTR pszDest, size_t cchDest, LPCWSTR pszSrc) noexcept {
    if (!pszDest || cchDest == 0 || !pszSrc) return E_FAIL;
    wcsncat_s(pszDest, cchDest, pszSrc, _TRUNCATE);
    return S_OK;
}
inline HRESULT StringCchVPrintfA(LPSTR pszDest, size_t cchDest, LPCSTR pszFormat, va_list argList) noexcept {
    if (!pszDest || cchDest == 0 || !pszFormat) return E_FAIL;
    _vsnprintf_s(pszDest, cchDest, _TRUNCATE, pszFormat, argList);
    return S_OK;
}
inline HRESULT StringCchVPrintfW(LPWSTR pszDest, size_t cchDest, LPCWSTR pszFormat, va_list argList) noexcept {
    if (!pszDest || cchDest == 0 || !pszFormat) return E_FAIL;
    _vsnwprintf_s(pszDest, cchDest, _TRUNCATE, pszFormat, argList);
    return S_OK;
}
inline HRESULT StringCchPrintfA(LPSTR pszDest, size_t cchDest, LPCSTR pszFormat, ...) noexcept {
    va_list args; va_start(args, pszFormat);
    HRESULT hr = StringCchVPrintfA(pszDest, cchDest, pszFormat, args);
    va_end(args);
    return hr;
}
inline HRESULT StringCchPrintfW(LPWSTR pszDest, size_t cchDest, LPCWSTR pszFormat, ...) noexcept {
    va_list args; va_start(args, pszFormat);
    HRESULT hr = StringCchVPrintfW(pszDest, cchDest, pszFormat, args);
    va_end(args);
    return hr;
}
#if defined(UNICODE) || defined(_UNICODE)
#define StringCchCopy    StringCchCopyW
#define StringCchCat     StringCchCatW
#define StringCchPrintf  StringCchPrintfW
#define StringCchVPrintf StringCchVPrintfW
#else
#define StringCchCopy    StringCchCopyA
#define StringCchCat     StringCchCatA
#define StringCchPrintf  StringCchPrintfA
#define StringCchVPrintf StringCchVPrintfA
#endif
