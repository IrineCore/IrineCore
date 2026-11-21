# =============================================================================
# IrineCore Client Engine (Windows x64/Win32) — BandiVideoLibrary 2.1.5.19 (BVL)
# =============================================================================
add_library(client_bvl STATIC
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/BandiVideoFileReader.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/BandiVideoLibrary.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/BandiVideoSound_DS.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/BandiVideoSound_NULL.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/BandiVideoTexture_DX9.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/EtBVLEngine.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/NativeVideoPlayer.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/stdafx.cpp"
)
target_include_directories(client_bvl PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IRINE_CLIENT_COMMON_INCLUDES}
)
target_link_libraries(client_bvl PRIVATE dep_video dsound winmm)
target_compile_definitions(client_bvl PRIVATE WIN32 NDEBUG _LIB _CLIENT UNICODE _UNICODE)