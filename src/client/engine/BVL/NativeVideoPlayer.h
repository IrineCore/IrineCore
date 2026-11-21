// =============================================================================
// IrineCore Client Engine — Native x64/Win32 IBandiVideo2 Playback Backend
// Self-contained AVI demuxer + VP8 (libvpx) + MPEG-1/MP2 (pl_mpeg) + waveOut
// Eliminates the legacy 32-bit-only bdvid32.dll / missing bdvid64.dll dependency.
// =============================================================================
#pragma once

#include "bandivid.h"

HRESULT CreateNativeBandiVideo2(UINT sdk_version, void** pp,
                                IBandiVideoFileReader* reader,
                                IBandiVideoSound* sound);
