# =============================================================================
# IrineCore Client Engine (Windows Win32 Only) — Bandicam Video Library (BVL)
# =============================================================================
add_library(client_bvl STATIC
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/BandiVideoLibrary.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/BandiVideoTexture_DX9.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/EtBVLEngine.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BVL/stdafx.cpp"
)
target_include_directories(client_bvl PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IRINE_CLIENT_COMMON_INCLUDES}
)
target_compile_definitions(client_bvl PRIVATE WIN32 NDEBUG _LIB _CLIENT UNICODE _UNICODE)