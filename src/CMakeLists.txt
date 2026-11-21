# =============================================================================
# IrineCore — src/CMakeLists.txt
# Top-level source coordinator for cross-platform servers and Windows-only client/launcher.
# =============================================================================
option(IRINE_BUILD_SERVERS "Build IrineCore Dragon Nest server executables (x64, Windows & Linux)" ON)
option(IRINE_BUILD_CLIENT  "Build IrineCore DragonNest client & DnLauncher (Win32, Windows only)" OFF)

if(IRINE_BUILD_SERVERS)
    add_subdirectory(server)
endif()

if(IRINE_BUILD_CLIENT)
    if(NOT WIN32)
        message(FATAL_ERROR "[IrineCore] DragonNest Client and DnLauncher are strictly Windows (Win32) only.")
    endif()
    add_subdirectory(client)
endif()
