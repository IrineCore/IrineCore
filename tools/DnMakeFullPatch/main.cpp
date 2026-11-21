// =============================================================================
// IrineCore — DnMakeFullPatch
// Builds Dragon Nest full-patch distributions:
//   1. Standard Full Patch mode (matches original DnMakeFullPatch):
//      Compresses every client file to <out>/<relpath>.zip, writes
//      fullpatch_svr.cfg (origSize|zipSize|md5|relPath) and fullpatch_svr.md5.
//   2. Patch PAK Merge mode (--merge-paks):
//      Consolidates sequential PatchXXXXXXXX.pak archives (honoring D delete
//      entries from PatchXXXXXXXX.txt) into one single .pak + .pak.md5.
// =============================================================================

#include "PakArchive.h"
#include "ToolUtils.h"

#include <algorithm>
#include <cstdio>
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
        << "IrineCore DnMakeFullPatch — Full Patch & Patch Consolidation Builder\n"
        << "\n"
        << "Usage:\n"
        << "  1) Standard Full Patch (per-file .zip + fullpatch_svr.cfg + fullpatch_svr.md5):\n"
        << "     DnMakeFullPatch <input-client-dir> <output-dir> [--ignore <cfg>]\n"
        << "\n"
        << "  2) Merge sequential PatchXXXXXXXX.pak archives into one consolidated .pak:\n"
        << "     DnMakeFullPatch --merge-paks <patch-root-dir> --out <output.pak> [--from <ver>] [--to <ver>]\n"
        << "     DnMakeFullPatch --merge-paks <a.pak> <b.pak> [...] --out <output.pak>\n"
        << "\n"
        << "Options:\n"
        << "  --ignore <cfg>     Optional ignore rules file (RMakeCmd.cfg format)\n"
        << "  --merge-paks       Consolidate multiple .pak files (or versioned patch dir) into one .pak\n"
        << "  --out <path>       Output .pak file path (required for --merge-paks)\n"
        << "  --from <ver>       Start version inclusive when merging a patch root directory\n"
        << "  --to <ver>         End version inclusive when merging a patch root directory\n"
        << "  --upper-ext        Write .MD5 instead of .md5 for merged pak checksum\n"
        << "  --help, -h         Show this help message\n";
}

struct FullPatchEntry {
    uint64_t originalSize = 0;
    uint64_t compressedSize = 0;
    std::string md5;
    std::string relPath;
};

int RunStandardFullPatch(const fs::path& inputDir, const fs::path& outputDir, const fs::path& ignoreCfg) {
    std::error_code ec;
    if (!fs::exists(inputDir, ec) || !fs::is_directory(inputDir, ec)) {
        std::cerr << "[DnMakeFullPatch] ERROR: Input directory does not exist: " << inputDir.string() << "\n";
        return 1;
    }

    fs::create_directories(outputDir, ec);
    if (ec) {
        std::cerr << "[DnMakeFullPatch] ERROR: Failed to create output directory: " << outputDir.string() << "\n";
        return 1;
    }

    IgnoreFilter filter;
    filter.AddDefaults();
    if (!ignoreCfg.empty() && !filter.LoadFromConfigFile(ignoreCfg, "client")) {
        std::cerr << "[DnMakeFullPatch] WARNING: Could not read ignore config: " << ignoreCfg.string() << "\n";
    }

    std::vector<DiscoveredFile> files = CollectFilesRecursive(inputDir, &filter);
    if (files.empty()) {
        std::cerr << "[DnMakeFullPatch] ERROR: No files found in " << inputDir.string() << "\n";
        return 1;
    }

    std::cout << "[DnMakeFullPatch] Building full patch from " << inputDir.string()
              << " (" << files.size() << " files) -> " << outputDir.string() << "\n";

    std::vector<FullPatchEntry> manifest;
    manifest.reserve(files.size());

    uint64_t totalOrig = 0;
    uint64_t totalZip = 0;

    for (size_t i = 0; i < files.size(); ++i) {
        const auto& df = files[i];
        std::string md5, err;
        if (!ComputeFileMD5(df.diskPath, md5, &err)) {
            std::cerr << "[DnMakeFullPatch] ERROR: " << err << "\n";
            return 1;
        }

        std::string portableRel = df.relPathBackslash;
        std::replace(portableRel.begin(), portableRel.end(), '\\', '/');
        fs::path zipPath = outputDir / fs::path(portableRel + ".zip");
        std::string entryName = df.diskPath.filename().string();

        uint64_t zipSize = 0;
        if (!WriteSingleFileZip(zipPath, entryName, df.diskPath, &zipSize, &err)) {
            std::cerr << "[DnMakeFullPatch] ERROR: " << err << "\n";
            return 1;
        }

        FullPatchEntry entry;
        entry.originalSize = df.fileSize;
        entry.compressedSize = zipSize;
        entry.md5 = md5;
        entry.relPath = df.relPathBackslash;
        manifest.push_back(std::move(entry));

        totalOrig += df.fileSize;
        totalZip += zipSize;

        if ((i + 1) % 250 == 0 || i + 1 == files.size()) {
            std::cout << "  [" << (i + 1) << "/" << files.size() << "] " << df.relPathBackslash << "\n";
        }
    }

    // Write fullpatch_svr.cfg in exact original format: "%10llu|%10llu|%-32s|%s\n"
    const fs::path cfgPath = outputDir / "fullpatch_svr.cfg";
    {
        std::ofstream cfg(cfgPath, std::ios::binary | std::ios::trunc);
        if (!cfg.is_open()) {
            std::cerr << "[DnMakeFullPatch] ERROR: Failed to write " << cfgPath.string() << "\n";
            return 1;
        }
        char lineBuf[1024];
        for (const auto& e : manifest) {
            std::snprintf(lineBuf, sizeof(lineBuf), "%10llu|%10llu|%-32s|%s\n",
                          static_cast<unsigned long long>(e.originalSize),
                          static_cast<unsigned long long>(e.compressedSize),
                          e.md5.c_str(),
                          e.relPath.c_str());
            cfg << lineBuf;
        }
        cfg.close();
    }

    // Write fullpatch_svr.md5 (MD5 of fullpatch_svr.cfg)
    std::string cfgMd5, err;
    if (!ComputeFileMD5(cfgPath, cfgMd5, &err)) {
        std::cerr << "[DnMakeFullPatch] ERROR: " << err << "\n";
        return 1;
    }

    const fs::path md5Path = outputDir / "fullpatch_svr.md5";
    {
        std::ofstream mf(md5Path, std::ios::binary | std::ios::trunc);
        if (!mf.is_open()) {
            std::cerr << "[DnMakeFullPatch] ERROR: Failed to write " << md5Path.string() << "\n";
            return 1;
        }
        mf << cfgMd5 << "\n";
        mf.close();
    }

    std::cout << "[DnMakeFullPatch] Done!\n"
              << "  Files packaged : " << manifest.size() << "\n"
              << "  Original total : " << FormatByteSize(totalOrig) << "\n"
              << "  Zipped total   : " << FormatByteSize(totalZip) << "\n"
              << "  Manifest       : " << cfgPath.string() << "\n"
              << "  Manifest MD5   : " << cfgMd5 << " (" << md5Path.string() << ")\n";
    return 0;
}

struct PakMergeStep {
    int version = -1;
    fs::path pakPath;
    fs::path txtPath;
};

int RunMergePaks(const std::vector<fs::path>& inputs,
                 const fs::path& outPak,
                 int fromVer,
                 int toVer,
                 bool upperExt) {
    if (inputs.empty() || outPak.empty()) {
        std::cerr << "[DnMakeFullPatch] ERROR: --merge-paks requires input path(s) and --out <output.pak>.\n";
        return 1;
    }

    std::vector<PakMergeStep> steps;
    std::error_code ec;

    for (const auto& inp : inputs) {
        if (fs::is_directory(inp, ec)) {
            // Discover versioned folders <0000000N>/Patch<0000000N>.pak
            for (const auto& entry : fs::directory_iterator(inp, ec)) {
                if (!entry.is_directory(ec)) continue;
                const std::string dirName = entry.path().filename().string();
                if (dirName.empty() || !std::all_of(dirName.begin(), dirName.end(), ::isdigit)) continue;
                const int ver = std::stoi(dirName);
                if (fromVer > 0 && ver < fromVer) continue;
                if (toVer > 0 && ver > toVer) continue;

                const std::string ver8 = FormatPatchVersion8(ver);
                const fs::path pakFile = entry.path() / ("Patch" + ver8 + ".pak");
                const fs::path txtFile = entry.path() / ("Patch" + ver8 + ".txt");
                if (fs::exists(pakFile, ec)) {
                    steps.push_back({ ver, pakFile, fs::exists(txtFile, ec) ? txtFile : fs::path{} });
                }
            }
            std::sort(steps.begin(), steps.end(), [](const PakMergeStep& a, const PakMergeStep& b) {
                return a.version < b.version;
            });
        } else if (fs::is_regular_file(inp, ec)) {
            fs::path txtSibling = inp;
            txtSibling.replace_extension(".txt");
            steps.push_back({ -1, inp, fs::exists(txtSibling, ec) ? txtSibling : fs::path{} });
        } else {
            std::cerr << "[DnMakeFullPatch] ERROR: Input not found: " << inp.string() << "\n";
            return 1;
        }
    }

    if (steps.empty()) {
        std::cerr << "[DnMakeFullPatch] ERROR: No .pak files found to merge.\n";
        return 1;
    }

    // Track winning source (stepIndex, entryIndex) per normalized virtual path
    struct WinningEntry {
        size_t stepIndex = 0;
        size_t entryIndex = 0;
    };

    std::vector<std::unique_ptr<PakReader>> readers;
    readers.reserve(steps.size());
    std::map<std::string, WinningEntry> activeFiles;
    std::set<std::string> copyEntries;
    std::set<std::string> deleteEntries;

    for (size_t s = 0; s < steps.size(); ++s) {
        auto reader = std::make_unique<PakReader>();
        std::string err;
        if (!reader->Open(steps[s].pakPath, &err)) {
            std::cerr << "[DnMakeFullPatch] ERROR: " << err << "\n";
            return 1;
        }

        // Apply delete list first (matching DnLauncher order)
        if (!steps[s].txtPath.empty()) {
            std::ifstream tf(steps[s].txtPath);
            std::string line;
            while (std::getline(tf, line)) {
                while (!line.empty() && (line.back() == '\r' || line.back() == '\n')) line.pop_back();
                if (line.size() < 3) continue;
                const char code = line[0];
                std::string pathStr = line.substr(2);
                const std::string norm = NormalizePakVirtualPath(pathStr);
                if (code == 'D') {
                    activeFiles.erase(norm);
                    copyEntries.erase(StripLeadingPakSeparator(pathStr));
                    deleteEntries.insert(StripLeadingPakSeparator(pathStr));
                } else if (code == 'C') {
                    deleteEntries.erase(StripLeadingPakSeparator(pathStr));
                    copyEntries.insert(StripLeadingPakSeparator(pathStr));
                }
            }
        }

        const auto& entries = reader->Entries();
        for (size_t idx = 0; idx < entries.size(); ++idx) {
            if (!entries[idx].IsActive()) continue;
            const std::string norm = NormalizePakVirtualPath(entries[idx].szFileName);
            activeFiles[norm] = { s, idx };
            deleteEntries.erase(StripLeadingPakSeparator(norm));
        }

        std::cout << "  [merge] +" << steps[s].pakPath.filename().string()
                  << " (" << reader->ActiveEntryCount() << " active entries)\n";
        readers.push_back(std::move(reader));
    }

    PakWriter writer;
    if (!writer.Create(outPak)) {
        std::cerr << "[DnMakeFullPatch] ERROR: Failed to create output .pak: " << outPak.string() << "\n";
        return 1;
    }

    std::vector<uint8_t> rawBuf;
    for (const auto& kv : activeFiles) {
        const auto& win = kv.second;
        const auto& info = readers[win.stepIndex]->Entries()[win.entryIndex];
        std::string err;
        if (!readers[win.stepIndex]->ReadCompressed(win.entryIndex, rawBuf, &err)) {
            std::cerr << "[DnMakeFullPatch] ERROR: " << err << "\n";
            return 1;
        }
        if (!writer.AddRawCompressedEntry(kv.first, info.dwOriginalSize, rawBuf.data(), info.dwCompressSize, &err)) {
            std::cerr << "[DnMakeFullPatch] ERROR: " << err << "\n";
            return 1;
        }
    }

    std::string err;
    if (!writer.Close(&err)) {
        std::cerr << "[DnMakeFullPatch] ERROR: " << err << "\n";
        return 1;
    }

    // Write companion .txt list if copy or delete entries were accumulated
    if (!copyEntries.empty() || !deleteEntries.empty()) {
        fs::path outTxt = outPak;
        outTxt.replace_extension(".txt");
        std::ofstream tf(outTxt, std::ios::binary | std::ios::trunc);
        if (tf.is_open()) {
            for (const auto& d : deleteEntries) tf << "D " << d << "\n";
            bool hasLauncher = false;
            for (const auto& c : copyEntries) {
                if (ToLowerString(c) == "dnlauncher.exe") {
                    hasLauncher = true;
                    continue;
                }
                tf << "C " << c << "\n";
            }
            if (hasLauncher) tf << "C DnLauncher.exe\n";
            tf << " ";
            tf.close();
        }
    }

    std::string digest, sidecar;
    if (!WriteMD5Sidecar(outPak, upperExt, &digest, &sidecar, &err)) {
        std::cerr << "[DnMakeFullPatch] ERROR: " << err << "\n";
        return 1;
    }

    std::cout << "[DnMakeFullPatch] Merged " << steps.size() << " archive(s) -> " << outPak.string()
              << " (" << activeFiles.size() << " entries, MD5=" << digest << ")\n";
    return 0;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    bool mergePaks = false;
    bool upperExt = false;
    int fromVer = -1;
    int toVer = -1;
    fs::path outPak;
    fs::path ignoreCfg;
    std::vector<fs::path> positional;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            return 0;
        } else if (arg == "--merge-paks") {
            mergePaks = true;
        } else if (arg == "--upper-ext") {
            upperExt = true;
        } else if (arg == "--out" && i + 1 < argc) {
            outPak = argv[++i];
        } else if (arg == "--ignore" && i + 1 < argc) {
            ignoreCfg = argv[++i];
        } else if (arg == "--from" && i + 1 < argc) {
            fromVer = std::stoi(argv[++i]);
        } else if (arg == "--to" && i + 1 < argc) {
            toVer = std::stoi(argv[++i]);
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "[DnMakeFullPatch] Unknown option: " << arg << "\n";
            return 1;
        } else {
            positional.emplace_back(arg);
        }
    }

    if (mergePaks) {
        return RunMergePaks(positional, outPak, fromVer, toVer, upperExt);
    }

    if (positional.size() < 2) {
        std::cerr << "[DnMakeFullPatch] ERROR: Expected <input-client-dir> <output-dir>.\n";
        PrintUsage();
        return 1;
    }

    return RunStandardFullPatch(positional[0], positional[1], ignoreCfg);
}
