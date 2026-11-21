# =============================================================================
# IrineCore — modules/CMakeLists.txt
# Dynamic automatic module discovery & static library aggregation.
# Modules can exist inside modules/ and be disabled without modifying core code:
#   -DIRINE_MODULES=disabled
#   -DDISABLED_IRINE_MODULES="Player-Bots;examples"
#   -DIRINE_MODULE_PLAYER_BOTS=OFF
#   or by placing a '.disabled' file inside modules/<ModuleName>/
# =============================================================================
irine_discover_modules("${CMAKE_CURRENT_SOURCE_DIR}")

set(_generated_loader "${CMAKE_BINARY_DIR}/generated/ModuleLoader.cpp")
irine_generate_module_loader(
    "${CMAKE_CURRENT_SOURCE_DIR}/ModuleLoader.cpp.in"
    "${_generated_loader}"
)

add_library(irine_modules STATIC
    "${CMAKE_SOURCE_DIR}/common/events/ModuleRegistry.cpp"
    "${_generated_loader}"
)

get_property(_common_mod_targets GLOBAL PROPERTY IRINE_ENABLED_COMMON_MODULE_TARGETS)
get_property(_mod_defines        GLOBAL PROPERTY IRINE_ENABLED_MODULE_DEFINES)
get_property(_mod_includes       GLOBAL PROPERTY IRINE_ENABLED_MODULE_INCLUDES)

target_include_directories(irine_modules PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/common/events"
    ${_mod_includes}
    ${IRINE_COMMON_INCLUDES}
)
target_compile_definitions(irine_modules PRIVATE UNICODE _UNICODE)
if(_mod_defines)
    target_compile_definitions(irine_modules PUBLIC ${_mod_defines})
endif()

target_link_libraries(irine_modules PUBLIC
    irine_platform
    ${_common_mod_targets}
)
