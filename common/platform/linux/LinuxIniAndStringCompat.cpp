// =============================================================================
// IrineCore — common/platform/linux/LinuxIniAndStringCompat.cpp
// Linux implementations of UTF-16LE CRT string/format helpers, INI parser,
// POSIX event objects, thread spawning, and filesystem shims.
// =============================================================================
#if !defined(__linux__)
#error "common/platform/linux/LinuxIniAndStringCompat.cpp must only be compiled on Linux targets."
#endif

#include "../Platform.h"
#include <sys/stat.h>
#include <strings.h>
#include <fstream>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <vector>
#include <algorithm>
#include <cctype>

int MultiByteToWideChar(UINT /*CodePage*/, DWORD /*dwFlags*/, LPCSTR lpMultiByteStr, int cbMultiByte, LPWSTR lpWideCharStr, int cchWideChar)
{
    if (!lpMultiByteStr) return 0;
    bool nulTerm = (cbMultiByte < 0);
    std::wstring w = Irine::Platform::Utf8ToWide(lpMultiByteStr, nulTerm ? -1 : cbMultiByte);
    int needed = static_cast<int>(w.size()) + (nulTerm ? 1 : 0);
    if (cchWideChar == 0 || !lpWideCharStr) return needed;
    if (cchWideChar < needed) return 0;
    for (size_t i = 0; i < w.size(); ++i) lpWideCharStr[i] = w[i];
    if (nulTerm) lpWideCharStr[w.size()] = 0;
    return needed;
}

int WideCharToMultiByte(UINT /*CodePage*/, DWORD /*dwFlags*/, LPCWSTR lpWideCharStr, int cchWideChar, LPSTR lpMultiByteStr, int cbMultiByte, LPCSTR /*lpDefaultChar*/, BOOL* lpUsedDefaultChar)
{
    if (lpUsedDefaultChar) *lpUsedDefaultChar = FALSE;
    if (!lpWideCharStr) return 0;
    bool nulTerm = (cchWideChar < 0);
    std::string u8 = Irine::Platform::WideToUtf8(lpWideCharStr, nulTerm ? -1 : cchWideChar);
    int needed = static_cast<int>(u8.size()) + (nulTerm ? 1 : 0);
    if (cbMultiByte == 0 || !lpMultiByteStr) return needed;
    if (cbMultiByte < needed) return 0;
    std::memcpy(lpMultiByteStr, u8.data(), u8.size());
    if (nulTerm) lpMultiByteStr[u8.size()] = '\0';
    return needed;
}

int _stricmp(const char* s1, const char* s2)
{
    if (!s1 || !s2) return s1 == s2 ? 0 : (s1 ? 1 : -1);
    return ::strcasecmp(s1, s2);
}

int _strnicmp(const char* s1, const char* s2, size_t n)
{
    if (!s1 || !s2) return s1 == s2 ? 0 : (s1 ? 1 : -1);
    return ::strncasecmp(s1, s2, n);
}

static inline WCHAR ToLowerW(WCHAR c)
{
    return (c >= L'A' && c <= L'Z') ? static_cast<WCHAR>(c + (L'a' - L'A')) : c;
}

int _wcsicmp(const WCHAR* s1, const WCHAR* s2)
{
    if (!s1 || !s2) return s1 == s2 ? 0 : (s1 ? 1 : -1);
    while (*s1 && *s2)
    {
        WCHAR c1 = ToLowerW(*s1), c2 = ToLowerW(*s2);
        if (c1 != c2) return static_cast<int>(c1) - static_cast<int>(c2);
        ++s1; ++s2;
    }
    return static_cast<int>(ToLowerW(*s1)) - static_cast<int>(ToLowerW(*s2));
}

int _wcsnicmp(const WCHAR* s1, const WCHAR* s2, size_t n)
{
    if (n == 0) return 0;
    if (!s1 || !s2) return s1 == s2 ? 0 : (s1 ? 1 : -1);
    for (size_t i = 0; i < n; ++i)
    {
        WCHAR c1 = ToLowerW(s1[i]), c2 = ToLowerW(s2[i]);
        if (c1 != c2 || c1 == 0) return static_cast<int>(c1) - static_cast<int>(c2);
    }
    return 0;
}

errno_t memcpy_s(void* dest, size_t destSize, const void* src, size_t count)
{
    if (!dest || !src || count > destSize) return EINVAL;
    std::memcpy(dest, src, count);
    return 0;
}

errno_t memmove_s(void* dest, size_t destSize, const void* src, size_t count)
{
    if (!dest || !src || count > destSize) return EINVAL;
    std::memmove(dest, src, count);
    return 0;
}

errno_t wmemcpy_s(WCHAR* dest, size_t destElements, const WCHAR* src, size_t count)
{
    if (!dest || !src || count > destElements) return EINVAL;
    std::memcpy(dest, src, count * sizeof(WCHAR));
    return 0;
}

errno_t strcpy_s(char* dest, size_t destSize, const char* src)
{
    if (!dest || destSize == 0 || !src) return EINVAL;
    size_t len = std::strlen(src);
    if (len >= destSize) { dest[0] = '\0'; return ERANGE; }
    std::memcpy(dest, src, len + 1);
    return 0;
}

errno_t strncpy_s(char* dest, size_t destSize, const char* src, size_t count)
{
    if (!dest || destSize == 0 || !src) return EINVAL;
    size_t maxCopy = (count == _TRUNCATE) ? (destSize - 1) : std::min(count, destSize - 1);
    size_t len = 0;
    while (len < maxCopy && src[len] != '\0') { dest[len] = src[len]; ++len; }
    dest[len] = '\0';
    return 0;
}

errno_t strcat_s(char* dest, size_t destSize, const char* src)
{
    if (!dest || destSize == 0 || !src) return EINVAL;
    size_t dlen = std::strlen(dest);
    size_t slen = std::strlen(src);
    if (dlen + slen >= destSize) return ERANGE;
    std::memcpy(dest + dlen, src, slen + 1);
    return 0;
}

static size_t WcsLen2(const WCHAR* s)
{
    if (!s) return 0;
    size_t n = 0;
    while (s[n] != 0) ++n;
    return n;
}

errno_t wcscpy_s(WCHAR* dest, size_t destElements, const WCHAR* src)
{
    if (!dest || destElements == 0 || !src) return EINVAL;
    size_t len = WcsLen2(src);
    if (len >= destElements) { dest[0] = 0; return ERANGE; }
    std::memcpy(dest, src, (len + 1) * sizeof(WCHAR));
    return 0;
}

errno_t wcsncpy_s(WCHAR* dest, size_t destElements, const WCHAR* src, size_t count)
{
    if (!dest || destElements == 0 || !src) return EINVAL;
    size_t maxCopy = (count == _TRUNCATE) ? (destElements - 1) : std::min(count, destElements - 1);
    size_t len = 0;
    while (len < maxCopy && src[len] != 0) { dest[len] = src[len]; ++len; }
    dest[len] = 0;
    return 0;
}

errno_t wcscat_s(WCHAR* dest, size_t destElements, const WCHAR* src)
{
    if (!dest || destElements == 0 || !src) return EINVAL;
    size_t dlen = WcsLen2(dest);
    size_t slen = WcsLen2(src);
    if (dlen + slen >= destElements) return ERANGE;
    std::memcpy(dest + dlen, src, (slen + 1) * sizeof(WCHAR));
    return 0;
}

int     _wtoi(const WCHAR* str)   { return std::atoi(Irine::Platform::WideToUtf8(str).c_str()); }
long    _wtol(const WCHAR* str)   { return std::atol(Irine::Platform::WideToUtf8(str).c_str()); }
double  _wtof(const WCHAR* str)   { return std::atof(Irine::Platform::WideToUtf8(str).c_str()); }
int64_t _wtoi64(const WCHAR* str) { return std::atoll(Irine::Platform::WideToUtf8(str).c_str()); }
int64_t _atoi64(const char* str)  { return str ? std::atoll(str) : 0; }

// Cross-platform INI file reader supporting UTF-8 and UTF-16LE BOM
static std::wstring TrimW(const std::wstring& s)
{
    size_t start = 0;
    while (start < s.size() && (s[start] == L' ' || s[start] == L'\t' || s[start] == L'\r' || s[start] == L'\n')) ++start;
    size_t end = s.size();
    while (end > start && (s[end - 1] == L' ' || s[end - 1] == L'\t' || s[end - 1] == L'\r' || s[end - 1] == L'\n')) --end;
    return s.substr(start, end - start);
}

DWORD GetPrivateProfileStringW(
    LPCWSTR lpAppName,
    LPCWSTR lpKeyName,
    LPCWSTR lpDefault,
    LPWSTR  lpReturnedString,
    DWORD   nSize,
    LPCWSTR lpFileName) noexcept
{
    if (!lpReturnedString || nSize == 0) return 0;
    std::wstring defStr = lpDefault ? std::wstring(lpDefault, WcsLen2(lpDefault)) : std::wstring();
    auto copyOut = [&](const std::wstring& val) -> DWORD {
        size_t n = std::min(val.size(), static_cast<size_t>(nSize - 1));
        if (n > 0) std::memcpy(lpReturnedString, val.data(), n * sizeof(WCHAR));
        lpReturnedString[n] = 0;
        return static_cast<DWORD>(n);
    };
    if (!lpAppName || !lpKeyName || !lpFileName) return copyOut(defStr);

    std::string path = Irine::Platform::ResolveCaseInsensitivePath(Irine::Platform::WideToUtf8(lpFileName), false);
    std::ifstream ifs(path, std::ios::binary);
    if (!ifs) return copyOut(defStr);

    std::vector<uint8_t> raw((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    std::wstring content;
    if (raw.size() >= 2 && raw[0] == 0xFF && raw[1] == 0xFE)
    {
        size_t chars = (raw.size() - 2) / 2;
        content.resize(chars);
        std::memcpy(&content[0], raw.data() + 2, chars * 2);
    }
    else
    {
        size_t offset = (raw.size() >= 3 && raw[0] == 0xEF && raw[1] == 0xBB && raw[2] == 0xBF) ? 3 : 0;
        content = Irine::Platform::Utf8ToWide(reinterpret_cast<const char*>(raw.data() + offset), static_cast<int>(raw.size() - offset));
    }

    std::wstring targetSection(lpAppName, WcsLen2(lpAppName));
    std::wstring targetKey(lpKeyName, WcsLen2(lpKeyName));
    bool inSection = false;
    size_t pos = 0;
    while (pos < content.size())
    {
        size_t eol = content.find(L'\n', pos);
        if (eol == std::wstring::npos) eol = content.size();
        std::wstring line = TrimW(content.substr(pos, eol - pos));
        pos = eol + 1;

        if (line.empty() || line[0] == L';' || line[0] == L'#') continue;
        if (line.front() == L'[' && line.back() == L']')
        {
            std::wstring sec = TrimW(line.substr(1, line.size() - 2));
            inSection = (_wcsicmp(sec.c_str(), targetSection.c_str()) == 0);
            continue;
        }
        if (inSection)
        {
            size_t eq = line.find(L'=');
            if (eq != std::wstring::npos)
            {
                std::wstring key = TrimW(line.substr(0, eq));
                if (_wcsicmp(key.c_str(), targetKey.c_str()) == 0)
                {
                    std::wstring val = TrimW(line.substr(eq + 1));
                    return copyOut(val);
                }
            }
        }
    }
    return copyOut(defStr);
}

UINT GetPrivateProfileIntW(LPCWSTR lpAppName, LPCWSTR lpKeyName, INT nDefault, LPCWSTR lpFileName) noexcept
{
    WCHAR buf[64] = { 0 };
    DWORD len = GetPrivateProfileStringW(lpAppName, lpKeyName, L"", buf, 64, lpFileName);
    if (len == 0 || buf[0] == 0) return static_cast<UINT>(nDefault);
    return static_cast<UINT>(_wtoi(buf));
}
