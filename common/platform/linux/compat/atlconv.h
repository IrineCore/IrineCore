#pragma once
#include "windows.h"
#include <alloca.h>
#include <string>

#ifndef USES_CONVERSION
#define USES_CONVERSION LPCSTR _lpa = nullptr; (void)_lpa; LPCWSTR _lpw = nullptr; (void)_lpw
#endif

namespace IrineAtlCompat {
    inline std::wstring ToWide(const char* s) { return s ? Irine::Platform::Utf8ToWide(s) : std::wstring(); }
    inline std::string  ToNarrow(const WCHAR* s) { return s ? Irine::Platform::WideToUtf8(s) : std::string(); }
}

class CA2W {
    std::wstring m_w;
public:
    explicit CA2W(const char* s) : m_w(IrineAtlCompat::ToWide(s)) {}
    operator LPCWSTR() const { return m_w.c_str(); }
    operator LPWSTR() { return m_w.empty() ? const_cast<LPWSTR>(L"") : &m_w[0]; }
};
class CW2A {
    std::string m_a;
public:
    explicit CW2A(const WCHAR* s) : m_a(IrineAtlCompat::ToNarrow(s)) {}
    operator LPCSTR() const { return m_a.c_str(); }
    operator LPSTR() { return m_a.empty() ? const_cast<LPSTR>("") : &m_a[0]; }
};
class CA2A {
    std::string m_a;
public:
    explicit CA2A(const char* s) : m_a(s ? s : "") {}
    operator LPCSTR() const { return m_a.c_str(); }
    operator LPSTR() { return m_a.empty() ? const_cast<LPSTR>("") : &m_a[0]; }
};
class CW2W {
    std::wstring m_w;
public:
    explicit CW2W(const WCHAR* s) : m_w(s ? s : L"") {}
    operator LPCWSTR() const { return m_w.c_str(); }
    operator LPWSTR() { return m_w.empty() ? const_cast<LPWSTR>(L"") : &m_w[0]; }
};
using CA2WEX_DEFAULT = CA2W;
using CW2AEX_DEFAULT = CW2A;
using CA2CW = CA2W;
using CW2CA = CW2A;
using CA2CA = CA2A;
using CW2CW = CW2W;
#define A2W(s)  (static_cast<LPCWSTR>(CA2W(s)))
#define W2A(s)  (static_cast<LPCSTR>(CW2A(s)))
#define A2CW(s) A2W(s)
#define W2CA(s) W2A(s)
#if defined(UNICODE) || defined(_UNICODE)
using CT2A  = CW2A;
using CT2CA = CW2A;
using CA2T  = CA2W;
using CA2CT = CA2W;
using CT2W  = CW2W;
using CT2CW = CW2W;
using CW2T  = CW2W;
using CW2CT = CW2W;
#define T2A(s)  W2A(s)
#define T2CA(s) W2CA(s)
#define A2T(s)  A2W(s)
#define A2CT(s) A2CW(s)
#define T2W(s)  (s)
#define T2CW(s) (s)
#define W2T(s)  (s)
#define W2CT(s) (s)
#else
using CT2W  = CA2W;
using CT2CW = CA2W;
using CW2T  = CW2A;
using CW2CT = CW2A;
using CT2A  = CA2A;
using CT2CA = CA2A;
using CA2T  = CA2A;
using CA2CT = CA2A;
#define T2A(s)  (s)
#define T2CA(s) (s)
#define A2T(s)  (s)
#define A2CT(s) (s)
#define T2W(s)  A2W(s)
#define T2CW(s) A2CW(s)
#define W2T(s)  W2A(s)
#define W2CT(s) W2CA(s)
#endif
