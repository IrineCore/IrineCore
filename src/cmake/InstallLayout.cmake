# =============================================================================
# IrineCore — src/cmake/InstallLayout.cmake
# Distribution install layout for env/dist:
#   env/dist/
#     ├── config/           (core server .ini and .dat files)
#     ├── module/           (installed module .ini files)
#     ├── systems/          (ChannelInfo.xml and system definitions)
#     ├── <server-binaries> (DN*Server64 executables)
#     └── <dlls>            (any runtime dynamic libraries, if present)
# =============================================================================
function(irine_install_server_target TARGET_NAME)
    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}"
    )
endfunction()

function(irine_install_runtime_layout)
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/apps/template/config/"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/config"
    )
    install(DIRECTORY "${CMAKE_SOURCE_DIR}/apps/template/System/"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/systems"
    )
    install(CODE "file(MAKE_DIRECTORY \"${CMAKE_INSTALL_PREFIX}/module\")")

    # Install any runtime dynamic libraries (*.dll / *.so*) if present
    if(WIN32)
        file(GLOB _runtime_dlls
            "${CMAKE_SOURCE_DIR}/deps/runtime/*.dll"
            "${CMAKE_SOURCE_DIR}/common/VoiceChat/DolbyAxon/IceLib64/*.dll"
        )
        if(_runtime_dlls)
            install(FILES ${_runtime_dlls} DESTINATION "${CMAKE_INSTALL_PREFIX}")
        endif()
    elseif(UNIX)
        file(GLOB _runtime_sos
            "${CMAKE_SOURCE_DIR}/deps/runtime/*.so*"
        )
        if(_runtime_sos)
            install(FILES ${_runtime_sos} DESTINATION "${CMAKE_INSTALL_PREFIX}")
        endif()
    endif()
endfunction()
