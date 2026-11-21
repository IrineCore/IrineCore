#include "ToolUtils.h"
#include "md5.h"

#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <zlib.h>

namespace fs = std::filesystem;

namespace irine::tools {

std::string ToLowerString(const std::string& s) {
    std::string out = s;
    for (char& ch : out) {
        ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
    }
    return out;
}

static std::string TrimWhitespace(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace(static_cast<unsigned char>(s[start]))) ++start;
    size_t end = s.size();
    while (end > start && std::isspace(static_cast<unsigned char>(s[end - 1]))) --end;
    return s.substr(start, end - start);
}

// =============================================================================
// IgnoreFilter
// =============================================================================

void IgnoreFilter::AddDefaults() {
    exactFileNames.insert("thumbs.db");
    exactFileNames.insert("desktop.ini");
    exactFileNames.insert(".ds_store");
    exactFileNames.insert("rmakeresult.txt");
    exactFileNames.insert("rmakeerror.txt");

    extensions.insert(".pdb");
    extensions.insert(".ilk");
    extensions.insert(".tmp");
    extensions.insert(".bak");

    dirNames.insert(".git");
    dirNames.insert(".svn");
    dirNames.insert("_svn");
    dirNames.insert(".vs");
}

bool IgnoreFilter::LoadFromConfigFile(const fs::path& cfgPath, const std::string& activeSection) {
    std::ifstream in(cfgPath);
    if (!in.is_open()) {
        return false;
    }

    const std::string targetSection = ToLowerString(activeSection);
    std::string currentSection = "common";
    std::string line;
    while (std::getline(in, line)) {
        std::string trimmed = TrimWhitespace(line);
        if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';') {
            continue;
        }
        if (trimmed.size() >= 2 && trimmed.front() == '[' && trimmed.back() == ']') {
            currentSection = ToLowerString(trimmed.substr(1, trimmed.size() - 2));
            continue;
        }
        if (currentSection != "common" && currentSection != targetSection) {
            continue;
        }

        std::string entry = ToLowerString(trimmed);
        if (entry[0] == '/' || entry[0] == '\\') {
            dirNames.insert(entry.substr(1));
        } else if (entry.size() >= 2 && entry[0] == '*' && entry[1] == '.') {
            extensions.insert(entry.substr(1)); // keep leading '.'
        } else {
            exactFileNames.insert(entry);
        }
    }
    return true;
}

bool IgnoreFilter::ShouldIgnoreDirectory(const std::string& dirName) const {
    return dirNames.find(ToLowerString(dirName)) != dirNames.end();
}

bool IgnoreFilter::ShouldIgnoreFile(const std::string& fileName) const {
    const std::string lower = ToLowerString(fileName);
    if (exactFileNames.find(lower) != exactFileNames.end()) {
        return true;
    }
    const size_t dotPos = lower.rfind('.');
    if (dotPos != std::string::npos) {
        if (extensions.find(lower.substr(dotPos)) != extensions.end()) {
            return true;
        }
    }
    return false;
}

// =============================================================================
// Path Classification & Collection
// =============================================================================

bool IsPakAssetRelativePath(const std::string& relPath) {
    std::string lower = ToLowerString(relPath);
    std::replace(lower.begin(), lower.end(), '/', '\\');
    size_t start = 0;
    while (start < lower.size() && lower[start] == '\\') ++start;
    std::string s = lower.substr(start);

    if (s.rfind("resource\\", 0) == 0 || s == "resource") return true;
    if (s.rfind("mapdata\\", 0) == 0 || s == "mapdata") return true;
    return false;
}

std::vector<DiscoveredFile> CollectFilesRecursive(const fs::path& rootDir, const IgnoreFilter* filter) {
    std::vector<DiscoveredFile> result;
    std::error_code ec;
    if (!fs::exists(rootDir, ec) || !fs::is_directory(rootDir, ec)) {
        return result;
    }

    fs::recursive_directory_iterator it(rootDir, fs::directory_options::skip_permission_denied, ec);
    fs::recursive_directory_iterator end;
    for (; !ec && it != end; it.increment(ec)) {
        const fs::directory_entry& entry = *it;
        const std::string name = entry.path().filename().string();

        if (entry.is_directory(ec)) {
            if (filter && filter->ShouldIgnoreDirectory(name)) {
                it.disable_recursion_pending();
            }
            continue;
        }
        if (!entry.is_regular_file(ec)) {
            continue;
        }
        if (filter && filter->ShouldIgnoreFile(name)) {
            continue;
        }

        fs::path rel = fs::relative(entry.path(), rootDir, ec);
        if (ec) continue;

        std::string relStr = rel.generic_string();
        std::replace(relStr.begin(), relStr.end(), '/', '\\');

        DiscoveredFile df;
        df.diskPath = entry.path();
        df.relPathBackslash = relStr;
        df.fileSize = entry.file_size(ec);
        if (ec) df.fileSize = 0;
        result.push_back(std::move(df));
    }

    std::sort(result.begin(), result.end(), [](const DiscoveredFile& a, const DiscoveredFile& b) {
        return ToLowerString(a.relPathBackslash) < ToLowerString(b.relPathBackslash);
    });
    return result;
}

// =============================================================================
// MD5 Helpers
// =============================================================================

static std::string DigestToHex(const md5_byte_t digest[16]) {
    static const char kHex[] = "0123456789abcdef";
    std::string out(32, '0');
    for (int i = 0; i < 16; ++i) {
        out[i * 2] = kHex[(digest[i] >> 4) & 0x0F];
        out[i * 2 + 1] = kHex[digest[i] & 0x0F];
    }
    return out;
}

std::string ComputeBufferMD5(const void* data, size_t size) {
    md5_state_t state;
    md5_init(&state);
    const auto* bytes = static_cast<const md5_byte_t*>(data);
    size_t remaining = size;
    while (remaining > 0) {
        const int chunk = static_cast<int>(std::min<size_t>(remaining, 1024u * 1024u));
        md5_append(&state, bytes, chunk);
        bytes += chunk;
        remaining -= static_cast<size_t>(chunk);
    }
    md5_byte_t digest[16];
    md5_finish(&state, digest);
    return DigestToHex(digest);
}

bool ComputeFileMD5(const fs::path& filePath, std::string& outHex, std::string* outError) {
    std::ifstream in(filePath, std::ios::binary);
    if (!in.is_open()) {
        if (outError) *outError = "Failed to open file for MD5: " + filePath.string();
        return false;
    }

    md5_state_t state;
    md5_init(&state);

    std::vector<char> buffer(64 * 1024);
    while (in.good()) {
        in.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const std::streamsize got = in.gcount();
        if (got > 0) {
            md5_append(&state, reinterpret_cast<const md5_byte_t*>(buffer.data()), static_cast<int>(got));
        }
    }

    if (in.bad()) {
        if (outError) *outError = "I/O error while reading file for MD5: " + filePath.string();
        return false;
    }

    md5_byte_t digest[16];
    md5_finish(&state, digest);
    outHex = DigestToHex(digest);
    return true;
}

bool WriteMD5Sidecar(const fs::path& targetFile,
                     bool uppercaseExt,
                     std::string* outDigest,
                     std::string* outSidecarPath,
                     std::string* outError) {
    std::string digest;
    if (!ComputeFileMD5(targetFile, digest, outError)) {
        return false;
    }

    fs::path sidecar = targetFile;
    sidecar += (uppercaseExt ? ".MD5" : ".md5");

    std::ofstream out(sidecar, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        if (outError) *outError = "Failed to write MD5 sidecar: " + sidecar.string();
        return false;
    }
    out << digest << "\n";
    out.close();

    if (outDigest) *outDigest = digest;
    if (outSidecarPath) *outSidecarPath = sidecar.string();
    return true;
}

bool VerifyMD5Sidecar(const fs::path& targetFile,
                      std::string* outExpected,
                      std::string* outActual,
                      std::string* outError) {
    fs::path sidecar = targetFile;
    sidecar += ".md5";
    std::error_code ec;
    if (!fs::exists(sidecar, ec)) {
        sidecar = targetFile;
        sidecar += ".MD5";
        if (!fs::exists(sidecar, ec)) {
            if (outError) *outError = "MD5 sidecar not found for " + targetFile.string();
            return false;
        }
    }

    std::ifstream in(sidecar);
    if (!in.is_open()) {
        if (outError) *outError = "Failed to open MD5 sidecar: " + sidecar.string();
        return false;
    }
    std::string token;
    in >> token;
    in.close();

    std::string expected = ToLowerString(TrimWhitespace(token));
    std::string actual;
    if (!ComputeFileMD5(targetFile, actual, outError)) {
        return false;
    }

    if (outExpected) *outExpected = expected;
    if (outActual) *outActual = actual;
    return expected == actual;
}

// =============================================================================
// Standard PKZIP Single-File Writer (via dep_zlib DEFLATE + CRC32)
// =============================================================================

namespace {
void WriteLE16(std::ostream& os, uint16_t v) {
    const uint8_t b[2] = {
        static_cast<uint8_t>(v & 0xFFu),
        static_cast<uint8_t>((v >> 8) & 0xFFu)
    };
    os.write(reinterpret_cast<const char*>(b), 2);
}

void WriteLE32(std::ostream& os, uint32_t v) {
    const uint8_t b[4] = {
        static_cast<uint8_t>(v & 0xFFu),
        static_cast<uint8_t>((v >> 8) & 0xFFu),
        static_cast<uint8_t>((v >> 16) & 0xFFu),
        static_cast<uint8_t>((v >> 24) & 0xFFu)
    };
    os.write(reinterpret_cast<const char*>(b), 4);
}
} // namespace

bool WriteSingleFileZip(const fs::path& zipPath,
                        const std::string& entryName,
                        const fs::path& sourceFile,
                        uint64_t* outZipFileSize,
                        std::string* outError) {
    std::ifstream in(sourceFile, std::ios::binary | std::ios::ate);
    if (!in.is_open()) {
        if (outError) *outError = "Failed to open source file for zip: " + sourceFile.string();
        return false;
    }
    const std::streamoff rawSize = in.tellg();
    if (rawSize < 0) {
        if (outError) *outError = "Failed to read file size: " + sourceFile.string();
        return false;
    }
    if (static_cast<uint64_t>(rawSize) >= 0xFFFFFFFFULL) {
        if (outError) *outError = "File exceeds 4 GiB standard ZIP limit: " + sourceFile.string();
        return false;
    }
    in.seekg(0, std::ios::beg);

    std::error_code ec;
    if (zipPath.has_parent_path()) {
        fs::create_directories(zipPath.parent_path(), ec);
    }

    std::ofstream out(zipPath, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        if (outError) *outError = "Failed to create zip file: " + zipPath.string();
        return false;
    }

    std::string cleanName = entryName;
    std::replace(cleanName.begin(), cleanName.end(), '\\', '/');
    const uint16_t nameLen = static_cast<uint16_t>(cleanName.size());
    const uint32_t origSize = static_cast<uint32_t>(rawSize);
    const uint16_t method = (origSize == 0) ? 0u : 8u; // STORE if empty, else DEFLATE

    // Placeholder Local File Header (30 bytes + name)
    WriteLE32(out, 0x04034b50u); // signature
    WriteLE16(out, 20u);         // version needed
    WriteLE16(out, 0u);          // flags
    WriteLE16(out, method);      // compression method
    WriteLE16(out, 0u);          // mod time
    WriteLE16(out, 0x0021u);     // mod date (1980-01-01)
    WriteLE32(out, 0u);          // crc32 (backfilled)
    WriteLE32(out, 0u);          // compressed size (backfilled)
    WriteLE32(out, origSize);    // uncompressed size
    WriteLE16(out, nameLen);     // file name length
    WriteLE16(out, 0u);          // extra field length
    out.write(cleanName.data(), nameLen);

    uLong crc = crc32(0L, Z_NULL, 0);
    uint32_t compSize = 0;

    if (origSize > 0) {
        z_stream zs{};
        if (deflateInit2(&zs, Z_DEFAULT_COMPRESSION, Z_DEFLATED, -MAX_WBITS, 8, Z_DEFAULT_STRATEGY) != Z_OK) {
            if (outError) *outError = "deflateInit2 failed for " + sourceFile.string();
            return false;
        }

        std::vector<uint8_t> inBuf(64 * 1024);
        std::vector<uint8_t> outBuf(64 * 1024);

        uint64_t remaining = origSize;
        while (remaining > 0) {
            const size_t toRead = static_cast<size_t>(std::min<uint64_t>(remaining, inBuf.size()));
            in.read(reinterpret_cast<char*>(inBuf.data()), static_cast<std::streamsize>(toRead));
            const size_t got = static_cast<size_t>(in.gcount());
            if (got != toRead) {
                deflateEnd(&zs);
                if (outError) *outError = "Short read while zipping " + sourceFile.string();
                return false;
            }
            remaining -= got;
            crc = crc32(crc, inBuf.data(), static_cast<uInt>(got));

            zs.next_in = inBuf.data();
            zs.avail_in = static_cast<uInt>(got);
            const int flush = (remaining == 0) ? Z_FINISH : Z_NO_FLUSH;

            do {
                zs.next_out = outBuf.data();
                zs.avail_out = static_cast<uInt>(outBuf.size());
                const int ret = deflate(&zs, flush);
                if (ret == Z_STREAM_ERROR) {
                    deflateEnd(&zs);
                    if (outError) *outError = "deflate stream error on " + sourceFile.string();
                    return false;
                }
                const size_t produced = outBuf.size() - zs.avail_out;
                if (produced > 0) {
                    out.write(reinterpret_cast<const char*>(outBuf.data()), static_cast<std::streamsize>(produced));
                    compSize += static_cast<uint32_t>(produced);
                }
            } while (zs.avail_out == 0);
        }

        deflateEnd(&zs);
    }

    const uint32_t cdOffset = static_cast<uint32_t>(out.tellp());

    // Central Directory File Header
    WriteLE32(out, 0x02014b50u); // signature
    WriteLE16(out, 20u);         // version made by
    WriteLE16(out, 20u);         // version needed
    WriteLE16(out, 0u);          // flags
    WriteLE16(out, method);      // compression method
    WriteLE16(out, 0u);          // mod time
    WriteLE16(out, 0x0021u);     // mod date
    WriteLE32(out, static_cast<uint32_t>(crc));
    WriteLE32(out, compSize);
    WriteLE32(out, origSize);
    WriteLE16(out, nameLen);
    WriteLE16(out, 0u);          // extra len
    WriteLE16(out, 0u);          // comment len
    WriteLE16(out, 0u);          // disk start
    WriteLE16(out, 0u);          // internal attr
    WriteLE32(out, 0u);          // external attr
    WriteLE32(out, 0u);          // relative offset of local header
    out.write(cleanName.data(), nameLen);

    const uint32_t cdEnd = static_cast<uint32_t>(out.tellp());
    const uint32_t cdSize = cdEnd - cdOffset;

    // End of Central Directory Record
    WriteLE32(out, 0x06054b50u); // signature
    WriteLE16(out, 0u);          // disk number
    WriteLE16(out, 0u);          // start disk
    WriteLE16(out, 1u);          // entries on disk
    WriteLE16(out, 1u);          // total entries
    WriteLE32(out, cdSize);      // central directory size
    WriteLE32(out, cdOffset);    // central directory offset
    WriteLE16(out, 0u);          // comment len

    const uint64_t finalZipSize = static_cast<uint64_t>(out.tellp());

    // Backfill CRC32 and compressed size in Local File Header (offset 14)
    out.seekp(14, std::ios::beg);
    WriteLE32(out, static_cast<uint32_t>(crc));
    WriteLE32(out, compSize);
    out.flush();

    const bool ok = out.good();
    out.close();
    if (!ok) {
        if (outError) *outError = "Failed finalizing zip file: " + zipPath.string();
        return false;
    }

    if (outZipFileSize) *outZipFileSize = finalZipSize;
    return true;
}

// =============================================================================
// Version.cfg & PatchInfoServer.cfg Helpers
// =============================================================================

std::string BuildVersionCfgContent(int version, int moduleVersion) {
    std::ostringstream oss;
    oss << "Version " << version << "\r\nModule " << moduleVersion << "\r\n";
    return oss.str();
}

bool WriteVersionCfgFile(const fs::path& path, int version, int moduleVersion, std::string* outError) {
    std::error_code ec;
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path(), ec);
    }
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        if (outError) *outError = "Failed to write Version.cfg: " + path.string();
        return false;
    }
    const std::string content = BuildVersionCfgContent(version, moduleVersion);
    out.write(content.data(), static_cast<std::streamsize>(content.size()));
    out.close();
    return out.good();
}

bool ReadPatchInfoServerCfg(const fs::path& path, PatchInfoServerData& outData) {
    outData = PatchInfoServerData{};
    std::ifstream in(path);
    if (!in.is_open()) {
        return false;
    }

    std::string line;
    while (std::getline(in, line)) {
        std::istringstream iss(line);
        std::string key;
        int val = 0;
        if (!(iss >> key)) continue;
        std::string lowerKey = ToLowerString(key);
        if (lowerKey == "version" && (iss >> val)) {
            outData.version = val;
        } else if (lowerKey == "skipnum" && (iss >> val)) {
            if (std::find(outData.skipNumbers.begin(), outData.skipNumbers.end(), val) == outData.skipNumbers.end()) {
                outData.skipNumbers.push_back(val);
            }
        }
    }
    std::sort(outData.skipNumbers.begin(), outData.skipNumbers.end());
    return true;
}

bool WritePatchInfoServerCfg(const fs::path& path, const PatchInfoServerData& data, std::string* outError) {
    std::error_code ec;
    if (path.has_parent_path()) {
        fs::create_directories(path.parent_path(), ec);
    }
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    if (!out.is_open()) {
        if (outError) *outError = "Failed to write PatchInfoServer.cfg: " + path.string();
        return false;
    }
    out << "version " << data.version << "\n";
    for (int skip : data.skipNumbers) {
        out << "SkipNum " << skip << "\n";
    }
    out.close();
    return out.good();
}

std::string FormatPatchVersion8(int version) {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%08d", version);
    return buf;
}

std::string FormatByteSize(uint64_t bytes) {
    const char* units[] = { "B", "KiB", "MiB", "GiB" };
    double val = static_cast<double>(bytes);
    size_t u = 0;
    while (val >= 1024.0 && u + 1 < 4) {
        val /= 1024.0;
        ++u;
    }
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(u == 0 ? 0 : 2) << val << " " << units[u];
    return oss.str();
}

} // namespace irine::tools
