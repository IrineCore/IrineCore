// =============================================================================
// IrineCore — tests/unit/TestPlatformAndMath.cpp
// Unit tests for PlatformTypes, UTF-16LE WCHAR, and EtMathPortable.h.
// =============================================================================
#include "../../common/platform/Platform.h"
#include "EtMathPortable.h"
#include <boost/format.hpp>
#include <cmath>
#include <cstdio>
#include <sstream>

int RunUnitTests()
{
    int failures = 0;
    auto check = [&](bool cond, const char* msg) {
        if (!cond) { std::fprintf(stderr, "  [FAIL][Unit] %s\n", msg); ++failures; }
        else       { std::printf("  [PASS][Unit] %s\n", msg); }
    };

    // 1. Binary ABI layout checks
    check(sizeof(WCHAR) == 2, "sizeof(WCHAR) == 2 (UTF-16LE wire & ODBC ABI invariant)");
    check(sizeof(D3DXVECTOR2) == 8,  "sizeof(D3DXVECTOR2) == 8");
    check(sizeof(D3DXVECTOR3) == 12, "sizeof(D3DXVECTOR3) == 12");
    check(sizeof(D3DXVECTOR4) == 16, "sizeof(D3DXVECTOR4) == 16");
    check(sizeof(D3DXMATRIX)  == 64, "sizeof(D3DXMATRIX) == 64");
    check(sizeof(D3DXQUATERNION) == 16, "sizeof(D3DXQUATERNION) == 16");
    check(sizeof(D3DXPLANE)   == 16, "sizeof(D3DXPLANE) == 16");
    check(sizeof(D3DXCOLOR)   == 16, "sizeof(D3DXCOLOR) == 16");

    // 2. UTF-16LE <-> UTF-8 conversion round-trip & 2-byte WCHAR formatting
    std::string u8 = Irine::Platform::WideToUtf8(L"IrineCore Cap 60 Server");
    std::wstring w = Irine::Platform::Utf8ToWide(u8.c_str());
    check(u8 == "IrineCore Cap 60 Server" && w == L"IrineCore Cap 60 Server",
          "WideToUtf8 <-> Utf8ToWide round-trip fidelity");

    WCHAR fmtBuf[128] = { 0 };
    swprintf_s(fmtBuf, _countof(fmtBuf), L"Lv.%d %s", 60, L"IrineCore");
    check(std::wstring(fmtBuf) == L"Lv.60 IrineCore",
          "swprintf_s formats 2-byte WCHAR strings and integers");

    std::wostringstream woss;
    woss << L"Cap" << 60 << L":" << L"OK";
    check(woss.str() == L"Cap60:OK",
          "std::wostringstream works with 2-byte wchar_t");

    bool boostWformatOk = false;
    try {
        std::wstring bf = boost::str(boost::wformat(L"Player %s reached level %d (%.2f%%)") % L"Irine" % 60 % 99.5);
        std::wstring bfPos = boost::str(boost::wformat(L"[%1%] HP=%2%/%3%") % L"\xC544\xC774\xB9B0" % 12345 % 20000);
        boostWformatOk = (bf == L"Player Irine reached level 60 (99.50%)") &&
                         (bfPos == L"[\xC544\xC774\xB9B0] HP=12345/20000");
        if (!boostWformatOk) {
            std::fprintf(stderr, "    boost::wformat got: '%s' | '%s'\n",
                         Irine::Platform::WideToUtf8(bf.c_str()).c_str(),
                         Irine::Platform::WideToUtf8(bfPos.c_str()).c_str());
        }
    } catch (const std::exception& ex) {
        std::fprintf(stderr, "    boost::wformat threw: %s\n", ex.what());
    }
    check(boostWformatOk, "boost::wformat (printf + positional + UTF-16LE CJK) works with 2-byte wchar_t");

    // 3. EtMathPortable vector & matrix operations
    D3DXVECTOR3 a(1.0f, 0.0f, 0.0f), b(0.0f, 1.0f, 0.0f), c;
    D3DXVec3Cross(&c, &a, &b);
    check(std::fabs(c.x) < 1e-6f && std::fabs(c.y) < 1e-6f && std::fabs(c.z - 1.0f) < 1e-6f,
          "D3DXVec3Cross((1,0,0), (0,1,0)) == (0,0,1)");

    D3DXMATRIX mTrans, mInv, mProd;
    D3DXMatrixTranslation(&mTrans, 10.0f, -20.0f, 30.5f);
    float det = 0.0f;
    D3DXMatrixInverse(&mInv, &det, &mTrans);
    D3DXMatrixMultiply(&mProd, &mTrans, &mInv);
    check(std::fabs(det - 1.0f) < 1e-5f && D3DXMatrixIsIdentity(&mProd) != 0,
          "D3DXMatrixTranslation * D3DXMatrixInverse == Identity");

    return failures;
}
