// =============================================================================
// IrineCore Client Engine — Native x64/Win32 IBandiVideo2 Playback Backend
// Decodes all Dragon Nest AVI movies natively (VP80, MPEG/MPG1 video + MP2/PCM audio).
// =============================================================================
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "NativeVideoPlayer.h"

#include <mmsystem.h>
#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <new>
#include <vector>

#include "vpx/vpx_decoder.h"
#include "vpx/vp8dx.h"
#include "pl_mpeg.h"

namespace {

constexpr uint32_t MakeFourCC(char a, char b, char c, char d) {
    return (uint32_t)(uint8_t)a |
           ((uint32_t)(uint8_t)b << 8) |
           ((uint32_t)(uint8_t)c << 16) |
           ((uint32_t)(uint8_t)d << 24);
}

inline uint16_t ReadLE16(const uint8_t* p) {
    return (uint16_t)p[0] | ((uint16_t)p[1] << 8);
}

inline uint32_t ReadLE32(const uint8_t* p) {
    return (uint32_t)p[0] |
           ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) |
           ((uint32_t)p[3] << 24);
}

inline int32_t ReadLE32S(const uint8_t* p) {
    return (int32_t)ReadLE32(p);
}

inline bool FourCCEqualsNoCase(uint32_t fcc, const char tag[4]) {
    for (int i = 0; i < 4; ++i) {
        unsigned char a = (unsigned char)((fcc >> (i * 8)) & 0xFF);
        unsigned char b = (unsigned char)tag[i];
        if (a >= 'a' && a <= 'z') a = (unsigned char)(a - 'a' + 'A');
        if (b >= 'a' && b <= 'z') b = (unsigned char)(b - 'a' + 'A');
        if (a != b) return false;
    }
    return true;
}

inline uint8_t ClampByte(int v) {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return (uint8_t)v;
}

inline uint16_t PackRGB565(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3));
}

inline uint16_t PackRGB555(uint8_t r, uint8_t g, uint8_t b) {
    return (uint16_t)(((r & 0xF8) << 7) | ((g & 0xF8) << 2) | (b >> 3));
}

enum class VideoCodec {
    None,
    VP8,
    MPEG1,
};

enum class AudioCodec {
    None,
    MP2,
    PCM,
};

struct AviChunk {
    INT64 offset = 0;
    uint32_t size = 0;
};

struct WaveOutSlot {
    WAVEHDR hdr{};
    std::vector<int16_t> pcm;
    bool inFlight = false;
};

class CNativeBandiVideo2 final : public IBandiVideo2 {
public:
    CNativeBandiVideo2(IBandiVideoFileReader* reader, IBandiVideoSound* sound)
        : m_refCount(1)
        , m_reader(reader)
        , m_sound(sound)
    {
        LARGE_INTEGER freq{};
        QueryPerformanceFrequency(&freq);
        m_qpcFreq = (freq.QuadPart > 0) ? (double)freq.QuadPart : 10000000.0;
    }

    ~CNativeBandiVideo2() {
        Close();
    }

    // IUnknown
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, void**) override {
        return E_NOTIMPL;
    }

    ULONG STDMETHODCALLTYPE AddRef() override {
        return (ULONG)InterlockedIncrement(&m_refCount);
    }

    ULONG STDMETHODCALLTYPE Release() override {
        LONG count = InterlockedDecrement(&m_refCount);
        if (count == 0) {
            delete this;
            return 0;
        }
        return (ULONG)count;
    }

    // IBandiVideo2
    HRESULT Verify(LPCSTR, LPCSTR) override {
        return BV_OK;
    }

    INT GetVersion() override {
        return BVL_VERSION;
    }

    HRESULT Open(LPCWSTR path_name, BOOL) override {
        Close();

        if (!path_name || path_name[0] == L'\0') {
            m_status = BVL_STATUS_ERROR;
            return BVERR_OPEN_FAILED;
        }

        m_status = BVL_STATUS_OPENING;

        if (m_reader) {
            if (FAILED(m_reader->Open(path_name))) {
                m_status = BVL_STATUS_CLOSED;
                return BVERR_OPEN_FAILED;
            }
            m_readerOpened = true;
            m_fileSize = m_reader->GetFileSize();
        } else {
            FILE* fp = nullptr;
            if (_wfopen_s(&fp, path_name, L"rb") != 0 || !fp) {
                m_status = BVL_STATUS_CLOSED;
                return BVERR_OPEN_FAILED;
            }
            m_file = fp;
            _fseeki64(m_file, 0, SEEK_END);
            m_fileSize = _ftelli64(m_file);
            _fseeki64(m_file, 0, SEEK_SET);
        }

        if (m_fileSize < 12 || !ParseAvi()) {
            Close();
            m_status = BVL_STATUS_CLOSED;
            return BVERR_OPEN_FAILED;
        }

        if (!InitDecoders()) {
            Close();
            m_status = BVL_STATUS_CLOSED;
            return BVERR_OPEN_FAILED;
        }

        m_status = BVL_STATUS_READY;
        return BV_OK;
    }

    HRESULT Close() override {
        ShutdownAudioOut();
        DestroyDecoders();

        if (m_readerOpened && m_reader) {
            m_reader->Close();
            m_readerOpened = false;
        }
        if (m_file) {
            fclose(m_file);
            m_file = nullptr;
        }

        m_fileSize = 0;
        m_videoChunks.clear();
        m_audioChunks.clear();
        m_packetBuf.clear();
        m_audioChunkBuf.clear();
        m_cachedY.clear();
        m_cachedU.clear();
        m_cachedV.clear();
        m_cachedWidth = 0;
        m_cachedHeight = 0;
        m_hasCachedFrame = false;

        m_videoCodec = VideoCodec::None;
        m_audioCodec = AudioCodec::None;
        m_videoStreamId = -1;
        m_audioStreamId = -1;
        m_width = 0;
        m_height = 0;
        m_fpsRate = 30;
        m_fpsScale = 1;
        m_fps = 30.0;
        m_totalFrames = 0;
        m_audioSampleRate = 0;
        m_audioChannels = 0;
        m_audioBitsPerSample = 16;

        m_nextVideoChunk = 0;
        m_nextAudioChunk = 0;
        m_audioChunkOffset = 0;
        m_mpegVideoEndSent = false;
        m_mpegAudioEndSent = false;
        m_decodedFrames = 0;
        m_lastShownFrame = -1;
        m_videoEof = false;
        m_audioEof = false;
        m_firstFramePending = false;
        m_baseTimeSec = 0.0;
        m_playStartQpc = 0;
        m_status = BVL_STATUS_CLOSED;

        return BV_OK;
    }

    HRESULT Play() override {
        if (m_status == BVL_STATUS_CLOSED || m_status == BVL_STATUS_ERROR) {
            return BVERR_NOT_OPENED;
        }

        if (m_status == BVL_STATUS_PLAYING) {
            return BV_OK;
        }

        if (m_status == BVL_STATUS_PAUSED) {
            return Pause(FALSE);
        }

        if (m_status == BVL_STATUS_PLAYEND) {
            ResetPlaybackToStart();
        }

        m_playStartQpc = QueryQpc();
        m_status = BVL_STATUS_PLAYING;

        EnsureAudioOutOpen();
        PumpAudio();

        return BV_OK;
    }

    HRESULT Stop() override {
        if (m_status == BVL_STATUS_CLOSED) {
            return BVERR_NOT_OPENED;
        }
        ResetPlaybackToStart();
        m_status = BVL_STATUS_READY;
        return BV_OK;
    }

    HRESULT Seek(INT64 value, BVL_SEEK_FLAG seek_flag) override {
        if (m_status == BVL_STATUS_CLOSED) {
            return BVERR_NOT_OPENED;
        }

        const bool wasPlaying = (m_status == BVL_STATUS_PLAYING);
        ResetPlaybackToStart();

        INT64 targetFrame = 0;
        if (seek_flag == BVL_SEEK_FRAME) {
            targetFrame = value;
        } else {
            double sec = (double)value / 10000000.0;
            targetFrame = (INT64)(sec * m_fps);
        }

        if (targetFrame > 0) {
            if (m_totalFrames > 0 && targetFrame >= m_totalFrames) {
                targetFrame = m_totalFrames - 1;
            }
            for (INT64 i = 0; i < targetFrame && !m_videoEof; ++i) {
                if (!DecodeNextVideoFrame()) break;
            }
            m_baseTimeSec = (double)targetFrame / m_fps;
            m_lastShownFrame = targetFrame - 1;
        }

        if (wasPlaying) {
            m_playStartQpc = QueryQpc();
            m_status = BVL_STATUS_PLAYING;
            EnsureAudioOutOpen();
            PumpAudio();
        } else {
            m_status = BVL_STATUS_READY;
        }

        return BV_OK;
    }

    HRESULT Pause(BOOL pause) override {
        if (m_status == BVL_STATUS_CLOSED) {
            return BVERR_NOT_OPENED;
        }

        if (pause) {
            if (m_status == BVL_STATUS_PLAYING) {
                m_baseTimeSec = GetElapsedSec();
                m_status = BVL_STATUS_PAUSED;
                if (m_hWaveOut) {
                    waveOutPause(m_hWaveOut);
                }
            }
        } else {
            if (m_status == BVL_STATUS_PAUSED) {
                m_playStartQpc = QueryQpc();
                m_status = BVL_STATUS_PLAYING;
                if (m_hWaveOut) {
                    waveOutRestart(m_hWaveOut);
                }
                PumpAudio();
            }
        }
        return BV_OK;
    }

    BOOL IsNextFrame() override {
        UpdatePlaybackState();
        if (m_status != BVL_STATUS_PLAYING) {
            return FALSE;
        }
        if (m_firstFramePending) {
            return TRUE;
        }
        const INT64 targetFrame = (INT64)(GetElapsedSec() * m_fps);
        return (targetFrame > m_lastShownFrame) ? TRUE : FALSE;
    }

    HRESULT GetFrame(BVL_FRAME& frame, BOOL can_skip) override {
        if (m_status == BVL_STATUS_CLOSED) {
            return BVERR_NOT_OPENED;
        }
        if (!frame.frame_buf || frame.pitch <= 0 || frame.width <= 0 || frame.height <= 0) {
            return BVERR_FAIL;
        }

        INT64 targetFrame = 0;
        if (m_firstFramePending) {
            targetFrame = 0;
        } else {
            targetFrame = (INT64)(GetElapsedSec() * m_fps);
            if (!can_skip && m_lastShownFrame >= 0 && targetFrame > m_lastShownFrame + 1) {
                targetFrame = m_lastShownFrame + 1;
            }
        }

        if (m_totalFrames > 0 && targetFrame >= m_totalFrames) {
            targetFrame = m_totalFrames - 1;
        }

        // Decode up to targetFrame (cap catch-up work per tick to prevent UI freezes after alt-tab).
        constexpr int kMaxDecodePerTick = 12;
        int decodedThisTick = 0;
        while (!m_videoEof && (m_decodedFrames - 1) < targetFrame && decodedThisTick < kMaxDecodePerTick) {
            if (!DecodeNextVideoFrame()) {
                break;
            }
            ++decodedThisTick;
        }

        if (!m_hasCachedFrame) {
            // If video stream ended without producing any frame, fail cleanly.
            if (m_videoEof) {
                m_status = BVL_STATUS_PLAYEND;
                return BVERR_FAIL;
            }
            // Clear black fallback frame
            uint8_t* dst = static_cast<uint8_t*>(frame.frame_buf);
            for (INT y = 0; y < frame.height; ++y) {
                std::memset(dst + y * frame.pitch, 0, (size_t)frame.pitch);
            }
        } else {
            ConvertCachedYUVToFrame(frame);
        }

        const INT64 shownIdx = (m_decodedFrames > 0) ? (m_decodedFrames - 1) : 0;
        frame.frame_number = shownIdx;
        frame.frame_time = (INT64)((double)shownIdx / m_fps * 10000000.0);

        m_lastShownFrame = std::max(m_lastShownFrame, shownIdx);
        m_firstFramePending = false;

        PumpAudio();
        return BV_OK;
    }

    HRESULT GetVideoInfo(BVL_VIDEO_INFO& info) override {
        if (m_status == BVL_STATUS_CLOSED) {
            return BVERR_NOT_OPENED;
        }
        info.width = m_width;
        info.height = m_height;
        info.fps_rate = m_fpsRate;
        info.fps_scale = m_fpsScale;
        info.samplerate = m_audioSampleRate;
        info.channels = m_audioChannels;
        info.total_frames = m_totalFrames;
        info.total_time = (m_fps > 0.0)
            ? (INT64)((double)m_totalFrames / m_fps * 10000000.0)
            : 0;
        return BV_OK;
    }

    HRESULT GetStatus(BVL_STATUS& status) override {
        UpdatePlaybackState();
        status = m_status;
        return BV_OK;
    }

    HRESULT SetVolume(INT volume) override {
        if (volume < 0) volume = 0;
        if (volume > 255) volume = 255;
        m_volume = volume;
        ApplyWaveOutVolume();
        return BV_OK;
    }

private:
    // -------------------------------------------------------------------------
    // Low-level Stream I/O (IBandiVideoFileReader or FILE*)
    // -------------------------------------------------------------------------
    bool ReadAt(INT64 offset, void* dst, size_t bytes) {
        if (bytes == 0) return true;
        if (offset < 0 || offset + (INT64)bytes > m_fileSize) return false;

        if (m_reader) {
            if (m_reader->SetPosition(offset) != offset) {
                return false;
            }
            size_t total = 0;
            uint8_t* out = static_cast<uint8_t*>(dst);
            while (total < bytes) {
                INT chunk = (INT)std::min<size_t>(bytes - total, 0x1000000);
                INT got = m_reader->Read(out + total, chunk);
                if (got <= 0) return false;
                total += (size_t)got;
            }
            return true;
        }
        if (m_file) {
            if (_fseeki64(m_file, offset, SEEK_SET) != 0) return false;
            return fread(dst, 1, bytes, m_file) == bytes;
        }
        return false;
    }

    // -------------------------------------------------------------------------
    // RIFF AVI 1.0 / OpenDML Demuxer
    // -------------------------------------------------------------------------
    bool ParseAvi() {
        uint8_t hdr[12];
        INT64 pos = 0;

        while (pos + 12 <= m_fileSize) {
            if (!ReadAt(pos, hdr, 12)) break;
            const uint32_t riffId = ReadLE32(hdr);
            const uint32_t riffSize = ReadLE32(hdr + 4);
            const uint32_t formType = ReadLE32(hdr + 8);

            if (riffId != MakeFourCC('R', 'I', 'F', 'F')) {
                break;
            }

            INT64 riffEnd = pos + 8 + (INT64)riffSize;
            if (riffEnd > m_fileSize) riffEnd = m_fileSize;

            if (formType == MakeFourCC('A', 'V', 'I', ' ') ||
                formType == MakeFourCC('A', 'V', 'I', 'X')) {
                ParseContainer(pos + 12, riffEnd, false);
            }

            pos = riffEnd + (riffEnd & 1);
        }

        if (m_videoCodec == VideoCodec::None || m_width <= 0 || m_height <= 0 || m_videoChunks.empty()) {
            return false;
        }

        // Compute rational FPS (prefer strh dwRate/dwScale, fall back to avih dwMicroSecPerFrame).
        if (m_strhScale > 0 && m_strhRate > 0) {
            double candidate = (double)m_strhRate / (double)m_strhScale;
            if (candidate >= 1.0 && candidate <= 240.0) {
                m_fpsRate = (INT)m_strhRate;
                m_fpsScale = (INT)m_strhScale;
            }
        }
        if (m_fpsRate <= 0 || m_fpsScale <= 0) {
            if (m_avihMicroSecPerFrame > 0) {
                m_fpsRate = 1000000;
                m_fpsScale = (INT)m_avihMicroSecPerFrame;
            } else {
                m_fpsRate = 30;
                m_fpsScale = 1;
            }
        }
        m_fps = (double)m_fpsRate / (double)m_fpsScale;
        if (m_fps < 1.0 || m_fps > 240.0) {
            m_fps = 30.0;
            m_fpsRate = 30;
            m_fpsScale = 1;
        }

        m_totalFrames = (INT64)m_videoChunks.size();
        return true;
    }

    void ParseContainer(INT64 start, INT64 end, bool insideMovi) {
        uint8_t ch[8];
        INT64 pos = start;

        while (pos + 8 <= end) {
            if (!ReadAt(pos, ch, 8)) break;
            const uint32_t fourcc = ReadLE32(ch);
            const uint32_t size = ReadLE32(ch + 4);
            const INT64 dataStart = pos + 8;
            INT64 dataEnd = dataStart + (INT64)size;
            if (dataEnd > end) dataEnd = end;

            if (fourcc == MakeFourCC('L', 'I', 'S', 'T')) {
                if (size >= 4 && dataStart + 4 <= end) {
                    uint8_t listTypeBuf[4];
                    if (ReadAt(dataStart, listTypeBuf, 4)) {
                        const uint32_t listType = ReadLE32(listTypeBuf);
                        if (listType == MakeFourCC('h', 'd', 'r', 'l')) {
                            ParseHdrl(dataStart + 4, dataEnd);
                        } else if (listType == MakeFourCC('m', 'o', 'v', 'i')) {
                            ParseContainer(dataStart + 4, dataEnd, true);
                        } else if (listType == MakeFourCC('r', 'e', 'c', ' ') && insideMovi) {
                            ParseContainer(dataStart + 4, dataEnd, true);
                        }
                    }
                }
            } else if (insideMovi) {
                const uint8_t c0 = ch[0];
                const uint8_t c1 = ch[1];
                if (c0 >= '0' && c0 <= '9' && c1 >= '0' && c1 <= '9') {
                    const int streamIdx = (c0 - '0') * 10 + (c1 - '0');
                    const uint8_t t0 = (uint8_t)tolower(ch[2]);
                    const uint8_t t1 = (uint8_t)tolower(ch[3]);

                    if (streamIdx == m_videoStreamId && t0 == 'd' && (t1 == 'c' || t1 == 'b')) {
                        m_videoChunks.push_back({ dataStart, (uint32_t)(dataEnd - dataStart) });
                    } else if (streamIdx == m_audioStreamId && t0 == 'w' && t1 == 'b') {
                        if (dataEnd > dataStart) {
                            m_audioChunks.push_back({ dataStart, (uint32_t)(dataEnd - dataStart) });
                        }
                    }
                }
            }

            pos = dataEnd + (size & 1u);
        }
    }

    void ParseHdrl(INT64 start, INT64 end) {
        uint8_t ch[8];
        INT64 pos = start;
        int currentStreamIdx = 0;

        while (pos + 8 <= end) {
            if (!ReadAt(pos, ch, 8)) break;
            const uint32_t fourcc = ReadLE32(ch);
            const uint32_t size = ReadLE32(ch + 4);
            const INT64 dataStart = pos + 8;
            INT64 dataEnd = dataStart + (INT64)size;
            if (dataEnd > end) dataEnd = end;

            if (fourcc == MakeFourCC('a', 'v', 'i', 'h') && (dataEnd - dataStart) >= 40) {
                uint8_t avih[40];
                if (ReadAt(dataStart, avih, 40)) {
                    m_avihMicroSecPerFrame = ReadLE32(avih + 0);
                    const int32_t w = ReadLE32S(avih + 32);
                    const int32_t h = ReadLE32S(avih + 36);
                    if (w > 0 && h > 0) {
                        m_width = w;
                        m_height = h;
                    }
                }
            } else if (fourcc == MakeFourCC('L', 'I', 'S', 'T') && size >= 4) {
                uint8_t listTypeBuf[4];
                if (ReadAt(dataStart, listTypeBuf, 4)) {
                    if (ReadLE32(listTypeBuf) == MakeFourCC('s', 't', 'r', 'l')) {
                        ParseStrl(dataStart + 4, dataEnd, currentStreamIdx++);
                    }
                }
            }

            pos = dataEnd + (size & 1u);
        }
    }

    void ParseStrl(INT64 start, INT64 end, int streamIdx) {
        uint8_t ch[8];
        INT64 pos = start;
        uint32_t streamType = 0;
        uint32_t streamHandler = 0;
        uint32_t scale = 0;
        uint32_t rate = 0;

        while (pos + 8 <= end) {
            if (!ReadAt(pos, ch, 8)) break;
            const uint32_t fourcc = ReadLE32(ch);
            const uint32_t size = ReadLE32(ch + 4);
            const INT64 dataStart = pos + 8;
            INT64 dataEnd = dataStart + (INT64)size;
            if (dataEnd > end) dataEnd = end;

            if (fourcc == MakeFourCC('s', 't', 'r', 'h') && (dataEnd - dataStart) >= 32) {
                uint8_t strh[32];
                if (ReadAt(dataStart, strh, 32)) {
                    streamType = ReadLE32(strh + 0);
                    streamHandler = ReadLE32(strh + 4);
                    scale = ReadLE32(strh + 20);
                    rate = ReadLE32(strh + 24);
                }
            } else if (fourcc == MakeFourCC('s', 't', 'r', 'f')) {
                if (streamType == MakeFourCC('v', 'i', 'd', 's') && (dataEnd - dataStart) >= 40) {
                    uint8_t bih[40];
                    if (ReadAt(dataStart, bih, 40)) {
                        const int32_t biW = ReadLE32S(bih + 4);
                        const int32_t biH = ReadLE32S(bih + 8);
                        const uint32_t biCompression = ReadLE32(bih + 16);

                        VideoCodec codec = IdentifyVideoCodec(biCompression);
                        if (codec == VideoCodec::None) {
                            codec = IdentifyVideoCodec(streamHandler);
                        }
                        if (codec != VideoCodec::None && m_videoStreamId < 0) {
                            m_videoStreamId = streamIdx;
                            m_videoCodec = codec;
                            if (biW > 0) m_width = biW;
                            if (biH != 0) m_height = (biH < 0) ? -biH : biH;
                            m_strhScale = scale;
                            m_strhRate = rate;
                        }
                    }
                } else if (streamType == MakeFourCC('a', 'u', 'd', 's') && (dataEnd - dataStart) >= 16) {
                    uint8_t wfx[18] = {};
                    const size_t toRead = std::min<size_t>((size_t)(dataEnd - dataStart), sizeof(wfx));
                    if (ReadAt(dataStart, wfx, toRead)) {
                        const uint16_t formatTag = ReadLE16(wfx + 0);
                        const uint16_t channels = ReadLE16(wfx + 2);
                        const uint32_t sampleRate = ReadLE32(wfx + 4);
                        const uint16_t bitsPerSample = ReadLE16(wfx + 14);

                        if (m_audioStreamId < 0 && channels >= 1 && channels <= 2 && sampleRate >= 8000) {
                            if (formatTag == 0x0050) { // WAVE_FORMAT_MPEG (MP2 Layer II)
                                m_audioStreamId = streamIdx;
                                m_audioCodec = AudioCodec::MP2;
                                m_audioChannels = channels;
                                m_audioSampleRate = (INT)sampleRate;
                                m_audioBitsPerSample = 16;
                            } else if (formatTag == 0x0001 && (bitsPerSample == 8 || bitsPerSample == 16)) {
                                m_audioStreamId = streamIdx;
                                m_audioCodec = AudioCodec::PCM;
                                m_audioChannels = channels;
                                m_audioSampleRate = (INT)sampleRate;
                                m_audioBitsPerSample = bitsPerSample;
                            }
                        }
                    }
                }
            }

            pos = dataEnd + (size & 1u);
        }
    }

    static VideoCodec IdentifyVideoCodec(uint32_t fcc) {
        if (FourCCEqualsNoCase(fcc, "VP80")) {
            return VideoCodec::VP8;
        }
        if (FourCCEqualsNoCase(fcc, "MPEG") || FourCCEqualsNoCase(fcc, "MPG1")) {
            return VideoCodec::MPEG1;
        }
        return VideoCodec::None;
    }

    // -------------------------------------------------------------------------
    // Decoder Lifecycle (libvpx VP8 / pl_mpeg MPEG-1 & MP2)
    // -------------------------------------------------------------------------
    bool InitDecoders() {
        DestroyDecoders();

        m_nextVideoChunk = 0;
        m_nextAudioChunk = 0;
        m_audioChunkOffset = 0;
        m_mpegVideoEndSent = false;
        m_mpegAudioEndSent = false;
        m_decodedFrames = 0;
        m_lastShownFrame = -1;
        m_videoEof = false;
        m_audioEof = m_audioChunks.empty() || (m_audioCodec == AudioCodec::None);
        m_hasCachedFrame = false;
        m_firstFramePending = true;
        m_baseTimeSec = 0.0;

        if (m_videoCodec == VideoCodec::VP8) {
            if (vpx_codec_dec_init(&m_vpxCtx, vpx_codec_vp8_dx(), NULL, 0) != VPX_CODEC_OK) {
                return false;
            }
            m_vpxInitialized = true;
        } else if (m_videoCodec == VideoCodec::MPEG1) {
            m_mpegVideoBuf = plm_buffer_create_with_capacity(256 * 1024);
            if (!m_mpegVideoBuf) return false;
            plm_buffer_set_load_callback(m_mpegVideoBuf, &CNativeBandiVideo2::OnMpegVideoLoad, this);
            m_mpegVideo = plm_video_create_with_buffer(m_mpegVideoBuf, 0);
            if (!m_mpegVideo) {
                DestroyDecoders();
                return false;
            }
            if (plm_video_has_header(m_mpegVideo)) {
                int w = plm_video_get_width(m_mpegVideo);
                int h = plm_video_get_height(m_mpegVideo);
                if (w > 0 && h > 0) {
                    m_width = w;
                    m_height = h;
                }
            }
        } else {
            return false;
        }

        if (m_audioCodec == AudioCodec::MP2 && !m_audioChunks.empty()) {
            m_mpegAudioBuf = plm_buffer_create_with_capacity(64 * 1024);
            if (m_mpegAudioBuf) {
                plm_buffer_set_load_callback(m_mpegAudioBuf, &CNativeBandiVideo2::OnMpegAudioLoad, this);
                m_mpegAudio = plm_audio_create_with_buffer(m_mpegAudioBuf, 0);
                if (m_mpegAudio && plm_audio_has_header(m_mpegAudio)) {
                    int sr = plm_audio_get_samplerate(m_mpegAudio);
                    if (sr > 0) m_audioSampleRate = sr;
                }
            }
        }

        return true;
    }

    void DestroyDecoders() {
        if (m_vpxInitialized) {
            vpx_codec_destroy(&m_vpxCtx);
            m_vpxInitialized = false;
        }
        if (m_mpegVideo) {
            plm_video_destroy(m_mpegVideo);
            m_mpegVideo = nullptr;
        }
        if (m_mpegVideoBuf) {
            plm_buffer_destroy(m_mpegVideoBuf);
            m_mpegVideoBuf = nullptr;
        }
        if (m_mpegAudio) {
            plm_audio_destroy(m_mpegAudio);
            m_mpegAudio = nullptr;
        }
        if (m_mpegAudioBuf) {
            plm_buffer_destroy(m_mpegAudioBuf);
            m_mpegAudioBuf = nullptr;
        }
    }

    void ResetPlaybackToStart() {
        ResetAudioOut();
        InitDecoders();
    }

    static void OnMpegVideoLoad(plm_buffer_t* buf, void* user) {
        auto* self = static_cast<CNativeBandiVideo2*>(user);
        while (self->m_nextVideoChunk < self->m_videoChunks.size()) {
            const AviChunk& ch = self->m_videoChunks[self->m_nextVideoChunk++];
            if (ch.size == 0) continue;
            self->m_packetBuf.resize(ch.size);
            if (self->ReadAt(ch.offset, self->m_packetBuf.data(), ch.size)) {
                plm_buffer_write(buf, self->m_packetBuf.data(), ch.size);
                return;
            }
        }
        if (!self->m_mpegVideoEndSent) {
            self->m_mpegVideoEndSent = true;
            // Append MPEG-1 sequence end code so plm_video flushes the last buffered frame
            // and guarantees total_size > 0 for plm_buffer_has_ended().
            uint8_t seqEnd[4] = { 0x00, 0x00, 0x01, 0xB7 };
            plm_buffer_write(buf, seqEnd, sizeof(seqEnd));
            plm_buffer_signal_end(buf);
        }
    }

    static void OnMpegAudioLoad(plm_buffer_t* buf, void* user) {
        auto* self = static_cast<CNativeBandiVideo2*>(user);
        while (self->m_nextAudioChunk < self->m_audioChunks.size()) {
            const AviChunk& ch = self->m_audioChunks[self->m_nextAudioChunk++];
            if (ch.size == 0) continue;
            self->m_audioChunkBuf.resize(ch.size);
            if (self->ReadAt(ch.offset, self->m_audioChunkBuf.data(), ch.size)) {
                plm_buffer_write(buf, self->m_audioChunkBuf.data(), ch.size);
                return;
            }
        }
        if (!self->m_mpegAudioEndSent) {
            self->m_mpegAudioEndSent = true;
            if (plm_buffer_get_size(buf) == 0) {
                uint8_t pad[4] = { 0, 0, 0, 0 };
                plm_buffer_write(buf, pad, sizeof(pad));
            }
            plm_buffer_signal_end(buf);
        }
    }

    // -------------------------------------------------------------------------
    // Frame Decoding & Caching
    // -------------------------------------------------------------------------
    bool DecodeNextVideoFrame() {
        if (m_videoEof) return false;

        if (m_videoCodec == VideoCodec::VP8) {
            if (m_nextVideoChunk >= m_videoChunks.size()) {
                m_videoEof = true;
                return false;
            }
            const AviChunk& ch = m_videoChunks[m_nextVideoChunk++];
            if (ch.size == 0) {
                // AVI drop/repeat frame: keep previous frame and advance timeline.
                ++m_decodedFrames;
                return true;
            }

            m_packetBuf.resize(ch.size);
            if (!ReadAt(ch.offset, m_packetBuf.data(), ch.size)) {
                m_videoEof = true;
                return false;
            }

            if (vpx_codec_decode(&m_vpxCtx, m_packetBuf.data(), ch.size, NULL, 0) != VPX_CODEC_OK) {
                // Skip corrupted packet but advance timeline so playback does not hang.
                ++m_decodedFrames;
                return true;
            }

            vpx_codec_iter_t iter = nullptr;
            vpx_image_t* img = vpx_codec_get_frame(&m_vpxCtx, &iter);
            if (img) {
                CacheYUV420(
                    (int)img->d_w, (int)img->d_h,
                    img->planes[VPX_PLANE_Y], img->stride[VPX_PLANE_Y],
                    img->planes[VPX_PLANE_U], img->stride[VPX_PLANE_U],
                    img->planes[VPX_PLANE_V], img->stride[VPX_PLANE_V]
                );
            }
            ++m_decodedFrames;
            return true;
        }

        if (m_videoCodec == VideoCodec::MPEG1 && m_mpegVideo) {
            plm_frame_t* frame = plm_video_decode(m_mpegVideo);
            if (!frame) {
                m_videoEof = true;
                return false;
            }
            CacheYUV420(
                (int)frame->width, (int)frame->height,
                frame->y.data, (int)frame->y.width,
                frame->cb.data, (int)frame->cb.width,
                frame->cr.data, (int)frame->cr.width
            );
            ++m_decodedFrames;
            return true;
        }

        m_videoEof = true;
        return false;
    }

    void CacheYUV420(int w, int h,
                     const uint8_t* yPlane, int yStride,
                     const uint8_t* uPlane, int uStride,
                     const uint8_t* vPlane, int vStride) {
        if (w <= 0 || h <= 0 || !yPlane || !uPlane || !vPlane) return;

        const int uvW = (w + 1) >> 1;
        const int uvH = (h + 1) >> 1;
        m_cachedWidth = w;
        m_cachedHeight = h;
        m_cachedY.resize((size_t)w * (size_t)h);
        m_cachedU.resize((size_t)uvW * (size_t)uvH);
        m_cachedV.resize((size_t)uvW * (size_t)uvH);

        for (int y = 0; y < h; ++y) {
            std::memcpy(m_cachedY.data() + (size_t)y * (size_t)w, yPlane + (size_t)y * (size_t)yStride, (size_t)w);
        }
        for (int y = 0; y < uvH; ++y) {
            std::memcpy(m_cachedU.data() + (size_t)y * (size_t)uvW, uPlane + (size_t)y * (size_t)uStride, (size_t)uvW);
            std::memcpy(m_cachedV.data() + (size_t)y * (size_t)uvW, vPlane + (size_t)y * (size_t)vStride, (size_t)uvW);
        }
        m_hasCachedFrame = true;
    }

    void ConvertCachedYUVToFrame(BVL_FRAME& frame) const {
        const int dstW = std::min((int)frame.width, m_cachedWidth);
        const int dstH = std::min((int)frame.height, m_cachedHeight);
        if (dstW <= 0 || dstH <= 0) return;

        const int uvW = (m_cachedWidth + 1) >> 1;
        uint8_t* dstBase = static_cast<uint8_t*>(frame.frame_buf);
        const int pitch = frame.pitch;
        const BVL_PIXEL_FORMAT fmt = frame.pixel_format;

        if (fmt == BVLPF_X8R8G8B8 || fmt == BVLPF_A8R8G8B8) {
            for (int y = 0; y < dstH; ++y) {
                const uint8_t* yRow = m_cachedY.data() + (size_t)y * (size_t)m_cachedWidth;
                const uint8_t* uRow = m_cachedU.data() + (size_t)(y >> 1) * (size_t)uvW;
                const uint8_t* vRow = m_cachedV.data() + (size_t)(y >> 1) * (size_t)uvW;
                uint32_t* out = reinterpret_cast<uint32_t*>(dstBase + (size_t)y * (size_t)pitch);

                int x = 0;
                for (; x + 1 < dstW; x += 2) {
                    const int d = (int)uRow[x >> 1] - 128;
                    const int e = (int)vRow[x >> 1] - 128;
                    const int rDelta = 409 * e + 128;
                    const int gDelta = -100 * d - 208 * e + 128;
                    const int bDelta = 516 * d + 128;

                    const int c0 = 298 * ((int)yRow[x] - 16);
                    const uint32_t r0 = ClampByte((c0 + rDelta) >> 8);
                    const uint32_t g0 = ClampByte((c0 + gDelta) >> 8);
                    const uint32_t b0 = ClampByte((c0 + bDelta) >> 8);
                    out[x] = 0xFF000000u | (r0 << 16) | (g0 << 8) | b0;

                    const int c1 = 298 * ((int)yRow[x + 1] - 16);
                    const uint32_t r1 = ClampByte((c1 + rDelta) >> 8);
                    const uint32_t g1 = ClampByte((c1 + gDelta) >> 8);
                    const uint32_t b1 = ClampByte((c1 + bDelta) >> 8);
                    out[x + 1] = 0xFF000000u | (r1 << 16) | (g1 << 8) | b1;
                }
                if (x < dstW) {
                    const int c = 298 * ((int)yRow[x] - 16);
                    const int d = (int)uRow[x >> 1] - 128;
                    const int e = (int)vRow[x >> 1] - 128;
                    const uint32_t r = ClampByte((c + 409 * e + 128) >> 8);
                    const uint32_t g = ClampByte((c - 100 * d - 208 * e + 128) >> 8);
                    const uint32_t b = ClampByte((c + 516 * d + 128) >> 8);
                    out[x] = 0xFF000000u | (r << 16) | (g << 8) | b;
                }
            }
            return;
        }

        // Generic path for R8G8B8, R5G6B5, R5G5B5
        for (int y = 0; y < dstH; ++y) {
            const uint8_t* yRow = m_cachedY.data() + (size_t)y * (size_t)m_cachedWidth;
            const uint8_t* uRow = m_cachedU.data() + (size_t)(y >> 1) * (size_t)uvW;
            const uint8_t* vRow = m_cachedV.data() + (size_t)(y >> 1) * (size_t)uvW;
            uint8_t* outBytes = dstBase + (size_t)y * (size_t)pitch;

            for (int x = 0; x < dstW; ++x) {
                const int c = 298 * ((int)yRow[x] - 16);
                const int d = (int)uRow[x >> 1] - 128;
                const int e = (int)vRow[x >> 1] - 128;
                const uint8_t r = ClampByte((c + 409 * e + 128) >> 8);
                const uint8_t g = ClampByte((c - 100 * d - 208 * e + 128) >> 8);
                const uint8_t b = ClampByte((c + 516 * d + 128) >> 8);

                if (fmt == BVLPF_R8G8B8) {
                    outBytes[x * 3 + 0] = b;
                    outBytes[x * 3 + 1] = g;
                    outBytes[x * 3 + 2] = r;
                } else if (fmt == BVLPF_R5G6B5) {
                    reinterpret_cast<uint16_t*>(outBytes)[x] = PackRGB565(r, g, b);
                } else if (fmt == BVLPF_R5G5B5) {
                    reinterpret_cast<uint16_t*>(outBytes)[x] = PackRGB555(r, g, b);
                }
            }
        }
    }

    // -------------------------------------------------------------------------
    // Audio Decoding & Windows waveOut Streaming
    // -------------------------------------------------------------------------
    void EnsureAudioOutOpen() {
        if (m_hWaveOut || m_audioCodec == AudioCodec::None || m_audioChunks.empty()) {
            return;
        }
        if (m_audioSampleRate <= 0 || m_audioChannels <= 0) {
            return;
        }

        // pl_mpeg always outputs stereo interleaved float samples (2 channels).
        const WORD outChannels = (m_audioCodec == AudioCodec::MP2) ? 2 : (WORD)m_audioChannels;

        WAVEFORMATEX wfx{};
        wfx.wFormatTag = WAVE_FORMAT_PCM;
        wfx.nChannels = outChannels;
        wfx.nSamplesPerSec = (DWORD)m_audioSampleRate;
        wfx.wBitsPerSample = 16;
        wfx.nBlockAlign = (WORD)(wfx.nChannels * (wfx.wBitsPerSample / 8));
        wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;

        if (waveOutOpen(&m_hWaveOut, WAVE_MAPPER, &wfx, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR) {
            m_hWaveOut = nullptr;
            return;
        }

        m_waveSlots.clear();
        m_waveSlots.resize(kAudioBufferCount);
        ApplyWaveOutVolume();
    }

    void ApplyWaveOutVolume() {
        if (!m_hWaveOut) return;
        const DWORD level = (DWORD)((m_volume * 65535 + 127) / 255);
        waveOutSetVolume(m_hWaveOut, (level << 16) | level);
    }

    void ResetAudioOut() {
        if (!m_hWaveOut) return;
        waveOutReset(m_hWaveOut);
        for (auto& slot : m_waveSlots) {
            if (slot.hdr.dwFlags & WHDR_PREPARED) {
                waveOutUnprepareHeader(m_hWaveOut, &slot.hdr, sizeof(WAVEHDR));
            }
            slot.hdr = {};
            slot.inFlight = false;
        }
    }

    void ShutdownAudioOut() {
        if (!m_hWaveOut) return;
        ResetAudioOut();
        waveOutClose(m_hWaveOut);
        m_hWaveOut = nullptr;
        m_waveSlots.clear();
    }

    void PumpAudio() {
        if (!m_hWaveOut || m_status != BVL_STATUS_PLAYING) return;

        for (auto& slot : m_waveSlots) {
            if (slot.inFlight) {
                if ((slot.hdr.dwFlags & WHDR_DONE) == 0) {
                    continue;
                }
                waveOutUnprepareHeader(m_hWaveOut, &slot.hdr, sizeof(WAVEHDR));
                slot.hdr = {};
                slot.inFlight = false;
            }

            if (m_audioEof) continue;

            if (!FillAudioSlot(slot)) {
                m_audioEof = true;
                continue;
            }

            slot.hdr = {};
            slot.hdr.lpData = reinterpret_cast<LPSTR>(slot.pcm.data());
            slot.hdr.dwBufferLength = (DWORD)(slot.pcm.size() * sizeof(int16_t));
            if (waveOutPrepareHeader(m_hWaveOut, &slot.hdr, sizeof(WAVEHDR)) == MMSYSERR_NOERROR) {
                if (waveOutWrite(m_hWaveOut, &slot.hdr, sizeof(WAVEHDR)) == MMSYSERR_NOERROR) {
                    slot.inFlight = true;
                } else {
                    waveOutUnprepareHeader(m_hWaveOut, &slot.hdr, sizeof(WAVEHDR));
                    slot.hdr = {};
                }
            }
        }
    }

    bool FillAudioSlot(WaveOutSlot& slot) {
        if (m_audioCodec == AudioCodec::MP2 && m_mpegAudio) {
            plm_samples_t* samples = plm_audio_decode(m_mpegAudio);
            if (!samples || samples->count == 0) {
                return false;
            }
            const size_t totalSamples = (size_t)samples->count * 2;
            slot.pcm.resize(totalSamples);
            for (size_t i = 0; i < totalSamples; ++i) {
                float f = samples->interleaved[i];
                if (f > 1.0f) f = 1.0f;
                else if (f < -1.0f) f = -1.0f;
                slot.pcm[i] = (int16_t)(f * 32767.0f);
            }
            return true;
        }

        if (m_audioCodec == AudioCodec::PCM) {
            constexpr size_t kTargetSamples = 1152;
            const size_t outSamples = kTargetSamples * (size_t)m_audioChannels;
            slot.pcm.clear();
            slot.pcm.reserve(outSamples);

            while (slot.pcm.size() < outSamples && m_nextAudioChunk < m_audioChunks.size()) {
                const AviChunk& ch = m_audioChunks[m_nextAudioChunk];
                if (m_audioChunkOffset == 0) {
                    m_audioChunkBuf.resize(ch.size);
                    if (!ReadAt(ch.offset, m_audioChunkBuf.data(), ch.size)) {
                        ++m_nextAudioChunk;
                        continue;
                    }
                }

                if (m_audioBitsPerSample == 16) {
                    while (slot.pcm.size() < outSamples && m_audioChunkOffset + 2 <= m_audioChunkBuf.size()) {
                        int16_t s = (int16_t)ReadLE16(m_audioChunkBuf.data() + m_audioChunkOffset);
                        slot.pcm.push_back(s);
                        m_audioChunkOffset += 2;
                    }
                } else {
                    while (slot.pcm.size() < outSamples && m_audioChunkOffset < m_audioChunkBuf.size()) {
                        int16_t s = (int16_t)(((int)m_audioChunkBuf[m_audioChunkOffset] - 128) << 8);
                        slot.pcm.push_back(s);
                        m_audioChunkOffset += 1;
                    }
                }

                if (m_audioChunkOffset >= m_audioChunkBuf.size()) {
                    ++m_nextAudioChunk;
                    m_audioChunkOffset = 0;
                }
            }
            return !slot.pcm.empty();
        }

        return false;
    }

    // -------------------------------------------------------------------------
    // Timing & State Machine
    // -------------------------------------------------------------------------
    static LONGLONG QueryQpc() {
        LARGE_INTEGER now{};
        QueryPerformanceCounter(&now);
        return now.QuadPart;
    }

    double GetElapsedSec() const {
        if (m_status != BVL_STATUS_PLAYING) {
            return m_baseTimeSec;
        }
        const LONGLONG now = QueryQpc();
        const double delta = (now > m_playStartQpc) ? ((double)(now - m_playStartQpc) / m_qpcFreq) : 0.0;
        return m_baseTimeSec + delta;
    }

    void UpdatePlaybackState() {
        if (m_status != BVL_STATUS_PLAYING) return;

        PumpAudio();

        if (!m_firstFramePending) {
            const INT64 targetFrame = (INT64)(GetElapsedSec() * m_fps);
            if ((m_totalFrames > 0 && targetFrame >= m_totalFrames) ||
                (m_videoEof && targetFrame >= m_decodedFrames)) {
                m_status = BVL_STATUS_PLAYEND;
            }
        }
    }

private:
    static constexpr size_t kAudioBufferCount = 16;

    volatile LONG m_refCount;
    IBandiVideoFileReader* m_reader = nullptr;
    IBandiVideoSound* m_sound = nullptr;
    bool m_readerOpened = false;
    FILE* m_file = nullptr;
    INT64 m_fileSize = 0;

    // AVI metadata
    VideoCodec m_videoCodec = VideoCodec::None;
    AudioCodec m_audioCodec = AudioCodec::None;
    int m_videoStreamId = -1;
    int m_audioStreamId = -1;
    INT m_width = 0;
    INT m_height = 0;
    uint32_t m_avihMicroSecPerFrame = 0;
    uint32_t m_strhScale = 0;
    uint32_t m_strhRate = 0;
    INT m_fpsRate = 30;
    INT m_fpsScale = 1;
    double m_fps = 30.0;
    INT64 m_totalFrames = 0;
    INT m_audioSampleRate = 0;
    INT m_audioChannels = 0;
    uint16_t m_audioBitsPerSample = 16;

    std::vector<AviChunk> m_videoChunks;
    std::vector<AviChunk> m_audioChunks;
    size_t m_nextVideoChunk = 0;
    size_t m_nextAudioChunk = 0;
    size_t m_audioChunkOffset = 0;
    bool m_mpegVideoEndSent = false;
    bool m_mpegAudioEndSent = false;

    std::vector<uint8_t> m_packetBuf;
    std::vector<uint8_t> m_audioChunkBuf;

    // Decoders
    vpx_codec_ctx_t m_vpxCtx{};
    bool m_vpxInitialized = false;
    plm_buffer_t* m_mpegVideoBuf = nullptr;
    plm_video_t* m_mpegVideo = nullptr;
    plm_buffer_t* m_mpegAudioBuf = nullptr;
    plm_audio_t* m_mpegAudio = nullptr;

    // Cached YUV420p frame
    std::vector<uint8_t> m_cachedY;
    std::vector<uint8_t> m_cachedU;
    std::vector<uint8_t> m_cachedV;
    int m_cachedWidth = 0;
    int m_cachedHeight = 0;
    bool m_hasCachedFrame = false;

    // waveOut
    HWAVEOUT m_hWaveOut = nullptr;
    std::vector<WaveOutSlot> m_waveSlots;
    INT m_volume = 255;

    // Playback state & clock
    BVL_STATUS m_status = BVL_STATUS_CLOSED;
    double m_qpcFreq = 10000000.0;
    double m_baseTimeSec = 0.0;
    LONGLONG m_playStartQpc = 0;
    INT64 m_decodedFrames = 0;
    INT64 m_lastShownFrame = -1;
    bool m_videoEof = false;
    bool m_audioEof = false;
    bool m_firstFramePending = false;
};

} // namespace

HRESULT CreateNativeBandiVideo2(UINT sdk_version, void** pp,
                                IBandiVideoFileReader* reader,
                                IBandiVideoSound* sound) {
    (void)sdk_version;
    if (!pp) return BVERR_FAIL;
    CNativeBandiVideo2* player = new (std::nothrow) CNativeBandiVideo2(reader, sound);
    if (!player) return BVERR_OUT_OF_MEMORY;
    *pp = static_cast<IBandiVideo2*>(player);
    return BV_OK;
}
