#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace irine::tools {

static constexpr const char* kPakMagicString = "EyedentityGames Packing File 0.1";
static constexpr int32_t kPakVersionNoChecksum = 10;
static constexpr int32_t kPakVersionCurrent = 11;
static constexpr uint32_t kPakHeaderTotalBytes = 1024;
static constexpr uint32_t kPakDefaultSplitBytes = 512u * 1024u * 1024u;
static constexpr uint32_t kPakMaxSafeBytes = 1900u * 1024u * 1024u; // safely under 32-bit DWORD offsets
static constexpr size_t kPakMaxNameLen = 256;

struct PakFileHeader {
    char szHeaderString[256];
    int32_t nVersion;
    int32_t nFileCount;
    uint32_t dwFileInfoOffset;
    uint8_t bRequireHeaderWrite;
    char cPadding[3];
};
static_assert(sizeof(PakFileHeader) == 272, "PakFileHeader ABI mismatch with SPackingFileHeader");

struct PakFileInfo {
    char szFileName[256];
    uint32_t dwCompressSize;
    uint32_t dwOriginalSize;
    uint32_t dwAllocSize;
    uint32_t dwOffset;
    uint32_t dwSeedValue;
    uint32_t dwChecksum;
    char cReserved[36];

    bool IsActive() const { return dwCompressSize > 0 && dwOriginalSize > 0 && szFileName[0] != '\0'; }
};
static_assert(sizeof(PakFileInfo) == 316, "PakFileInfo ABI mismatch with SPackingFileInfo");

// Normalizes a relative path into Dragon Nest's canonical .pak virtual path:
// leading '\', backslash separators, lowercase ASCII (e.g. "\resource\ext\itemtable.dnt").
std::string NormalizePakVirtualPath(const std::string& rawPath);

// Strips any leading '/' or '\' and converts separators to '\'.
std::string StripLeadingPakSeparator(const std::string& rawPath);

class PakWriter {
public:
    PakWriter() = default;
    ~PakWriter();

    PakWriter(const PakWriter&) = delete;
    PakWriter& operator=(const PakWriter&) = delete;

    bool Create(const std::filesystem::path& pakPath, int compressionLevel = 1);
    bool AddBytes(const std::string& virtualPath, const uint8_t* data, size_t size, std::string* outError = nullptr);
    bool AddFileFromDisk(const std::string& virtualPath, const std::filesystem::path& diskPath, std::string* outError = nullptr);
    bool AddRawCompressedEntry(const std::string& virtualPath,
                               uint32_t originalSize,
                               const uint8_t* compressedData,
                               uint32_t compressedSize,
                               std::string* outError = nullptr);

    bool Close(std::string* outError = nullptr);
    bool IsOpen() const { return m_stream.is_open(); }

    uint64_t EstimatedFinalSizeBytes() const;
    uint32_t CurrentDataOffset() const { return m_currentOffset; }
    size_t EntryCount() const { return m_entries.size(); }
    const std::filesystem::path& Path() const { return m_path; }

private:
    std::filesystem::path m_path;
    std::ofstream m_stream;
    int m_compressionLevel = 1;
    uint32_t m_currentOffset = kPakHeaderTotalBytes;
    std::vector<PakFileInfo> m_entries;
    std::unordered_map<std::string, size_t> m_indexByPath;
    bool m_closed = true;
};

class PakReader {
public:
    PakReader() = default;
    ~PakReader();

    PakReader(const PakReader&) = delete;
    PakReader& operator=(const PakReader&) = delete;

    bool Open(const std::filesystem::path& pakPath, std::string* outError = nullptr);
    void Close();
    bool IsOpen() const { return m_stream.is_open(); }

    const PakFileHeader& Header() const { return m_header; }
    const std::vector<PakFileInfo>& Entries() const { return m_entries; }
    size_t ActiveEntryCount() const;
    int FindEntry(const std::string& virtualPath) const;

    bool ReadCompressed(size_t index, std::vector<uint8_t>& outData, std::string* outError = nullptr);
    bool ReadDecompressed(size_t index, std::vector<uint8_t>& outData, std::string* outError = nullptr);
    bool VerifyAll(size_t* outCheckedFiles, std::string* outError = nullptr);
    bool ExtractAll(const std::filesystem::path& outputDir,
                    size_t* outExtractedFiles,
                    bool verbose = false,
                    std::string* outError = nullptr);

    const std::filesystem::path& Path() const { return m_path; }

private:
    std::filesystem::path m_path;
    std::ifstream m_stream;
    uint64_t m_fileSize = 0;
    PakFileHeader m_header{};
    std::vector<PakFileInfo> m_entries;
    std::unordered_map<std::string, size_t> m_indexByPath;
};

} // namespace irine::tools
