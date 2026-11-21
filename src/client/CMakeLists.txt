# =============================================================================
# IrineCore — src/client/CMakeLists.txt
# Strictly Windows Only (x64 or Win32) — Builds DragonNest Client & DnLauncher.
# =============================================================================
if(NOT WIN32)
    message(FATAL_ERROR "[IrineCore] src/client (DragonNest & DnLauncher) is strictly Windows-only.")
endif()

# Client-only Windows engine libraries (DirectX 9, FreeType UI, static FMOD, Cutscene, BugReporter, BVL/BCL)
add_subdirectory(engine/BugReporter)
add_subdirectory(engine/BVL)
add_subdirectory(engine/BCL)
add_subdirectory(engine/EternityEngine)
add_subdirectory(engine/EtInterface)
add_subdirectory(engine/FMODEngine)
add_subdirectory(engine/RTCutSceneCore)

# Client Applications
add_subdirectory(DnLauncher)
add_subdirectory(DragonNest)

# Apply release & hardening optimizations to client libraries & executables
foreach(_client_tgt
    client_bugreporter
    client_bvl
    client_bcl
    client_d3ddevice9
    client_eternityengine
    client_etinterface
    client_fmodengine
    client_rtcutscenecore
    ResourceUSA
    DnLauncher
    DragonNest
)
    if(TARGET "${_client_tgt}")
        irine_apply_client_optimizations("${_client_tgt}")
    endif()
endforeach()

# Install optional arch-matched runtime DLLs (FMOD is statically linked into DragonNest.exe)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    foreach(_opt_dll
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/BVL/bdvid64.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/BCL/bdcap64.dll"
    )
        if(EXISTS "${_opt_dll}")
            install(FILES "${_opt_dll}" DESTINATION "${CMAKE_INSTALL_PREFIX}")
        endif()
    endforeach()
else()
    foreach(_opt_dll
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/BVL/bdvid32.dll"
        "${CMAKE_CURRENT_SOURCE_DIR}/engine/BCL/bdcap32.dll"
    )
        if(EXISTS "${_opt_dll}")
            install(FILES "${_opt_dll}" DESTINATION "${CMAKE_INSTALL_PREFIX}")
        endif()
    endforeach()
endif()
