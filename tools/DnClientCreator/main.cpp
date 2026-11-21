// =============================================================================
// IrineCore — DnClientCreator
// Builds a complete, ready-to-play Dragon Nest client distribution by packing
// an unpacked GameRes / asset folder into split Resource00.pak..ResourceNN.pak
// archives (100% compatible with CEtPackingFile, DragonNest.exe, and DnLauncher),
// embedding \version.cfg, copying binaries/configs, and providing .pak inspect,
// extract, verify, and optimize utilities.
// =============================================================================

#include "PakArchive.h"
#include "ToolUtils.h"

#include <algorithm>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace irine::tools;

namespace {

void PrintUsage() {
    std::cout
        << "IrineCore DnClientCreator — Dragon Nest Client Builder & .pak Archive Manager\n"
        << "\n"
        << "Usage:\n"
        << "  1) Build a complete packed Client from a GameRes / complete asset folder:\n"
        << "     DnClientCreator --input <gameres-dir> --out <client-dir> [options]\n"
        << "\n"
        << "  2) List entries in a .pak archive:\n"
        << "     DnClientCreator --list <file.pak>\n"
        << "\n"
        << "  3) Extract a .pak file (or all Resource*.pak in a client directory) back to GameRes:\n"
        << "     DnClientCreator --extract <file.pak|client-dir> --out <extract-dir> [--verbose]\n"
        << "\n"
        << "  4) Verify integrity & decompression of a .pak file (or all .pak files in a directory):\n"
        << "     DnClientCreator --verify <file.pak|client-dir>\n"
        << "\n"
        << "  5) Optimize a .pak file (reclaim dead slots left by patches without recompressing):\n"
        << "     DnClientCreator --optimize <input.pak> [--out <output.pak>]\n"
        << "\n"
        << "Build Options:\n"
        << "  --input <dir>         Source GameRes / complete asset directory (containing Resource/, MapData/, etc.)\n"
        << "  --out <dir>           Output client directory\n"
        << "  --bin-dir <dir>       Optional directory of client binaries/DLLs (e.g. env/client) to copy into --out\n"
        << "  --nation <code>       Optional overlay folder suffix (merges Resource_<code> & MapData_<code> on top)\n"
        << "  --version <N>         Client version written to Version.cfg and embedded in Resource00.pak (default: 1)\n"
        << "  --module-version <N>  Module version written alongside --version (default: same as --version)\n"
        << "  --max-pak-mb <MB>     Maximum target size per ResourceXX.pak in MiB (default: 512, range: 16..1800)\n"
        << "  --prefix <name>       Archive filename prefix (default: Resource -> Resource00.pak, Resource01.pak, ...)\n"
        << "  --pack-all            Pack all files from --input into .pak (instead of only Resource/ and MapData/)\n"
        << "  --virtual-root <dir>  Prepend a virtual subfolder when using --pack-all (e.g. Resource)\n"
        << "  --level <0..9>        zlib compression level (default: 1 = Z_BEST_SPEED)\n"
        << "  --ignore <cfg>        Optional ignore rules file (RMakeCmd.cfg format)\n"
        << "  --md5                 Write .md5 sidecars for each ResourceXX.pak and ClientMD5List.txt\n"
        << "  --verify-output       Re-open and verify every generated .pak archive after packing\n"
        << "  --verbose, -v         Print per-file progress\n"
        << "  --help, -h            Show this help message\n";
}

std::string FormatPakFileName(const std::string& prefix, int index) {
    char buf[128];
    std::snprintf(buf, sizeof(buf), "%s%02d.pak", prefix.c_str(), index);
    return buf;
}

int CmdListPak(const fs::path& pakPath) {
    PakReader reader;
    std::string err;
    if (!reader.Open(pakPath, &err)) {
        std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
        return 1;
    }

    const auto& hdr = reader.Header();
    std::cout << "Archive : " << pakPath.string() << "\n"
              << "Header  : " << hdr.szHeaderString << "\n"
              << "Version : " << hdr.nVersion << "\n"
              << "Entries : " << reader.ActiveEntryCount() << " active / " << hdr.nFileCount << " total slots\n"
              << "Table   : offset " << hdr.dwFileInfoOffset << "\n\n";

    uint64_t totalOrig = 0;
    uint64_t totalComp = 0;
    for (const auto& e : reader.Entries()) {
        if (!e.IsActive()) continue;
        totalOrig += e.dwOriginalSize;
        totalComp += e.dwCompressSize;
        const double ratio = (e.dwOriginalSize > 0)
            ? (100.0 * static_cast<double>(e.dwCompressSize) / static_cast<double>(e.dwOriginalSize))
            : 0.0;
        std::cout << std::setw(10) << e.dwOriginalSize << "  "
                  << std::setw(10) << e.dwCompressSize << "  "
                  << std::fixed << std::setprecision(1) << std::setw(5) << ratio << "%  "
                  << e.szFileName << "\n";
    }
    std::cout << "\nTotal uncompressed: " << FormatByteSize(totalOrig)
              << " | Compressed: " << FormatByteSize(totalComp) << "\n";
    return 0;
}

std::vector<fs::path> ResolvePakTargets(const fs::path& target) {
    std::vector<fs::path> result;
    std::error_code ec;
    if (fs::is_regular_file(target, ec)) {
        result.push_back(target);
    } else if (fs::is_directory(target, ec)) {
        for (const auto& entry : fs::directory_iterator(target, ec)) {
            if (!entry.is_regular_file(ec)) continue;
            if (ToLowerString(entry.path().extension().string()) == ".pak") {
                result.push_back(entry.path());
            }
        }
        std::sort(result.begin(), result.end());
    }
    return result;
}

int CmdExtract(const fs::path& target, const fs::path& outDir, bool verbose) {
    if (outDir.empty()) {
        std::cerr << "[DnClientCreator] ERROR: --extract requires --out <extract-dir>.\n";
        return 1;
    }

    const auto paks = ResolvePakTargets(target);
    if (paks.empty()) {
        std::cerr << "[DnClientCreator] ERROR: No .pak files found at " << target.string() << "\n";
        return 1;
    }

    size_t totalExtracted = 0;
    for (const auto& pak : paks) {
        PakReader reader;
        std::string err;
        if (!reader.Open(pak, &err)) {
            std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
            return 1;
        }
        size_t extracted = 0;
        std::cout << "[DnClientCreator] Extracting " << pak.filename().string()
                  << " (" << reader.ActiveEntryCount() << " files) -> " << outDir.string() << "\n";
        if (!reader.ExtractAll(outDir, &extracted, verbose, &err)) {
            std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
            return 1;
        }
        totalExtracted += extracted;
    }

    std::cout << "[DnClientCreator] Extracted " << totalExtracted << " file(s) from " << paks.size() << " archive(s).\n";
    return 0;
}

int CmdVerify(const fs::path& target) {
    const auto paks = ResolvePakTargets(target);
    if (paks.empty()) {
        std::cerr << "[DnClientCreator] ERROR: No .pak files found at " << target.string() << "\n";
        return 1;
    }

    size_t totalChecked = 0;
    for (const auto& pak : paks) {
        PakReader reader;
        std::string err;
        if (!reader.Open(pak, &err)) {
            std::cerr << "[DnClientCreator] FAIL (" << pak.filename().string() << "): " << err << "\n";
            return 1;
        }
        size_t checked = 0;
        if (!reader.VerifyAll(&checked, &err)) {
            std::cerr << "[DnClientCreator] FAIL (" << pak.filename().string() << "): " << err << "\n";
            return 1;
        }
        std::cout << "[DnClientCreator] OK: " << pak.filename().string()
                  << " (" << checked << " active / " << reader.Entries().size() << " total entries verified)\n";
        totalChecked += checked;
    }

    std::cout << "[DnClientCreator] Verified " << totalChecked << " file(s) across " << paks.size() << " archive(s).\n";
    return 0;
}

int CmdOptimize(const fs::path& inputPak, const fs::path& outputPak) {
    std::error_code ec;
    const bool inPlace = outputPak.empty();
    const fs::path tempOut = inPlace ? fs::path(inputPak.string() + ".opt.tmp") : outputPak;

    PakReader reader;
    std::string err;
    if (!reader.Open(inputPak, &err)) {
        std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
        return 1;
    }

    PakWriter writer;
    if (!writer.Create(tempOut)) {
        std::cerr << "[DnClientCreator] ERROR: Cannot create " << tempOut.string() << "\n";
        return 1;
    }

    size_t kept = 0;
    size_t dropped = 0;
    std::vector<uint8_t> raw;
    const auto& entries = reader.Entries();
    for (size_t i = 0; i < entries.size(); ++i) {
        if (!entries[i].IsActive()) {
            ++dropped;
            continue;
        }
        if (!reader.ReadCompressed(i, raw, &err)) {
            std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
            return 1;
        }
        if (!writer.AddRawCompressedEntry(entries[i].szFileName, entries[i].dwOriginalSize, raw.data(), entries[i].dwCompressSize, &err)) {
            std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
            return 1;
        }
        ++kept;
    }

    reader.Close();
    if (!writer.Close(&err)) {
        std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
        return 1;
    }

    const uint64_t oldSize = fs::file_size(inputPak, ec);
    const uint64_t newSize = fs::file_size(tempOut, ec);

    if (inPlace) {
        fs::remove(inputPak, ec);
        fs::rename(tempOut, inputPak, ec);
        if (ec) {
            std::cerr << "[DnClientCreator] ERROR: Failed replacing " << inputPak.string() << " with optimized archive.\n";
            return 1;
        }
    }

    std::cout << "[DnClientCreator] Optimized " << (inPlace ? inputPak.string() : tempOut.string())
              << ": kept " << kept << " entries, reclaimed " << dropped << " dead slot(s) ("
              << FormatByteSize(oldSize) << " -> " << FormatByteSize(newSize) << ").\n";
    return 0;
}

bool CopyFileEnsuringDirs(const fs::path& src, const fs::path& dst, std::string* outError) {
    std::error_code ec;
    if (dst.has_parent_path()) {
        fs::create_directories(dst.parent_path(), ec);
    }
    fs::copy_file(src, dst, fs::copy_options::overwrite_existing, ec);
    if (ec) {
        if (outError) *outError = "Failed copying " + src.string() + " -> " + dst.string() + ": " + ec.message();
        return false;
    }
    return true;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    fs::path inputDir;
    fs::path outputDir;
    fs::path binDir;
    fs::path ignoreCfg;
    fs::path listTarget;
    fs::path extractTarget;
    fs::path verifyTarget;
    fs::path optimizeTarget;
    std::string nation;
    std::string pakPrefix = "Resource";
    std::string virtualRoot;
    int version = 1;
    int moduleVersion = -1;
    uint32_t maxPakMiB = 512;
    int compressionLevel = 1;
    bool packAll = false;
    bool writeMd5 = false;
    bool verifyOutput = false;
    bool verbose = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            return 0;
        } else if (arg == "--list" && i + 1 < argc) {
            listTarget = argv[++i];
        } else if (arg == "--extract" && i + 1 < argc) {
            extractTarget = argv[++i];
        } else if (arg == "--verify" && i + 1 < argc) {
            verifyTarget = argv[++i];
        } else if (arg == "--optimize" && i + 1 < argc) {
            optimizeTarget = argv[++i];
        } else if (arg == "--input" && i + 1 < argc) {
            inputDir = argv[++i];
        } else if (arg == "--out" && i + 1 < argc) {
            outputDir = argv[++i];
        } else if (arg == "--bin-dir" && i + 1 < argc) {
            binDir = argv[++i];
        } else if (arg == "--nation" && i + 1 < argc) {
            nation = argv[++i];
        } else if (arg == "--version" && i + 1 < argc) {
            version = std::stoi(argv[++i]);
        } else if (arg == "--module-version" && i + 1 < argc) {
            moduleVersion = std::stoi(argv[++i]);
        } else if (arg == "--max-pak-mb" && i + 1 < argc) {
            maxPakMiB = static_cast<uint32_t>(std::stoul(argv[++i]));
        } else if (arg == "--prefix" && i + 1 < argc) {
            pakPrefix = argv[++i];
        } else if (arg == "--pack-all") {
            packAll = true;
        } else if (arg == "--virtual-root" && i + 1 < argc) {
            virtualRoot = argv[++i];
        } else if (arg == "--level" && i + 1 < argc) {
            compressionLevel = std::stoi(argv[++i]);
        } else if (arg == "--ignore" && i + 1 < argc) {
            ignoreCfg = argv[++i];
        } else if (arg == "--md5") {
            writeMd5 = true;
        } else if (arg == "--verify-output") {
            verifyOutput = true;
        } else if (arg == "--verbose" || arg == "-v") {
            verbose = true;
        } else {
            std::cerr << "[DnClientCreator] Unknown option: " << arg << "\n";
            return 1;
        }
    }

    if (!listTarget.empty()) return CmdListPak(listTarget);
    if (!extractTarget.empty()) return CmdExtract(extractTarget, outputDir, verbose);
    if (!verifyTarget.empty()) return CmdVerify(verifyTarget);
    if (!optimizeTarget.empty()) return CmdOptimize(optimizeTarget, outputDir);

    if (inputDir.empty() || outputDir.empty()) {
        std::cerr << "[DnClientCreator] ERROR: --input <gameres-dir> and --out <client-dir> are required.\n";
        PrintUsage();
        return 1;
    }

    if (moduleVersion < 0) moduleVersion = version;
    if (maxPakMiB < 16) maxPakMiB = 16;
    if (maxPakMiB > 1800) maxPakMiB = 1800;
    const uint64_t maxPakBytes = static_cast<uint64_t>(maxPakMiB) * 1024ULL * 1024ULL;

    std::error_code ec;
    if (!fs::exists(inputDir, ec) || !fs::is_directory(inputDir, ec)) {
        std::cerr << "[DnClientCreator] ERROR: Input directory does not exist: " << inputDir.string() << "\n";
        return 1;
    }
    fs::create_directories(outputDir, ec);
    if (ec) {
        std::cerr << "[DnClientCreator] ERROR: Cannot create output directory: " << outputDir.string() << "\n";
        return 1;
    }

    IgnoreFilter filter;
    filter.AddDefaults();
    if (!ignoreCfg.empty() && !filter.LoadFromConfigFile(ignoreCfg, "client")) {
        std::cerr << "[DnClientCreator] WARNING: Could not read ignore config: " << ignoreCfg.string() << "\n";
    }

    // Collect files from --input. If --nation is given, Resource_<nation> and MapData_<nation>
    // overlay Resource and MapData cleanly (matching RMakeCmd behavior).
    std::vector<DiscoveredFile> discovered = CollectFilesRecursive(inputDir, &filter);

    struct StagedAsset {
        fs::path diskPath;
        std::string virtualPath; // normalized pak path (for pak entries) or relative path (for loose copy)
        uint64_t fileSize = 0;
    };

    std::map<std::string, StagedAsset> pakAssets;   // keyed by lowercase normalized virtual path
    std::map<std::string, StagedAsset> looseAssets; // keyed by lowercase relative path

    const std::string lowerNationResPrefix = nation.empty() ? "" : ("resource_" + ToLowerString(nation) + "\\");
    const std::string lowerNationMapPrefix = nation.empty() ? "" : ("mapdata_" + ToLowerString(nation) + "\\");

    auto stageOne = [&](const DiscoveredFile& df, bool isNationOverlay) {
        std::string rel = df.relPathBackslash;
        std::string lowerRel = ToLowerString(rel);

        if (!nation.empty()) {
            if (lowerRel.rfind(lowerNationResPrefix, 0) == 0) {
                if (!isNationOverlay) return;
                rel = "Resource\\" + rel.substr(lowerNationResPrefix.size());
                lowerRel = ToLowerString(rel);
            } else if (lowerRel.rfind(lowerNationMapPrefix, 0) == 0) {
                if (!isNationOverlay) return;
                rel = "MapData\\" + rel.substr(lowerNationMapPrefix.size());
                lowerRel = ToLowerString(rel);
            } else if (isNationOverlay) {
                return;
            }
        }

        // Skip any existing Version.cfg or Resource*.pak in input when building a fresh packed client
        const std::string base = ToLowerString(df.diskPath.filename().string());
        if (base == "version.cfg") return;

        const bool shouldPack = packAll || IsPakAssetRelativePath(rel);
        if (shouldPack) {
            std::string virtRel = rel;
            if (packAll && !virtualRoot.empty()) {
                virtRel = StripLeadingPakSeparator(virtualRoot) + "\\" + rel;
            }
            const std::string normVirt = NormalizePakVirtualPath(virtRel);
            pakAssets[normVirt] = { df.diskPath, normVirt, df.fileSize };
        } else {
            looseAssets[lowerRel] = { df.diskPath, rel, df.fileSize };
        }
    };

    // Pass 1: base files; Pass 2: nation overlays overwrite matching base entries
    for (const auto& df : discovered) stageOne(df, false);
    if (!nation.empty()) {
        for (const auto& df : discovered) stageOne(df, true);
    }

    if (pakAssets.empty()) {
        std::cerr << "[DnClientCreator] WARNING: No Resource/ or MapData/ files matched in " << inputDir.string() << ".\n"
                  << "  Tip: If pointing directly at an inner Resource folder, pass --pack-all --virtual-root Resource\n";
    }

    std::cout << "[DnClientCreator] Creating Dragon Nest client in " << outputDir.string() << "\n"
              << "  Source folder  : " << inputDir.string() << "\n"
              << "  Assets to pack : " << pakAssets.size() << " file(s)\n"
              << "  Root files     : " << looseAssets.size() << " file(s)\n"
              << "  Split limit    : " << maxPakMiB << " MiB per " << pakPrefix << "XX.pak\n"
              << "  Version        : " << version << " (Module " << moduleVersion << ")\n";

    std::vector<fs::path> createdPaks;
    int pakIndex = 0;
    PakWriter writer;
    std::string err;

    auto openNextPak = [&]() -> bool {
        const fs::path pakPath = outputDir / FormatPakFileName(pakPrefix, pakIndex);
        if (!writer.Create(pakPath, compressionLevel)) {
            std::cerr << "[DnClientCreator] ERROR: Failed creating " << pakPath.string() << "\n";
            return false;
        }
        createdPaks.push_back(pakPath);
        std::cout << "  [pak] Opening " << pakPath.filename().string() << "\n";

        // Embed \version.cfg inside Resource00.pak so DnLauncher (_USE_SINGLE_CLIENT) reads client version cleanly.
        if (pakIndex == 0) {
            const std::string verContent = BuildVersionCfgContent(version, moduleVersion);
            if (!writer.AddBytes("\\version.cfg",
                                 reinterpret_cast<const uint8_t*>(verContent.data()),
                                 verContent.size(),
                                 &err)) {
                std::cerr << "[DnClientCreator] ERROR: Failed embedding \\version.cfg: " << err << "\n";
                return false;
            }
        }
        return true;
    };

    if (!openNextPak()) return 1;

    size_t packedCount = 0;
    size_t skippedEmpty = 0;
    uint64_t totalRawBytes = 0;

    for (const auto& kv : pakAssets) {
        const auto& asset = kv.second;
        if (asset.fileSize == 0) {
            ++skippedEmpty;
            continue;
        }

        // Roll over to next ResourceXX.pak before exceeding maxPakBytes (as long as current pak already has assets)
        if (writer.EntryCount() > (pakIndex == 0 ? 1u : 0u) &&
            writer.EstimatedFinalSizeBytes() >= maxPakBytes) {
            if (!writer.Close(&err)) {
                std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
                return 1;
            }
            ++pakIndex;
            if (!openNextPak()) return 1;
        }

        if (!writer.AddFileFromDisk(asset.virtualPath, asset.diskPath, &err)) {
            std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
            return 1;
        }

        ++packedCount;
        totalRawBytes += asset.fileSize;
        if (verbose) {
            std::cout << "    + " << asset.virtualPath << " (" << asset.fileSize << " bytes)\n";
        } else if (packedCount % 1000 == 0 || packedCount == pakAssets.size()) {
            std::cout << "  [pack] " << packedCount << "/" << pakAssets.size() << " assets packed...\n";
        }
    }

    if (!writer.Close(&err)) {
        std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
        return 1;
    }

    // Copy non-Resource/MapData loose files from --input into client root
    size_t copiedRoot = 0;
    for (const auto& kv : looseAssets) {
        std::string portableRel = kv.second.virtualPath;
        std::replace(portableRel.begin(), portableRel.end(), '\\', '/');
        if (!CopyFileEnsuringDirs(kv.second.diskPath, outputDir / fs::path(portableRel), &err)) {
            std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
            return 1;
        }
        ++copiedRoot;
    }

    // Copy compiled binaries & runtime DLLs from --bin-dir if provided
    size_t copiedBins = 0;
    if (!binDir.empty()) {
        if (!fs::exists(binDir, ec) || !fs::is_directory(binDir, ec)) {
            std::cerr << "[DnClientCreator] ERROR: --bin-dir does not exist: " << binDir.string() << "\n";
            return 1;
        }
        std::vector<DiscoveredFile> binFiles = CollectFilesRecursive(binDir, &filter);
        for (const auto& bf : binFiles) {
            std::string portableRel = bf.relPathBackslash;
            std::replace(portableRel.begin(), portableRel.end(), '\\', '/');
            if (!CopyFileEnsuringDirs(bf.diskPath, outputDir / fs::path(portableRel), &err)) {
                std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
                return 1;
            }
            ++copiedBins;
        }
    }

    // Write loose Version.cfg in client root alongside Resource00.pak
    if (!WriteVersionCfgFile(outputDir / "Version.cfg", version, moduleVersion, &err)) {
        std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
        return 1;
    }

    // Optional post-pack verification
    if (verifyOutput) {
        std::cout << "[DnClientCreator] Verifying generated .pak archives...\n";
        for (const auto& pak : createdPaks) {
            PakReader reader;
            size_t checked = 0;
            if (!reader.Open(pak, &err) || !reader.VerifyAll(&checked, &err)) {
                std::cerr << "[DnClientCreator] ERROR: Post-pack verify failed on " << pak.string() << ": " << err << "\n";
                return 1;
            }
            std::cout << "  [verify] OK " << pak.filename().string() << " (" << checked << " entries)\n";
        }
    }

    // Optional MD5 sidecars + ClientMD5List.txt
    if (writeMd5) {
        std::ofstream manifest(outputDir / "ClientMD5List.txt", std::ios::binary | std::ios::trunc);
        for (const auto& pak : createdPaks) {
            std::string digest, sidecar;
            if (!WriteMD5Sidecar(pak, false, &digest, &sidecar, &err)) {
                std::cerr << "[DnClientCreator] ERROR: " << err << "\n";
                return 1;
            }
            if (manifest.is_open()) {
                manifest << digest << "  " << pak.filename().string() << "\n";
            }
        }
        manifest.close();
    }

    uint64_t totalPakBytes = 0;
    for (const auto& pak : createdPaks) {
        totalPakBytes += fs::file_size(pak, ec);
    }

    std::cout << "[DnClientCreator] Client build complete!\n"
              << "  Output dir     : " << outputDir.string() << "\n"
              << "  Archives       : " << createdPaks.size() << " (" << FormatByteSize(totalPakBytes) << " total)\n"
              << "  Packed assets  : " << packedCount << " (" << FormatByteSize(totalRawBytes) << " raw";
    if (skippedEmpty > 0) std::cout << ", " << skippedEmpty << " empty skipped";
    std::cout << ")\n"
              << "  Root/Bin files : " << (copiedRoot + copiedBins) << " copied\n"
              << "  Version.cfg    : Version " << version << " / Module " << moduleVersion << "\n";
    return 0;
}
