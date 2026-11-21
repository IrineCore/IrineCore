# =============================================================================
# IrineCore — deps/video/CMakeLists.txt
# Bundled pure-C video/audio decoders for native x64/Win32 client playback:
#   - libvpx v1.14.1 (BSD-3-Clause) — VP8 decoder (TitleLoading/OpeningTitle/Assassin)
#   - pl_mpeg        (MIT)          — MPEG-1 video & MP2 audio decoder (Publisher/Eyedentity/skills)
# =============================================================================

set(DEP_VIDEO_VPX_SOURCES
    libvpx/vp8/common/alloccommon.c
    libvpx/vp8/common/blockd.c
    libvpx/vp8/common/dequantize.c
    libvpx/vp8/common/entropy.c
    libvpx/vp8/common/entropymode.c
    libvpx/vp8/common/entropymv.c
    libvpx/vp8/common/extend.c
    libvpx/vp8/common/filter.c
    libvpx/vp8/common/findnearmv.c
    libvpx/vp8/common/generic/systemdependent.c
    libvpx/vp8/common/idct_blk.c
    libvpx/vp8/common/idctllm.c
    libvpx/vp8/common/loopfilter_filters.c
    libvpx/vp8/common/mbpitch.c
    libvpx/vp8/common/modecont.c
    libvpx/vp8/common/quant_common.c
    libvpx/vp8/common/reconinter.c
    libvpx/vp8/common/reconintra.c
    libvpx/vp8/common/reconintra4x4.c
    libvpx/vp8/common/rtcd.c
    libvpx/vp8/common/setupintrarecon.c
    libvpx/vp8/common/swapyv12buffer.c
    libvpx/vp8/common/treecoder.c
    libvpx/vp8/common/vp8_loopfilter.c
    libvpx/vp8/decoder/dboolhuff.c
    libvpx/vp8/decoder/decodeframe.c
    libvpx/vp8/decoder/decodemv.c
    libvpx/vp8/decoder/detokenize.c
    libvpx/vp8/decoder/onyxd_if.c
    libvpx/vp8/vp8_dx_iface.c
    libvpx/vpx/src/vpx_codec.c
    libvpx/vpx/src/vpx_decoder.c
    libvpx/vpx/src/vpx_image.c
    libvpx/vpx_config.c
    libvpx/vpx_dsp/intrapred.c
    libvpx/vpx_dsp/vpx_dsp_rtcd.c
    libvpx/vpx_mem/vpx_mem.c
    libvpx/vpx_scale/generic/yv12config.c
    libvpx/vpx_scale/generic/yv12extend.c
    libvpx/vpx_scale/vpx_scale_rtcd.c
)

add_library(dep_video STATIC
    ${DEP_VIDEO_VPX_SOURCES}
    pl_mpeg/pl_mpeg_impl.c
)

target_include_directories(dep_video PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/libvpx"
    "${CMAKE_CURRENT_SOURCE_DIR}/pl_mpeg"
)

if(MSVC)
    target_compile_definitions(dep_video PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_options(dep_video PRIVATE
        /wd4018 /wd4100 /wd4127 /wd4146 /wd4244 /wd4245 /wd4267 /wd4305 /wd4310 /wd4324 /wd4389 /wd4701 /wd4706 /wd4996
    )
endif()
