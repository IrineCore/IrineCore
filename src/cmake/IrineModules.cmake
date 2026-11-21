# =============================================================================
# IrineCore — src/cmake/IrineModules.cmake
# Dynamic module discovery, per-module CMake toggles,
# auto-generated ModuleLoader.cpp, and clean static/disabled isolation.
#
# Supported controls (without deleting module folders from modules/):
#   -DIRINE_MODULES=static|disabled         (global switch; default: static)
#   -DDISABLED_IRINE_MODULES="ModA;ModB"    (semicolon-separated folder names)
#   -DIRINE_MODULE_<UPPER_NAME>=ON|OFF      (auto-generated per-module option)
#   modules/<ModName>/.disabled             (drop a marker file to skip)
#   modules/disabled/<ModName>              (ignored folder)
# =============================================================================

set(IRINE_MODULES "static" CACHE STRING "Build mode for IrineCore modules (static | disabled)")
set_property(CACHE IRINE_MODULES PROPERTY STRINGS static disabled)

set(DISABLED_IRINE_MODULES "" CACHE STRING
    "Semicolon-separated list of module folder names under modules/ to exclude from the build (e.g. Player-Bots;examples)")

# Global properties populated during irine_discover_modules()
set_property(GLOBAL PROPERTY IRINE_ENABLED_MODULES "")
set_property(GLOBAL PROPERTY IRINE_ENABLED_COMMON_MODULE_TARGETS "")
set_property(GLOBAL PROPERTY IRINE_ENABLED_GAMESERVER_MODULE_TARGETS "")
set_property(GLOBAL PROPERTY IRINE_ENABLED_MODULE_DEFINES "")
set_property(GLOBAL PROPERTY IRINE_ENABLED_MODULE_INCLUDES "")
set_property(GLOBAL PROPERTY IRINE_ENABLED_MODULE_LOADERS "")
set_property(GLOBAL PROPERTY IRINE_ENABLED_MODULE_CONFIGS "")

function(irine_sanitize_module_name RAW_NAME OUT_IDENT OUT_UPPER)
    string(REGEX REPLACE "[^A-Za-z0-9_]" "_" _ident "${RAW_NAME}")
    string(TOUPPER "${_ident}" _upper)
    set(${OUT_IDENT} "${_ident}" PARENT_SCOPE)
    set(${OUT_UPPER} "${_upper}" PARENT_SCOPE)
endfunction()

function(irine_install_module_configs MODULE_ROOT_DIR)
    # Auto-discover all config files across both modules/ and conf/ layouts
    set(_all_cfgs "")
    if(EXISTS "${MODULE_ROOT_DIR}/modules")
        file(GLOB _mod_inis
            "${MODULE_ROOT_DIR}/modules/*.ini"
            "${MODULE_ROOT_DIR}/modules/*.conf"
            "${MODULE_ROOT_DIR}/modules/*.conf.dist"
        )
        list(APPEND _all_cfgs ${_mod_inis})
    endif()
    if(EXISTS "${MODULE_ROOT_DIR}/conf")
        file(GLOB _conf_files
            "${MODULE_ROOT_DIR}/conf/*.ini"
            "${MODULE_ROOT_DIR}/conf/*.conf"
            "${MODULE_ROOT_DIR}/conf/*.conf.dist"
        )
        list(APPEND _all_cfgs ${_conf_files})
    endif()
    if(_all_cfgs)
        install(FILES ${_all_cfgs} DESTINATION "${CMAKE_INSTALL_PREFIX}/module")
        foreach(_cfg IN LISTS _all_cfgs)
            get_filename_component(_cfg_name "${_cfg}" NAME)
            set_property(GLOBAL APPEND PROPERTY IRINE_ENABLED_MODULE_CONFIGS "${_cfg_name}")
        endforeach()
    endif()
    if(EXISTS "${MODULE_ROOT_DIR}/data/sql")
        file(GLOB _sql_files "${MODULE_ROOT_DIR}/data/sql/*.sql")
        if(_sql_files)
            install(FILES ${_sql_files} DESTINATION "${CMAKE_INSTALL_PREFIX}/module/sql")
        endif()
    endif()
endfunction()

# Declarative module registration macro:
#   irine_register_module(
#       NAME "Player-Bots"
#       [LOADER_FUNC "Addmod_Player_BotsScripts"]
#       [COMMON_SOURCES src/PlayerBotsModule.cpp ...]
#       [GAMESERVER_SOURCES src/DnAIBotController.cpp ...]
#       [INCLUDE_DIRS ...]
#       [COMPILE_DEFINITIONS ...]
#       [LINK_LIBRARIES ...]
#       [NO_CONFIG_INSTALL]
#   )
function(irine_register_module)
    set(_flags NO_CONFIG_INSTALL)
    set(_one NAME LOADER_FUNC)
    set(_multi COMMON_SOURCES GAMESERVER_SOURCES INCLUDE_DIRS COMPILE_DEFINITIONS LINK_LIBRARIES)
    cmake_parse_arguments(MOD "${_flags}" "${_one}" "${_multi}" ${ARGN})

    if(NOT MOD_NAME)
        get_filename_component(MOD_NAME "${CMAKE_CURRENT_SOURCE_DIR}" NAME)
    endif()

    irine_sanitize_module_name("${MOD_NAME}" _ident _upper)
    string(TOLOWER "${_ident}" _lower)

    if(NOT MOD_LOADER_FUNC)
        set(MOD_LOADER_FUNC "Addmod_${_ident}Scripts")
    endif()

    set(_common_target "mod_${_lower}")
    set(_gs_target "mod_${_lower}_gameserver")
    set(_mod_define "IRINE_MOD_${_upper}=1")
    set(_src_inc "${CMAKE_CURRENT_SOURCE_DIR}/src")

    # Common library (IModule descriptor, config loading, cross-server scripts)
    if(MOD_COMMON_SOURCES)
        add_library(${_common_target} STATIC ${MOD_COMMON_SOURCES})
        target_include_directories(${_common_target} PUBLIC
            "${_src_inc}"
            ${MOD_INCLUDE_DIRS}
            ${IRINE_COMMON_INCLUDES}
        )
        target_compile_definitions(${_common_target} PRIVATE
            UNICODE _UNICODE
            ${MOD_COMPILE_DEFINITIONS}
        )
        target_compile_definitions(${_common_target} PUBLIC "${_mod_define}")
        target_link_libraries(${_common_target} PUBLIC
            irine_platform
            ${MOD_LINK_LIBRARIES}
        )
        set_property(GLOBAL APPEND PROPERTY IRINE_ENABLED_COMMON_MODULE_TARGETS "${_common_target}")
    endif()

    # Optional GameServer-specific library (for modules that hook DNGameServer internals)
    if(MOD_GAMESERVER_SOURCES)
        add_library(${_gs_target} STATIC ${MOD_GAMESERVER_SOURCES})
        target_include_directories(${_gs_target} PUBLIC
            "${_src_inc}"
            ${MOD_INCLUDE_DIRS}
        )
        target_include_directories(${_gs_target} PRIVATE
            "${CMAKE_SOURCE_DIR}/src/server/DNGameServer"
            ${IRINE_COMMON_INCLUDES}
        )
        target_compile_definitions(${_gs_target} PRIVATE
            WIN64 NDEBUG _CONSOLE _GAMESERVER _FINAL_BUILD UNICODE _UNICODE
            "${_mod_define}"
            ${MOD_COMPILE_DEFINITIONS}
        )
        target_compile_definitions(${_gs_target} PUBLIC "${_mod_define}")
        target_link_libraries(${_gs_target} PUBLIC
            irine_platform
            ${MOD_LINK_LIBRARIES}
        )
        if(TARGET ${_common_target})
            target_link_libraries(${_gs_target} PUBLIC ${_common_target})
        endif()
        set_property(GLOBAL APPEND PROPERTY IRINE_ENABLED_GAMESERVER_MODULE_TARGETS "${_gs_target}")
    endif()

    set_property(GLOBAL APPEND PROPERTY IRINE_ENABLED_MODULES "${MOD_NAME}")
    set_property(GLOBAL APPEND PROPERTY IRINE_ENABLED_MODULE_DEFINES "${_mod_define}")
    if(EXISTS "${_src_inc}")
        set_property(GLOBAL APPEND PROPERTY IRINE_ENABLED_MODULE_INCLUDES "${_src_inc}")
    endif()
    if(MOD_COMMON_SOURCES AND MOD_LOADER_FUNC)
        set_property(GLOBAL APPEND PROPERTY IRINE_ENABLED_MODULE_LOADERS "${MOD_LOADER_FUNC}")
    endif()

    if(NOT MOD_NO_CONFIG_INSTALL)
        irine_install_module_configs("${CMAKE_CURRENT_SOURCE_DIR}")
    endif()
endfunction()

# Scans modules/ for all subdirectories, creates per-module cache options,
# includes enabled modules, and skips disabled modules cleanly.
function(irine_discover_modules MODULES_ROOT)
    # Reset global properties for clean re-configure
    set_property(GLOBAL PROPERTY IRINE_ENABLED_MODULES "")
    set_property(GLOBAL PROPERTY IRINE_ENABLED_COMMON_MODULE_TARGETS "")
    set_property(GLOBAL PROPERTY IRINE_ENABLED_GAMESERVER_MODULE_TARGETS "")
    set_property(GLOBAL PROPERTY IRINE_ENABLED_MODULE_DEFINES "")
    set_property(GLOBAL PROPERTY IRINE_ENABLED_MODULE_INCLUDES "")
    set_property(GLOBAL PROPERTY IRINE_ENABLED_MODULE_LOADERS "")
    set_property(GLOBAL PROPERTY IRINE_ENABLED_MODULE_CONFIGS "")

    message(STATUS "* IrineCore Modules (IRINE_MODULES=${IRINE_MODULES}):")

    file(GLOB _entries RELATIVE "${MODULES_ROOT}" "${MODULES_ROOT}/*")
    list(SORT _entries)

    foreach(_entry IN LISTS _entries)
        set(_mod_dir "${MODULES_ROOT}/${_entry}")
        if(NOT IS_DIRECTORY "${_mod_dir}")
            continue()
        endif()
        # Ignore hidden dirs, helper dirs, and disabled/ folder
        if(_entry MATCHES "^[._]" OR _entry STREQUAL "disabled")
            continue()
        endif()
        # A valid module folder has either a CMakeLists.txt or a src/ directory
        if(NOT EXISTS "${_mod_dir}/CMakeLists.txt" AND NOT EXISTS "${_mod_dir}/src")
            continue()
        endif()

        irine_sanitize_module_name("${_entry}" _ident _upper)
        set(_opt_name "IRINE_MODULE_${_upper}")
        option(${_opt_name} "Build and integrate IrineCore module '${_entry}'" ON)

        set(_is_enabled TRUE)
        set(_disable_reason "")

        if(IRINE_MODULES STREQUAL "disabled")
            set(_is_enabled FALSE)
            set(_disable_reason "IRINE_MODULES=disabled")
        elseif(EXISTS "${_mod_dir}/.disabled")
            set(_is_enabled FALSE)
            set(_disable_reason ".disabled marker file")
        elseif("${_entry}" IN_LIST DISABLED_IRINE_MODULES)
            set(_is_enabled FALSE)
            set(_disable_reason "DISABLED_IRINE_MODULES")
        elseif(NOT ${_opt_name})
            set(_is_enabled FALSE)
            set(_disable_reason "${_opt_name}=OFF")
        endif()

        if(NOT _is_enabled)
            message(STATUS "  - ${_entry} [DISABLED: ${_disable_reason}]")
            continue()
        endif()

        message(STATUS "  + ${_entry} [ENABLED]")
        if(EXISTS "${_mod_dir}/CMakeLists.txt")
            add_subdirectory("${_mod_dir}" "${CMAKE_BINARY_DIR}/modules/${_entry}")
        else()
            # Zero-boilerplate auto-registration for drop-in src/*.cpp modules
            file(GLOB_RECURSE _auto_srcs CONFIGURE_DEPENDS
                "${_mod_dir}/src/*.cpp"
                "${_mod_dir}/src/*.cxx"
                "${_mod_dir}/src/*.cc"
            )
            if(_auto_srcs)
                irine_register_module(
                    NAME "${_entry}"
                    LOADER_FUNC "Addmod_${_ident}Scripts"
                    COMMON_SOURCES ${_auto_srcs}
                )
            endif()
        endif()
    endforeach()
endfunction()

function(irine_generate_module_loader TEMPLATE_PATH OUTPUT_PATH)
    get_property(_loaders GLOBAL PROPERTY IRINE_ENABLED_MODULE_LOADERS)

    set(IRINE_MODULE_LOADER_DECLARATIONS "")
    set(IRINE_MODULE_LOADER_CALLS "")
    foreach(_fn IN LISTS _loaders)
        string(APPEND IRINE_MODULE_LOADER_DECLARATIONS "void ${_fn}();\n")
        string(APPEND IRINE_MODULE_LOADER_CALLS "    ${_fn}();\n")
    endforeach()

    if(NOT IRINE_MODULE_LOADER_DECLARATIONS)
        set(IRINE_MODULE_LOADER_DECLARATIONS "// No modules enabled in this build.\n")
    endif()
    if(NOT IRINE_MODULE_LOADER_CALLS)
        set(IRINE_MODULE_LOADER_CALLS "    // No modules enabled in this build.\n")
    endif()

    configure_file("${TEMPLATE_PATH}" "${OUTPUT_PATH}" @ONLY)

    # Auto-generate module/modules.manifest listing all enabled module config files
    get_property(_mod_cfgs GLOBAL PROPERTY IRINE_ENABLED_MODULE_CONFIGS)
    list(REMOVE_DUPLICATES _mod_cfgs)
    list(SORT _mod_cfgs)
    set(_manifest_content "# Auto-generated by IrineCore CMake — enabled module config files\n")
    foreach(_cfg_name IN LISTS _mod_cfgs)
        string(APPEND _manifest_content "${_cfg_name}\n")
    endforeach()
    set(_manifest_out "${CMAKE_BINARY_DIR}/generated/modules.manifest")
    file(WRITE "${_manifest_out}" "${_manifest_content}")
    install(FILES "${_manifest_out}" DESTINATION "${CMAKE_INSTALL_PREFIX}/module")
endfunction()

function(irine_link_gameserver_modules TARGET_NAME)
    get_property(_gs_targets GLOBAL PROPERTY IRINE_ENABLED_GAMESERVER_MODULE_TARGETS)
    if(_gs_targets)
        target_link_libraries(${TARGET_NAME} PRIVATE ${_gs_targets})
    endif()
endfunction()
