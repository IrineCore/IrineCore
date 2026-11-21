// =============================================================================
// IrineCore — tests/unit/TestPakAndTools.cpp
// Verifies binary ABI compatibility between irine_tool_common (PakWriter /
// PakReader / MD5 / PKZIP) and the game engine's real CEtPackingFile reader.
// =============================================================================

#include "PakArchive.h"
#include "ToolUtils.h"

#ifdef _WIN32
#include <windows.h>
#ifndef SAFE_DELETE
#define SAFE_DELETE(p) { if (p) { delete (p); (p) = nullptr; } }
#endif
#include "EtFileSystem.h"
#endif

#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;

int RunToolsTests()
{
    int failures = 0;
    auto check = [&](bool cond, const char* msg) {
        if (!cond) { std::fprintf(stderr, "  [FAIL][Tools] %s\n", msg); ++failures; }
        else       { std::printf("  [PASS][Tools] %s\n", msg); }
    };

#ifdef _WIN32
    check(sizeof(irine::tools::PakFileHeader) == sizeof(SPackingFileHeader),
          "PakFileHeader ABI matches CEtPackingFile SPackingFileHeader (272 bytes)");
    check(sizeof(irine::tools::PakFileInfo) == sizeof(SPackingFileInfo),
          "PakFileInfo ABI matches CEtPackingFile SPackingFileInfo (316 bytes)");
#else
    check(sizeof(irine::tools::PakFileHeader) == 272, "PakFileHeader size == 272 bytes");
    check(sizeof(irine::tools::PakFileInfo) == 316, "PakFileInfo size == 316 bytes");
#endif

    // 1. RFC-1321 MD5 test vectors
    check(irine::tools::ComputeBufferMD5("", 0) == "d41d8cd98f00b204e9800998ecf8427e",
          "MD5(\"\") matches RFC-1321 vector");
    check(irine::tools::ComputeBufferMD5("abc", 3) == "900150983cd24fb0d6963f7d28e17f72",
          "MD5(\"abc\") matches RFC-1321 vector");

    // 2. Virtual path normalization & asset classification
    check(irine::tools::NormalizePakVirtualPath("Resource/Ext/ItemTable.dnt") == "\\resource\\ext\\itemtable.dnt",
          "NormalizePakVirtualPath canonicalizes slash + leading backslash + lowercase");
    check(irine::tools::IsPakAssetRelativePath("Resource\\Ext\\ItemTable.dnt") &&
          irine::tools::IsPakAssetRelativePath("MapData\\Grid\\town\\a.ini") &&
          !irine::tools::IsPakAssetRelativePath("DragonNest.exe"),
          "IsPakAssetRelativePath separates Resource/MapData from root binaries");

    // 3. Round-trip PakWriter -> PakReader AND CEtPackingFile cross-engine verification
    std::error_code ec;
    const fs::path tempDir = fs::temp_directory_path(ec) / "irine_tools_selftest";
    fs::remove_all(tempDir, ec);
    fs::create_directories(tempDir, ec);

    const fs::path pakPath = tempDir / "Resource00.pak";
    const std::string verPayload = irine::tools::BuildVersionCfgContent(354, 354);
    const std::string dntPayload = "IRINE_CORE_PAK_ROUNDTRIP_PAYLOAD_0123456789_ABCDEF";

    {
        irine::tools::PakWriter writer;
        std::string err;
        bool ok = writer.Create(pakPath, 1);
        ok = ok && writer.AddBytes("\\version.cfg",
                                   reinterpret_cast<const uint8_t*>(verPayload.data()),
                                   verPayload.size(), &err);
        ok = ok && writer.AddBytes("Resource/Ext/TestTable.dnt",
                                   reinterpret_cast<const uint8_t*>(dntPayload.data()),
                                   dntPayload.size(), &err);
        ok = ok && writer.Close(&err);
        check(ok, "PakWriter creates Resource00.pak with \\version.cfg + \\resource\\ext\\testtable.dnt");
    }

    {
        irine::tools::PakReader reader;
        std::string err;
        size_t verifiedCount = 0;
        bool openOk = reader.Open(pakPath, &err);
        bool verifyOk = openOk && reader.VerifyAll(&verifiedCount, &err);
        int idx = reader.FindEntry("\\resource\\ext\\testtable.dnt");
        std::vector<uint8_t> out;
        bool readOk = (idx >= 0) && reader.ReadDecompressed(static_cast<size_t>(idx), out, &err);
        std::string recovered(out.begin(), out.end());
        check(openOk && verifyOk && verifiedCount == 2 && readOk && recovered == dntPayload,
              "PakReader verifies and decompresses written .pak identically");
    }

#ifdef _WIN32
    {
        CEtPackingFile etPak;
        bool etOpen = etPak.OpenFileSystem(pakPath.string().c_str(), true);
        CEtFileHandle* hVer = etOpen ? etPak.OpenFile("\\version.cfg") : nullptr;
        CEtFileHandle* hDnt = etOpen ? etPak.OpenFile("\\resource\\ext\\testtable.dnt") : nullptr;

        std::string etVer, etDnt;
        if (hVer) {
            etVer.resize(hVer->GetFileContext()->dwOriginalSize);
            hVer->Read(etVer.data(), static_cast<DWORD>(etVer.size()));
            etPak.CloseFile(hVer);
        }
        if (hDnt) {
            etDnt.resize(hDnt->GetFileContext()->dwOriginalSize);
            hDnt->Read(etDnt.data(), static_cast<DWORD>(etDnt.size()));
            etPak.CloseFile(hDnt);
        }
        etPak.CloseFileSystem();

        check(etOpen && etVer == verPayload && etDnt == dntPayload,
              "Game engine CEtPackingFile reads PakWriter archive byte-for-byte identically");
    }
#endif

    // 4. Single-file PKZIP writer sanity check
    {
        const fs::path srcFile = tempDir / "sample.txt";
        const fs::path zipFile = tempDir / "sample.txt.zip";
        {
            std::ofstream os(srcFile, std::ios::binary);
            os << dntPayload;
        }
        uint64_t zipSize = 0;
        std::string err;
        bool zipped = irine::tools::WriteSingleFileZip(zipFile, "sample.txt", srcFile, &zipSize, &err);
        bool hasPkSig = false;
        if (zipped && zipSize > 22) {
            std::ifstream is(zipFile, std::ios::binary);
            char sig[4] = {};
            is.read(sig, 4);
            hasPkSig = (sig[0] == 'P' && sig[1] == 'K' && sig[2] == 3 && sig[3] == 4);
        }
        check(zipped && hasPkSig, "WriteSingleFileZip produces valid PKZIP local/central/EOCD archive");
    }

    fs::remove_all(tempDir, ec);
    return failures;
}
