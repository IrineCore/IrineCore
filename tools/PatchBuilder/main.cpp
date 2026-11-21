// =============================================================================
// IrineCore — PatchBuilder
// Builds versioned Dragon Nest incremental patches compatible with DnLauncher
// and deploy/webserver/scripts/sync-patches.sh:
//   <out>/<0000000N>/Patch<0000000N>.pak
//   <out>/<0000000N>/Patch<0000000N>.pak.md5
//   <out>/<0000000N>/Patch<0000000N>.txt
//   <out>/PatchInfoServer.cfg
// =============================================================================

#include "PakArchive.h"
#include "ToolUtils.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace irine::tools;

namespace {

void PrintUsage() {
    std::cout
        << "IrineCore PatchBuilder — Incremental Patch & PatchInfoServer.cfg Generator\n"
        << "\n"
        << "Usage:\n"
        << "  1) Staging folder mode (pack all files in a staging folder into a patch):\n"
        << "     PatchBuilder --input <staging-dir> --out <patch-root> [--version <N>] [options]\n"
        << "\n"
        << "  2) Diff mode (automatically diff old baseline vs new build/GameRes):\n"
        << "     PatchBuilder --old <old-dir> --new <new-dir> --out <patch-root> [--version <N>] [options]\n"
        << "\n"
        << "Options:\n"
        << "  --input <dir>        Folder of changed files (Resource/, MapData/, root binaries/DLLs)\n"
        << "  --old <dir>          Previous client or GameRes directory (supports packed Resource*.pak too)\n"
        << "  --new <dir>          Updated client or GameRes directory\n"
        << "  --out <dir>          Patch repository root (receives <0000000N>/ and PatchInfoServer.cfg)\n"
        << "  --version <N>        Patch version number (auto-increments from PatchInfoServer.cfg if omitted)\n"
        << "  --delete-list <file> Optional text file listing relative paths to delete (one per line)\n"
        << "  --skip-version <N>   Add SkipNum <N> to PatchInfoServer.cfg (repeatable)\n"
        << "  --ignore <cfg>       Optional ignore rules file (RMakeCmd.cfg format)\n"
        << "  --level <0..9>       zlib compression level (default: 1 = Z_BEST_SPEED)\n"
        << "  --upper-ext          Write .MD5 extension instead of .md5\n"
        << "  --verify             Verify generated PatchXXXXXXXX.pak after creation\n"
        << "  --help, -h           Show this help message\n";
}

struct FileFingerprint {
    uint64_t size = 0;
    std::string md5;
    std::string originalRelPath;
};

// Indexes either an unpacked folder or a client directory containing Resource*.pak archives.
bool IndexBaselineDirectory(const fs::path& dir,
                            const IgnoreFilter& filter,
                            std::map<std::string, FileFingerprint>& outMap) {
    outMap.clear();
    std::error_code ec;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        std::cerr << "[PatchBuilder] ERROR: Baseline directory not found: " << dir.string() << "\n";
        return false;
    }

    // First index any Resource*.pak files inside dir (so users can diff a packed client against unpacked GameRes!)
    std::vector<fs::path> pakFiles;
    for (const auto& entry : fs::directory_iterator(dir, ec)) {
        if (!entry.is_regular_file(ec)) continue;
        const std::string lower = ToLowerString(entry.path().filename().string());
        if (lower.rfind("resource", 0) == 0 && lower.size() > 4 && lower.substr(lower.size() - 4) == ".pak") {
            pakFiles.push_back(entry.path());
        }
    }
    std::sort(pakFiles.begin(), pakFiles.end());

    std::vector<uint8_t> decomp;
    for (const auto& pakPath : pakFiles) {
        PakReader reader;
        std::string err;
        if (!reader.Open(pakPath, &err)) {
            std::cerr << "[PatchBuilder] WARNING: Skipping unreadable pak " << pakPath.string() << ": " << err << "\n";
            continue;
        }
        const auto& entries = reader.Entries();
        for (size_t i = 0; i < entries.size(); ++i) {
            if (!entries[i].IsActive()) continue;
            const std::string rel = StripLeadingPakSeparator(entries[i].szFileName);
            const std::string key = ToLowerString(rel);
            if (key == "version.cfg") continue;
            if (!reader.ReadDecompressed(i, decomp, &err)) {
                std::cerr << "[PatchBuilder] ERROR: Failed reading " << rel << " from " << pakPath.string() << ": " << err << "\n";
                return false;
            }
            FileFingerprint fp;
            fp.size = decomp.size();
            fp.md5 = ComputeBufferMD5(decomp.data(), decomp.size());
            fp.originalRelPath = rel;
            outMap[key] = std::move(fp);
        }
    }

    // Then index loose files in dir (ignoring Resource*.pak and Version.cfg when paks were present)
    std::vector<DiscoveredFile> loose = CollectFilesRecursive(dir, &filter);
    for (const auto& df : loose) {
        const std::string key = ToLowerString(df.relPathBackslash);
        if (!pakFiles.empty()) {
            const std::string base = ToLowerString(df.diskPath.filename().string());
            if ((base.rfind("resource", 0) == 0 && base.size() > 4 &&
                 (base.substr(base.size() - 4) == ".pak" || (base.size() > 8 && base.substr(base.size() - 8) == ".pak.md5"))) ||
                base == "version.cfg" || base == "clientmd5list.txt") {
                continue;
            }
        }
        std::string md5, err;
        if (!ComputeFileMD5(df.diskPath, md5, &err)) {
            std::cerr << "[PatchBuilder] ERROR: " << err << "\n";
            return false;
        }
        FileFingerprint fp;
        fp.size = df.fileSize;
        fp.md5 = md5;
        fp.originalRelPath = df.relPathBackslash;
        outMap[key] = std::move(fp);
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
    fs::path oldDir;
    fs::path newDir;
    fs::path outRoot;
    fs::path deleteListPath;
    fs::path ignoreCfg;
    int version = -1;
    int compressionLevel = 1;
    bool upperExt = false;
    bool verify = false;
    std::vector<int> skipVersions;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            return 0;
        } else if (arg == "--input" && i + 1 < argc) {
            inputDir = argv[++i];
        } else if (arg == "--old" && i + 1 < argc) {
            oldDir = argv[++i];
        } else if (arg == "--new" && i + 1 < argc) {
            newDir = argv[++i];
        } else if (arg == "--out" && i + 1 < argc) {
            outRoot = argv[++i];
        } else if (arg == "--version" && i + 1 < argc) {
            version = std::stoi(argv[++i]);
        } else if (arg == "--delete-list" && i + 1 < argc) {
            deleteListPath = argv[++i];
        } else if (arg == "--skip-version" && i + 1 < argc) {
            skipVersions.push_back(std::stoi(argv[++i]));
        } else if (arg == "--ignore" && i + 1 < argc) {
            ignoreCfg = argv[++i];
        } else if (arg == "--level" && i + 1 < argc) {
            compressionLevel = std::stoi(argv[++i]);
        } else if (arg == "--upper-ext") {
            upperExt = true;
        } else if (arg == "--verify") {
            verify = true;
        } else {
            std::cerr << "[PatchBuilder] Unknown option: " << arg << "\n";
            return 1;
        }
    }

    const bool diffMode = !oldDir.empty() || !newDir.empty();
    if (outRoot.empty()) {
        std::cerr << "[PatchBuilder] ERROR: --out <patch-root> is required.\n";
        return 1;
    }
    if (diffMode && (oldDir.empty() || newDir.empty())) {
        std::cerr << "[PatchBuilder] ERROR: Diff mode requires both --old and --new directories.\n";
        return 1;
    }
    if (!diffMode && inputDir.empty()) {
        std::cerr << "[PatchBuilder] ERROR: Provide either --input <dir> or --old <dir> --new <dir>.\n";
        return 1;
    }

    // Load existing PatchInfoServer.cfg (if present) for auto-versioning & preserving SkipNum entries
    PatchInfoServerData serverInfo;
    const fs::path serverCfgPath = outRoot / "PatchInfoServer.cfg";
    ReadPatchInfoServerCfg(serverCfgPath, serverInfo);
    if (version <= 0) {
        version = (serverInfo.version > 0) ? (serverInfo.version + 1) : 1;
    }
    for (int sv : skipVersions) {
        if (std::find(serverInfo.skipNumbers.begin(), serverInfo.skipNumbers.end(), sv) == serverInfo.skipNumbers.end()) {
            serverInfo.skipNumbers.push_back(sv);
        }
    }
    std::sort(serverInfo.skipNumbers.begin(), serverInfo.skipNumbers.end());

    IgnoreFilter filter;
    filter.AddDefaults();
    if (!ignoreCfg.empty() && !filter.LoadFromConfigFile(ignoreCfg, "client")) {
        std::cerr << "[PatchBuilder] WARNING: Could not read ignore config: " << ignoreCfg.string() << "\n";
    }

    std::vector<DiscoveredFile> modifiedFiles;
    std::vector<std::string> deleteFiles;

    if (diffMode) {
        std::cout << "[PatchBuilder] Indexing baseline: " << oldDir.string() << "\n";
        std::map<std::string, FileFingerprint> oldIndex;
        if (!IndexBaselineDirectory(oldDir, filter, oldIndex)) {
            return 1;
        }

        std::cout << "[PatchBuilder] Scanning updated build: " << newDir.string() << "\n";
        std::vector<DiscoveredFile> newFiles = CollectFilesRecursive(newDir, &filter);
        std::set<std::string> seenInNew;

        for (const auto& df : newFiles) {
            const std::string key = ToLowerString(df.relPathBackslash);
            const std::string base = ToLowerString(df.diskPath.filename().string());
            if (base == "version.cfg") continue;
            seenInNew.insert(key);

            auto it = oldIndex.find(key);
            if (it == oldIndex.end()) {
                modifiedFiles.push_back(df);
                continue;
            }
            if (it->second.size != df.fileSize) {
                modifiedFiles.push_back(df);
                continue;
            }
            std::string newMd5, err;
            if (!ComputeFileMD5(df.diskPath, newMd5, &err)) {
                std::cerr << "[PatchBuilder] ERROR: " << err << "\n";
                return 1;
            }
            if (newMd5 != it->second.md5) {
                modifiedFiles.push_back(df);
            }
        }

        for (const auto& kv : oldIndex) {
            if (seenInNew.find(kv.first) == seenInNew.end()) {
                deleteFiles.push_back(kv.second.originalRelPath);
            }
        }
    } else {
        std::error_code ec;
        if (!fs::exists(inputDir, ec) || !fs::is_directory(inputDir, ec)) {
            std::cerr << "[PatchBuilder] ERROR: Input staging directory not found: " << inputDir.string() << "\n";
            return 1;
        }
        modifiedFiles = CollectFilesRecursive(inputDir, &filter);
    }

    // Optional explicit delete list
    if (!deleteListPath.empty()) {
        std::ifstream dl(deleteListPath);
        if (!dl.is_open()) {
            std::cerr << "[PatchBuilder] ERROR: Failed to open delete list: " << deleteListPath.string() << "\n";
            return 1;
        }
        std::string line;
        while (std::getline(dl, line)) {
            while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' ')) line.pop_back();
            if (line.empty() || line[0] == '#') continue;
            deleteFiles.push_back(StripLeadingPakSeparator(line));
        }
    }

    if (modifiedFiles.empty() && deleteFiles.empty()) {
        std::cout << "[PatchBuilder] No changes detected — nothing to patch.\n";
        return 0;
    }

    std::sort(deleteFiles.begin(), deleteFiles.end());
    deleteFiles.erase(std::unique(deleteFiles.begin(), deleteFiles.end()), deleteFiles.end());

    const std::string ver8 = FormatPatchVersion8(version);
    const fs::path versionDir = outRoot / ver8;
    std::error_code ec;
    fs::create_directories(versionDir, ec);

    const fs::path pakPath = versionDir / ("Patch" + ver8 + ".pak");
    const fs::path txtPath = versionDir / ("Patch" + ver8 + ".txt");

    std::cout << "[PatchBuilder] Building patch v" << version << " (" << ver8 << ")\n"
              << "  Added/Modified : " << modifiedFiles.size() << " file(s)\n"
              << "  Deleted        : " << deleteFiles.size() << " file(s)\n";

    // Create PatchXXXXXXXX.pak containing ALL modified files (both Resource/MapData pak entries AND Copy list entries)
    PakWriter writer;
    if (!writer.Create(pakPath, compressionLevel)) {
        std::cerr << "[PatchBuilder] ERROR: Failed to create " << pakPath.string() << "\n";
        return 1;
    }

    std::vector<std::string> copyList;
    size_t pakAssetCount = 0;

    for (const auto& df : modifiedFiles) {
        std::string err;
        if (!writer.AddFileFromDisk(df.relPathBackslash, df.diskPath, &err)) {
            std::cerr << "[PatchBuilder] ERROR: " << err << "\n";
            return 1;
        }
        if (IsPakAssetRelativePath(df.relPathBackslash)) {
            ++pakAssetCount;
        } else {
            copyList.push_back(df.relPathBackslash);
        }
    }

    std::string err;
    if (!writer.Close(&err)) {
        std::cerr << "[PatchBuilder] ERROR: " << err << "\n";
        return 1;
    }

    // Write PatchXXXXXXXX.txt (exact DnLauncher format: D <path> and C <path>, DnLauncher.exe last, trailing space)
    {
        std::ofstream txt(txtPath, std::ios::binary | std::ios::trunc);
        if (!txt.is_open()) {
            std::cerr << "[PatchBuilder] ERROR: Failed to write " << txtPath.string() << "\n";
            return 1;
        }

        for (const auto& del : deleteFiles) {
            txt << "D " << del << "\r\n";
        }

        std::string deferredLauncher;
        for (const auto& cp : copyList) {
            if (ToLowerString(cp) == "dnlauncher.exe") {
                deferredLauncher = cp;
                continue;
            }
            txt << "C " << cp << "\r\n";
        }
        if (!deferredLauncher.empty()) {
            txt << "C " << deferredLauncher << "\r\n";
        }

        // Prevent 0-byte PatchXXXXXXXX.txt so HTTP servers/DnLauncher always download cleanly
        if (deleteFiles.empty() && copyList.empty()) {
            txt << " ";
        }
        txt.close();
    }

    // Verify .pak integrity if requested
    if (verify) {
        PakReader reader;
        size_t checked = 0;
        if (!reader.Open(pakPath, &err) || !reader.VerifyAll(&checked, &err)) {
            std::cerr << "[PatchBuilder] ERROR: Verification failed: " << err << "\n";
            return 1;
        }
        std::cout << "  [verify] Verified " << checked << " entry(ies) in " << pakPath.filename().string() << "\n";
    }

    // Write PatchXXXXXXXX.pak.md5 (or .MD5)
    std::string digest, sidecar;
    if (!WriteMD5Sidecar(pakPath, upperExt, &digest, &sidecar, &err)) {
        std::cerr << "[PatchBuilder] ERROR: " << err << "\n";
        return 1;
    }

    // Update PatchInfoServer.cfg
    serverInfo.version = version;
    if (!WritePatchInfoServerCfg(serverCfgPath, serverInfo, &err)) {
        std::cerr << "[PatchBuilder] ERROR: " << err << "\n";
        return 1;
    }

    std::cout << "[PatchBuilder] Patch build succeeded!\n"
              << "  Patch PAK  : " << pakPath.string() << " (" << pakAssetCount << " pak assets, " << copyList.size() << " root files)\n"
              << "  Patch MD5  : " << digest << " (" << sidecar << ")\n"
              << "  Patch List : " << txtPath.string() << "\n"
              << "  Server Cfg : " << serverCfgPath.string() << " (version " << version << ")\n";
    return 0;
}
