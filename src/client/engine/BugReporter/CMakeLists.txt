# =============================================================================
# IrineCore Client Engine (Windows Win32 Only) — BugReporter
# =============================================================================
add_library(client_bugreporter STATIC
    "${CMAKE_SOURCE_DIR}/src/client/engine/BugReporter/BugReporter.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BugReporter/ExceptionHandler.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BugReporter/StackWalker.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BugReporter/SystemInfo.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BugReporter/RyeolErrMgmtFunc.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BugReporter/RyeolException.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BugReporter/RyeolHttpClient.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BugReporter/RyeolStringFunc.cpp"
    "${CMAKE_SOURCE_DIR}/src/client/engine/BugReporter/stdafx.cpp"
)
target_include_directories(client_bugreporter PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IRINE_CLIENT_COMMON_INCLUDES}
)
target_compile_definitions(client_bugreporter PRIVATE WIN32 NDEBUG _LIB _CLIENT UNICODE _UNICODE)