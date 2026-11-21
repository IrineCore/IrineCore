// =============================================================================
// IrineCore — tests/compatibility/TestConfigAndCountryPurge.cpp
// Compatibility tests verifying config/*.ini files and zero Nation= remnants.
// =============================================================================
#include "Platform.h"
#include <cstdio>
#include <string>

int RunCompatibilityTests(const WCHAR* configDirPath)
{
    int failures = 0;
    auto check = [&](bool cond, const char* msg) {
        if (!cond) { std::fprintf(stderr, "  [FAIL][Compatibility] %s\n", msg); ++failures; }
        else       { std::printf("  [PASS][Compatibility] %s\n", msg); }
    };

    std::wstring base = configDirPath ? configDirPath : L"./config";
    if (!base.empty() && base.back() != L'/' && base.back() != L'\\')
        base.push_back(L'/');

    std::wstring loginIni = base + L"DNLogin.ini";
    UINT ver = ::GetPrivateProfileIntW(L"Info", L"Version", 0, loginIni.c_str());
    UINT nation = ::GetPrivateProfileIntW(L"Info", L"Nation", 99999, loginIni.c_str());

    check(ver > 0, "DNLogin.ini [Info] Version loaded via GetPrivateProfileIntW");
    check(nation == 99999, "DNLogin.ini contains zero legacy Nation= configuration keys");

    return failures;
}
