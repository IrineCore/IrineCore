# =============================================================================
# IrineCore — tools/CMakeLists.txt
# Client packaging, patching, and checksum utilities (Windows & Linux x64)
# =============================================================================

add_library(irine_tool_common STATIC
    "common/PakArchive.cpp"
    "common/ToolUtils.cpp"
)
target_include_directories(irine_tool_common PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/common"
    "${CMAKE_SOURCE_DIR}/common/utilities"
    "${CMAKE_SOURCE_DIR}/deps/zlib"
)
target_link_libraries(irine_tool_common PUBLIC
    irine_utility
    dep_zlib
)
set_target_properties(irine_tool_common PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

function(irine_add_tool TARGET_NAME SOURCE_FILE)
    add_executable(${TARGET_NAME} "${SOURCE_FILE}")
    target_link_libraries(${TARGET_NAME} PRIVATE irine_tool_common)
    set_target_properties(${TARGET_NAME} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    install(TARGETS ${TARGET_NAME}
        RUNTIME DESTINATION "${CMAKE_INSTALL_PREFIX}/tools"
    )
endfunction()

irine_add_tool(DnMakeMD5       "DnMakeMD5/main.cpp")
irine_add_tool(DnMakeFullPatch "DnMakeFullPatch/main.cpp")
irine_add_tool(PatchBuilder    "PatchBuilder/main.cpp")
irine_add_tool(DnClientCreator "DnClientCreator/main.cpp")
