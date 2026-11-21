// =============================================================================
// IrineCore — DnMakeMD5
// Generates and verifies RFC-1321 MD5 sidecar (.md5 / .MD5) files and manifests
// compatible with DnLauncher and Dragon Nest patch servers.
// =============================================================================

#include "ToolUtils.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace fs = std::filesystem;
using namespace irine::tools;

namespace {

void PrintUsage() {
    std::cout
        << "IrineCore DnMakeMD5 — MD5 Sidecar & Checksum Manifest Tool\n"
        << "\n"
        << "Usage:\n"
        << "  DnMakeMD5 <file1> [file2 ...] [--upper-ext] [--stdout-only]\n"
        << "  DnMakeMD5 --dir <folder> [--recursive] [--ext <.pak>] [--upper-ext] [--manifest <file>]\n"
        << "  DnMakeMD5 --verify <file|manifest>\n"
        << "\n"
        << "Options:\n"
        << "  --upper-ext        Write .MD5 extension instead of .md5\n"
        << "  --stdout-only      Print digest(s) without writing sidecar files\n"
        << "  --dir <folder>     Generate checksums for all files in a directory\n"
        << "  --recursive, -r    Scan --dir recursively\n"
        << "  --ext <.ext>       Only include files matching extension (repeatable, e.g. --ext .pak)\n"
        << "  --manifest <file>  Write consolidated '<md5>  <relative-path>' manifest file\n"
        << "  --verify <target>  Verify file against its .md5/.MD5 sidecar, or verify a manifest file\n"
        << "  --help, -h         Show this help message\n";
}

bool MatchesExtensions(const fs::path& p, const std::vector<std::string>& exts) {
    if (exts.empty()) return true;
    const std::string ext = ToLowerString(p.extension().string());
    for (const auto& allowed : exts) {
        if (ext == allowed) return true;
    }
    return false;
}

int VerifyManifestOrFile(const fs::path& target) {
    std::error_code ec;
    if (!fs::exists(target, ec) || !fs::is_regular_file(target, ec)) {
        std::cerr << "[DnMakeMD5] ERROR: Target file not found: " << target.string() << "\n";
        return 1;
    }

    // Check if target has an adjacent .md5 or .MD5 sidecar (or sibling stem.md5 for fullpatch_svr.cfg) -> single file verify.
    fs::path sideLower = target;
    sideLower += ".md5";
    fs::path sideUpper = target;
    sideUpper += ".MD5";
    fs::path sideStem = target;
    sideStem.replace_extension(".md5");
    if (fs::exists(sideLower, ec) || fs::exists(sideUpper, ec)) {
        std::string expected, actual, err;
        if (!VerifyMD5Sidecar(target, &expected, &actual, &err)) {
            if (!err.empty()) {
                std::cerr << "[DnMakeMD5] ERROR: " << err << "\n";
            } else {
                std::cerr << "[DnMakeMD5] MISMATCH: " << target.string()
                          << "\n  expected: " << expected
                          << "\n  actual:   " << actual << "\n";
            }
            return 1;
        }
        std::cout << "[DnMakeMD5] OK: " << target.filename().string() << " (" << actual << ")\n";
        return 0;
    }
    if (ToLowerString(target.filename().string()) == "fullpatch_svr.cfg" && fs::exists(sideStem, ec)) {
        std::ifstream sf(sideStem);
        std::string expected, actual, err;
        sf >> expected;
        if (!ComputeFileMD5(target, actual, &err)) {
            std::cerr << "[DnMakeMD5] ERROR: " << err << "\n";
            return 1;
        }
        if (ToLowerString(expected) != actual) {
            std::cerr << "[DnMakeMD5] MISMATCH: " << target.string()
                      << "\n  expected: " << expected
                      << "\n  actual:   " << actual << "\n";
            return 1;
        }
        std::cout << "[DnMakeMD5] OK: " << target.filename().string() << " (" << actual << ")\n";
        return 0;
    }

    // Otherwise treat target as a manifest file ('<md5>  <relpath>' per line).
    std::ifstream in(target);
    if (!in.is_open()) {
        std::cerr << "[DnMakeMD5] ERROR: Failed to open manifest: " << target.string() << "\n";
        return 1;
    }

    const fs::path baseDir = target.has_parent_path() ? target.parent_path() : fs::current_path();
    size_t checked = 0;
    size_t failed = 0;
    std::string line;
    while (std::getline(in, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::istringstream iss(line);
        std::string expected;
        if (!(iss >> expected)) continue;
        std::string rest;
        std::getline(iss, rest);
        size_t pos = rest.find_first_not_of(" \t*");
        if (pos == std::string::npos) continue;
        std::string rel = rest.substr(pos);
        while (!rel.empty() && (rel.back() == '\r' || rel.back() == '\n')) rel.pop_back();

        fs::path filePath = baseDir / fs::path(rel);
        std::string actual, err;
        if (!ComputeFileMD5(filePath, actual, &err)) {
            std::cerr << "[DnMakeMD5] FAIL: " << rel << " (" << err << ")\n";
            ++failed;
        } else if (ToLowerString(expected) != actual) {
            std::cerr << "[DnMakeMD5] MISMATCH: " << rel << " (expected=" << expected << ", actual=" << actual << ")\n";
            ++failed;
        } else {
            ++checked;
        }
    }

    std::cout << "[DnMakeMD5] Manifest verification complete: " << checked << " passed, " << failed << " failed.\n";
    return failed == 0 ? 0 : 1;
}

} // namespace

int main(int argc, char* argv[]) {
    if (argc < 2) {
        PrintUsage();
        return 1;
    }

    bool upperExt = false;
    bool stdoutOnly = false;
    bool recursive = false;
    fs::path dirPath;
    fs::path manifestPath;
    fs::path verifyPath;
    std::vector<std::string> extensions;
    std::vector<fs::path> inputFiles;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            PrintUsage();
            return 0;
        } else if (arg == "--upper-ext") {
            upperExt = true;
        } else if (arg == "--stdout-only") {
            stdoutOnly = true;
        } else if (arg == "--recursive" || arg == "-r") {
            recursive = true;
        } else if (arg == "--dir" && i + 1 < argc) {
            dirPath = argv[++i];
        } else if (arg == "--ext" && i + 1 < argc) {
            std::string ext = ToLowerString(argv[++i]);
            if (!ext.empty() && ext[0] != '.') ext = "." + ext;
            extensions.push_back(ext);
        } else if (arg == "--manifest" && i + 1 < argc) {
            manifestPath = argv[++i];
        } else if (arg == "--verify" && i + 1 < argc) {
            verifyPath = argv[++i];
        } else if (!arg.empty() && arg[0] == '-') {
            std::cerr << "[DnMakeMD5] Unknown option: " << arg << "\n";
            return 1;
        } else {
            inputFiles.emplace_back(arg);
        }
    }

    if (!verifyPath.empty()) {
        return VerifyManifestOrFile(verifyPath);
    }

    if (!dirPath.empty()) {
        std::error_code ec;
        if (!fs::exists(dirPath, ec) || !fs::is_directory(dirPath, ec)) {
            std::cerr << "[DnMakeMD5] ERROR: Directory not found: " << dirPath.string() << "\n";
            return 1;
        }

        std::ofstream manifestOut;
        if (!manifestPath.empty()) {
            if (manifestPath.has_parent_path()) {
                fs::create_directories(manifestPath.parent_path(), ec);
            }
            manifestOut.open(manifestPath, std::ios::binary | std::ios::trunc);
            if (!manifestOut.is_open()) {
                std::cerr << "[DnMakeMD5] ERROR: Cannot create manifest: " << manifestPath.string() << "\n";
                return 1;
            }
        }

        std::vector<fs::path> found;
        if (recursive) {
            IgnoreFilter filter;
            filter.AddDefaults();
            for (const auto& df : CollectFilesRecursive(dirPath, &filter)) {
                const std::string ext = ToLowerString(df.diskPath.extension().string());
                if (ext == ".md5") continue;
                if (MatchesExtensions(df.diskPath, extensions)) {
                    found.push_back(df.diskPath);
                }
            }
        } else {
            for (const auto& entry : fs::directory_iterator(dirPath, ec)) {
                if (!entry.is_regular_file(ec)) continue;
                const std::string ext = ToLowerString(entry.path().extension().string());
                if (ext == ".md5") continue;
                if (MatchesExtensions(entry.path(), extensions)) {
                    found.push_back(entry.path());
                }
            }
            std::sort(found.begin(), found.end());
        }

        size_t processed = 0;
        for (const auto& file : found) {
            std::string digest, sidecar, err;
            if (stdoutOnly || !manifestPath.empty()) {
                if (!ComputeFileMD5(file, digest, &err)) {
                    std::cerr << "[DnMakeMD5] ERROR: " << err << "\n";
                    return 1;
                }
            } else {
                if (!WriteMD5Sidecar(file, upperExt, &digest, &sidecar, &err)) {
                    std::cerr << "[DnMakeMD5] ERROR: " << err << "\n";
                    return 1;
                }
            }

            std::string rel = fs::relative(file, dirPath, ec).generic_string();
            if (manifestOut.is_open()) {
                manifestOut << digest << "  " << rel << "\n";
            }
            std::cout << digest << "  " << rel << "\n";
            ++processed;
        }

        if (manifestOut.is_open()) {
            manifestOut.close();
            std::cout << "[DnMakeMD5] Wrote manifest (" << processed << " entries): " << manifestPath.string() << "\n";
        } else {
            std::cout << "[DnMakeMD5] Processed " << processed << " file(s).\n";
        }
        return 0;
    }

    if (inputFiles.empty()) {
        std::cerr << "[DnMakeMD5] ERROR: No input file(s) specified.\n";
        PrintUsage();
        return 1;
    }

    for (const auto& file : inputFiles) {
        std::string digest, sidecar, err;
        if (stdoutOnly) {
            if (!ComputeFileMD5(file, digest, &err)) {
                std::cerr << "[DnMakeMD5] ERROR: " << err << "\n";
                return 1;
            }
            std::cout << digest << "  " << file.string() << "\n";
        } else {
            if (!WriteMD5Sidecar(file, upperExt, &digest, &sidecar, &err)) {
                std::cerr << "[DnMakeMD5] ERROR: " << err << "\n";
                return 1;
            }
            std::cout << "[DnMakeMD5] " << sidecar << " -> " << digest << "\n";
        }
    }

    return 0;
}
