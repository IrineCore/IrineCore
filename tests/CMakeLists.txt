# =============================================================================
# IrineCore — tests/CMakeLists.txt
# Automated Unit, Integration, Networking & Compatibility Test Suite
# Links only irine_modules so tests build cleanly regardless of which modules
# are enabled or disabled via CMake.
# =============================================================================
set(_IRINE_TEST_SOURCES
    "TestMain.cpp"
    "unit/TestPlatformAndMath.cpp"
    "integration/TestModuleFramework.cpp"
    "networking/TestWirePacketAbi.cpp"
    "compatibility/TestConfigAndCountryPurge.cpp"
)
if(IRINE_BUILD_TOOLS)
    list(APPEND _IRINE_TEST_SOURCES "unit/TestPakAndTools.cpp")
endif()

add_executable(irine_tests ${_IRINE_TEST_SOURCES})
target_include_directories(irine_tests PRIVATE ${IRINE_COMMON_INCLUDES})
target_compile_definitions(irine_tests PRIVATE
    UNICODE _UNICODE
    $<$<BOOL:${IRINE_BUILD_TOOLS}>:IRINE_HAS_TOOLS_TESTS>
)
target_link_libraries(irine_tests PRIVATE
    irine_modules
    irine_platform
    $<$<BOOL:${IRINE_BUILD_TOOLS}>:irine_tool_common>
    $<$<BOOL:${IRINE_BUILD_TOOLS}>:irine_etfilesystem>
    $<$<BOOL:${IRINE_BUILD_TOOLS}>:irine_utility>
    $<$<BOOL:${IRINE_BUILD_TOOLS}>:dep_zlib>
)
if(WIN32)
    target_link_libraries(irine_tests PRIVATE ws2_32 mswsock winmm)
elseif(UNIX)
    target_link_libraries(irine_tests PRIVATE pthread dl)
endif()
if(IRINE_BUILD_TOOLS)
    set_target_properties(irine_tests PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
endif()
add_test(NAME IrineCoreTestSuite
    COMMAND irine_tests "${CMAKE_SOURCE_DIR}/apps/template/config" "${CMAKE_SOURCE_DIR}/modules"
)
