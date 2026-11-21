# =============================================================================
# IrineCore Server Application â€” src/DNLogServer/CMakeLists.txt
# =============================================================================
add_executable(DNLogServer
    "${CMAKE_SOURCE_DIR}/common/configuration/IniFile.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/shared/Util.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLogServer/DNConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLogServer/DNConnectionManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLogServer/DNIocpManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLogServer/DNSQLConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLogServer/DNSQLConnectionManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLogServer/DNThreadManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/database/SQLConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLogServer/DNLogServer.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLogServer/stdafx.cpp"
    "${CMAKE_SOURCE_DIR}/common/memory/Buffer.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/Connection.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/ConnectionManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/events/EventAcceptor.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/IocpManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/ListenSocket.cpp"
    "${CMAKE_SOURCE_DIR}/common/logging/Log.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/MiniDump.cpp"
    "${CMAKE_SOURCE_DIR}/common/threading/SendThread.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/SocketContext.cpp"
    "${CMAKE_SOURCE_DIR}/common/synchronization/SpinBuffer.cpp"
    "${CMAKE_SOURCE_DIR}/common/threading/Thread.cpp"
    "${CMAKE_SOURCE_DIR}/common/threading/ThreadManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/Protocol/DNCountryUnicodeSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/ExceptionReport.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/FileSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/memory/MemoryMap.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/MiniDumpSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/StringSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/TimeSet.cpp"
    $<$<BOOL:${WIN32}>:${CMAKE_SOURCE_DIR}/src/server/DNLogServer/DNLogServer.rc>
)
set_target_properties(DNLogServer PROPERTIES OUTPUT_NAME "DNLogServer64")
target_include_directories(DNLogServer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IRINE_COMMON_INCLUDES}
)
target_compile_definitions(DNLogServer PRIVATE WIN64 NDEBUG _CONSOLE _LOGSERVER UNICODE _UNICODE)
target_link_libraries(DNLogServer PRIVATE
    irine_modules
    irine_platform
    irine_utility
    $<$<BOOL:${WIN32}>:ws2_32 mswsock odbc32 odbccp32 winmm dbghelp psapi version wininet mpr ole32 oleaut32 shell32 advapi32 gdi32 user32>
    $<$<BOOL:${UNIX}>:odbc pthread dl>
)
irine_install_server_target(DNLogServer)
