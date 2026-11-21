# =============================================================================
# IrineCore — OpenSource Dragon Nest Server Emulator
# Root CMake Build Configuration (Windows & Linux)
#   - Cross-Platform Servers (x64, Windows & Linux): -DIRINE_BUILD_SERVERS=ON
#   - Client & Launcher (x64 or Win32, Windows only): -A x64 -DIRINE_BUILD_SERVERS=OFF -DIRINE_BUILD_CLIENT=ON
# =============================================================================
cmake_minimum_required(VERSION 3.20)

cmake_policy(SET CMP0091 NEW)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

option(IRINE_BUILD_SERVERS "Build IrineCore Dragon Nest server executables (x64, Windows & Linux)" ON)
option(IRINE_BUILD_TOOLS   "Build IrineCore packaging, patching, and client creator tools (x64, Windows & Linux)" ON)
option(IRINE_BUILD_CLIENT  "Build IrineCore DragonNest client & DnLauncher (x64 / Win32, Windows only)" OFF)
option(IRINE_CLIENT_LTO    "Enable Whole-Program Optimization (/GL + /LTCG) for Release Client & Launcher" OFF)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if(IRINE_BUILD_CLIENT AND NOT IRINE_BUILD_SERVERS)
        set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/env/client" CACHE PATH "IrineCore client distribution output directory" FORCE)
    else()
        set(CMAKE_INSTALL_PREFIX "${CMAKE_SOURCE_DIR}/env/dist" CACHE PATH "IrineCore server distribution output directory" FORCE)
    endif()
endif()

project(IrineCore VERSION 1.0.0 LANGUAGES C CXX)

if(IRINE_BUILD_CLIENT)
    if(NOT WIN32)
        message(FATAL_ERROR "[IrineCore] DragonNest Client and DnLauncher are strictly Windows only.")
    endif()
    set(IRINE_DXSDK_DIR "$ENV{DXSDK_DIR}" CACHE PATH "DirectX 9 SDK (June 2010) installation directory")
    if(NOT IRINE_DXSDK_DIR OR NOT EXISTS "${IRINE_DXSDK_DIR}/Include/d3dx9.h")
        if(EXISTS "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)/Include/d3dx9.h")
            set(IRINE_DXSDK_DIR "C:/Program Files (x86)/Microsoft DirectX SDK (June 2010)" CACHE PATH "DirectX 9 SDK (June 2010) installation directory" FORCE)
        else()
            message(FATAL_ERROR "[IrineCore] DirectX 9 SDK (June 2010) not found. Install DXSDK or pass -DIRINE_DXSDK_DIR=<path>.")
        endif()
    endif()
    set(IRINE_DXSDK_INCLUDE_DIR "${IRINE_DXSDK_DIR}/Include")
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(IRINE_DXSDK_LIB_DIR "${IRINE_DXSDK_DIR}/Lib/x64")
        set(IRINE_CLIENT_ARCH   "x64")
    else()
        set(IRINE_DXSDK_LIB_DIR "${IRINE_DXSDK_DIR}/Lib/x86")
        set(IRINE_CLIENT_ARCH   "Win32")
    endif()
    # Append DX9 SDK AFTER Windows 10 SDK so Windows 10 dxgitype.h/wincodec.h take precedence over DXSDK 2010
    set(CMAKE_VS_SDK_INCLUDE_DIRECTORIES "$(IncludePath);${IRINE_DXSDK_INCLUDE_DIR}")
    set(CMAKE_VS_SDK_LIBRARY_DIRECTORIES "$(LibraryPath);${IRINE_DXSDK_LIB_DIR}")
    message(STATUS "[IrineCore] Client target architecture: ${IRINE_CLIENT_ARCH} | DirectX 9 SDK: ${IRINE_DXSDK_DIR}")
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/src/cmake")
include(CompilerFlags)
include(IncludePaths)
include(IrineModules)
include(InstallLayout)

add_subdirectory(deps)
add_subdirectory(common)

if(IRINE_BUILD_SERVERS)
    add_subdirectory(modules)
endif()

add_subdirectory(src)

if(IRINE_BUILD_TOOLS AND NOT (IRINE_BUILD_CLIENT AND NOT IRINE_BUILD_SERVERS))
    add_subdirectory(tools)
endif()

if(IRINE_BUILD_SERVERS)
    enable_testing()
    add_subdirectory(tests)
    # Install server configs, systems definitions, and runtime dynamic libraries into env/dist
    irine_install_runtime_layout()
endif()
