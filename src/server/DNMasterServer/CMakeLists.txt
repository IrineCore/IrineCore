# =============================================================================
# IrineCore Server Application â€” src/DNMasterServer/CMakeLists.txt
# =============================================================================
add_executable(DNMasterServer
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNIocpManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/configuration/IniFile.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/shared/Util.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNMasterServer.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/stdafx.cpp"
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
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/HttpClientManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNDataManagerBase.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNExtManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/ExceptionReport.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/FileSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/memory/MemoryMap.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/MiniDumpSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/StringSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/TimeSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/Protocol/DNCountryUnicodeSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/Directory.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/HttpClient.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/PsUpdater.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNDivisionManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNDoorsConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNGameConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNLogConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNLoginConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNVillageConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DnFarm.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNPvP.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNUser.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNWaitUserManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNGuildWar.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNGuildWarManager.cpp"
    $<$<BOOL:${WIN32}>:${CMAKE_SOURCE_DIR}/src/server/DNMasterServer/DNMasterServer.rc>
)
set_target_properties(DNMasterServer PROPERTIES OUTPUT_NAME "DNMasterServer64")
target_include_directories(DNMasterServer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IRINE_COMMON_INCLUDES}
)
target_compile_definitions(DNMasterServer PRIVATE WIN64 NDEBUG _CONSOLE _MASTERSERVER _FINAL_BUILD UNICODE _UNICODE)
target_link_libraries(DNMasterServer PRIVATE
    irine_modules
    irine_platform
    irine_etstringmanager
    irine_etresmanager
    irine_etfilesystem
    irine_utility
    dep_tinyxml
    dep_zlib
    $<$<BOOL:${WIN32}>:ws2_32 mswsock odbc32 odbccp32 winmm dbghelp psapi version wininet mpr ole32 oleaut32 shell32 advapi32 gdi32 user32>
    $<$<BOOL:${UNIX}>:odbc pthread dl>
)
irine_install_server_target(DNMasterServer)
