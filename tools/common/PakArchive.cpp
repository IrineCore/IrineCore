#include "PakArchive.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>
#include <limits>
#include <zlib.h>

namespace fs = std::filesystem;

namespace irine::tools {

namespace {
constexpr uint32_t kEtSeedCount = 50;

std::string ToLowerAscii(const std::string& in) {
    std::string out = in;
    for (char& ch : out) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return out;
}

bool IsSafeRelativePath(const std::string& rel) {
    if (rel.empty()) return false;
    if (rel.find("..") != std::string::npos) {
        // Check path segments carefully so filenames containing ".." like "a..b.dds" are allowed,
        // while parent traversal segments ".." are rejected.
        size_t pos = 0;
        while (pos <= rel.size()) {
            size_t next = rel.find_first_of("\\/", pos);
            std::string seg = (next == std::string::npos) ? rel.substr(pos) : rel.substr(pos, next - pos);
            if (seg == "..") return false;
            if (next == std::string::npos) break;
            pos = next + 1;
        }
    }
    if (rel.size() >= 2 && rel[1] == ':') return false;
    return true;
}
} // namespace

std::string StripLeadingPakSeparator(const std::string& rawPath) {
    std::string normalized;
    normalized.reserve(rawPath.size());
    for (char ch : rawPath) {
        if (ch == '/') ch = '\\';
        if (ch == '\\' && !normalized.empty() && normalized.back() == '\\') {
            continue;
        }
        normalized.push_back(ch);
    }
    size_t start = 0;
    while (start < normalized.size() && normalized[start] == '\\') {
        ++start;
    }
    while (normalized.size() > start && (normalized.back() == ' ' || normalized.back() == '\t' ||
                                         normalized.back() == '\r' || normalized.back() == '\n')) {
        normalized.pop_back();
    }
    return normalized.substr(start);
}

std::string NormalizePakVirtualPath(const std::string& rawPath) {
    std::string stripped = StripLeadingPakSeparator(rawPath);
    std::string lower = ToLowerAscii(stripped);
    if (lower.empty()) return {};
    return "\\" + lower;
}

// =============================================================================
// PakWriter
// =============================================================================

PakWriter::~PakWriter() {
    if (!m_closed) {
        Close(nullptr);
    }
}

bool PakWriter::Create(const fs::path& pakPath, int compressionLevel) {
    if (!m_closed) {
        Close(nullptr);
    }

    m_path = pakPath;
    m_compressionLevel = std::max(0, std::min(9, compressionLevel));
    m_entries.clear();
    m_indexByPath.clear();
    m_currentOffset = kPakHeaderTotalBytes;

    std::error_code ec;
    if (pakPath.has_parent_path()) {
        fs::create_directories(pakPath.parent_path(), ec);
    }

    m_stream.open(pakPath, std::ios::binary | std::ios::trunc);
    if (!m_stream.is_open()) {
        return false;
    }

    PakFileHeader hdr{};
    std::strncpy(hdr.szHeaderString, kPakMagicString, sizeof(hdr.szHeaderString) - 1);
    hdr.nVersion = kPakVersionCurrent;
    hdr.nFileCount = 0;
    hdr.dwFileInfoOffset = kPakHeaderTotalBytes;
    hdr.bRequireHeaderWrite = 1; // marked dirty until Close() finalizes

    std::vector<char> headerBlock(kPakHeaderTotalBytes, 0);
    std::memcpy(headerBlock.data(), &hdr, sizeof(hdr));
    m_stream.write(headerBlock.data(), static_cast<std::streamsize>(headerBlock.size()));
    if (!m_stream.good()) {
        m_stream.close();
        return false;
    }

    m_closed = false;
    return true;
}

bool PakWriter::AddBytes(const std::string& virtualPath, const uint8_t* data, size_t size, std::string* outError) {
    if (m_closed || !m_stream.is_open()) {
        if (outError) *outError = "PakWriter is not open";
        return false;
    }
    if (size == 0) {
        // CEtPackingFile skips 0-byte entries (dwOriginalSize == 0 marks deleted slots).
        return true;
    }
    if (size > static_cast<size_t>(std::numeric_limits<uint32_t>::max() - 1024)) {
        if (outError) *outError = "File exceeds 32-bit .pak entry size limit: " + virtualPath;
        return false;
    }

    const uLong srcLen = static_cast<uLong>(size);
    uLongf dstLen = compressBound(srcLen);
    std::vector<uint8_t> compressed(dstLen);

    const int zrc = compress2(compressed.data(), &dstLen, data, srcLen, m_compressionLevel);
    if (zrc != Z_OK) {
        if (outError) *outError = "zlib compress2 failed (rc=" + std::to_string(zrc) + ") for " + virtualPath;
        return false;
    }

    return AddRawCompressedEntry(virtualPath,
                                 static_cast<uint32_t>(size),
                                 compressed.data(),
                                 static_cast<uint32_t>(dstLen),
                                 outError);
}

bool PakWriter::AddFileFromDisk(const std::string& virtualPath, const fs::path& diskPath, std::string* outError) {
    std::ifstream in(diskPath, std::ios::binary | std::ios::ate);
    if (!in.is_open()) {
        if (outError) *outError = "Failed to open input file: " + diskPath.string();
        return false;
    }

    const std::streamoff fileSize = in.tellg();
    if (fileSize < 0) {
        if (outError) *outError = "Failed to determine file size: " + diskPath.string();
        return false;
    }
    if (fileSize == 0) {
        return true;
    }

    in.seekg(0, std::ios::beg);
    std::vector<uint8_t> buffer(static_cast<size_t>(fileSize));
    if (!in.read(reinterpret_cast<char*>(buffer.data()), fileSize)) {
        if (outError) *outError = "Failed to read input file: " + diskPath.string();
        return false;
    }

    return AddBytes(virtualPath, buffer.data(), buffer.size(), outError);
}

bool PakWriter::AddRawCompressedEntry(const std::string& virtualPath,
                                      uint32_t originalSize,
                                      const uint8_t* compressedData,
                                      uint32_t compressedSize,
                                      std::string* outError) {
    if (m_closed || !m_stream.is_open()) {
        if (outError) *outError = "PakWriter is not open";
        return false;
    }
    if (originalSize == 0 || compressedSize == 0) {
        return true;
    }

    const std::string normPath = NormalizePakVirtualPath(virtualPath);
    if (normPath.empty() || normPath.size() >= kPakMaxNameLen) {
        if (outError) {
            *outError = "Virtual path is empty or exceeds 255 characters (" + std::to_string(normPath.size()) + "): " + normPath;
        }
        return false;
    }

    const uint64_t nextOffset = static_cast<uint64_t>(m_currentOffset) + compressedSize;
    const uint64_t futureTableBytes = static_cast<uint64_t>(m_entries.size() + 1) * sizeof(PakFileInfo);
    if (nextOffset + futureTableBytes >= 0xF0000000ULL) {
        if (outError) *outError = "Archive would overflow 32-bit .pak offset limit";
        return false;
    }

    // If duplicate path added to same archive, mark old entry as deleted (like CEtPackingFile::AddFile).
    auto existingIt = m_indexByPath.find(normPath);
    if (existingIt != m_indexByPath.end()) {
        m_entries[existingIt->second].dwCompressSize = 0;
        m_entries[existingIt->second].dwOriginalSize = 0;
    }

    m_stream.write(reinterpret_cast<const char*>(compressedData), static_cast<std::streamsize>(compressedSize));
    if (!m_stream.good()) {
        if (outError) *outError = "Write error while writing entry: " + normPath;
        return false;
    }

    uint32_t seedKey = 0;
    if (compressedSize >= sizeof(uint32_t)) {
        std::memcpy(&seedKey, compressedData, sizeof(uint32_t));
    }

    PakFileInfo info{};
    std::memcpy(info.szFileName, normPath.data(), normPath.size());
    info.szFileName[normPath.size()] = '\0';
    info.dwCompressSize = compressedSize;
    info.dwOriginalSize = originalSize;
    info.dwAllocSize = compressedSize;
    info.dwOffset = m_currentOffset;
    info.dwSeedValue = seedKey % kEtSeedCount;
    info.dwChecksum = 0;

    m_entries.push_back(info);
    m_indexByPath[normPath] = m_entries.size() - 1;
    m_currentOffset = static_cast<uint32_t>(nextOffset);
    return true;
}

uint64_t PakWriter::EstimatedFinalSizeBytes() const {
    return static_cast<uint64_t>(m_currentOffset) +
           static_cast<uint64_t>(m_entries.size()) * sizeof(PakFileInfo);
}

bool PakWriter::Close(std::string* outError) {
    if (m_closed) return true;
    m_closed = true;

    if (!m_stream.is_open()) {
        return false;
    }

    // Write file info table at m_currentOffset
    if (!m_entries.empty()) {
        m_stream.seekp(static_cast<std::streamoff>(m_currentOffset), std::ios::beg);
        m_stream.write(reinterpret_cast<const char*>(m_entries.data()),
                       static_cast<std::streamsize>(m_entries.size() * sizeof(PakFileInfo)));
        if (!m_stream.good()) {
            if (outError) *outError = "Failed to write file info table to " + m_path.string();
            m_stream.close();
            return false;
        }
    }

    // Finalize header at offset 0
    PakFileHeader hdr{};
    std::strncpy(hdr.szHeaderString, kPakMagicString, sizeof(hdr.szHeaderString) - 1);
    hdr.nVersion = kPakVersionCurrent;
    hdr.nFileCount = static_cast<int32_t>(m_entries.size());
    hdr.dwFileInfoOffset = m_currentOffset;
    hdr.bRequireHeaderWrite = 0;

    m_stream.seekp(0, std::ios::beg);
    m_stream.write(reinterpret_cast<const char*>(&hdr), sizeof(hdr));
    m_stream.flush();

    const bool ok = m_stream.good();
    m_stream.close();
    if (!ok && outError) {
        *outError = "Failed to finalize header in " + m_path.string();
    }
    return ok;
}

// =============================================================================
// PakReader
// =============================================================================

PakReader::~PakReader() {
    Close();
}

void PakReader::Close() {
    if (m_stream.is_open()) {
        m_stream.close();
    }
    m_entries.clear();
    m_indexByPath.clear();
    m_fileSize = 0;
    std::memset(&m_header, 0, sizeof(m_header));
}

bool PakReader::Open(const fs::path& pakPath, std::string* outError) {
    Close();
    m_path = pakPath;

    m_stream.open(pakPath, std::ios::binary | std::ios::ate);
    if (!m_stream.is_open()) {
        if (outError) *outError = "Failed to open .pak file: " + pakPath.string();
        return false;
    }

    const std::streamoff sz = m_stream.tellg();
    if (sz < static_cast<std::streamoff>(sizeof(PakFileHeader))) {
        if (outError) *outError = "File is too small to be a valid .pak archive: " + pakPath.string();
        Close();
        return false;
    }
    m_fileSize = static_cast<uint64_t>(sz);

    m_stream.seekg(0, std::ios::beg);
    if (!m_stream.read(reinterpret_cast<char*>(&m_header), sizeof(m_header))) {
        if (outError) *outError = "Failed to read .pak header: " + pakPath.string();
        Close();
        return false;
    }

    if (m_header.nVersion < kPakVersionNoChecksum || m_header.nVersion > kPakVersionCurrent) {
        if (outError) {
            *outError = "Unsupported .pak version " + std::to_string(m_header.nVersion) + " in " + pakPath.string();
        }
        Close();
        return false;
    }

    if (m_header.nVersion > kPakVersionNoChecksum && m_header.bRequireHeaderWrite != 0) {
        if (outError) {
            *outError = "Archive has incomplete header flag (bRequireHeaderWrite != 0): " + pakPath.string();
        }
        Close();
        return false;
    }

    if (m_header.nFileCount < 0) {
        if (outError) *outError = "Invalid negative file count in " + pakPath.string();
        Close();
        return false;
    }

    const uint64_t tableBytes = static_cast<uint64_t>(m_header.nFileCount) * sizeof(PakFileInfo);
    uint64_t tableOffset = 0;
    if (m_header.nVersion > kPakVersionNoChecksum) {
        tableOffset = m_header.dwFileInfoOffset;
    } else {
        if (m_fileSize < tableBytes) {
            if (outError) *outError = "Truncated v10 .pak file info table: " + pakPath.string();
            Close();
            return false;
        }
        tableOffset = m_fileSize - tableBytes;
        m_header.dwFileInfoOffset = static_cast<uint32_t>(tableOffset);
    }

    if (tableOffset + tableBytes > m_fileSize) {
        if (outError) *outError = "File info table extends past EOF in " + pakPath.string();
        Close();
        return false;
    }

    if (m_header.nFileCount > 0) {
        m_entries.resize(static_cast<size_t>(m_header.nFileCount));
        m_stream.seekg(static_cast<std::streamoff>(tableOffset), std::ios::beg);
        if (!m_stream.read(reinterpret_cast<char*>(m_entries.data()), static_cast<std::streamsize>(tableBytes))) {
            if (outError) *outError = "Failed to read file info table in " + pakPath.string();
            Close();
            return false;
        }

        for (size_t i = 0; i < m_entries.size(); ++i) {
            m_entries[i].szFileName[kPakMaxNameLen - 1] = '\0';
            if (m_entries[i].dwCompressSize > m_entries[i].dwAllocSize) {
                if (outError) {
                    *outError = "Corrupt entry #" + std::to_string(i) + " (compressSize > allocSize) in " + pakPath.string();
                }
                Close();
                return false;
            }
            if (m_entries[i].IsActive()) {
                m_indexByPath[NormalizePakVirtualPath(m_entries[i].szFileName)] = i;
            }
        }
    }

    return true;
}

size_t PakReader::ActiveEntryCount() const {
    size_t count = 0;
    for (const auto& e : m_entries) {
        if (e.IsActive()) ++count;
    }
    return count;
}

int PakReader::FindEntry(const std::string& virtualPath) const {
    const std::string norm = NormalizePakVirtualPath(virtualPath);
    auto it = m_indexByPath.find(norm);
    if (it == m_indexByPath.end()) return -1;
    return static_cast<int>(it->second);
}

bool PakReader::ReadCompressed(size_t index, std::vector<uint8_t>& outData, std::string* outError) {
    if (!m_stream.is_open() || index >= m_entries.size()) {
        if (outError) *outError = "Invalid entry index";
        return false;
    }
    const PakFileInfo& info = m_entries[index];
    if (!info.IsActive()) {
        outData.clear();
        return true;
    }
    if (static_cast<uint64_t>(info.dwOffset) + info.dwCompressSize > m_fileSize) {
        if (outError) *outError = std::string("Entry payload out of bounds: ") + info.szFileName;
        return false;
    }

    outData.resize(info.dwCompressSize);
    m_stream.seekg(static_cast<std::streamoff>(info.dwOffset), std::ios::beg);
    if (!m_stream.read(reinterpret_cast<char*>(outData.data()), static_cast<std::streamsize>(info.dwCompressSize))) {
        if (outError) *outError = std::string("Failed reading compressed entry: ") + info.szFileName;
        return false;
    }
    return true;
}

bool PakReader::ReadDecompressed(size_t index, std::vector<uint8_t>& outData, std::string* outError) {
    if (!m_stream.is_open() || index >= m_entries.size()) {
        if (outError) *outError = "Invalid entry index";
        return false;
    }
    const PakFileInfo& info = m_entries[index];
    if (!info.IsActive()) {
        outData.clear();
        return true;
    }

    std::vector<uint8_t> compressed;
    if (!ReadCompressed(index, compressed, outError)) {
        return false;
    }

    outData.resize(info.dwOriginalSize);
    uLongf dstLen = info.dwOriginalSize;
    const int zrc = uncompress(outData.data(), &dstLen, compressed.data(), static_cast<uLong>(compressed.size()));
    if (zrc != Z_OK || dstLen != info.dwOriginalSize) {
        if (outError) {
            *outError = std::string("zlib uncompress failed (rc=") + std::to_string(zrc) + ") for " + info.szFileName;
        }
        return false;
    }
    return true;
}

bool PakReader::VerifyAll(size_t* outCheckedFiles, std::string* outError) {
    size_t checked = 0;
    std::vector<uint8_t> buf;
    for (size_t i = 0; i < m_entries.size(); ++i) {
        if (!m_entries[i].IsActive()) continue;
        if (!ReadDecompressed(i, buf, outError)) {
            if (outCheckedFiles) *outCheckedFiles = checked;
            return false;
        }
        ++checked;
    }
    if (outCheckedFiles) *outCheckedFiles = checked;
    return true;
}

bool PakReader::ExtractAll(const fs::path& outputDir,
                           size_t* outExtractedFiles,
                           bool verbose,
                           std::string* outError) {
    std::error_code ec;
    fs::create_directories(outputDir, ec);

    size_t extracted = 0;
    std::vector<uint8_t> data;
    for (size_t i = 0; i < m_entries.size(); ++i) {
        const PakFileInfo& info = m_entries[i];
        if (!info.IsActive()) continue;

        std::string rel = StripLeadingPakSeparator(info.szFileName);
        if (!IsSafeRelativePath(rel)) {
            if (outError) *outError = "Refusing unsafe path traversal entry: " + rel;
            if (outExtractedFiles) *outExtractedFiles = extracted;
            return false;
        }

        std::string portableRel = rel;
        std::replace(portableRel.begin(), portableRel.end(), '\\', '/');
        fs::path destPath = outputDir / fs::path(portableRel);

        if (destPath.has_parent_path()) {
            fs::create_directories(destPath.parent_path(), ec);
        }

        if (!ReadDecompressed(i, data, outError)) {
            if (outExtractedFiles) *outExtractedFiles = extracted;
            return false;
        }

        std::ofstream out(destPath, std::ios::binary | std::ios::trunc);
        if (!out.is_open()) {
            if (outError) *outError = "Failed to open output file for extraction: " + destPath.string();
            if (outExtractedFiles) *outExtractedFiles = extracted;
            return false;
        }
        if (!data.empty()) {
            out.write(reinterpret_cast<const char*>(data.data()), static_cast<std::streamsize>(data.size()));
        }
        if (!out.good()) {
            if (outError) *outError = "Write failure extracting: " + destPath.string();
            if (outExtractedFiles) *outExtractedFiles = extracted;
            return false;
        }
        out.close();

        ++extracted;
        if (verbose) {
            std::cout << "  [extract] " << rel << " (" << info.dwOriginalSize << " bytes)\n";
        }
    }

    if (outExtractedFiles) *outExtractedFiles = extracted;
    return true;
}

} // namespace irine::tools
