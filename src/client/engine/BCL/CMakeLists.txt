# =============================================================================
# IrineCore Client Engine (Windows Win32 Only) — Bandicam Capture Library (BCL)
# =============================================================================
add_library(client_bcl STATIC
    "${CMAKE_SOURCE_DIR}/src/client/engine/BCL/EtBCLEngine.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BCL/stdafx.cpp"
)
target_include_directories(client_bcl PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IRINE_CLIENT_COMMON_INCLUDES}
)
target_compile_definitions(client_bcl PRIVATE WIN32 NDEBUG _LIB _CLIENT UNICODE _UNICODE)