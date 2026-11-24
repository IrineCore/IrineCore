# =============================================================================
# IrineCore — src/cmake/InstallLayout.cmake
# Distribution install layout for env/dist:
#   env/dist/
#     ├── config/           (core server .ini.dist defaults + user .ini overrides & .dat files)
#     ├── module/           (installed module .ini.dist defaults + user .ini overrides + modules.manifest)
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
    # Install server .ini.dist templates (always refreshed), and seed local *.ini
    # files ONLY if they do not already exist so user customizations are never
    # overwritten on reinstall/upgrade.
    file(GLOB _tpl_inis "${CMAKE_SOURCE_DIR}/apps/template/config/*.ini.dist")
    foreach(_ini_dist IN LISTS _tpl_inis)
        get_filename_component(_dist_name "${_ini_dist}" NAME)
        string(REGEX REPLACE "\\.dist$" "" _ini_name "${_dist_name}")
        install(FILES "${_ini_dist}"
            DESTINATION "${CMAKE_INSTALL_PREFIX}/config"
        )
        install(CODE "
            if(NOT EXISTS \"\${CMAKE_INSTALL_PREFIX}/config/${_ini_name}\")
                message(STATUS \"Seeding: \${CMAKE_INSTALL_PREFIX}/config/${_ini_name}\")
                configure_file(\"${_ini_dist}\" \"\${CMAKE_INSTALL_PREFIX}/config/${_ini_name}\" COPYONLY)
            endif()
        ")
    endforeach()

    # Install non-INI state/data files (*.dat, etc.) if missing
    file(GLOB _tpl_other "${CMAKE_SOURCE_DIR}/apps/template/config/*")
    foreach(_other IN LISTS _tpl_other)
        if(_other MATCHES "\\.ini(\\.dist)?$")
            continue()
        endif()
        get_filename_component(_other_name "${_other}" NAME)
        install(CODE "
            if(NOT EXISTS \"\${CMAKE_INSTALL_PREFIX}/config/${_other_name}\")
                file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}/config\" TYPE FILE FILES \"${_other}\")
            endif()
        ")
    endforeach()

    install(DIRECTORY "${CMAKE_SOURCE_DIR}/apps/template/System/"
        DESTINATION "${CMAKE_INSTALL_PREFIX}/systems"
    )
    install(CODE "file(MAKE_DIRECTORY \"\${CMAKE_INSTALL_PREFIX}/module\")")

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
