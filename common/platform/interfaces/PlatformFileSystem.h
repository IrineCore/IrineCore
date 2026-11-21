// =============================================================================
// IrineCore — common/platform/interfaces/PlatformFileSystem.h
// Cross-platform filesystem helpers & path separator normalization.
// =============================================================================
#pragma once

#include "PlatformTypes.h"
#include <string>

namespace Irine {
namespace Platform {

// Normalizes path separators ('\\' -> '/' on Linux, '/' -> '\\' on Windows)
std::string  NormalizePath(const std::string& path);
std::wstring NormalizePathW(const std::wstring& path);

bool EnsureDirectoryExists(const std::string& path);
std::string GetExecutableDirectory();

} // namespace Platform
} // namespace Irine

#if !defined(_WIN32)

BOOL  CreateDirectoryA(LPCSTR lpPathName, void* lpSecurityAttributes) noexcept;
BOOL  CreateDirectoryW(LPCWSTR lpPathName, void* lpSecurityAttributes) noexcept;
DWORD GetCurrentDirectoryA(DWORD nBufferLength, LPSTR lpBuffer) noexcept;
DWORD GetCurrentDirectoryW(DWORD nBufferLength, LPWSTR lpBuffer) noexcept;
BOOL  SetCurrentDirectoryA(LPCSTR lpPathName) noexcept;
BOOL  SetCurrentDirectoryW(LPCWSTR lpPathName) noexcept;
DWORD GetModuleFileNameA(HMODULE hModule, LPSTR lpFilename, DWORD nSize) noexcept;
DWORD GetModuleFileNameW(HMODULE hModule, LPWSTR lpFilename, DWORD nSize) noexcept;
BOOL  DeleteFileA(LPCSTR lpFileName) noexcept;
BOOL  DeleteFileW(LPCWSTR lpFileName) noexcept;
BOOL  CopyFileA(LPCSTR lpExistingFileName, LPCSTR lpNewFileName, BOOL bFailIfExists) noexcept;
BOOL  CopyFileW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists) noexcept;

int _access(const char* path, int mode) noexcept;
int _waccess(const WCHAR* path, int mode) noexcept;
int _mkdir(const char* path) noexcept;
int _wmkdir(const WCHAR* path) noexcept;

#endif // !_WIN32
