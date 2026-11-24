// =============================================================================
// IrineCore — common/platform/linux/LinuxPrefix.h
// Force-included on Linux/UNIX C++ targets before any translation unit header.
// 1. Disables libstdc++ extern templates so std::wstring / streams instantiate
//    with 2-byte wchar_t (-fshort-wchar) instead of calling 4-byte libstdc++.so.
// 2. Provides portable MSVC keyword/qualifier compatibility macros for GCC/Clang.
// =============================================================================
#pragma once

#if defined(__cplusplus) && !defined(_WIN32)

#if defined(__GNUC__) && !defined(_LIBCPP_VERSION)
#include <bits/c++config.h>
#undef _GLIBCXX_EXTERN_TEMPLATE
#define _GLIBCXX_EXTERN_TEMPLATE 0
#endif

// Calling conventions (no-op on x86_64 System V ABI)
#ifndef __cdecl
#define __cdecl
#endif
#ifndef __stdcall
#define __stdcall
#endif
#ifndef __fastcall
#define __fastcall
#endif
#ifndef __thiscall
#define __thiscall
#endif

#include <cfloat>
#include <algorithm>

#ifndef _inline
#define _inline inline
#endif
#ifndef NOERROR
#define NOERROR 0L
#endif

// MSVC inline / declspec shims
#ifndef __forceinline
#define __forceinline inline __attribute__((always_inline))
#endif

#ifndef __declspec
#define IRINE_DECLSPEC_novtable
#define IRINE_DECLSPEC_dllexport  __attribute__((visibility("default")))
#define IRINE_DECLSPEC_dllimport
#define IRINE_DECLSPEC_thread     __thread
#define IRINE_DECLSPEC_noinline   __attribute__((noinline))
#define IRINE_DECLSPEC_nothrow    __attribute__((nothrow))
#define IRINE_DECLSPEC_selectany  __attribute__((weak))
#define IRINE_DECLSPEC_deprecated __attribute__((deprecated))
#define IRINE_DECLSPEC_align(n)   __attribute__((aligned(n)))
#define IRINE_DECLSPEC_uuid(x)
#define __declspec(x) IRINE_DECLSPEC_##x
#endif

// MSVC CRT assert macros (used in deps/tinyxml, common, server)
#ifndef _ASSERT
#ifdef _DEBUG
#include <cassert>
#define _ASSERT(expr)  assert(expr)
#define _ASSERTE(expr) assert(expr)
#else
#define _ASSERT(expr)  ((void)0)
#define _ASSERTE(expr) ((void)0)
#endif
#endif

#ifndef _ASSERT_EXPR
#define _ASSERT_EXPR(expr, msg) _ASSERT(expr)
#endif

#ifndef _CRT_WIDE
#define _CRT_WIDE_(s) L##s
#define _CRT_WIDE(s)  _CRT_WIDE_(s)
#endif

#if defined(__x86_64__) && !defined(_AMD64_)
#define _AMD64_ 1
#endif

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(P) ((void)(P))
#endif

// MSVC <stdlib.h> / <algorithm> helpers
#ifndef _MAX_PATH
#define _MAX_PATH  260
#endif
#ifndef _MAX_DRIVE
#define _MAX_DRIVE 3
#define _MAX_DIR   256
#define _MAX_FNAME 256
#define _MAX_EXT   256
#endif

#endif // __cplusplus && !_WIN32
