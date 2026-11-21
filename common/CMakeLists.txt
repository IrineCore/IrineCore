# =============================================================================
# IrineCore — common/CMakeLists.txt
# Shared Core & Cross-Platform Abstraction Static Libraries
# Supports both x64 Servers (Windows/Linux) and Win32 Client/Launcher (Windows only).
# =============================================================================
if(IRINE_BUILD_CLIENT AND NOT IRINE_BUILD_SERVERS)
    set(_COMMON_ACTIVE_INCLUDES ${IRINE_CLIENT_COMMON_INCLUDES})
    set(_COMMON_ARCH_DEFS WIN32)
else()
    set(_COMMON_ACTIVE_INCLUDES ${IRINE_COMMON_INCLUDES})
    set(_COMMON_ARCH_DEFS $<$<EQUAL:${CMAKE_SIZEOF_VOID_P},8>:WIN64>)
endif()

if(WIN32)
    add_library(irine_platform STATIC
        "${CMAKE_SOURCE_DIR}/common/core/IrineApp.cpp"
        "${CMAKE_SOURCE_DIR}/common/platform/windows/WinPlatform.cpp"
    )
elseif(UNIX)
    add_library(irine_platform STATIC
        "${CMAKE_SOURCE_DIR}/common/core/IrineApp.cpp"
        "${CMAKE_SOURCE_DIR}/common/platform/linux/LinuxPlatform.cpp"
        "${CMAKE_SOURCE_DIR}/common/platform/linux/LinuxIniAndStringCompat.cpp"
    )
endif()
target_include_directories(irine_platform PUBLIC
    "${CMAKE_SOURCE_DIR}/common/core"
    "${CMAKE_SOURCE_DIR}/common/platform"
    "${CMAKE_SOURCE_DIR}/common/platform/interfaces"
)
target_compile_definitions(irine_platform PRIVATE UNICODE _UNICODE)

add_library(irine_utility STATIC
    "${CMAKE_SOURCE_DIR}/common/utilities/LiteHTMLAttributes.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/LiteHTMLEntityResolver.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/LiteHTMLReader.cpp"
    "${CMAKE_SOURCE_DIR}/common/synchronization/MultiSync.cpp"
    "${CMAKE_SOURCE_DIR}/common/memory/MemPool.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/ExceptionReport.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/MiniDumpSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/MtRandom.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/PrandGen.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/Access.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/AssertX.cpp"
    "${CMAKE_SOURCE_DIR}/common/threading/BackgroundLoader.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/CSVWriter.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/DebugSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/Directory.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/DNTableFile.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/ExceptionDump.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/FileSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/FireWall.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/HttpClient.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/md5.cpp"
    "${CMAKE_SOURCE_DIR}/common/memory/MemoryMap.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/MultiCommon.cpp"
    "${CMAKE_SOURCE_DIR}/common/memory/ObjectPool.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/PerfCheck.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/SHA256.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/Stdafx.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/Stream.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/StringSet.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/StringUtil.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/SundriesFunc.cpp"
    "${CMAKE_SOURCE_DIR}/common/utilities/TimeSet.cpp"
)
target_include_directories(irine_utility PRIVATE
    "${CMAKE_SOURCE_DIR}/common/utilities"
    ${_COMMON_ACTIVE_INCLUDES}
)
target_compile_definitions(irine_utility PRIVATE ${_COMMON_ARCH_DEFS} NDEBUG _LIB)

add_library(irine_etfilesystem STATIC
    "${CMAKE_SOURCE_DIR}/common/EtFileSystem/stdafx.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtFileSystem/EtFileIO.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtFileSystem/EtFileSystem.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtFileSystem/EtPackFileIO.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtFileSystem/EtPackingStream.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtFileSystem/EtStdFileIO.cpp"
)
target_include_directories(irine_etfilesystem PRIVATE
    "${CMAKE_SOURCE_DIR}/common/EtFileSystem"
    ${_COMMON_ACTIVE_INCLUDES}
)
target_compile_definitions(irine_etfilesystem PRIVATE ${_COMMON_ARCH_DEFS} NDEBUG _LIB _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES _CRT_SECURE_NO_WARNINGS)

add_library(irine_etresmanager STATIC
    "${CMAKE_SOURCE_DIR}/common/EtResManager/EtResourceMng.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtResManager/stdafx.cpp"
)
target_include_directories(irine_etresmanager PRIVATE
    "${CMAKE_SOURCE_DIR}/common/EtResManager"
    ${_COMMON_ACTIVE_INCLUDES}
)
target_compile_definitions(irine_etresmanager PRIVATE ${_COMMON_ARCH_DEFS} NDEBUG _LIB UNICODE _UNICODE)

add_library(irine_etstringmanager STATIC
    "${CMAKE_SOURCE_DIR}/common/EtStringManager/EtUIXML.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtStringManager/stdafx.cpp"
)
target_include_directories(irine_etstringmanager PRIVATE
    "${CMAKE_SOURCE_DIR}/common/EtStringManager"
    ${_COMMON_ACTIVE_INCLUDES}
)
target_compile_definitions(irine_etstringmanager PRIVATE ${_COMMON_ARCH_DEFS} NDEBUG _LIB TIXML_USE_STL)

set(_ENGINEUTIL_SOURCES
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/EtCollisionFunc.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/EtCollisionPrimitive.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/EtComputeDist.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/EtConvexVolume.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/EtFindCollision.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/EtPrimitive.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/EtTestCollision.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/EngineUtil.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/EtMatrixEx.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/FrameSync.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/stdafx.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/TimeImp.cpp"
    "${CMAKE_SOURCE_DIR}/common/EngineUtil/VelocityFunc.cpp"
)
if(WIN32 AND IRINE_BUILD_CLIENT)
    list(APPEND _ENGINEUTIL_SOURCES
        "${CMAKE_SOURCE_DIR}/src/client/engine/Input/InputDevice.cpp"
        "${CMAKE_SOURCE_DIR}/src/client/engine/Input/InputReceiver.cpp"
        "${CMAKE_SOURCE_DIR}/src/client/engine/Input/CountryByCountryKeyboard.cpp"
    )
endif()

add_library(irine_engineutil STATIC ${_ENGINEUTIL_SOURCES})
target_include_directories(irine_engineutil PRIVATE
    "${CMAKE_SOURCE_DIR}/common/EngineUtil"
    ${_COMMON_ACTIVE_INCLUDES}
)
target_compile_definitions(irine_engineutil PRIVATE
    ${_COMMON_ARCH_DEFS}
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${IRINE_BUILD_CLIENT}>>:_CLIENT>
    NDEBUG _LIB TIXML_USE_STL
)

add_library(irine_etworldbase STATIC
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/stdafx.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtRandomDungeon.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtMaze.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtMazeMask.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtRandomDungeonRoom.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorld.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorldGrid.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorldSector.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorldProp.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorldSound.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorldSoundEnvi.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorldWater.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorldWaterRiver.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorldDecal.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorldEventArea.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtWorldEventControl.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtNavigationMeshGenerator.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/mtxlib.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/navigationcell.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/navigationmesh.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtTrigger.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtTriggerElement.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase/EtTriggerObject.cpp"
)
target_include_directories(irine_etworldbase PRIVATE
    "${CMAKE_SOURCE_DIR}/common/EtWorldBase"
    ${_COMMON_ACTIVE_INCLUDES}
)
target_compile_definitions(irine_etworldbase PRIVATE
    ${_COMMON_ARCH_DEFS}
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${IRINE_BUILD_CLIENT}>>:_CLIENT>
    NDEBUG _LIB UNICODE _UNICODE
)

add_library(irine_etactioncorebase STATIC
    "${CMAKE_SOURCE_DIR}/common/EtActionCoreBase/EtActionBase.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtActionCoreBase/EtActionCoreMng.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtActionCoreBase/EtActionSignal.cpp"
    "${CMAKE_SOURCE_DIR}/common/EtActionCoreBase/stdafx.cpp"
)
target_include_directories(irine_etactioncorebase PRIVATE
    "${CMAKE_SOURCE_DIR}/common/EtActionCoreBase"
    ${_COMMON_ACTIVE_INCLUDES}
)
target_compile_definitions(irine_etactioncorebase PRIVATE
    ${_COMMON_ARCH_DEFS}
    $<$<AND:$<BOOL:${WIN32}>,$<BOOL:${IRINE_BUILD_CLIENT}>>:_CLIENT>
    NDEBUG _LIB UNICODE _UNICODE
)

if(IRINE_BUILD_SERVERS)
    add_library(irine_eternityenginesrv STATIC
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtEngine.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EternityEngine.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtAniObject.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtCamera.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtCollisionEntity.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtObject.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtError.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/stdafx.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtAni.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtAniKey.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtBone.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtLoader.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtMesh.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtResource.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtSkin.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtSubMesh.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtTerrain.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtTerrainArea.cpp"
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv/EtCollisionMng.cpp"
    )
    target_include_directories(irine_eternityenginesrv PRIVATE
        "${CMAKE_SOURCE_DIR}/common/EternityEngineSrv"
        ${IRINE_COMMON_INCLUDES}
    )
    target_compile_definitions(irine_eternityenginesrv PRIVATE ${_COMMON_ARCH_DEFS} NDEBUG _LIB UNICODE _UNICODE)
endif()
