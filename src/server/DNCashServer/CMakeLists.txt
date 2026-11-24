# =============================================================================
# IrineCore Server Application â€” src/DNCashServer/CMakeLists.txt
# =============================================================================
add_executable(DNCashServer
    "${CMAKE_SOURCE_DIR}/common/configuration/IniFile.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/shared/Util.cpp"
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
    "${CMAKE_SOURCE_DIR}/common/utilities/HttpClient.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/HttpClientManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/DNCashServer.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/stdafx.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/DNConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/DNIocpManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNLogConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/DNManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/DNThreadManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/DNUserDeleteManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/DNUserRepository.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/DNSQLManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNSQLMembership.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNSQLWorld.cpp"
    "${CMAKE_SOURCE_DIR}/common/database/SQLConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNExtManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/DNBillingConnection.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/ExceptionReport.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/FileSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/memory/MemoryMap.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/MiniDumpSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/StringSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/TimeSet.cpp"
    "${CMAKE_SOURCE_DIR}/game/inventory/DNLimitedCashItemRepository.cpp"
)
if(WIN32)
    target_sources(DNCashServer PRIVATE "${CMAKE_SOURCE_DIR}/src/server/DNCashServer/DNCashServer.rc")
endif()
set_target_properties(DNCashServer PROPERTIES OUTPUT_NAME "DNCashServer64")
target_include_directories(DNCashServer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IRINE_COMMON_INCLUDES}
)
target_compile_definitions(DNCashServer PRIVATE $<$<BOOL:${WIN32}>:WIN64> NDEBUG _CONSOLE _CASHSERVER _FINAL_BUILD UNICODE _UNICODE)
target_link_libraries(DNCashServer PRIVATE
    irine_modules
    irine_platform
    irine_etstringmanager
    irine_etresmanager
    irine_etfilesystem
    irine_utility
    dep_tinyxml
    dep_zlib
)
if(WIN32)
    target_link_libraries(DNCashServer PRIVATE
        ws2_32 mswsock odbc32 odbccp32 winmm dbghelp psapi version wininet mpr ole32 oleaut32 shell32 advapi32 gdi32 user32
    )
elseif(UNIX)
    target_link_libraries(DNCashServer PRIVATE odbc pthread dl)
endif()
irine_install_server_target(DNCashServer)
