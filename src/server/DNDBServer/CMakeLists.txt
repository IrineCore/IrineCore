# =============================================================================
# IrineCore Server Application â€” src/DNDBServer/CMakeLists.txt
# =============================================================================
add_executable(DNDBServer
    "${CMAKE_SOURCE_DIR}/common/configuration/IniFile.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/shared/Util.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNIocpManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNLogConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNThreadManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNSQLConnectionManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNSQLMembership.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNSQLWorld.cpp"
    "${CMAKE_SOURCE_DIR}/common/database/SQLConnection.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNAdjustTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNAlteiaWorldTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNAppellationTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNAuthTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNBestFriendTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNCashShopTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNDarkLairTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNDonationTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNEtcTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNFarmTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNFriendTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNGuildRecruitSystemTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNGuildTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNIsolateTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNItemTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNJobSystemTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNLogTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNMailTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNMarketTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNMasterSystemTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNMissionTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNPartyTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNPrivateChatChannelTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNPvPTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNQuestTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNReputationTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNSecondarySkillTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNSkillTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNStampSystemTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNDWCTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNStatusTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNMessageTask.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNDBServer.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/ServerCommon/DNExtManager.cpp"
    "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/stdafx.cpp"
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
)
if(WIN32)
    target_sources(DNDBServer PRIVATE "${CMAKE_SOURCE_DIR}/src/server/DNDBServer/DNDBServer.rc")
endif()
set_target_properties(DNDBServer PROPERTIES OUTPUT_NAME "DNDBServer64")
target_include_directories(DNDBServer PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}"
    ${IRINE_COMMON_INCLUDES}
)
target_compile_definitions(DNDBServer PRIVATE $<$<BOOL:${WIN32}>:WIN64> NDEBUG _CONSOLE _DBSERVER _FINAL_BUILD UNICODE _UNICODE)
target_link_libraries(DNDBServer PRIVATE
    irine_modules
    irine_platform
    irine_etresmanager
    irine_etfilesystem
    irine_utility
    dep_zlib
)
if(WIN32)
    target_link_libraries(DNDBServer PRIVATE
        ws2_32 mswsock odbc32 odbccp32 winmm dbghelp psapi version wininet mpr ole32 oleaut32 shell32 advapi32 gdi32 user32
    )
elseif(UNIX)
    target_link_libraries(DNDBServer PRIVATE odbc pthread dl)
endif()
irine_install_server_target(DNDBServer)
