#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <vector>

namespace irine::tools {

struct IgnoreFilter {
    std::unordered_set<std::string> exactFileNames; // lowercase basename (e.g. "thumbs.db")
    std::unordered_set<std::string> extensions;     // lowercase with dot (e.g. ".pdb")
    std::unordered_set<std::string> dirNames;       // lowercase folder name (e.g. ".git", ".svn")

    void AddDefaults();
    bool LoadFromConfigFile(const std::filesystem::path& cfgPath, const std::string& activeSection = "client");
    bool ShouldIgnoreDirectory(const std::string& dirName) const;
    bool ShouldIgnoreFile(const std::string& fileName) const;
};

struct DiscoveredFile {
    std::filesystem::path diskPath;
    std::string relPathBackslash; // relative path using '\', no leading separator (e.g. "Resource\Ext\ItemTable.dnt")
    uint64_t fileSize = 0;
};

// Lowercase ASCII helper
std::string ToLowerString(const std::string& s);

// Returns true for assets belonging inside Dragon Nest ResourceXX.pak archives (Resource\* or MapData\*).
bool IsPakAssetRelativePath(const std::string& relPath);

// Recursively scans rootDir, applying optional IgnoreFilter, returning deterministic sorted file list.
std::vector<DiscoveredFile> CollectFilesRecursive(const std::filesystem::path& rootDir,
                                                  const IgnoreFilter* filter = nullptr);

// Streaming RFC-1321 MD5 calculation (lowercase 32-character hex string).
bool ComputeFileMD5(const std::filesystem::path& filePath, std::string& outHex, std::string* outError = nullptr);
std::string ComputeBufferMD5(const void* data, size_t size);

// Writes a sidecar .md5 / .MD5 file containing the hex digest (matching DnMakeMD5 & DnLauncher).
bool WriteMD5Sidecar(const std::filesystem::path& targetFile,
                     bool uppercaseExt,
                     std::string* outDigest = nullptr,
                     std::string* outSidecarPath = nullptr,
                     std::string* outError = nullptr);

// Verifies targetFile against its sidecar (.md5 or .MD5) file.
bool VerifyMD5Sidecar(const std::filesystem::path& targetFile,
                      std::string* outExpected = nullptr,
                      std::string* outActual = nullptr,
                      std::string* outError = nullptr);

// Creates a standard PKZIP archive containing a single file entry compressed with zlib DEFLATE.
// Matches DnMakeFullPatch's per-file .zip output format.
bool WriteSingleFileZip(const std::filesystem::path& zipPath,
                        const std::string& entryName,
                        const std::filesystem::path& sourceFile,
                        uint64_t* outZipFileSize = nullptr,
                        std::string* outError = nullptr);

// Dragon Nest Version.cfg & PatchInfoServer.cfg helpers (exact DnLauncher format).
std::string BuildVersionCfgContent(int version, int moduleVersion);
bool WriteVersionCfgFile(const std::filesystem::path& path, int version, int moduleVersion, std::string* outError = nullptr);

struct PatchInfoServerData {
    int version = 0;
    std::vector<int> skipNumbers;
};

bool ReadPatchInfoServerCfg(const std::filesystem::path& path, PatchInfoServerData& outData);
bool WritePatchInfoServerCfg(const std::filesystem::path& path, const PatchInfoServerData& data, std::string* outError = nullptr);

// Formats version as 8-digit zero-padded string (e.g. 354 -> "00000354").
std::string FormatPatchVersion8(int version);

// Formats byte count cleanly (e.g. "512.00 MiB").
std::string FormatByteSize(uint64_t bytes);

} // namespace irine::tools
