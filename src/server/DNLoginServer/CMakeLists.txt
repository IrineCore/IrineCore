# =============================================================================
# IrineCore Server Application â€” src/DNLoginServer/CMakeLists.txt
# =============================================================================
add_executable(DNLoginServer
    "${CMAKE_SOURCE_DIR}/src/server/shared/CloseSystem.cpp"
    "${CMAKE_SOURCE_DIR}/common/configuration/IniFile.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/SlangFilter.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/shared/Util.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/DNIocpManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNLogConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/DNMasterConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/DNMasterConnectionManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/DNUserConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/DNUserConnectionManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNSQLMembership.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/DNSQLMembershipManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNSQLWorld.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/DNSQLWorldManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/database/SQLConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/DNLoginServer.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/stdafx.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNDataManagerBase.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNExtManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/AcceptorEx.cpp"
    "${CMAKE_SOURCE_DIR}/common/memory/Buffer.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/Connection.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/ConnectionManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/CryptPacket.cpp"
    "${CMAKE_SOURCE_DIR}/common/events/EventAcceptor.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/IocpManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/ListenSocket.cpp"
    "${CMAKE_SOURCE_DIR}/common/logging/Log.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/MiniDump.cpp"
    "${CMAKE_SOURCE_DIR}/common/threading/SendThread.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/SocketContext.cpp"
    "${CMAKE_SOURCE_DIR}/common/networking/SocketContextMgr.cpp"
    "${CMAKE_SOURCE_DIR}/common/synchronization/SpinBuffer.cpp"
    "${CMAKE_SOURCE_DIR}/common/threading/Thread.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/HttpClient.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/HttpClientManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/Protocol/DNCountryUnicodeSet.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/AuthManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/DNAuthManager.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/ExceptionReport.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/FileSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/memory/MemoryMap.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/MiniDumpSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/StringSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/TimeSet.cpp"
)
if(WIN32)
    target_sources(DNLoginServer PRIVATE "${CMAKE_SOURCE_DIR}/src/server/DNLoginServer/DNLoginServer.rc")
endif()
set_target_properties(DNLoginServer PROPERTIES OUTPUT_NAME "DNLoginServer64")
target_include_directories(DNLoginServer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IRINE_COMMON_INCLUDES}
)
target_compile_definitions(DNLoginServer PRIVATE $<$<BOOL:${WIN32}>:WIN64> NDEBUG _CONSOLE _LOGINSERVER _FINAL_BUILD UNICODE _UNICODE)
target_link_libraries(DNLoginServer PRIVATE
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
    target_link_libraries(DNLoginServer PRIVATE
        ws2_32 mswsock odbc32 odbccp32 winmm dbghelp psapi version wininet mpr ole32 oleaut32 shell32 advapi32 gdi32 user32
    )
elseif(UNIX)
    target_link_libraries(DNLoginServer PRIVATE odbc pthread dl)
endif()
irine_install_server_target(DNLoginServer)
