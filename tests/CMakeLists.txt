# =============================================================================
# IrineCore — tests/CMakeLists.txt
# Automated Unit, Integration, Networking & Compatibility Test Suite
# Links only irine_modules so tests build cleanly regardless of which modules
# are enabled or disabled via CMake.
# =============================================================================
add_executable(irine_tests
    "TestMain.cpp"
    "unit/TestPlatformAndMath.cpp"
    "integration/TestModuleFramework.cpp"
    "networking/TestWirePacketAbi.cpp"
    "compatibility/TestConfigAndCountryPurge.cpp"
)
target_include_directories(irine_tests PRIVATE ${IRINE_COMMON_INCLUDES})
target_compile_definitions(irine_tests PRIVATE UNICODE _UNICODE)
target_link_libraries(irine_tests PRIVATE
    irine_modules
    irine_platform
    $<$<BOOL:${WIN32}>:ws2_32 mswsock winmm>
    $<$<BOOL:${UNIX}>:pthread dl>
)
add_test(NAME IrineCoreTestSuite
    COMMAND irine_tests "${CMAKE_SOURCE_DIR}/apps/template/config" "${CMAKE_SOURCE_DIR}/modules"
)
